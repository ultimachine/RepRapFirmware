/****************************************************************************************************

 RepRapFirmware - Network: RepRapPro Ormerod with Arduino Due controller

  2014-04-05 Created from portions taken out of Platform.cpp by dc42
  2014-04-07 Added portions of httpd.c. These portions are subject to the following copyright notice:

 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * (end httpd.c copyright notice)

 ****************************************************************************************************/

#include "Network.h"

#include "ConnectionState.h"
#include "NetworkTransaction.h"
#include "Platform.h"
#include "RepRap.h"
#include "Webserver.h"
#include "Version.h"

extern "C"
{
#include "ethernet_sam.h"

#include "lwipopts.h"

#ifdef LWIP_STATS
#include "lwip/src/include/lwip/stats.h"
#endif

#include "lwip/src/include/lwip/tcp.h"
#include "lwip/src/include/lwip/tcp_impl.h"

#include "contrib/apps/netbios/netbios.h"
#include "contrib/apps/mdns/mdns_responder.h"
}

static volatile bool lwipLocked = false;
static bool ethernetStarted = false;

static tcp_pcb *http_pcb = nullptr;
static tcp_pcb *ftp_main_pcb = nullptr;
static tcp_pcb *ftp_pasv_pcb = nullptr;
static tcp_pcb *telnet_pcb = nullptr;

static struct mdns_service mdns_services[] = {
	{
		.name = "\x05_echo\x04_tcp\x05local",
		.port = 7,
	},
	{
		.name = "\x05_http\x04_tcp\x05local",
		.port = DefaultHttpPort,
	},
	{
		.name = "\x04_ftp\x04_tcp\x05local",
		.port = FTP_PORT
	},
	{
		.name = "\x07_telnet\x04_tcp\x05local",
		.port = TELNET_PORT
	}
};

const size_t MDNS_HTTP_SERVICE_INDEX = 1;	// Index of the mDNS HTTP service above

static const char *mdns_txt_records[] = {
	"product=" FIRMWARE_NAME,
	"version=" VERSION,
	NULL
};

static bool closingDataPort = false;

ConnectionState *sendingConnection = nullptr;

static uint32_t sendingWindow32[(TCP_WND + 3)/4];						// should be 32-bit aligned for efficiency
char * const sendingWindow = reinterpret_cast<char *>(sendingWindow32);
uint16_t sendingWindowSize, sentDataOutstanding;
uint8_t sendingRetries;
err_t writeResult, outputResult;

static Port httpPort = DefaultHttpPort;

/*-----------------------------------------------------------------------------------*/

