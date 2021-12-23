/*
 * ble.h
 *
 *  Created on: Sep 30, 2021
 *      Author: Dhruv
 */

#ifndef SRC_BLE_H_
#define SRC_BLE_H_

#include "sl_bt_api.h"
#include "gatt_db.h"
#include "lcd.h"
#include "ble_device_type.h"
#include "gpio.h"

/* Macro Definitions */
#define UINT8_TO_BITSTREAM(p, n)      { *(p)++ = (uint8_t)(n); }

#define UINT32_TO_BITSTREAM(p, n)     { *(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
                                        *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); }

#define UINT32_TO_FLOAT(m, e)         (((uint32_t)(m) & 0x00FFFFFFU) | (uint32_t)((int32_t)(e) << 24))

#define AMBIENT 0
#define GESTURE_SNSR 1

#define LEFT  1
#define RIGHT 2
#define UP    3
#define DOWN  4

#define NOT_BONDED  0
#define BONDING     1
#define BONDED      2

#if DEVICE_IS_BLE_SERVER
typedef struct
{
   bd_addr myAddress;               /* Own Address */
   uint8_t advertisingSetHandle;    /* Advertising Handle */
   uint8_t amb_indications_enabled; /* Health Thermometer Indications enabled bool */
   uint8_t btn_indications_enabled; /* Button state indications enabled */
   uint8_t gesture_indications_enabled; /*Gesture sensor indications enabled */
   uint8_t gatt_server_connection;  /* Connection Handle */
   uint8_t in_flight;               /* Indication in flight */
   uint8_t bonding_state;           /* Bonded = 1/ Not bonded = 0 */
}ble_data_struct_t;
#else
typedef struct
{
   bd_addr myAddress;               /* Own Address */
   uint8_t amb_indications_enabled; /* Ambient Light Indications enabled bool */
   uint8_t gatt_server_connection;  /* Connection Handle */
   uint8_t ambient_service[16];      /* Thermometer Service UUID */
   uint8_t ambient_char[16];         /* Thermometer Measurement Characteristic UUID */
   uint8_t gesture_service[16];      /* Encrypted Service UUID */
   uint8_t gesture_char[16];      /* Encrypted Characteristic UUID (Button State) */
   uint32_t serviceHandle[2];       /* Service Handle */
   uint8_t characteristicHandle[2]; /* Characteristic Handle */
   bd_addr serverAddress;           /* Hard-coded address of server */
   uint8_t discoveryEvt;            /* Event in Discovery State Machine */
   uint8_t gesture_indications_enabled; /* Button state indications enabled */
   uint8_t bonding_state;           /* Bonded = 1/ Not bonded = 0 */
}ble_data_struct_t;
#endif

/* Function Prototypes */

/***************************************************************************//**
 * @name handle_ble_event
 *
 * @brief Contains the BLE State machine which handles all BLE events:
 *        Boot
 *        Connection Opened
 *        Connection Closed
 *        Connection Parameters
 *        Characteristic Status
 *        Indication Timeout
 *
 * @param[in] sl_bt_msg_t *evt - BLE Event
 *
 * @return void
 ******************************************************************************/
void handle_ble_event(sl_bt_msg_t *evt);

/***************************************************************************//**
 * @name SendTemperature
 *
 * @brief Sends the temperature from I2C to BLE for writing and sending
 *        indications to the Client when indications are enabled.
 *
 * @param[in] float Temperature - Temperature in C
 *
 * @return void
 ******************************************************************************/
void SendTemperature(float Temperature);

/***************************************************************************//**
 * @name send_gesture_value
 *
 * @brief Sends the gesture value from I2C to BLE for writing and sending
 *        indications to the Client when indications are enabled.
 *
 * @param[in] uint8_t gesture_val - value
 *
 * @return void
 ******************************************************************************/
void send_gesture_value(uint8_t gesture_val);

/***************************************************************************//**
 * @name SendLightValue
 *
 * @brief Sends the ambient light value from ADC to BLE for writing and sending
 *        indications to the Client when indications are enabled.
 *
 * @param[in] uint16_t analog_val - Light value
 *
 * @return void
 ******************************************************************************/
void SendLightValue(uint16_t analog_val);

/***************************************************************************//**
 * @name getBleDataPtr
 *
 * @brief Gets the BLE Data pointer. Used to obtain the following:
 *        BLE Address
 *        Advertising Handle
 *        Indication Enabled (Read Temperature)
 *        Connection Handle
 *
 * @param[in] float Temperature - Temperature in C
 *
 * @return ble_data_struct_t* - Pointer to instance of ble_data
 ******************************************************************************/
ble_data_struct_t* getBleDataPtr();

/***************************************************************************//**
 * @name getSensorValue
 *
 * @brief Get the value of the sensor received from the ble indication.
 *
 * @param[in] uint8_t sensortype - Type of sensor - AMBIENT/GESTURE
 *
 * @return uint16_t - Sensor value as received from indication
 ******************************************************************************/
uint16_t getSensorValue(uint8_t sensortype);

#endif /* SRC_BLE_H_ */
