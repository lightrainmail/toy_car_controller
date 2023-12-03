/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "nrf24l01p.h"
#include "oled.h"
#include "stream_buffer.h"
#include "adc.h"
#include "stream_buffer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
TaskHandle_t ADCHandle;
BaseType_t ADCRetval;

StreamBufferHandle_t streamBufferHandle;
StreamBufferHandle_t streamBufferHandle1;
/* USER CODE END Variables */
/* Definitions for Main */
osThreadId_t MainHandle;
const osThreadAttr_t Main_attributes = {
  .name = "Main",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for OLED */
osThreadId_t OLEDHandle;
const osThreadAttr_t OLED_attributes = {
  .name = "OLED",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void APP_ADC(void * param);
/* USER CODE END FunctionPrototypes */

void APP_main(void *argument);
void APP_OLED(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Main */
  MainHandle = osThreadNew(APP_main, NULL, &Main_attributes);

  /* creation of OLED */
  OLEDHandle = osThreadNew(APP_OLED, NULL, &OLED_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /*Creat ADC task*/
  ADCRetval = xTaskCreate(APP_ADC,"ADC",128,NULL,osPriorityHigh,ADCHandle);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  streamBufferHandle = xStreamBufferCreate(8,4);
  streamBufferHandle1 = xStreamBufferCreate(8,4);
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_APP_main */
/**
  * @brief  Function implementing the Main thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_APP_main */
void APP_main(void *argument)
{
  /* USER CODE BEGIN APP_main */
  uint16_t streamBuffer[2];
  uint8_t TxBuff[4];
  /* Infinite loop */
  for(;;)
  {
    xStreamBufferReceive(streamBufferHandle,streamBuffer,4, pdMS_TO_TICKS(10));
    TxBuff[0] = (uint8_t)streamBuffer[0];
    TxBuff[1] = (uint8_t)(streamBuffer[0] >> 8);
    TxBuff[2] = (uint8_t)streamBuffer[1];
    TxBuff[3] = (uint8_t)(streamBuffer[1] >> 8);
    nrf24l01p_tx_transmit(TxBuff);
    xStreamBufferSend(streamBufferHandle1,streamBuffer,4,0);
    osDelay(1);
  }
  /* USER CODE END APP_main */
}

/* USER CODE BEGIN Header_APP_OLED */
/**
* @brief Function implementing the OLED thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_APP_OLED */
void APP_OLED(void *argument)
{
  /* USER CODE BEGIN APP_OLED */
  uint16_t adcValue[2];
  OLED_ShowString(2,4,(uint8_t*)"ADC1:");
  OLED_ShowString(2,6,(uint8_t*)"ADC2:");
  /* Infinite loop */
  for(;;)
  {
    xStreamBufferReceive(streamBufferHandle1,adcValue,4,0);
    OLED_ShowNum(2 + 8*5,4,adcValue[0],4,SIZE);
    OLED_ShowNum(2 + 8*5,6,adcValue[1],4,SIZE);
    osDelay(1);
  }
  /* USER CODE END APP_OLED */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void APP_ADC(void *param) {
  uint16_t adcValue[2];
  for(;;) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_Start(&hadc2);
    if(HAL_ADC_PollForConversion(&hadc1,10) == HAL_OK) {
      adcValue[0] = HAL_ADC_GetValue(&hadc1);
    }
    if(HAL_ADC_PollForConversion(&hadc2,10) == HAL_OK) {
      adcValue[1] = HAL_ADC_GetValue(&hadc2);
    }
    xStreamBufferSend(streamBufferHandle,adcValue,4, pdMS_TO_TICKS(10));

    vTaskDelay(1);
  }
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if(GPIO_Pin == NRF24L01P_IRQ_PIN_NUMBER) {
    nrf24l01p_tx_irq(); //clear interrupt flag
  }
}
/* USER CODE END Application */

