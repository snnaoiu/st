/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "main.h"
#include "adc.h"
#include "can.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "ds18b20.h"
#include "oled.h"
#include "bmp.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct{
	uint8_t run;//调度标志
	uint16_t timCount;//时间片计数值
	uint16_t timRload;//时间片重载值
	void (*pTaskFuncCb)(void);//函数指针变量
} TaskComps_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TASK_NUM_MAX (sizeof(g_taskComps) / sizeof(g_taskComps[0]))
#define RTC_BKUP_MAGIC   0x0002
#define RTC_BKUP_YEAR     RTC_BKP_DR2
#define RTC_BKUP_MONTH    RTC_BKP_DR3
#define RTC_BKUP_DATE     RTC_BKP_DR4
#define RTC_BKUP_WEEKDAY  RTC_BKP_DR5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t g_page = 0;
float g_adc_voltage = 0.0f;
float g_temp_c = 0.0f;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint8_t KEY_Scan(void);
void KeyTask(void);
void UiTask(void);
void TempTask(void);
void PrintfTask(void);
void CanTask(void);
static void DrawHomePage(void);
static void DrawRtcPage(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static TaskComps_t g_taskComps[] = {
		{0, 10, 10, KeyTask},
		{0, 200, 200, UiTask},
		{0, 2000, 2000, TempTask},
		{0, 1000, 1000, PrintfTask},
		/*添加业务模块模块*/
};

static void TaskHandler(void){
	for(uint8_t i = 0; i < TASK_NUM_MAX; i++){
		if(g_taskComps[i].run){//
			g_taskComps[i].run = 0;
			if(g_taskComps[i].pTaskFuncCb == NULL){
				continue;
			}
			g_taskComps[i].pTaskFuncCb();
		}
	}
}

static void TaskScheduleCb(void)
{
	for (uint8_t i = 0; i < TASK_NUM_MAX; i++)
	{
		if (g_taskComps[i].timCount)
		{
			g_taskComps[i].timCount--;
			if (g_taskComps[i].timCount == 0)
			{
				g_taskComps[i].run = 1;
				g_taskComps[i].timCount = g_taskComps[i].timRload;
			}
		}
	}
}

static void AppInit(void)
{
	TaskScheduleCbReg(TaskScheduleCb);
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC2_Init();
  MX_CAN_Init();
  MX_RTC_Init();
  MX_USART3_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  __HAL_RCC_BKP_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  CAN_FilterTypeDef filter;

  filter.FilterActivation = ENABLE;
  filter.FilterBank = 0;
  filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  filter.FilterIdHigh = 0x0000;
  filter.FilterIdLow = 0x0000;
  filter.FilterMaskIdHigh = 0x0000;
  filter.FilterMaskIdLow = 0x0000;
  filter.FilterMode = CAN_FILTERMODE_IDMASK;
  filter.FilterScale = CAN_FILTERSCALE_32BIT;

  HAL_CAN_ConfigFilter(&hcan, &filter);

  OLED_Init();
  OLED_ColorTurn(0);
  OLED_DisplayTurn(0);
  OLED_Refresh();

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  HAL_PWR_EnableBkUpAccess();

  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != RTC_BKUP_MAGIC)
  {
      sTime.Hours = 23;
      sTime.Minutes = 58;
      sTime.Seconds = 0;
      HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

      sDate.Year = 26;
      sDate.Month = 6;
      sDate.Date = 4;
      sDate.WeekDay = RTC_WEEKDAY_MONDAY;
      HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, RTC_BKUP_MAGIC);
      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKUP_YEAR, sDate.Year);
      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKUP_MONTH, sDate.Month);
      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKUP_DATE, sDate.Date);
      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKUP_WEEKDAY, sDate.WeekDay);
  }
  else
  {
      sDate.Year = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKUP_YEAR);
      sDate.Month = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKUP_MONTH);
      sDate.Date = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKUP_DATE);
      sDate.WeekDay = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKUP_WEEKDAY);

      HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  }



  AppInit();
  DWT_Delay_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
//	  HAL_Delay(100);
//	  GPIOC->BSRR = 0x0200;//0
//	  for(int i = 0; i < 2; i++){__NOP();}
//	  GPIOC->BRR = 0x0200;
//	  for(int i = 0; i < 2; i++){__NOP();}
	  //for(int i = 0; i < 100; i++){__NOP();}

	  TaskHandler();

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void KeyTask(void){
	uint8_t key = KEY_Scan();

	if(key == 1)
	{
		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
	}

	if(key == 2)
	{
		HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
	}

	if(key == 3){
		g_page = 1;
	}
}

