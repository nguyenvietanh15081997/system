/*
 * Ds1307.h process tasks time (DS1307, system AI)
 * - Get time data of system AI
 * - Get, set time data of DS1307 module
 */
#ifndef GATEWAYMANAGER_DS1307_H_
#define GATEWAYMANAGER_DS1307_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <endian.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mraa/i2c.h"

extern uint8_t dataDecTimeDs1307[7];
extern uint8_t dataTimeInternet[7];

/*
 * This function convert BCD code to Decimal code
 *
 * @param dataBCD number of BCD code
 * @return dataDEC number of Decimal code
 */
uint8_t BCD2DEC(uint8_t dataBCD);

/*
 * This function convert Decimal code to BCD code
 *
 * @param dataDEC number of Decimal code
 * @return dataBCD number of BCD code
 */
uint8_t DEC2BCD(uint8_t dataDEC);

/*
 * Initiate i2c to read, write Ds1307 module
 * This function have to call before read, write Ds1307 module
 *
 * @param null
 * @return null
 */
void Ds1307_Init();

/*
 * Get time data to DS1307 module
 * - Read time data to DS1307 module
 * - Time data saved to array dataDecTimeDs1307
 *
 * @param null
 * @return null
 */
void TimeForDs1307();

/*
 * Get time data to system AI
 *
 * @param null
 * @return null
 */
void TimeForInternet();

/*
 * Set time data for Ds1307 module
 *
 * @param seconds
 * @param minutes
 * @param hours
 * @param days weeknds
 * @param dates day of the months
 * @param months
 * @param years
 * @return null
 */
void SetTimeForDs1307(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t days, uint8_t dates, uint8_t months, uint8_t years);

/*
 * Thread to perform time-related tasks
 */
void * Time_Thread(void *argv);

#ifdef __cplusplus
}
#endif

#endif
