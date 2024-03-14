/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fonts.h"
#include "ssd1306.h"
#include "test.h"
#include "string.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUFFER_SIZE 256
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t temp;
typedef struct{
	uint8_t data[100];
	int length;
	uint8_t isComplete;
	uint8_t dataError;
	char utc_time[20];
	uint8_t validFlag;
	char latitude[20];
	char n_s;
	char longitude[20];
	char e_w;
	uint8_t isParseData;
}DataFrame_t;

DataFrame_t reDataFrame;

uint8_t uart_rx_buffer[BUFFER_SIZE];
volatile uint16_t uart_rx_index = 0;
volatile uint8_t uart_rx_flag = 0;

float latitude = 0;
float longitude = 0;
float time = 0;
uint8_t hour, minute;
char display[50];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void receData_Parse(void);
void data_display(void);
void floatToString(float num, char *str) ;

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	 __HAL_UART_ENABLE_IT(&huart2,UART_IT_RXNE);
	HAL_UART_Receive_IT(&huart2, &temp, 1);
	 HAL_TIM_Base_Start(&htim1);
	SSD1306_Init();
	SSD1306_Clear();

		
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
		SSD1306_DrawRectangle(0,0,128,15,1);
		SSD1306_DrawRectangle(0,16,128,48,1);
		SSD1306_GotoXY(50,5);
		SSD1306_Puts("GPS",&Font_7x10,1);
		receData_Parse();
		if(reDataFrame.dataError == 1&& reDataFrame.isParseData == 0)
		{
			SSD1306_GotoXY(2,27);
			SSD1306_Puts("ERROR",&Font_7x10,1);
		}
		else 
		{
			data_display();
			floatToString(latitude, display);
			SSD1306_GotoXY(2,25);
			SSD1306_Puts(display,&Font_7x10,1);
			floatToString(longitude, display);
			SSD1306_GotoXY(2,36);
			SSD1306_Puts(display,&Font_7x10,1);
			SSD1306_GotoXY(2,47);
			sprintf(display, "Time %d : %02d", hour, minute);
			SSD1306_Puts(display,&Font_7x10,1);
		}
		SSD1306_UpdateScreen(); // update screen
		delay_ms(800);
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	if(huart->Instance == USART2)
		{
			temp = USART2->RDR;
			if(temp == '$')
			{
				uart_rx_index = 0;
			}
			uart_rx_buffer[uart_rx_index ++] = temp;
			if(uart_rx_buffer[0] == '$' && uart_rx_buffer[4] == 'M' && uart_rx_buffer[5] == 'C')
			{
				if(temp == '\n')
				{
          memset(reDataFrame.data, 0, sizeof(reDataFrame.data));
          memcpy(reDataFrame.data, uart_rx_buffer, uart_rx_index);
					reDataFrame.isComplete = 1;
					uart_rx_index = 0;
					memset(uart_rx_buffer, 0, BUFFER_SIZE);
				}
			 }
			if(uart_rx_index >= BUFFER_SIZE)
			{
				uart_rx_index = BUFFER_SIZE;
			}
			HAL_UART_Receive_IT(huart, &temp, 1);
		}
		
}
void receData_Parse(void) {
    char *point;
    char *nextpoint;
    uint8_t tempVar = 0;

    if (reDataFrame.isComplete) {
        reDataFrame.isComplete = 0;
        reDataFrame.dataError = 0;
        reDataFrame.isParseData = 0; 			
        point = (char *)reDataFrame.data;

        for (tempVar = 0; tempVar < 6; tempVar++) {
            nextpoint = strstr(point, ",");
            if (nextpoint == NULL) {
                reDataFrame.dataError = 1;
                break;
            }

            nextpoint++; 

            switch (tempVar) {
                case 0:
                    
                    break;
                case 1:
                    memcpy(reDataFrame.utc_time, point, nextpoint - point - 1);
                    break;
                case 2:
                    reDataFrame.validFlag = *point;
                    break;
                case 3:
                    memcpy(reDataFrame.latitude, point, nextpoint - point - 1);
                    break;
                case 4:
                    reDataFrame.n_s = *point;
                    break;
                case 5:
                    memcpy(reDataFrame.longitude, point, nextpoint - point - 1);
                    break;
                case 6:
                    reDataFrame.e_w = *point;
                    break;
            }

            point = nextpoint;
        }

        if (reDataFrame.validFlag == 'A') {
            reDataFrame.dataError = 0;
        } else if (reDataFrame.validFlag == 'V') {
            reDataFrame.dataError = 1;
        }

        reDataFrame.isParseData = 1; 
    }
}
void data_display(void)
{
	uint16_t temp1 = 0;
	uint16_t temp2 = 0;
	uint16_t temp3 =0;
	latitude = strtod(reDataFrame.latitude, NULL);
	longitude = strtod(reDataFrame.longitude, NULL);
	time = strtod(reDataFrame.utc_time, NULL);
	hour = (uint8_t)((((uint16_t)time / 10000)+7) % 24) ; 
	minute = (uint8_t)(((uint16_t)time % (10000))/100);
	
	if((latitude - 10000.0)>=0)
	{
		temp1 = (((uint16_t)latitude / 10000) % 10) * 100 + (((uint16_t)latitude / 1000) % 10) * 10 + ((uint16_t)latitude / 100) % 10;
		latitude = latitude - (float)temp1 * 100;
		latitude = (float)temp1 + latitude / 60;
	}
	else
	{
		temp1 = (((uint16_t)latitude / 1000) % 10) * 10 +((uint16_t)latitude / 100) %10;
		latitude = latitude -(float)temp1 * 100;
		latitude = (float)temp1 + latitude/60;
	}
	
	if((longitude - 10000.0)>=0)
	{
		temp1 = (((uint16_t)longitude / 10000) % 10) * 100 + (((uint16_t)longitude / 1000) % 10) * 10 + ((uint16_t)longitude / 100) % 10;
		longitude = longitude - (float)temp1 * 100;
		longitude = (float)temp1 + longitude / 60;
	}
	else
	{
		temp1 = (((uint16_t)longitude / 1000) % 10) * 10 +((uint16_t)longitude / 100) %10;
		longitude = longitude -(float)temp1 * 100;
		longitude = (float)temp1 + longitude/60;
	}
	
}
void floatToString(float num, char *str) 
{
    sprintf(str, "%.4f", num);
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