void UiTask(void)
{
    static uint8_t last_page = 0xFF;
    static uint32_t rtc_enter_tick = 0;
    static uint32_t last_rtc_tick = 0;

    uint32_t now = HAL_GetTick();

    // 页面切换时只处理一次
    if (last_page != g_page)
    {
        last_page = g_page;

        if (g_page == 0)
        {
            DrawHomePage();
        }
        else if (g_page == 1)
        {
            rtc_enter_tick = now;
            last_rtc_tick = 0;
            OLED_Clear();   // 只清一次，不要在这里刷新
        }
    }

    switch (g_page)
    {
        case 0:
            break;

        case 1:
            // 5秒后自动返回主页
            if (now - rtc_enter_tick >= 5000)
            {
                g_page = 0;
                last_page = 0xFF;   // 让下次进入主页时重新画
                return;             // 这里不要手动画主页
            }

            // 1秒更新一次时间
            if (last_rtc_tick == 0 || now - last_rtc_tick >= 1000)
            {
                last_rtc_tick = now;
                DrawRtcPage();
            }
            break;
    }
}

void TempTask(void)
{
    static uint8_t temp_state = 0;

    if (temp_state == 0)
    {
        DS18B20_StartConvert();
        temp_state = 1;
    }
    else
    {
        g_temp_c = DS18B20_ReadTemp();
        temp_state = 0;
    }
}

void PrintfTask(void)
{
    HAL_UART_Transmit(&huart3, "hello", 5, HAL_MAX_DELAY);
}

void CanTask(void){

	  HAL_CAN_Start(&hcan);
	  CAN_TxHeaderTypeDef txHeader;

	  uint32_t txMailbox;
	  uint8_t txData[8] = {10,20,30,40,50,60,70,80};

	  txHeader.StdId = 0x123;
	  txHeader.IDE = CAN_ID_STD;
	  txHeader.RTR = CAN_RTR_DATA;
	  txHeader.DLC = 8;

	  HAL_CAN_AddTxMessage(&hcan, &txHeader, txData, &txMailbox);//发送数据


	  CAN_RxHeaderTypeDef rxHeader;
	  uint8_t rxData[8];
	  if(HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) > 0)//读取数据
	  {
	  HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &rxHeader, rxData);
	  printf("RX: %d %d %d %d %d %d %d %d\r\n",
	  	      rxData[0], rxData[1], rxData[2], rxData[3],
	  	      rxData[4], rxData[5], rxData[6], rxData[7]);
	  }
}

uint8_t KEY_Scan(void)
{
    static uint8_t last_key = 0;
    uint8_t key = 0;

    if(HAL_GPIO_ReadPin(WK_UP_GPIO_Port, WK_UP_Pin) == GPIO_PIN_SET)
    {
        key = 1;
    }
    else if(HAL_GPIO_ReadPin(KEY_1_GPIO_Port, KEY_1_Pin) == GPIO_PIN_RESET)
    {
        key = 2;
    }
    else if(HAL_GPIO_ReadPin(KEY_2_GPIO_Port, KEY_2_Pin) == GPIO_PIN_RESET)
    {
        key = 3;
    }

    if(key != 0 && last_key == 0)
    {
        last_key = key;
        return key;
    }

    if(key == 0)
    {
        last_key = 0;
    }

    return 0;
}

static void DrawHomePage(void)
{
    OLED_Clear();
    OLED_ShowChinese(46,0,3,16,1);
    OLED_ShowChinese(64,0,4,16,1);
    OLED_ShowString(42,16,"WOYUN",16,1);
    OLED_Refresh();
}

static void DrawRtcPage(void)
{
    char buf[32];
    char buf2[64];
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);


    sprintf(buf,"%04d-%02d-%02d",
            2000 + sDate.Year,
            sDate.Month,
            sDate.Date);
    OLED_ShowString(0, 0, buf, 16, 1);

    sprintf(buf, "%02d:%02d:%02d  %.1fC",
            sTime.Hours,
            sTime.Minutes,
            sTime.Seconds,
            g_temp_c);
    OLED_ShowString(0, 16, buf, 16, 1);


    HAL_UART_Transmit(&huart3, (uint8_t *)buf2, strlen(buf2), 100);

    OLED_Refresh();
}

int __write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
