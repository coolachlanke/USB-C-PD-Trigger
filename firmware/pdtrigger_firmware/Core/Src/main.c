/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 dConstruct Robotics Pte Ltd.
  * All rights reserved.
  *
  ******************************************************************************/

//////////////////////// Includes ////////////////////////
#include "main.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//////////////////////// Defines ////////////////////////
#define CYPD_I2C_ADDR   (0x08 << 1)  // HAL expects 8-bit addr

#define REG_DATA_MEM_START   0x1800
#define REG_SELECT_SINK_PDO  0x1005
#define REG_PD_RESPONSE      0x1400
#define REG_CURRENT_PDO      0x1010
#define REG_CURRENT_RDO      0x1014
#define REG_BUS_VOLTAGE      0x100D

//////////////////////// Globals ////////////////////////
I2C_HandleTypeDef hi2c3;
UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C3_Init(void);

//////////////////////// Helpers ////////////////////////
static uint16_t swap16(uint16_t x) {
    return (x >> 8) | (x << 8);
}

HAL_StatusTypeDef CYPD_Read(uint16_t reg, uint8_t *buf, uint16_t len) {
    uint16_t reg_swapped = swap16(reg);
    return HAL_I2C_Mem_Read(&hi2c3, CYPD_I2C_ADDR,
                            reg_swapped, I2C_MEMADD_SIZE_16BIT,
                            buf, len, HAL_MAX_DELAY);
}

HAL_StatusTypeDef CYPD_Write(uint16_t reg, uint8_t *buf, uint16_t len) {
    uint16_t reg_swapped = swap16(reg);
    return HAL_I2C_Mem_Write(&hi2c3, CYPD_I2C_ADDR,
                             reg_swapped, I2C_MEMADD_SIZE_16BIT,
                             buf, len, HAL_MAX_DELAY);
}

void uart_printf(const char *msg) {
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

//////////////////////// Test ////////////////////////
// static const uint8_t snkp_minimal[32] = {
//     0x53, 0x4E, 0x4B, 0x50,  // "SNKP"
//     0x00, 0xF0, 0xCB, 0x00,  // PDO1
//     0x00, 0x00, 0x00, 0x00,  // PDO2
//     0x00, 0x00, 0x00, 0x00,  // PDO3
//     0x00, 0x00, 0x00, 0x00,  // PDO4
//     0x00, 0x00, 0x00, 0x00,  // PDO5
//     0x00, 0x00, 0x00, 0x00,  // PDO6
//     0x00, 0x00, 0x00, 0x00   // PDO7
// };

static const uint8_t snkp_minimal[32] = {
    // "SNKP"
    0x53, 0x4E, 0x4B, 0x50,
    // PDO1: 5V @ 3A  => 0x0001912C (little-endian)
    0x2C, 0x91, 0x01, 0x00,
    // PDO2: 9V @ 3A  => 0x0002D12C
    0x2C, 0xD1, 0x02, 0x00,
    // PDO3: 12V @ 3A => 0x0003C12C
    0x2C, 0xC1, 0x03, 0x00,
    // PDO4: 15V @ 3A => 0x0004B12C
    0x2C, 0xB1, 0x04, 0x00,
    // PDO5: 20V @ 3A => 0x0006412C
    0x2C, 0x41, 0x06, 0x00,
    // Padding (PDO6, PDO7 unused)
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};


void test_snkp_minimal(void) {
    char buf[128];
    uint8_t resp[4] = {0};

    uart_printf("=== CYPD3177 SNKP MINIMAL TEST ===\r\n");

    // Step 1: Write minimal SNKP block
    if (CYPD_Write(REG_DATA_MEM_START, (uint8_t*)snkp_minimal, 32) == HAL_OK) {
        uart_printf("SNKP minimal payload written\r\n");
    } else {
        uart_printf("ERROR: SNKP write failed\r\n");
        return;
    }

    HAL_Delay(50);

    // Step 2: Write SELECT_SINK_PDO = 0x01
    uint8_t mask = 0x1F;
    if (CYPD_Write(REG_SELECT_SINK_PDO, &mask, 1) == HAL_OK) {
        uart_printf("SELECT_SINK_PDO = 0x01 written\r\n");
    } else {
        uart_printf("ERROR: SELECT_SINK_PDO write failed\r\n");
        return;
    }

    HAL_Delay(50);

    // Step 3: Read PD_RESPONSE
    if (CYPD_Read(REG_PD_RESPONSE, resp, 4) == HAL_OK) {
        sprintf(buf, "PD_RESPONSE: Code=0x%02X Len=%u\r\n", resp[0], resp[1]);
        uart_printf(buf);
    } else {
        uart_printf("ERROR: PD_RESPONSE read failed\r\n");
    }

    // Step 4: Read CURRENT_PDO
    uint8_t pdo_raw[4] = {0};
    if (CYPD_Read(REG_CURRENT_PDO, pdo_raw, 4) == HAL_OK) {
        uint32_t cur_pdo = (pdo_raw[3]<<24)|(pdo_raw[2]<<16)|(pdo_raw[1]<<8)|pdo_raw[0];
        sprintf(buf, "CURRENT_PDO = 0x%08lX\r\n", (unsigned long)cur_pdo);
        uart_printf(buf);
    }

    // Step 5: Read CURRENT_RDO
    uint8_t rdo_raw[4] = {0};
    if (CYPD_Read(REG_CURRENT_RDO, rdo_raw, 4) == HAL_OK) {
        uint32_t cur_rdo = (rdo_raw[3]<<24)|(rdo_raw[2]<<16)|(rdo_raw[1]<<8)|rdo_raw[0];
        sprintf(buf, "CURRENT_RDO = 0x%08lX\r\n", (unsigned long)cur_rdo);
        uart_printf(buf);
    }

    // Step 6: Read VBUS
    uint8_t vbus = 0;
    if (CYPD_Read(REG_BUS_VOLTAGE, &vbus, 1) == HAL_OK) {
        sprintf(buf, "VBUS = %u mV\r\n", vbus * 100);
        uart_printf(buf);
    }

    uart_printf("=== TEST COMPLETE ===\r\n");
}

//////////////////////// Main ////////////////////////
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_I2C3_Init();

    HAL_Delay(3000);

    test_snkp_minimal();

    while (1) {
        HAL_Delay(1000);
    }
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.ClockSpeed = 100000;
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB0 PB3 PB4 PB5
                           PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
