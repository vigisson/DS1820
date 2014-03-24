DS1820 Communication Library for STM32Fxxx
===========

Author:    Vojtěch Vigner <vojtech.vigner@gmail.com>

Version:   V1.0.2

Date:      24-March-2014

Copyright: 2014, The BSD 3-Clause License.


This library provides DS1820 support for STM32Fxxx devices.

Additional Information
-----------
This library provides functions to manage the following functionalities
of the 1-Wire Digital Thermometer DS1820 by DALLAS Semiconductor:
  - Initialization and Configuration
  - Temperature Measurements
  - Device Power Informations
  - Temperature Threshold Configuration

How to use this library
-----------

1. Modify hardware specific section in OneWire.h file according to
your HW. Decide if you will be using parasite powered device/s and
enable or disable this support.

2. Initialize bus using DS1820_Init().

3. Now you can use all communication functions. 

4. See Example_DS1820.c for simple example. 

