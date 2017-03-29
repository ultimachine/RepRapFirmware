/*
 * he280_accel.h
 *
 *  Created on: Mar 9, 2017
 *      Author: ultiadmin
 */

#ifndef SRC_ARCHIM_HE280_ACCEL_H_
#define SRC_ARCHIM_HE280_ACCEL_H_

#include <stdint.h>

uint8_t accelerometer_write(uint8_t reg, uint8_t val);
uint8_t accelerometer_recv(uint8_t reg);
uint8_t accelerometer_init();
uint8_t accelerometer_status();


#endif /* SRC_ARCHIM_HE280_ACCEL_H_ */
