/*
 * he280_accel.h
 *
 *  Created on: Mar 9, 2017
 *      Author: ultiadmin
 */

#ifndef SRC_ARCHIM_HE280_ACCEL_H_
#define SRC_ARCHIM_HE280_ACCEL_H_

#include <stdint.h>

void accelerometer_write(uint8_t reg, uint8_t val);
void accelerometer_recv(uint8_t reg);
void accelerometer_init();
void accelerometer_status();


#endif /* SRC_ARCHIM_HE280_ACCEL_H_ */
