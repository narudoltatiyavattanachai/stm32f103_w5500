/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "w5500_spi.h"
#include "hello_world.h"
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"
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
uint32_t task00 = 0;
uint32_t task01 = 0;
uint32_t task02 = 0;
uint32_t task03 = 0;
static bool hw_init = false;

/* USER CODE END Variables */
/* Definitions for Task00_1ms */
osThreadId_t Task00_1msHandle;
const osThreadAttr_t Task00_1ms_attributes = {
  .name = "Task00_1ms",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task01_10ms */
osThreadId_t Task01_10msHandle;
const osThreadAttr_t Task01_10ms_attributes = {
  .name = "Task01_10ms",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task02_100ms */
osThreadId_t Task02_100msHandle;
const osThreadAttr_t Task02_100ms_attributes = {
  .name = "Task02_100ms",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task03_1000ms */
osThreadId_t Task03_1000msHandle;
const osThreadAttr_t Task03_1000ms_attributes = {
  .name = "Task03_1000ms",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartTask00(void *argument);
void StartTask01(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);

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
  /* creation of Task00_1ms */
  Task00_1msHandle = osThreadNew(StartTask00, NULL, &Task00_1ms_attributes);

  /* creation of Task01_10ms */
  Task01_10msHandle = osThreadNew(StartTask01, NULL, &Task01_10ms_attributes);

  /* creation of Task02_100ms */
  Task02_100msHandle = osThreadNew(StartTask02, NULL, &Task02_100ms_attributes);

  /* creation of Task03_1000ms */
  Task03_1000msHandle = osThreadNew(StartTask03, NULL, &Task03_1000ms_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartTask00 */
/**
  * @brief  Function implementing the Task00_1ms thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTask00 */
void StartTask00(void *argument)
{
  /* USER CODE BEGIN StartTask00 */

  /* Infinite loop */
  for(;;)
  {
    if (!hw_init) {
      w5500_spi_init();
      hw_init = true;
    }

    task00++;
    //printf("Task00: %lu\n", (unsigned long)task00);

    osDelay(1);
  }
  /* USER CODE END StartTask00 */
}

/* USER CODE BEGIN Header_StartTask01 */
/**
* @brief Function implementing the Task01_10ms thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask01 */
void StartTask01(void *argument)
{
  /* USER CODE BEGIN StartTask01 */
  /* Infinite loop */
  for(;;)
  {

	task01++;
	//printf("Task01: %lu\n", (unsigned long)task01);

	osDelay(10);
  }
  /* USER CODE END StartTask01 */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the Task02_100ms thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  for(;;)
  {

	task02++;
    //printf("Task02: %lu\n", (unsigned long)task02);

	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_10);

	osDelay(100);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the Task03_1000ms thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  for(;;)
  {

    task03++;
    printf("Task03: %lu\n", (unsigned long)task03);

	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);
	
	// Send UDP hello world message using centralized configuration
	if (hw_init) {  // Only send if W5500 is initialized
	    int32_t result = hello_world_send_udp();
	    if (result > 0) {
	        printf("Task03: UDP sent(%ld bytes)\n", (long)result);
	    } else {
	        printf("Task03: UDP error %ld\n", (long)result);
	    }
	}

	osDelay(1000);
  }
  /* USER CODE END StartTask03 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

