/*
 * gesture_main.c
 *
 *  Created on: 15-Nov-2021
 *      Author: Pradyumna
 *
 *      Reference : 1) https://github.com/sparkfun/SparkFun_APDS-9960_Sensor_Arduino_Library
 *                  2) https://learn.sparkfun.com/tutorials/apds-9960-rgb-and-gesture-sensor-hookup-guide/all
 *                  3) https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/si114x__algorithm_8c_source.html
 */

#include "gesture_main.h"
#include "stdbool.h"
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"


// Defines
#define I2C_ADDRESS                     0x39 // Address from Data sheet APDS9960
#define I2C_ADDRESS_MASK                0xFC // Address of UP buffer
#define I2C_RXBUFFER_SIZE                 10

// Buffers++
uint8_t i2c_txBuffer[] = "Gecko";
uint8_t i2c_txBufferSize = sizeof(i2c_txBuffer);
uint8_t i2c_rxBuffer[I2C_RXBUFFER_SIZE];
uint8_t i2c_rxBufferIndex;

// Transmission flags
volatile bool i2c_rxInProgress;
volatile bool i2c_startTx;

/* Members */
gesture_data_type gesture_data_;
int gesture_ud_delta_;
int gesture_lr_delta_;
int gesture_ud_count_;
int gesture_lr_count_;
int gesture_near_count_;
int gesture_far_count_;
int gesture_state_;
int gesture_motion_;

/*******************************************************************************
 * @name gesture_init
 * @brief Gesture sensor initialization function
 *
 * @param none
 * @return True if all registers are initialized, else False
 *
 *******************************************************************************/
int gesture_init(){

  uint8_t id = 0;

  /* Initialize I2C */
  I2CInit_gesture();

  if( !gesture_Read_Register(I2C0 ,APDS9960_I2C_ADDR, APDS9960_ID, &id)) {
       return false;
  }
  if( !(id == APDS9960_ID_1 || id == APDS9960_ID_2 || id == APDS9960_ID_3) ) {
       return false;
  }

  /* Set ENABLE register to 0 (disable all features) */
  if( !setMode(ALL, OFF) ) {
      return false;
  }

  /* Set default values for gesture sense registers */
      if( !setGestureEnterThresh(DEFAULT_GPENTH) ) {
          return false;
      }

      if( !setGestureExitThresh(DEFAULT_GEXTH) ) {
          return false;
      }

      if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_GCONF1, DEFAULT_GCONF1) ) {
          return false;
      }

      if( !setGestureGain(DEFAULT_GGAIN) ) {
          return false;
      }

      if( !setGestureLEDDrive(DEFAULT_GLDRIVE) ) {
          return false;
      }
      if( !setGestureWaitTime(DEFAULT_GWTIME) ) {
          return false;
      }
      if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_GOFFSET_U, DEFAULT_GOFFSET) ) {
          return false;
      }
      if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_GOFFSET_D, DEFAULT_GOFFSET)) {
          return false;
      }
      if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_GOFFSET_L, DEFAULT_GOFFSET) ) {
          return false;
      }
      if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_GOFFSET_R, DEFAULT_GOFFSET)) {
          return false;
      }
      if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_GPULSE, DEFAULT_GPULSE)) {
          return false;
      }
      if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_GCONF3, DEFAULT_GCONF3)) {
          return false;
      }
      if( !setGestureIntEnable(DEFAULT_GIEN) ) {
          return false;
      }

      return true;
}

/**
 * @brief Enables or disables a feature in the APDS-9960
 *
 * @param[in] mode which feature to enable
 * @param[in] enable ON (1) or OFF (0)
 * @return True if operation success. False otherwise.
 */
bool setMode(uint8_t mode, uint8_t enable){

  uint8_t reg_val;

  /* Read current ENABLE register */
  reg_val = getMode();
  if( reg_val == ERROR ) {
     return false;
  }

  /* Change bit(s) in ENABLE register */
  enable = enable & 0x01;
  if(mode <= 6 ) {
  if (enable) {
    reg_val |= (1 << mode);
  } else {
    reg_val &= ~(1 << mode);
  }
  } else if( mode == ALL ) {
    if (enable) {
    reg_val = 0x7F;
  } else {
    reg_val = 0x00;
  }
  }

   /* Write value back to ENABLE register */
   if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_ENABLE, reg_val) ) {
       return false;
   }

   return true;
}

