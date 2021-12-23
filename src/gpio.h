/*
   gpio.h
  
    Created on: Dec 12, 2018
        Author: Dan Walkes

    Updated by Dave Sluiter Sept 7, 2020. moved #defines from .c to .h file.
    Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_
#include <stdbool.h>
#include "em_gpio.h"
#include <string.h>


// Student Edit: Define these, 0's are placeholder values.
// See the radio board user guide at https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf
// and GPIO documentation at https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__GPIO.html
// to determine the correct values for these.

#define	LED0_port  gpioPortF // change to correct ports and pins
#define LED0_pin   4
#define LED1_port  gpioPortF
#define LED1_pin   5
#define I2C0_port  gpioPortC
#define I2C0_SCL   10
#define I2C0_SDA   11
#define SENS_port  gpioPortD
#define SENS_EN    15
#define DISP_port  gpioPortD
#define DISP_EN    13
#define PB0_port   gpioPortF
#define PB0_pin    6
#define PB1_port   gpioPortF
#define PB1_pin    7
#define AMB_port   gpioPortD
#define AMB_pin    10

#define PF0_port   gpioPortF
#define PF0_pin    0


// Function prototypes
void gpioInit();
void gpioLed0SetOn();
void gpioLed0SetOff();
void gpioLed1SetOn();
void gpioLed1SetOff();
void gpioSensorEnSetOn();
void gpioSetDisplayExtcomin(bool value);
void gpioAMBSensor(bool value);

#endif /* SRC_GPIO_H_ */
