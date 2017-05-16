/*
 * he280_accel.h
 *
 *  Created on: Mar 9, 2017
 *      Author: ultiadmin
 */

#ifndef SRC_ARCHIM_HE280_ACCEL_H_
#define SRC_ARCHIM_HE280_ACCEL_H_

#include <stdint.h>

uint8_t AccelerometerWrite(uint8_t reg, uint8_t val);
uint8_t AccelerometerRecv(uint8_t reg);
uint8_t AccelerometerInit();
uint8_t AccelerometerStatus();


#endif /* SRC_ARCHIM_HE280_ACCEL_H_ */
