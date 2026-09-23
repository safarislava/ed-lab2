/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

#include <stdio.h>
#include <time.h>

#include "kb.h"
#include "sdk_uart.h"
#include "pca9538.h"
#include "oled.h"
#include "fonts.h"
#include "tim.h"
#include "buzzer.h"

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
enum TimerState_t {
    STATE_INPUT,
    STATE_COUNTDOWN,
    STATE_ALARM,
    STATE_ERROR
};

typedef struct {
    uint8_t digits[4];
    uint8_t nums;
    uint32_t total_sec;
    uint32_t last_tick;
    enum TimerState_t state;
} Timer_t;

static void clearDigits(Timer_t* timer) {
    timer->digits[0] = 0;
    timer->digits[1] = 0;
    timer->digits[2] = 0;
    timer->digits[3] = 0;
    timer->nums = 0;
}

static void clearTimer(Timer_t* timer) {
    clearDigits(timer);
    timer->total_sec = 0;
    timer->last_tick = 0;
    timer->state = STATE_INPUT;
}

static void drawScreen(const char* header, const char* time_str) {
    oled_Fill(Black);
    if (header != NULL) {
        oled_SetCursor(4, 2);
        oled_WriteString((char*)header, Font_7x10, White);
    }
    if (time_str != NULL) {
        oled_SetCursor(36, 22);
        oled_WriteString((char*)time_str, Font_11x18, White);
    }
    oled_UpdateScreen();
}

static void proceedInput(Timer_t* timer) {
    char key = Get_Char();
    if (key >= '0' && key <= '9') {
        if (timer->nums < 4) {
            timer->digits[0] = timer->digits[1];
            timer->digits[1] = timer->digits[2];
            timer->digits[2] = timer->digits[3];
            timer->digits[3] = key - '0';
            timer->nums++;
        }
    } else if (key == '*') {
        clearTimer(timer);
    } else if (key == '#') {
        int minutes = timer->digits[0] * 10 + timer->digits[1];
        int seconds = timer->digits[2] * 10 + timer->digits[3];
        if (minutes > 59 || seconds > 59) {
            clearDigits(timer);
            timer->state = STATE_ERROR;
            timer->last_tick = HAL_GetTick();
            Buzzer_Set_Freq(N_C3);
            Buzzer_Set_Volume(BUZZER_VOLUME_MAX);
            return;
        }
        timer->total_sec = minutes * 60 + seconds;
        timer->last_tick = HAL_GetTick();
        timer->state = STATE_COUNTDOWN;
        return;
    }
    char time_str[10];
    sprintf(time_str, "%d%d:%d%d", timer->digits[0], timer->digits[1], timer->digits[2], timer->digits[3]);
    drawScreen("SET", time_str);
}

static void proceedCountDown(Timer_t* timer) {

}

static void proceedAlarm(Timer_t* timer) {

}

static void proceedError(Timer_t* timer) {
    uint32_t elapsed = HAL_GetTick() - timer->last_tick;
    if (elapsed > 150) {
        Buzzer_Set_Volume(BUZZER_VOLUME_MUTE);
    }
    char key = Get_Char();
    if (key != '\0' || elapsed >= 1500) {
        Buzzer_Set_Volume(BUZZER_VOLUME_MUTE);
        clearTimer(timer);
        return;
    }
    drawScreen("Error", "MIN > 59 || SEC > 59");
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
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
    MX_USART6_UART_Init();
    MX_TIM2_Init();
    /* USER CODE BEGIN 2 */
    oled_Init();
    Buzzer_Init();
    /* USER CODE END 2 */

    Timer_t timer;
    clearTimer(&timer);

    while (1) {
        switch (timer.state) {
            case STATE_INPUT:
                proceedInput(&timer);
                break;
            case STATE_COUNTDOWN:
                proceedCountDown(&timer);
                break;
            case STATE_ALARM:
                proceedAlarm(&timer);
                break;
            case STATE_ERROR:
                proceedError(&timer);
                break;
        }
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */

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
void assert_failed(uint8_t *file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