/**
 * @brief Sets the entry proximity threshold for gesture sensing
 *
 * @param[in] threshold proximity value needed to start gesture mode
 * @return True if operation successful. False otherwise.
 */
bool setGestureEnterThresh(uint8_t threshold)
{
    if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_GPENTH, threshold) ) {
        return false;
    }

    return true;
}

/**
 * @brief Sets the exit proximity threshold for gesture sensing
 *
 * @param[in] threshold proximity value needed to end gesture mode
 * @return True if operation successful. False otherwise.
 */
bool setGestureExitThresh(uint8_t threshold)
{
    if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_GEXTH, threshold)) {
        return false;
    }

    return true;
}

/**
 * @brief Sets the gain of the photodiode during gesture mode
 *
 * Value    Gain
 *   0       1x
 *   1       2x
 *   2       4x
 *   3       8x
 *
 * @param[in] gain the value for the photodiode gain
 * @return True if operation successful. False otherwise.
 */
bool setGestureGain(uint8_t gain)
{
    uint8_t val = 0;

    /* Read value from GCONF2 register */
    if( !gesture_Read_Register(I2C0 ,APDS9960_I2C_ADDR, APDS9960_GCONF2, &val) ) {
        return false;
    }

    /* Set bits in register to given value */
    gain &= 0b00000011;
    gain = gain << 5;
    val &= 0b10011111;
    val |= gain;

    /* Write register value back into GCONF2 register */
    if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_GCONF2, val) ) {
        return false;
    }

    return true;
}

/**
 * @brief Sets the LED drive current during gesture mode
 *
 * Value    LED Current
 *   0        100 mA
 *   1         50 mA
 *   2         25 mA
 *   3         12.5 mA
 *
 * @param[in] drive the value for the LED drive current
 * @return True if operation successful. False otherwise.
 */
bool setGestureLEDDrive(uint8_t drive)
{
    uint8_t val = 0;

    /* Read value from GCONF2 register */
    if( !gesture_Read_Register(I2C0 ,APDS9960_I2C_ADDR, APDS9960_GCONF2, &val) ) {
        return false;
    }

    /* Set bits in register to given value */
    drive &= 0b00000011;
    drive = drive << 3;
    val &= 0b11100111;
    val |= drive;

    /* Write register value back into GCONF2 register */
    if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_GCONF2, val) ) {
        return false;
    }

    return true;
}

/**
 * @brief Sets the time in low power mode between gesture detections
 *
 * Value    Wait time
 *   0          0 ms
 *   1          2.8 ms
 *   2          5.6 ms
 *   3          8.4 ms
 *   4         14.0 ms
 *   5         22.4 ms
 *   6         30.8 ms
 *   7         39.2 ms
 *
 * @param[in] the value for the wait time
 * @return True if operation successful. False otherwise.
 */
bool setGestureWaitTime(uint8_t time)
{
    uint8_t val = 0;

    /* Read value from GCONF2 register */
    if( !gesture_Read_Register(I2C0 ,APDS9960_I2C_ADDR, APDS9960_GCONF2, &val) ) {
        return false;
    }

    /* Set bits in register to given value */
    time &= 0b00000111;
    val &= 0b11111000;
    val |= time;

    /* Write register value back into GCONF2 register */
    if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_GCONF2, val) ) {
        return false;
    }

    return true;
}

/**
 * @brief Turns gesture-related interrupts on or off
 *
 * @param[in] enable 1 to enable interrupts, 0 to turn them off
 * @return True if operation successful. False otherwise.
 */
bool setGestureIntEnable(uint8_t enable)
{
    uint8_t val = 0;

    /* Read value from GCONF4 register */
    if( !gesture_Read_Register(I2C0 ,APDS9960_I2C_ADDR, APDS9960_GCONF4, &val)) {
        return false;
    }

    /* Set bits in register to given value */
    enable &= 0b00000001;
    enable = enable << 1;
    val &= 0b11111101;
    val |= enable;

    /* Write register value back into GCONF4 register */
    if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_GCONF4, val)) {
        return false;
    }

    return true;
}