extern "C"
{

// Lock functions for LWIP (LWIP generally isn't thread-safe)
bool LockLWIP()
{
	if (lwipLocked)
		return false;

	lwipLocked = true;
	return true;
}

void UnlockLWIP()
{
	lwipLocked = false;
}

// Callback functions for the EMAC driver and for LwIP

// Callback to report when the network interface has gone up or down.
// Note that this is only a rough indicator and may not be called when
// the IP address is changed on-the-fly!
static void ethernet_status_callback(struct netif *netif)
{
	if (netif_is_up(netif))
	{
		char ip[16];
		ipaddr_ntoa_r(&(netif->ip_addr), ip, sizeof(ip));
		reprap.GetPlatform()->MessageF(HOST_MESSAGE, "Network up, IP=%s\n", ip);
	}
	else
	{
		reprap.GetPlatform()->Message(HOST_MESSAGE, "Network down\n");
	}
}

// Called from ISR
static void ethernet_rx_callback(uint32_t ul_status)
{
	// Because the LWIP stack can become corrupted if we work with it in parallel,
	// we may have to wait for the next Spin() call to read the next packet.
	if (LockLWIP())
	{
		ethernet_task();
		UnlockLWIP();
	}
	else
	{
		ethernet_set_rx_callback(nullptr);
		reprap.GetNetwork()->ResetCallback();
	}
}

// Callback functions for LWIP (may be called from ISR)

static void conn_err(void *arg, err_t err)
{
	// Report the error to the monitor
	reprap.GetPlatform()->MessageF(HOST_MESSAGE, "Network: Connection error, code %d\n", err);

	// Tell the higher levels about the error
	ConnectionState *cs = (ConnectionState*)arg;
	if (cs != nullptr)
	{
		cs->isTerminated = true;
		reprap.GetNetwork()->ConnectionClosed(cs, false);
	}
}

static err_t conn_recv(void *arg, tcp_pcb *pcb, pbuf *p, err_t err)
{
	ConnectionState *cs = (ConnectionState*)arg;
	if (err == ERR_OK && cs != nullptr)
	{
		if (cs->pcb != pcb)
		{
			reprap.GetPlatform()->Message(HOST_MESSAGE, "Network: Mismatched pcb in conn_recv!\n");
			tcp_abort(pcb);
			return ERR_ABRT;
		}

		bool processingOk = true;
		if (p != nullptr)
		{
			// Tell higher levels that we are receiving data
			processingOk = reprap.GetNetwork()->ReceiveInput(p, cs);
		}
		else
		{
			// Tell higher levels that a connection has been closed
			processingOk = reprap.GetNetwork()->ConnectionClosedGracefully(cs);
		}

		if (!processingOk)
		{
			// Something went wrong, discard whatever has been received
			if (p != nullptr)
			{
				pbuf_free(p);
			}

			// Also reset the connection. This will call conn_err() too
			tcp_abort(pcb);
			return ERR_ABRT;
		}
	}

	return ERR_OK;
}

static err_t conn_accept(void *arg, tcp_pcb *pcb, err_t err)
{
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);

	/* Allocate a new ConnectionState for this connection */
	ConnectionState *cs = reprap.GetNetwork()->ConnectionAccepted(pcb);
	if (cs == nullptr)
	{
		tcp_abort(pcb);
		return ERR_ABRT;
	}

	/* Keep the listening PCBs running */
	switch (pcb->local_port)		// tell LWIP to accept further connections on the listening PCB
	{
		case FTP_PORT:		// FTP
			tcp_accepted(ftp_main_pcb);
			break;

		case TELNET_PORT:	// Telnet
			tcp_accepted(telnet_pcb);
			break;

		default:			// HTTP and FTP data
			tcp_accepted((pcb->local_port == httpPort) ? http_pcb : ftp_pasv_pcb);
			break;
	}
	tcp_arg(pcb, cs);			// tell LWIP that this is the structure we wish to be passed for our callbacks
	tcp_recv(pcb, conn_recv);	// tell LWIP that we wish to be informed of incoming data by a call to the conn_recv() function
	tcp_err(pcb, conn_err);

	return ERR_OK;
}

} // end extern "C"

/*-----------------------------------------------------------------------------------*/

void httpd_init()
{
	tcp_pcb* pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, httpPort);
	http_pcb = tcp_listen(pcb);
	tcp_accept(http_pcb, conn_accept);
}

void ftpd_init()
{
	tcp_pcb* pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, FTP_PORT);
	ftp_main_pcb = tcp_listen(pcb);
	tcp_accept(ftp_main_pcb, conn_accept);
}

void telnetd_init()
{
	tcp_pcb* pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, TELNET_PORT);
	telnet_pcb = tcp_listen(pcb);
	tcp_accept(telnet_pcb, conn_accept);
}

//***************************************************************************************************

// Network/Ethernet class

Network::Network(Platform* p) :
	platform(p), freeTransactions(nullptr), readyTransactions(nullptr), writingTransactions(nullptr),
	state(NetworkInactive), isEnabled(true), resetCallback(false),
	dataCs(nullptr), ftpCs(nullptr), telnetCs(nullptr), freeConnections(nullptr)
{
}

void Network::Init()
{
	for (size_t i = 0; i < NETWORK_TRANSACTION_COUNT; i++)
	{
		freeTransactions = new NetworkTransaction(freeTransactions);
	}

	for (size_t i = 0; i < MEMP_NUM_TCP_PCB; i++)
	{
		ConnectionState *cs = new ConnectionState;
		cs->next = freeConnections;
		freeConnections = cs;
	}

	strcpy(hostname, HOSTNAME);
	init_ethernet();

	httpd_init();
	ftpd_init();
	telnetd_init();
	netbios_init();

	longWait = platform->Time();
}

