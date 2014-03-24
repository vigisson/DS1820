/**
 *******************************************************************************
 * @file    DS1820.h
 * @author  Vojtech Vigner
 * @version V1.0.1
 * @date    12-February-2013
 * @brief   This file provides functions to manage the following functionalities
 *          of the 1-Wire Digital Thermometer DS1820 by DALLAS Semiconductor:           
 *              - Initialization and Configuration
 *              - Temperature Measurements
 *              - Device Power Informations
 *              - Temperature Threshold Configuration
 *          
 * @see     DS1820.c documentation
 *******************************************************************************
 */

#ifndef DS1820_H
#define	DS1820_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "stdint.h"

    /* Public DS1820 constants */
#define DS1820_ADDRESS_ALL      0
#define DS1820_FAMILY_CODE      0x10

    /* Return values definition */
    typedef enum _DS1820_State {
        DS1820_OK = 0,
        DS1820_ERROR = 1,
        DS1820_TEMP_ERROR = -10000,

        DS1820_PARASITE_POWER = 0x10,
        DS1820_EXTERNAL_POWER = 0x20
    } DS1820_State;

    /* Function headers */
    void DS1820_Init(void);

    /* Temperature measurement */
    DS1820_State DS1820_TemperatureConvert(uint64_t iAddress);
    int DS1820_TemperatureGet(uint64_t iAddress);

    /* Alarms */
    DS1820_State DS1820_TemperatureAlarmSet(uint64_t iAddress, int iHigh, int iLow);
    DS1820_State DS1820_TemperatureAlarmGet(uint64_t iAddress, int *iHigh, int *iLow);

    /* Configuration */
    DS1820_State DS1820_ConfigurationStore(uint64_t iAddress);
    DS1820_State DS1820_ConfigurationRecall(uint64_t iAddress);

    /* Device info */
    DS1820_State DS1820_PowerTypeGet(uint64_t iAddress);

    /* Device discovery */
    int DS1820_Search(uint64_t *Addresses, int iMaxDevices);


#ifdef	__cplusplus
}
#endif

#endif	/* DS1820_H */

