/**
 *******************************************************************************
 * @file    DS1820.c
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
 * @attention   
 *          Requires working OneWire library. Please notice Strong or Weak Pull
 *          Up states warnings for each function, this only applies for parasite 
 *          power configuration.
 * 
 * @verbatim
 *          ********************************************************************
 *                                How to use this library
 *          ********************************************************************
 *          1. Initialize using DS1820_Init().
 * 
 *          2. Use DS1820_Search to discover DS1820 sensors on the bus
 *
 *          3. Now you can use DS1820_TemperatureConvert to start temperature 
 *          measurements on selected device.
 * 
 *          4. Wait at least 500 ms to complete temperature conversion.
 *              
 *          5. Read temperature by DS1820_TemperatureGet function.          
 *  @endverbatim  
 *******************************************************************************
 */
#include "DS1820.h"
#include "OneWire.h"

/* DS1820 specific commands */
#define SCRATCHPAD_READ     0xBE
#define SCRATCHPAD_STORE    0x48
#define SCRATCHPAD_WRITE    0x4E
#define SCRATCHPAD_RECALL   0xB8
#define POWER_SUPPLY_READ   0xB4

/* DS1820 scratchpad length in bytes */
#define SCRATCHPAD_LENGTH   9
#define SCRATCHPAD_CRC_POS  (SCRATCHPAD_LENGTH - 1)

/* Internal functions */
static uint8_t ScratchPadRead(uint8_t *bBuffer);
static void ScratchPadWrite(uint8_t iThresholdHigh, uint8_t iThresholdLow);
static void ScratchPadStore(void);
static void  ScratchPadRecall(void);
static uint8_t PowerSupplyType(void);
static void TemperatureConvert(void);

/**
 * Initalizes and resets OneWire communication.
 */
void DS1820_Init(void) {
    OW_Init();
    OW_Reset();
}

/**
 * Initializes temperature measurement on DS1820 chip.
 * @warning This function sets communication pin in StrongPullUp state.
 * @warning The bus has to be in StrongPullUp state at least for 500 ms.
 * @param iAddress 64bit device address, use DS1820_ADDRESS_ALL for all 
 * devices.
 * @return DS1820_OK if successfull, DS1820_ERROR if failed.
 */
DS1820_State DS1820_TemperatureConvert(uint64_t iAddress) {

    /* Ready bus for communcation */
    OW_WeakPullUp();

    /* Device selection */
    if (OW_ROMMatch(iAddress) == OW_NO_DEV) return DS1820_ERROR;

    /* Issue convert temperature command */
    TemperatureConvert();

    /* Power up device */
    OW_StrongPullUp();

    return DS1820_OK;
}

/**
 * Reads tepmerature from specific device. You have to use TemperatureConvert 
 * function before calling TemperatureGet.
 * @param iAddress 64bit device address, use DS1820_ADDRESS_ALL to skip 
 * address match (only for single device on the bus).
 * @return Temperature in degrees of Celsius * 10 or DS1820_TEMP_ERROR in case 
 * of an error.
 */
int DS1820_TemperatureGet(uint64_t iAddress) {
    int32_t iTemp;
    uint8_t iSPad[SCRATCHPAD_LENGTH];

    /* Ready bus for communcation */
    OW_WeakPullUp();

    /* Select device, fail if not present */
    if (OW_ROMMatch(iAddress)) return DS1820_TEMP_ERROR;

    /* Read DS1820 scratchpad, fail if CRC do not match */
    if (ScratchPadRead(iSPad)) return DS1820_TEMP_ERROR;

    /* Calculate temperature from Scratchpad, step 1 */
    iTemp = (iSPad[1] == 0) ? ((int) iSPad[0] * 500) : ((int) iSPad[0] * -500);

    /* Calculate temperature from scratchpad, step 2 (High resolution) */
    iTemp += -250 + ((1000 * (iSPad[7] - iSPad[6])) / iSPad[7]);

    /* Return final value and cut unnecessary decimal places */
    return (int) (iTemp / 100);
}

/**
 * Function sets temperature alarm for high an low thresholds.
 * @param iAddress 64bit device address, use DS1820_ADDRESS_ALL to skip 
 * address match (only for single device on the bus).
 * @param iHigh High temperature threshold, in degrees of Celsius.
 * @param iLow Low temperature threshold, in degrees of Celsius.
 * @return DS1820_OK if successfull, DS1820_ERROR if failed.
 */
DS1820_State DS1820_TemperatureAlarmSet(uint64_t iAddress, int iHigh, int iLow) {
    uint8_t iConvHigh, iConvLow;

    /* Ready bus for communcation */
    OW_WeakPullUp();

    /* Select device, fail if not present */
    if (OW_ROMMatch(iAddress)) return DS1820_ERROR;

    iConvHigh = (iHigh >= 0) ? (uint8_t) iHigh & 0x7F : 0x80 | ((uint8_t) iHigh & 0x7F);

    iConvLow = (iLow >= 0) ? (uint8_t) iLow & 0x7F : 0x80 | ((uint8_t) iLow & 0x7F);

    ScratchPadWrite(iConvHigh, iConvLow);

    return DS1820_OK;
}

/**
 * Function receives temperature alarm for high an low thresholds.
 * @param iAddress 64bit device address, use DS1820_ADDRESS_ALL to skip 
 * address match (only for single device on the bus).
 * @param iHigh High temperature threshold, in degrees of Celsius.
 * @param iLow Low temperature threshold, in degrees of Celsius.
 * @return DS1820_OK if successfull, DS1820_ERROR if failed.
 */