void Network::Spin(bool full)
{
	if (LockLWIP())							// basically we can't do anything if we can't interact with LWIP
	{
		if (state == NetworkObtainingIP || state == NetworkActive)
		{
			// Is the link still up?
			if (!ethernet_link_established())
			{
				state = NetworkEstablishingLink;
				UnlockLWIP();

				platform->ClassReport(longWait);
				return;
			}

			// See if we can read any packets. They may include DHCP responses too
			ethernet_task();
			if (resetCallback)
			{
				resetCallback = false;
				ethernet_set_rx_callback(&ethernet_rx_callback);
			}

			// Have we obtained a valid IP address yet?
			if (state == NetworkObtainingIP)
			{
				const uint8_t *ip = ethernet_get_ipaddress();
				if (ip[0] != 0 && ip[1] != 0 && ip[2] != 0 && ip[3] != 0)
				{
					// Yes - we're good to go now
					state = NetworkActive;

					// Send mDNS announcement so that some routers can perform hostname mapping
					// if ths board is connected via a non-IGMP capable WiFi bridge (like the TP-Link WR701N)
					mdns_announce();
				}
			}

			// See if we can send anything - only if full spin i.e. not in the middle of file i/o
			if (full)
			{
				NetworkTransaction *transaction = writingTransactions;
				if (transaction != nullptr && sendingConnection == nullptr)
				{
					if (transaction->next != nullptr)
					{
						// Data is supposed to be sent and the last packet has been acknowledged.
						// Rotate the transactions so every client is served even while multiple files are sent
						NetworkTransaction *next = transaction->next;
						writingTransactions = next;
						AppendTransaction(&writingTransactions, transaction);
						transaction = next;
					}

					if (transaction->Send())
					{
						// This transaction can be released, do this here
						writingTransactions = transaction->next;
						PrependTransaction(&freeTransactions, transaction);

						// If there is more data to write on this connection, do it sometime soon
						NetworkTransaction *nextWrite = transaction->nextWrite;
						if (nextWrite != nullptr)
						{
							PrependTransaction(&writingTransactions, nextWrite);
						}
					}
				}
			}
		}
		else if (state == NetworkEstablishingLink && ethernet_establish_link())
		{
			if (!ethernetStarted)
			{
				start_ethernet(platform->GetIPAddress(), platform->NetMask(), platform->GateWay(), &ethernet_status_callback);
				ethernetStarted = true;

				// Initialise this one here, because it requires a configured IGMP network interface
				mdns_responder_init(mdns_services, ARRAY_SIZE(mdns_services), mdns_txt_records);
			}
			else
			{
				ethernet_set_configuration(platform->GetIPAddress(), platform->NetMask(), platform->GateWay());
			}
			state = NetworkObtainingIP;
		}

		UnlockLWIP();
	}
	platform->ClassReport(longWait);
}

void Network::Interrupt()
{
	if (state != NetworkInactive && LockLWIP())
	{
		ethernet_timers_update();
		UnlockLWIP();
	}
}

void Network::Diagnostics(MessageType mtype)
{
	platform->Message(mtype, "=== Network ===\n");

	size_t numFreeConnections = 0;
	ConnectionState *freeConn = freeConnections;
	while (freeConn != nullptr)
	{
		numFreeConnections++;
		freeConn = freeConn->next;
	}
	platform->MessageF(mtype, "Free connections: %d of %d\n", numFreeConnections, MEMP_NUM_TCP_PCB);

	size_t numFreeTransactions = 0;
	NetworkTransaction *freeTrans = freeTransactions;
	while (freeTrans != nullptr)
	{
		numFreeTransactions++;
		freeTrans = freeTrans->next;
	}
	platform->MessageF(mtype, "Free transactions: %d of %d\n", numFreeTransactions, NETWORK_TRANSACTION_COUNT);

#if LWIP_STATS
	// Normally we should NOT try to display LWIP stats here, because it uses debugPrintf(), which will hang the system if no USB cable is connected.
	if (reprap.Debug(moduleNetwork))
	{
		stats_display();
	}
#endif
}

void Network::ResetCallback()
{
	resetCallback = true;
}

// Called when data has been received. Return false if we cannot process it
bool Network::ReceiveInput(pbuf *pb, ConnectionState* cs)
{
	NetworkTransaction* r = freeTransactions;
	if (r == nullptr)
	{
		platform->Message(HOST_MESSAGE, "Network::ReceiveInput() - no free transactions!\n");
		return false;
	}

	freeTransactions = r->next;
	r->Set(pb, cs, receiving);

	AppendTransaction(&readyTransactions, r);
//	debugPrintf("Network - input received\n");
	return true;
}

