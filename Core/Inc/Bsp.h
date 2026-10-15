/******************************************************************************/
/**
 * @file    Bsp.h
 * @brief   Board Support Package - Camada de Abstracao de Hardware.
 * @author  Rodrigues - Formacao em Sistemas Embarcados Virtus-CC
 * @note    Estilo de codificacao inspirado nas normas do Laboratorio ASSERT (IFPB).
 * @{
 ******************************************************************************/
#ifndef _BSP_H_
#define _BSP_H_

/*******************************************************************************
 * INCLUDES NECESSARIOS
 ******************************************************************************/
#include "AssertTypes.h"

#if defined(STM32F411xE) || defined(STM32F401xC)
  #include "stm32f4xx_hal.h"
#elif defined(STM32H723xx)
  #include "stm32h7xx_hal.h"
#elif defined(STM32F767xx)
  #include "stm32f7xx_hal.h"
#else
  #include "stm32f4xx_hal.h"
#endif

/*******************************************************************************
 * CONFIGURACOES
 ******************************************************************************/
/// Selecao do papel deste no na comunicacao (1 = Transmissor / Master, 0 = Receptor / Slave)
#define dBSP_IS_TRANSMITTER_NODE          1

/*******************************************************************************
 * DEFINES PUBLICOS
 ******************************************************************************/
#define dBSP_PROTO_DATA_PIN               GPIO_PIN_0
#define dBSP_PROTO_DATA_PORT              GPIOA

#define dBSP_PROTO_CLK_PIN                GPIO_PIN_1
#define dBSP_PROTO_CLK_PORT               GPIOA
#define dBSP_PROTO_CLK_EXTI_IRQN          EXTI1_IRQn

#define dBSP_PROTO_ACK_PIN                GPIO_PIN_2
#define dBSP_PROTO_ACK_PORT               GPIOA

#if defined(STM32F411xE)
  #define dBSP_LED_PIN                    GPIO_PIN_13
  #define dBSP_LED_PORT                   GPIOC
#elif defined(STM32F767xx)
  #define dBSP_LED_PIN                    GPIO_PIN_0
  #define dBSP_LED_PORT                   GPIOB
#else
  #define dBSP_LED_PIN                    GPIO_PIN_13
  #define dBSP_LED_PORT                   GPIOC
#endif

/*******************************************************************************
 * PROTOTIPOS PUBLICOS
 ******************************************************************************/
/******************************************************************************/
/**
 * @brief Inicializa todo o hardware de baixo nivel (Clocks, GPIOs, NVIC e DWT).
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_Init(void);

/******************************************************************************/
/**
 * @brief Gera um atraso preciso em microssegundos utilizando o contador DWT.
 * @param us: tempo de atraso em microssegundos.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_DelayUs(u32 us);

/******************************************************************************/
/**
 * @brief Inverte o estado logico do LED indicador de status.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_LedToggle(void);

/******************************************************************************/
/**
 * @brief Escreve nivel logico no pino de Dados (DATA).
 * @param state: nivel booleano (true para HIGH, false para LOW).
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_DataWrite(bool state);

/******************************************************************************/
/**
 * @brief Le o nivel logico atual do pino de Dados (DATA).
 * @param Nenhum.
 * @retval bool: true para HIGH, false para LOW.
 ******************************************************************************/
bool Bsp_DataRead(void);

/******************************************************************************/
/**
 * @brief Escreve nivel logico no pino de Clock (CLK).
 * @param state: nivel booleano.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_ClkWrite(bool state);

/******************************************************************************/
/**
 * @brief Escreve nivel logico no pino de Confirmacao (ACK).
 * @param state: nivel booleano.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_AckWrite(bool state);

/******************************************************************************/
/**
 * @brief Le o nivel logico atual do pino de Confirmacao (ACK).
 * @param Nenhum.
 * @retval bool: true para HIGH, false para LOW.
 ******************************************************************************/
bool Bsp_AckRead(void);

#endif /* _BSP_H_ */
/** @} DOXYGEN GROUP TAG END OF FILE */
