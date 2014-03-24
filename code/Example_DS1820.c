/**
 *******************************************************************************
 * @file    Example_DS1820.c
 * @author  Vojtěch Vigner <vojtech.vigner@gmail.com>
 * @date    24-March-2014
 * @brief   Simple code example for DS1820 library.
 *******************************************************************************
 */

#include <stdio.h>
#include "DS1820.h"

#define MAX_DEVICES 	8
#define MAX_RETRIES 2

static void Delay(int iMiliSecons) {
    /* Some code */
};

static void LEDToggle(void) {
    /* Some code */
};

int main(void) {

    int i;
    int iRetry;
    uint64_t Address[MAX_DEVICES];

    /* Initialize DS1820 */
    DS1820_Init();

    /* Search for devices */
    do {
        Delay(250);
        LEDToggle();
        iDevCount = DS1820_Search(Address, MAX_DEVICES);
    } while (iDevCount <= 0);

    /* Main loop */
    while (1) {
        /* Start conversion on all devices */
        DS1820_TemperatureConvert(DS1820_ADDRESS_ALL);
        LEDToggle();
        
        /* A necessary delay for conversion, @ref DS1820 datasheet */
        Delay(750);
        
        /* Read the temperature value from all devices respectively 
         * 
         * The number of maximum retries is in the MAX_RETRIES constant,
         * this is useful for a long cable connection or if the signal is jammed.
         */
        for (i = 0; i < iDevCount; i++) {
            iRetry = 0;
            do {
                Temperature[i] = DS1820_TemperatureGet(Address[i]);
                iRetry++;
            } while ((Temperature[i] == DS1820_TEMP_ERROR) && (iRetry < MAX_RETRIES));
        }

        /* Print temperatures */
        for (i = 0; i < iDevCount; i++) {
            if (Temperature[i] == DS1820_TEMP_ERROR) {
                printf("; ---.-");
            } else {
                printf("; %3d.%01d", Temperature[i] / 10, Temperature[i] % 10);
            }
        }
        printf("\r\n");
    }
    return 0;
}