DS1820_State DS1820_TemperatureAlarmGet(uint64_t iAddress, int *iHigh, int *iLow) {
    uint8_t iSPad[SCRATCHPAD_LENGTH];

    /* Ready bus for communcation */
    OW_WeakPullUp();

    /* Select device, fail if not present */
    if (OW_ROMMatch(iAddress)) return DS1820_ERROR;

    /* Read DS1820 scratchpad, fail if CRC do not match */
    if (ScratchPadRead(iSPad)) return DS1820_ERROR;

    /* Calculate high temperature threshold from scratchpad */
    (*iHigh) = (iSPad[3] & 0x80) ? -((int) (iSPad[3] & 0x7F)) : (int) (iSPad[3] & 0x7F);

    /* Calculate low temperature threshold from scratchpad */
    (*iLow) = (iSPad[4] & 0x80) ? -((int) (iSPad[4] & 0x7F)) : (int) (iSPad[4] & 0x7F);

    return DS1820_OK;
}

/**
 * Saves device volatile configuration into internal EEPROM.
 * @warning This function sets communication pin in StrongPullUp state.
 * @warning The bus has to be in StrongPullUp state at least for 10 ms.
 * @param iAddress 64bit device address, use DS1820_ADDRESS_ALL for all 
 * devices.
 * @return DS1820_OK if successfull, DS1820_ERROR if failed.
 */
DS1820_State DS1820_ConfigurationStore(uint64_t iAddress) {
    /* Ready bus for communcation */
    OW_WeakPullUp();

    /* Select device, fail if not present */
    if (OW_ROMMatch(iAddress)) return DS1820_ERROR;

    /* Store configuration */
    ScratchPadStore();

    /* Power up device */
    OW_StrongPullUp();

    return DS1820_OK;
}

/**
 * Loads device configuration from internal EEPROM.
 * @param iAddress 64bit device address, use DS1820_ADDRESS_ALL for all 
 * devices.
 * @return DS1820_OK if successfull, DS1820_ERROR if failed.
 */
DS1820_State DS1820_ConfigurationRecall(uint64_t iAddress) {
    /* Ready bus for communcation */
    OW_WeakPullUp();

    /* Select device, fail if not present */
    if (OW_ROMMatch(iAddress)) return DS1820_ERROR;
    
    /* Recall configuration */
    ScratchPadRecall();
    
    return DS1820_OK;
}

/**
 * 
 * @param iAddress 64bit device address, use DS1820_ADDRESS_ALL for all 
 * devices.
 * @return DS1820_PARASITE_POWER (if at least one device is parasite powered) or
 * DS1820_EXTERNAL_POWER if successfull, DS1820_ERROR if failed.
 */
DS1820_State DS1820_PowerTypeGet(uint64_t iAddress){
     /* Ready bus for communcation */
    OW_WeakPullUp();

    /* Select device, fail if not present */
    if (OW_ROMMatch(iAddress)) return DS1820_ERROR;
    
    return PowerSupplyType();
}

/**
 * Function searches for DS1820 devices on the bus and stores them in to array.
 * @param Addresses Pointer to array for device addresses to be stored. 
 * @param iMaxDevices Maximum of devices to be searched.
 * @return Number of devices found.
 */
int DS1820_Search(uint64_t *Addresses, int iMaxDevices) {
    int iCount = 0;
    uint64_t iAddress;

    /* Ready bus for communcation */
    OW_WeakPullUp();

    /* Search for first DS1820 device */
    iAddress = OW_SearchFirst(0);

    /* Store all device addresses into a array */
    while ((iAddress) && (iCount < iMaxDevices)) {
        iCount++;
        Addresses[iCount - 1] = iAddress;
        iAddress = OW_SearchNext();
    }

    /* Reset communication */
    OW_Reset();

    return iCount;
}

/**
 * This internal function reads a device scratchpad and calculates CRC.
 * @param Buffer Scratchpad output
 * @return 0 if CRC match, 1 if do not.
 */
uint8_t ScratchPadRead(uint8_t *Buffer) {
    int i;
    uint8_t iCRC = 0;

    /* Issue read scratchpad command */
    OW_ByteWrite(SCRATCHPAD_READ);

    /* Read scratchpad */
    for (i = 0; i < SCRATCHPAD_LENGTH; i++) {
        Buffer[i] = OW_ByteRead();
        /* Calculate CRC */
        if (i != SCRATCHPAD_CRC_POS)
            iCRC = OW_CRCCalculate(iCRC, Buffer[i]);
    }
    /* Match CRC */
    return (iCRC != Buffer[SCRATCHPAD_CRC_POS]);
}

/**
 * This internal function write two bytes into a device scratchpad.
 * @param iThresholdHigh High temperature threshold, MSB is sign bit.
 * @param iThresholdLow Low temperature threshold, MSB is sign bit.
 */
void ScratchPadWrite(uint8_t iThresholdHigh, uint8_t iThresholdLow) {
    OW_ByteWrite(SCRATCHPAD_WRITE);
    OW_ByteWrite(iThresholdHigh);
    OW_ByteWrite(iThresholdLow);
}

/**
 * Store configuration into EEPROM.
 */
void ScratchPadStore(void) {
    OW_ByteWrite(SCRATCHPAD_STORE);
}

/**
 * Recall configuration from EEPROM.
 */
void ScratchPadRecall(void) {
    OW_ByteWrite(SCRATCHPAD_RECALL);
}

/**
 * Reads device power supply configuration.
 * @return DS1820_PARASITE_POWER or DS1820_EXTERNAL_POWER
 */
uint8_t PowerSupplyType(void) {
    OW_ByteWrite(POWER_SUPPLY_READ);
    return (OW_ByteRead()) ? DS1820_EXTERNAL_POWER : DS1820_PARASITE_POWER;
}

/**
 * Starts temperature conversion.
 */
void TemperatureConvert(void) {
    OW_ByteWrite(0x44);
}