/**
 * @brief Starts the gesture recognition engine on the APDS-9960
 *
 * @param[in] interrupts true to enable hardware external interrupt on gesture
 * @return True if engine enabled correctly. False on error.
 */
int enableGestureSensor(bool interrupts)
{

    /* Enable gesture mode
       Set ENABLE to 0 (power off)
       Set WTIME to 0xFF
       Set AUX to LED_BOOST_300
       Enable PON, WEN, PEN, GEN in ENABLE
    */
    resetGestureParameters();
    if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_WTIME, 0xFF)) {
        return false;
    }
    if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_PPULSE, DEFAULT_GESTURE_PPULSE)) {
        return false;
    }
    if( !setLEDBoost(LED_BOOST_300) ) {
        return false;
    }
    if( interrupts ) {
        if( !setGestureIntEnable(1) ) {
            return false;
        }
    } else {
        if( !setGestureIntEnable(0) ) {
            return false;
        }
    }
    if( !setGestureMode(1) ) {
        return false;
    }
    if( !enablePower() ){
        return false;
    }
    if( !setMode(WAIT_TIME, 1) ) {
        return false;
    }
    if( !setMode(PROXIMITY, 1) ) {
        return false;
    }
    if( !setMode(GESTURE, 1) ) {
        return false;
    }

    return true;
}

/**
 * @brief Ends the gesture recognition engine on the APDS-9960
 *
 * @return True if engine disabled correctly. False on error.
 */
bool disableGestureSensor()
{
    resetGestureParameters();
    if( !setGestureIntEnable(0) ) {
        return false;
    }
    if( !setGestureMode(0) ) {
        return false;
    }
    if( !setMode(GESTURE, 0) ) {
        return false;
    }

    return true;
}

/**
 * @brief Determines if there is a gesture available for reading
 *
 * @return True if gesture available. False otherwise.
 */