// This is called when a connection is being established and returns an initialised ConnectionState instance
// or NULL if no more items are available. This would reset the connection immediately
ConnectionState *Network::ConnectionAccepted(tcp_pcb *pcb)
{
	ConnectionState *cs = freeConnections;
	if (cs == nullptr)
	{
		platform->Message(HOST_MESSAGE, "Network::ConnectionAccepted() - no free ConnectionStates!\n");
		return nullptr;
	}

	NetworkTransaction* transaction = freeTransactions;
	if (transaction == nullptr)
	{
		platform->Message(HOST_MESSAGE, "Network::ConnectionAccepted() - no free transactions!\n");
		return nullptr;
	}

	// Initialise a new connection
	freeConnections = cs->next;
	cs->Init(pcb);

	// Notify the webserver about this
	transaction->Set(nullptr, cs, connected);
	freeTransactions = transaction->next;
	AppendTransaction(&readyTransactions, transaction);

	return cs;
}

// This is called when a connection is being closed or has gone down unexpectedly
void Network::ConnectionClosed(ConnectionState* cs, bool closeConnection)
{
	// Make sure these connections are not reused. Remove all references to it
	if (cs == dataCs)
	{
		// FTP data connection
		dataCs = nullptr;

		if (closingDataPort && ftp_pasv_pcb != nullptr)
		{
			tcp_accept(ftp_pasv_pcb, nullptr);
			tcp_close(ftp_pasv_pcb);
			ftp_pasv_pcb = nullptr;
		}
		closingDataPort = false;
	}
	if (cs == ftpCs)
	{
		// Main FTP connection
		ftpCs = nullptr;
	}
	if (cs == telnetCs)
	{
		telnetCs = nullptr;
	}
	if (cs == sendingConnection)
	{
		// Stop sending if the connection is going down
		sendingConnection = nullptr;
	}

	// Remove all callbacks and close the PCB if requested
	tcp_pcb *pcb = cs->pcb;
	tcp_sent(pcb, nullptr);
	tcp_recv(pcb, nullptr);
	tcp_poll(pcb, nullptr, TCP_WRITE_TIMEOUT / TCP_SLOW_INTERVAL / TCP_MAX_SEND_RETRIES);
	if (pcb != nullptr && closeConnection)
	{
		tcp_err(pcb, nullptr);
		tcp_close(pcb);
	}
	cs->pcb = nullptr;

	// Inform the Webserver that we are about to remove an existing connection
	reprap.GetWebserver()->ConnectionLost(cs);

	// Remove all transactions that point to cs from the list of ready transactions
	NetworkTransaction *previous = nullptr, *item = readyTransactions;
	while (item != nullptr)
	{
		if (item->cs == cs)
		{
			item->Discard();
			item = (previous == nullptr) ? readyTransactions : previous->next;
		}
		else
		{
			previous = item;
			item = item->next;
		}
	}

	// Do the same for the writing transaction. There is only one transaction on writingTransactions
	// per connection and cs->sendingTransaction points to it. Check if we have to free it here
	NetworkTransaction *sendingTransaction = cs->sendingTransaction;
	if (sendingTransaction != nullptr)
	{
		// Take care of other transactions that want to write data over the closed connection too
		NetworkTransaction *nextWrite = sendingTransaction->nextWrite;
		while (nextWrite != nullptr)
		{
			NetworkTransaction *temp = nextWrite;
			nextWrite = nextWrite->nextWrite;
			temp->Discard();
		}

		// Unlink the sending transaction from the writing transactions
		previous = nullptr;
		for(item = writingTransactions; item != nullptr; item = item->next)
		{
			if (item == sendingTransaction)
			{
				if (previous == nullptr)
				{
					writingTransactions = item->next;
				}
				else
				{
					previous->next = item->next;
				}
				break;
			}
			previous = item;
		}

		// Discard it. This will add it back to the list of free transactions too
		sendingTransaction->Discard();
	}

	// Free up this cs again
	cs->next = freeConnections;
	freeConnections = cs;
}

