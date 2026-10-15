/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  *                   Virtus-CC - Lab GPIO e NVIC STM32
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#if defined(STM32F411xE) || defined(STM32F401xC)
  #include "stm32f4xx_hal.h"
#elif defined(STM32H723xx)
  #include "stm32h7xx_hal.h"
#elif defined(STM32F767xx)
  #include "stm32f7xx_hal.h"
#else
  #include "stm32f4xx_hal.h"
#endif

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/

/* ========================================================================== */
/* SELEÇÃO DO PAPEL DO NÓ (MÓDULO A / TRANSMISSOR ou MÓDULO B / RECEPTOR)    */
/* Alterne entre 1 e 0 conforme a placa que estiver gravando                  */
/* ========================================================================== */
#define IS_TRANSMITTER_NODE       1  /**< 1 = Transmissor (A) | 0 = Receptor (B) */

/* Pinos do Protocolo Digital Customizado (Compatíveis com F411, H723 e F767) */
#define PROTO_DATA_Pin            GPIO_PIN_0
#define PROTO_DATA_GPIO_Port      GPIOA

#define PROTO_CLK_Pin             GPIO_PIN_1
#define PROTO_CLK_GPIO_Port       GPIOA
#define PROTO_CLK_EXTI_IRQn       EXTI1_IRQn

#define PROTO_ACK_Pin             GPIO_PIN_2
#define PROTO_ACK_GPIO_Port       GPIOA

/* LED de Status On-board */
#if defined(STM32F411xE)
  #define LED_Pin                 GPIO_PIN_13
  #define LED_GPIO_Port           GPIOC
#elif defined(STM32F767xx)
  #define LED_Pin                 GPIO_PIN_0   /* LED Verde Nucleo-144 */
  #define LED_GPIO_Port           GPIOB
#else
  #define LED_Pin                 GPIO_PIN_13
  #define LED_GPIO_Port           GPIOC
#endif

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