bool isGestureAvailable()
{
    uint8_t val = 0;

    /* Read value from GSTATUS register */
    if( !gesture_Read_Register(I2C0 ,APDS9960_I2C_ADDR, APDS9960_GSTATUS, &val)) {
        return ERROR;
    }

    /* Shift and mask out GVALID bit */
    val &= APDS9960_GVALID;

    /* Return true/false based on GVALID bit */
    if( val == 1) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief Reads and returns the contents of the ENABLE register
 *
 * @return Contents of the ENABLE register. 0xFF if error.
 */
uint8_t getMode()
{
    uint8_t enable_value = 0;

    /* Read current ENABLE register */
    if( !gesture_Read_Register(I2C0 ,APDS9960_I2C_ADDR, APDS9960_ENABLE, &enable_value)) {
        return ERROR;
    }

    return enable_value;
}

/**
 * @brief Processes a gesture event and returns best guessed gesture
 *
 * @return Number corresponding to gesture. -1 on error.
 */
int readGesture()
{
    uint8_t fifo_level = 0;
    uint8_t fifo_data[128];
    uint8_t gstatus = 0;
    int bytes_read = 0;
    int motion;
    int i;

    /* Make sure that power and gesture is on and data is valid */
    if( !isGestureAvailable() || !(getMode() & 0b01000001) ) {
        return DIR_NONE;
    }

    /* Keep looping as long as gesture data is valid */
    while(1) {

        /* Wait some time to collect next batch of FIFO data */
        timerWaitUs_polled(FIFO_PAUSE_TIME * 1000); // To understand to use LETIMER

        /* Get the contents of the STATUS register. Is data still valid? */
        if( !gesture_Read_Register(I2C0 ,APDS9960_I2C_ADDR, APDS9960_GSTATUS, &gstatus)) {
            return ERROR;
        }

        /* If we have valid data, read in FIFO */
        if( (gstatus & APDS9960_GVALID) == APDS9960_GVALID ) {

            /* Read the current FIFO level */
            if( !gesture_Read_Register(I2C0 ,APDS9960_I2C_ADDR, APDS9960_GFLVL, &fifo_level)) {
                return ERROR;
            }

#if DEBUG
            LOG_INFO ("FIFO Level: %d\r\n", fifo_level);
#endif

            /* If there's stuff in the FIFO, read it into our data block */
            if( fifo_level > 0) {
                bytes_read = gesture_Read_Block_Register(I2C0, APDS9960_I2C_ADDR,
                                                         APDS9960_GFIFO_U,
                                                         (fifo_level * 4),
                                                         (uint8_t*)fifo_data);

                if( bytes_read == -1 ) {
                    return ERROR;
                }
#if DEBUG
                LOG_INFO ("FIFO Dump: %d\r\n", fifo_level);
                for ( i = 0; i < bytes_read; i++ ) {
                  LOG_INFO (" %d\r\n", fifo_level);
                }
                LOG_INFO ("\r\n");


#endif

                /* If at least 1 set of data, sort the data into U/D/L/R */
                if( bytes_read >= 4 ) {
                    for( i = 0; i < bytes_read; i += 4 ) {
                        gesture_data_.u_data[gesture_data_.index] = \
                                                            fifo_data[i + 0];
                        gesture_data_.d_data[gesture_data_.index] = \
                                                            fifo_data[i + 1];
                        gesture_data_.l_data[gesture_data_.index] = \
                                                            fifo_data[i + 2];
                        gesture_data_.r_data[gesture_data_.index] = \
                                                            fifo_data[i + 3];
                        gesture_data_.index++;
                        gesture_data_.total_gestures++;
                    }

#if DEBUG
                LOG_INFO ("Up Data: : \r\n");
                for ( i = 0; i < gesture_data_.total_gestures; i++ ) {
                    LOG_INFO (" %d\r\n", gesture_data_.u_data[i]);
                }
                LOG_INFO ("\r\n");
#endif

                    /* Filter and process gesture data. Decode near/far state */
                    if( processGestureData() ) {
                        if( decodeGesture() ) {
                            //***TODO: U-Turn Gestures
                        }
                    }

                    /* Reset data */
                    gesture_data_.index = 0;
                    gesture_data_.total_gestures = 0;
                }
            }
        } else {

            /* Determine best guessed gesture and clean up */
            timerWaitUs_polled(FIFO_PAUSE_TIME * 1000);
            decodeGesture();
            motion = gesture_motion_;
#if DEBUG
            LOG_INFO ("END: %d\r\n", gesture_motion_);
#endif
            resetGestureParameters();
            LOG_INFO("Gesture = %d\r\n", motion);
            return motion;
        }
    }

    LOG_INFO ("After end of while loop");
}


/*******************************************************************************
 * High-level gesture controls
 ******************************************************************************/

/**
 * @brief Resets all the parameters in the gesture data member
 */
void resetGestureParameters()
{
    gesture_data_.index = 0;
    gesture_data_.total_gestures = 0;

    gesture_ud_delta_ = 0;
    gesture_lr_delta_ = 0;

    gesture_ud_count_ = 0;
    gesture_lr_count_ = 0;

    gesture_near_count_ = 0;
    gesture_far_count_ = 0;

    gesture_state_ = 0;
    gesture_motion_ = DIR_NONE;
}

/**
 * @brief Processes the raw gesture data to determine swipe direction
 *
 * @return True if near or far state seen. False otherwise.
 */
bool processGestureData()
{
    uint8_t u_first = 1;
    uint8_t d_first = 1;
    uint8_t l_first = 1;
    uint8_t r_first = 1;
    uint8_t u_last = 1;
    uint8_t d_last = 1;
    uint8_t l_last = 1;
    uint8_t r_last = 1;
    int ud_ratio_first;
    int lr_ratio_first;
    int ud_ratio_last;
    int lr_ratio_last;
    int ud_delta;
    int lr_delta;
    int i;

    /* If we have less than 4 total gestures, that's not enough */
    if( gesture_data_.total_gestures <= 4 ) {
        return false;
    }

    /* Check to make sure our data isn't out of bounds */
    if( (gesture_data_.total_gestures <= 32) && \
        (gesture_data_.total_gestures > 0) ) {

        /* Find the first value in U/D/L/R above the threshold */
        for( i = 0; i < gesture_data_.total_gestures; i++ ) {
            /* Warning: Couldn't adjust the condition for u/l/d/r buffers for data transfer */
            if( (gesture_data_.u_data[i] > GESTURE_THRESHOLD_OUT) &&
                (gesture_data_.d_data[i] > GESTURE_THRESHOLD_OUT) ||
                (gesture_data_.l_data[i] > GESTURE_THRESHOLD_OUT) &&
                (gesture_data_.r_data[i] > GESTURE_THRESHOLD_OUT) ) {

                u_first = gesture_data_.u_data[i];
                d_first = gesture_data_.d_data[i];
                l_first = gesture_data_.l_data[i];
                r_first = gesture_data_.r_data[i];
                break;
            }
        }

        /* Find the last value in U/D/L/R above the threshold */
        for( i = gesture_data_.total_gestures - 1; i >= 0; i-- ) {

            if( ((gesture_data_.u_data[i] > GESTURE_THRESHOLD_OUT) &&
                (gesture_data_.d_data[i] > GESTURE_THRESHOLD_OUT)) ||
                ((gesture_data_.l_data[i] > GESTURE_THRESHOLD_OUT) &&
                (gesture_data_.r_data[i] > GESTURE_THRESHOLD_OUT)) ) {

                u_last = gesture_data_.u_data[i];
                d_last = gesture_data_.d_data[i];
                l_last = gesture_data_.l_data[i];
                r_last = gesture_data_.r_data[i];
                break;
            }
        }
    }

    /* Calculate the first vs. last ratio of up/down and left/right */
    ud_ratio_first = ((u_first - d_first) * 100) / (u_first + d_first);
    lr_ratio_first = ((l_first - r_first) * 100) / (l_first + r_first);
    ud_ratio_last = ((u_last - d_last) * 100) / (u_last + d_last);
    lr_ratio_last = ((l_last - r_last) * 100) / (l_last + r_last);



    /* Determine the difference between the first and last ratios */
    ud_delta = ud_ratio_last - ud_ratio_first;
    lr_delta = lr_ratio_last - lr_ratio_first;



    /* Accumulate the UD and LR delta values */
    gesture_ud_delta_ += ud_delta;
    gesture_lr_delta_ += lr_delta;



    /* Determine U/D gesture */
    if( gesture_ud_delta_ >= GESTURE_SENSITIVITY_1 ) {
        gesture_ud_count_ = 1;
    } else if( gesture_ud_delta_ <= -GESTURE_SENSITIVITY_1 ) {
        gesture_ud_count_ = -1;
    } else {
        gesture_ud_count_ = 0;
    }

    /* Determine L/R gesture */
    if( gesture_lr_delta_ >= GESTURE_SENSITIVITY_1 ) {
        gesture_lr_count_ = 1;
    } else if( gesture_lr_delta_ <= -GESTURE_SENSITIVITY_1 ) {
        gesture_lr_count_ = -1;
    } else {
        gesture_lr_count_ = 0;
    }

    /* Determine Near/Far gesture */
    if( (gesture_ud_count_ == 0) && (gesture_lr_count_ == 0) ) {
        if( (abs(ud_delta) < GESTURE_SENSITIVITY_2) && \
            (abs(lr_delta) < GESTURE_SENSITIVITY_2) ) {

            if( (ud_delta == 0) && (lr_delta == 0) ) {
                gesture_near_count_++;
            } else if( (ud_delta != 0) || (lr_delta != 0) ) {
                gesture_far_count_++;
            }

            if( (gesture_near_count_ >= 10) && (gesture_far_count_ >= 2) ) {
                if( (ud_delta == 0) && (lr_delta == 0) ) {
                    gesture_state_ = NEAR_STATE;
                } else if( (ud_delta != 0) && (lr_delta != 0) ) {
                    gesture_state_ = FAR_STATE;
                }
                return true;
            }
        }
    } else {
        if( (abs(ud_delta) < GESTURE_SENSITIVITY_2) && \
            (abs(lr_delta) < GESTURE_SENSITIVITY_2) ) {

            if( (ud_delta == 0) && (lr_delta == 0) ) {
                gesture_near_count_++;
            }

            if( gesture_near_count_ >= 10 ) {
                gesture_ud_count_ = 0;
                gesture_lr_count_ = 0;
                gesture_ud_delta_ = 0;
                gesture_lr_delta_ = 0;
            }
        }
    }



    return false;
}

/**
 * @brief Determines swipe direction or near/far state
 *
 * @return True if near/far event. False otherwise.
 */
bool decodeGesture()
{
    /* Return if near or far event is detected */
    if( gesture_state_ == NEAR_STATE ) {
        gesture_motion_ = DIR_NEAR;
        return true;
    } else if ( gesture_state_ == FAR_STATE ) {
        gesture_motion_ = DIR_FAR;
        return true;
    }

    /* Determine swipe direction */
    if( (gesture_ud_count_ == -1) && (gesture_lr_count_ == 0) ) {
        gesture_motion_ = DIR_UP;
    } else if( (gesture_ud_count_ == 1) && (gesture_lr_count_ == 0) ) {
        gesture_motion_ = DIR_DOWN;
    } else if( (gesture_ud_count_ == 0) && (gesture_lr_count_ == 1) ) {
        gesture_motion_ = DIR_RIGHT;
    } else if( (gesture_ud_count_ == 0) && (gesture_lr_count_ == -1) ) {
        gesture_motion_ = DIR_LEFT;
    } else if( (gesture_ud_count_ == -1) && (gesture_lr_count_ == 1) ) {
        if( abs(gesture_ud_delta_) > abs(gesture_lr_delta_) ) {
            gesture_motion_ = DIR_UP;
        } else {
            gesture_motion_ = DIR_RIGHT;
        }
    } else if( (gesture_ud_count_ == 1) && (gesture_lr_count_ == -1) ) {
        if( abs(gesture_ud_delta_) > abs(gesture_lr_delta_) ) {
            gesture_motion_ = DIR_DOWN;
        } else {
            gesture_motion_ = DIR_LEFT;
        }
    } else if( (gesture_ud_count_ == -1) && (gesture_lr_count_ == -1) ) {
        if( abs(gesture_ud_delta_) > abs(gesture_lr_delta_) ) {
            gesture_motion_ = DIR_UP;
        } else {
            gesture_motion_ = DIR_LEFT;
        }
    } else if( (gesture_ud_count_ == 1) && (gesture_lr_count_ == 1) ) {
        if( abs(gesture_ud_delta_) > abs(gesture_lr_delta_) ) {
            gesture_motion_ = DIR_DOWN;
        } else {
            gesture_motion_ = DIR_RIGHT;
        }
    } else {
        return false;
    }

    return true;
}

/*******************************************************************************
 * Getters and setters for register values
 ******************************************************************************/
/**
 * @brief Sets the LED current boost value
 *
 * Value  Boost Current
 *   0        100%
 *   1        150%
 *   2        200%
 *   3        300%
 *
 * @param[in] drive the value (0-3) for current boost (100-300%)
 * @return True if operation successful. False otherwise.
 */
bool setLEDBoost(uint8_t boost)
{
    uint8_t val = 0;

    /* Read value from CONFIG2 register */
    if( !gesture_Read_Register(I2C0 ,APDS9960_I2C_ADDR, APDS9960_CONFIG2, &val)) {
        return false;
    }

    /* Set bits in register to given value */
    boost &= 0b00000011;
    boost = boost << 4;
    val &= 0b11001111;
    val |= boost;

    /* Write register value back into CONFIG2 register */
    if( !gesture_Write_Register(I2C0,APDS9960_I2C_ADDR, APDS9960_CONFIG2, val)) {
        return false;
    }

    return true;
}

/**
 * @brief Tells the state machine to either enter or exit gesture state machine
 *
 * @param[in] mode 1 to enter gesture state machine, 0 to exit.
 * @return True if operation successful. False otherwise.
 */
bool setGestureMode(uint8_t mode)
{
    uint8_t val = 0;

    /* Read value from GCONF4 register */
    if( !gesture_Read_Register(I2C0 ,APDS9960_I2C_ADDR, APDS9960_GCONF4, &val)) {
        return false;
    }

    /* Set bits in register to given value */
    mode &= 0b00000001;
    val &= 0b11111110;
    val |= mode;

    /* Write register value back into GCONF4 register */
    if( !gesture_Write_Register(I2C0 ,APDS9960_I2C_ADDR, APDS9960_GCONF4, val)) {
        return false;
    }

    return true;
}

/**
 * Turn the APDS-9960 on
 *
 * @return True if operation successful. False otherwise.
 */
bool enablePower()
{
    if( !setMode(POWER, 1) ) {
        return false;
    }

    return true;
}