// This enqueues a new transaction to indicate a graceful reset. Do this to keep the time line of incoming transactions valid.
// Return false if we cannot process this event, which would result in an immediate connection reset
bool Network::ConnectionClosedGracefully(ConnectionState *cs)
{
	// We need a valid transaction to do something.
	// If that fails, the connection will be reset and the error handler will take care of this connection
	NetworkTransaction *transaction = freeTransactions;
	if (transaction == nullptr)
	{
		platform->Message(HOST_MESSAGE, "Network::ConnectionClosedGracefully() - no free transactions!\n");
		return false;
	}

	// Invalidate the PCB
	tcp_sent(cs->pcb, nullptr);
	tcp_recv(cs->pcb, nullptr);
	tcp_poll(cs->pcb, nullptr, TCP_WRITE_TIMEOUT / TCP_SLOW_INTERVAL / TCP_MAX_SEND_RETRIES);
	tcp_err(cs->pcb, nullptr);
	tcp_close(cs->pcb);
	cs->pcb = nullptr;

	// Close the sending transaction (if any)
	NetworkTransaction *sendingTransaction = cs->sendingTransaction;
	if (sendingTransaction != nullptr)
	{
		sendingTransaction->Close();
	}

	// Notify the webserver about this event
	freeTransactions = transaction->next;
	transaction->Set(nullptr, cs, disconnected);
	AppendTransaction(&readyTransactions, transaction);
	return true;
}

bool Network::Lock()
{
	return LockLWIP();
}

void Network::Unlock()
{
	UnlockLWIP();
}

bool Network::InLwip() const
{
	return lwipLocked;
}

const uint8_t *Network::GetIPAddress() const
{
	return ethernet_get_ipaddress();
}

void Network::SetIPAddress(const uint8_t ipAddress[], const uint8_t netmask[], const uint8_t gateway[])
{
	if (state == NetworkObtainingIP || state == NetworkActive)
	{
		// This performs IP changes on-the-fly
		ethernet_set_configuration(ipAddress, netmask, gateway);
	}
}

// Set the network hostname. Removes all whitespaces and converts the name to lower-case.
void Network::SetHostname(const char *name)
{
	size_t i = 0;
	while (*name && i < ARRAY_UPB(hostname))
	{
		char c = *name++;
		if (c >= 'A' && c <= 'Z')
		{
			c += 'a' - 'A';
		}

		if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == '-') || (c == '_'))
		{
			hostname[i++] = c;
		}
	}

	if (i > 0)
	{
		hostname[i] = 0;
	}
	else
	{
		// Don't allow empty hostnames
		strcpy(hostname, HOSTNAME);
	}

	if (state == NetworkActive)
	{
		mdns_update_hostname();
	}
}

void Network::Enable()
{
	if (state == NetworkInactive)
	{
		if (!ethernetStarted)
		{
			// Allow the MAC address to be set only before LwIP is started...
			ethernet_configure_interface(platform->MACAddress(), hostname);
		}

		resetCallback = true;	// Reset EMAC RX callback on next Spin calls
		state = NetworkEstablishingLink;
		isEnabled = true;
	}
}

void Network::Disable()
{
	if (state != NetworkInactive)
	{
		resetCallback = false;
		ethernet_set_rx_callback(nullptr);
		state = NetworkInactive;
		isEnabled = false;
	}
}

// This is called by the web server to get the next networking transaction.
//
// If cs is NULL, the transaction from the head of readyTransactions will be retrieved.
// If cs is not NULL, the first transaction with the matching connection will be returned.
//
// This method also ensures that the retrieved transaction is moved to the first item of
// readyTransactions, so that a subsequent call with a NULL cs parameter will return exactly
// the same instance.
NetworkTransaction *Network::GetTransaction(const ConnectionState *cs)
{
	// See if there is any transaction at all
	NetworkTransaction *transaction = readyTransactions;
	if (transaction == nullptr)
	{
		return nullptr;
	}

	// If no specific connection is specified or if the first item already matches the
	// connection we are looking for, just return it
	if (cs == nullptr || transaction->GetConnection() == cs)
	{
		return transaction;
	}

	// We are looking for a specific transaction, but it's not the first item.
	// Search for it and move it to the head of readyTransactions
	NetworkTransaction *previous = transaction;
	for(NetworkTransaction *item = transaction->next; item != nullptr; item = item->next)
	{
		if (item->GetConnection() == cs)
		{
			previous->next = item->next;
			item->next = readyTransactions;
			readyTransactions = item;
			return item;
		}
		previous = item;
	}

	// We failed to find a valid transaction for the given connection
	return nullptr;
}

