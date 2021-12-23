/*
 * irq.c
 *
 *  Created on: Sep 8, 2021
 *      Author: Dhruv
 *      Brief : Contains all IRQ Handlers.
 */
#include "irq.h"
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

/* Use LFXO and prescalar for high energy modes */
#if ((LOWEST_ENERGY_MODE == EM0) || (LOWEST_ENERGY_MODE == EM1) || (LOWEST_ENERGY_MODE == EM2))
#define PRESCALAR_VALUE   4
#define ACTUAL_CLOCK_FREQ (32768/PRESCALAR_VALUE)

/* Use ULFRCO and prescalar for low energy modes */
#else
#define PRESCALAR_VALUE   1
#define ACTUAL_CLOCK_FREQ (1000/PRESCALAR_VALUE)
#endif

#define LETIMER_CTR_VAL   ((LETIMER_PERIOD_MS * ACTUAL_CLOCK_FREQ)/1000)
#define LETIMER_COMP1_VAL ((LETIMER_ON_TIME_MS * ACTUAL_CLOCK_FREQ)/1000)

I2C_TransferReturn_TypeDef I2CTransferReturn;
/* Keeps count of time passed since the system startup */
static uint32_t milliseconds;
uint8_t gestureFlag;


/***************************************************************************//**
 * @name LETIMER0_IRQHandler
 *
 * @brief
 *   Interrupt handler which sets events based on the interrupt.
 *
 * @param[in] osc
 *   none
 *
 * @return void
 ******************************************************************************/
void LETIMER0_IRQHandler()
{

  /* Check which IF is set */
  uint32_t flags = LETIMER_IntGetEnabled(LETIMER0);

  /* Clear the interrupt */
  LETIMER_IntClear(LETIMER0, flags);

  /* Set the UF Event */
  if(flags == LETIMER_IEN_UF)
    {
#if DEVICE_IS_BLE_SERVER
      schedulerSetEvent_UF();
#endif
      /* 3 seconds have passed */
      CORE_CRITICAL_SECTION(milliseconds += LETIMER_PERIOD_MS;);
    }

#if DEVICE_IS_BLE_SERVER
  /* Set the COMP1 Event */
  if(flags == LETIMER_IEN_COMP1)
    {
      LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);
      schedulerSetEvent_COMP1();
    }
#endif
}

uint32_t letimerMilliseconds()
{
  uint32_t ctrmilliseconds;

  ctrmilliseconds = (LETIMER_CTR_VAL - LETIMER_CounterGet(LETIMER0));
  ctrmilliseconds = (ctrmilliseconds*1000)/ACTUAL_CLOCK_FREQ;


  return (milliseconds + ctrmilliseconds);
}

#if DEVICE_IS_BLE_SERVER
/***************************************************************************//**
 * @name GPIO_EVEN_IRQHandler
 *
 * @brief
 *   Interrupt handler which sets events based on the GPIO even pin interrupt.
 *
 * @param[in] osc
 *   none
 *
 * @return void
 ******************************************************************************/
void GPIO_EVEN_IRQHandler()
{
  /* Check which IF is set */
  uint32_t flags = GPIO_IntGetEnabled();

  /* Clear the interrupt */
  GPIO_IntClear(flags);

  /* Set the button release event */
  if(flags == (1 << PB0_pin))
    {
      schedulerSetEvent_ButtonPressed();
    }
  else if(flags == (1 << PF0_pin))
  {
    gestureFlag = 1;
    schedulerSetEvent_GestureInt();
  }
}

void ADC0_IRQHandler()
{
  /* Check which IF is set */
  uint32_t flags = ADC_IntGetEnabled(ADC0);

  /* Clear the interrupt */
  ADC_IntClear(ADC0, flags);

  ADC_DataSingleGet(ADC0);

  //ADC_Start(ADC0, adcStartSingle);
  /* Set the ADC Conversion complete event */
  if(flags == ADC_IEN_SINGLE)
    schedulerSetEvent_ADC0_Single();

}
#else
/***************************************************************************//**
 * @name GPIO_EVEN_IRQHandler
 *
 * @brief
 *   Interrupt handler which sets events based on the GPIO even pin interrupt.
 *
 * @param[in] osc
 *   none
 *
 * @return void
 ******************************************************************************/
void GPIO_EVEN_IRQHandler()
{
  /* Check which IF is set */
  uint32_t flags = GPIO_IntGetEnabled();

  /* Clear the interrupt */
  GPIO_IntClear(flags);

  /* Set the button release event */
  if(flags == (1 << PB0_pin))
    {
      schedulerSetEvent_ButtonPressed_PB0();
    }

}
/***************************************************************************//**
 * @name GPIO_ODD_IRQHandler
 *
 * @brief
 *   Interrupt handler which sets events based on the GPIO odd pin interrupt.
 *
 * @param[in] osc
 *   none
 *
 * @return void
 ******************************************************************************/
void GPIO_ODD_IRQHandler()
{
  /* Check which IF is set */
  uint32_t flags = GPIO_IntGetEnabled();

  /* Clear the interrupt */
  GPIO_IntClear(flags);

  /* Set the button release event */
  if(flags == (1 << PB1_pin))
    {
      schedulerSetEvent_ButtonPressed_PB1();
    }

}
#endif