void Network::AppendTransaction(NetworkTransaction* volatile* list, NetworkTransaction *r)
{
	r->next = nullptr;
	while (*list != nullptr)
	{
		list = &((*list)->next);
	}
	*list = r;
}

void Network::PrependTransaction(NetworkTransaction* volatile* list, NetworkTransaction *r)
{
	r->next = *list;
	*list = r;
}

void Network::OpenDataPort(Port port)
{
	closingDataPort = false;
	tcp_pcb* pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, port);
	ftp_pasv_pcb = tcp_listen(pcb);
	tcp_accept(ftp_pasv_pcb, conn_accept);
}

Port Network::GetDataPort() const
{
	return (closingDataPort || (ftp_pasv_pcb == nullptr) ? 0 : ftp_pasv_pcb->local_port);
}

Port Network::GetHttpPort() const
{
	return httpPort;
}

void Network::SetHttpPort(Port port)
{
	if (port != httpPort)
	{
		// Close the old HTTP PCB and create a new one
		tcp_close(http_pcb);
		httpPort = port;
		httpd_init();

		// Update mDNS service
		mdns_services[MDNS_HTTP_SERVICE_INDEX].port = port;
		if (state == NetworkActive)
		{
			mdns_announce();
		}
	}
}

// Close FTP data port and purge associated PCB
void Network::CloseDataPort()
{
	// See if it's already being closed
	if (closingDataPort)
	{
		return;
	}
	closingDataPort = true;

	// Close remote connection of our data port or do it as soon as the last packet has been sent
	if (dataCs != nullptr)
	{
		NetworkTransaction *mySendingTransaction = dataCs->sendingTransaction;
		if (mySendingTransaction != nullptr)
		{
			mySendingTransaction->Close();
			return;
		}
	}

	// We can close it now, so do it here
	if (ftp_pasv_pcb != nullptr)
	{
		tcp_accept(ftp_pasv_pcb, nullptr);
		tcp_close(ftp_pasv_pcb);
		ftp_pasv_pcb = nullptr;
	}
	closingDataPort = false;
}

// These methods keep track of our connections in case we need to send to one of them
void Network::SaveDataConnection()
{
	dataCs = readyTransactions->cs;
}

void Network::SaveFTPConnection()
{
	ftpCs = readyTransactions->cs;
}

void Network::SaveTelnetConnection()
{
	telnetCs = readyTransactions->cs;
}

bool Network::AcquireFTPTransaction()
{
	return AcquireTransaction(ftpCs);
}

bool Network::AcquireDataTransaction()
{
	return AcquireTransaction(dataCs);
}

bool Network::AcquireTelnetTransaction()
{
	return AcquireTransaction(telnetCs);
}

// Retrieves the NetworkTransaction of a sending connection to which data can be appended to,
// or prepares a released NetworkTransaction, which can easily be sent via Commit().
bool Network::AcquireTransaction(ConnectionState *cs)
{
	// Make sure we have a valid connection
	if (cs == nullptr)
	{
		return false;
	}

	// Does the head of ready transaction already belong to cs and was it acquired before?
	NetworkTransaction *currentTransaction = readyTransactions;
	if (currentTransaction != nullptr && currentTransaction->GetConnection() == cs &&
			currentTransaction->GetStatus() == acquired)
	{
		// Yes - don't look for another one
		return true;
	}

	// No - try to allocate a free one
	NetworkTransaction *acquiredTransaction = freeTransactions;
	if (acquiredTransaction == nullptr)
	{
		platform->Message(HOST_MESSAGE, "Network: Could not acquire free transaction!\n");
		return false;
	}
	freeTransactions = acquiredTransaction->next;
	acquiredTransaction->Set(nullptr, cs, acquired);
	PrependTransaction(&readyTransactions, acquiredTransaction);

	return true;
}

/*static*/ Port Network::GetLocalPort(Connection conn) { return conn->GetLocalPort(); }
/*static*/ Port Network::GetRemotePort(Connection conn) { return conn->GetRemotePort(); }
/*static*/ uint32_t Network::GetRemoteIP(Connection conn) { return conn->GetRemoteIP(); }
/*static*/ bool Network::IsConnected(Connection conn) { return conn->IsConnected(); }
/*static*/ bool Network::IsTerminated(Connection conn) { return conn->IsTerminated(); }
/*static*/ void Network::Terminate(Connection conn) { conn->Terminate(); }

// End
