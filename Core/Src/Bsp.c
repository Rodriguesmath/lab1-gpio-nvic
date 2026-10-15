/******************************************************************************/
/**
 * @file    Bsp.c
 * @brief   Implementacao da camada BSP de baixo nivel.
 * @author  Rodrigues - Formacao em Sistemas Embarcados Virtus-CC
 * @note    Estilo de codificacao inspirado nas normas do Laboratorio ASSERT (IFPB).
 * @{
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "Bsp.h"

/*******************************************************************************
 * DEFINES LOCAIS (fixos, apenas auxiliar para calculos)
 ******************************************************************************/
#define dBSP_DWT_CYCLES_PER_US            (SystemCoreClock / 1000000UL)

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
/// Variaveis internas do modulo BSP
static struct
{
    bool isInitialized;
} bsp;

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/
static void Bsp_SystemClockConfig(void);
static void Bsp_GpioInit(void);
static void Bsp_DwtInit(void);

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/**
 * @brief Inicializa todo o hardware de baixo nivel (Clocks, GPIOs, NVIC e DWT).
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_Init(void)
{
    HAL_Init();
    Bsp_SystemClockConfig();
    Bsp_GpioInit();
    Bsp_DwtInit();

    bsp.isInitialized = true;
}

/******************************************************************************/
/**
 * @brief Gera um atraso preciso em microssegundos utilizando o contador DWT.
 * @param us: tempo de atraso em microssegundos.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_DelayUs(u32 us)
{
    u32 startCycles = DWT->CYCCNT;
    u32 waitCycles = us * dBSP_DWT_CYCLES_PER_US;

    while((DWT->CYCCNT - startCycles) < waitCycles)
    {
        // Aguarda os ciclos decorrerem
    }
}

/******************************************************************************/
/**
 * @brief Inverte o estado logico do LED indicador de status.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_LedToggle(void)
{
    HAL_GPIO_TogglePin(dBSP_LED_PORT, dBSP_LED_PIN);
}

/******************************************************************************/
/**
 * @brief Escreve nivel logico no pino de Dados (DATA).
 * @param state: nivel booleano (true para HIGH, false para LOW).
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_DataWrite(bool state)
{
    if(state == true)
    {
        HAL_GPIO_WritePin(dBSP_PROTO_DATA_PORT, dBSP_PROTO_DATA_PIN, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(dBSP_PROTO_DATA_PORT, dBSP_PROTO_DATA_PIN, GPIO_PIN_RESET);
    }
}

/******************************************************************************/
/**
 * @brief Le o nivel logico atual do pino de Dados (DATA).
 * @param Nenhum.
 * @retval bool: true para HIGH, false para LOW.
 ******************************************************************************/
bool Bsp_DataRead(void)
{
    bool pinState = false;

    if(HAL_GPIO_ReadPin(dBSP_PROTO_DATA_PORT, dBSP_PROTO_DATA_PIN) == GPIO_PIN_SET)
    {
        pinState = true;
    }

    return pinState;
}

/******************************************************************************/
/**
 * @brief Escreve nivel logico no pino de Clock (CLK).
 * @param state: nivel booleano.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_ClkWrite(bool state)
{
    if(state == true)
    {
        HAL_GPIO_WritePin(dBSP_PROTO_CLK_PORT, dBSP_PROTO_CLK_PIN, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(dBSP_PROTO_CLK_PORT, dBSP_PROTO_CLK_PIN, GPIO_PIN_RESET);
    }
}

/******************************************************************************/
/**
 * @brief Escreve nivel logico no pino de Confirmacao (ACK).
 * @param state: nivel booleano.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_AckWrite(bool state)
{
    if(state == true)
    {
        HAL_GPIO_WritePin(dBSP_PROTO_ACK_PORT, dBSP_PROTO_ACK_PIN, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(dBSP_PROTO_ACK_PORT, dBSP_PROTO_ACK_PIN, GPIO_PIN_RESET);
    }
}

/******************************************************************************/
/**
 * @brief Le o nivel logico atual do pino de Confirmacao (ACK).
 * @param Nenhum.
 * @retval bool: true para HIGH, false para LOW.
 ******************************************************************************/
bool Bsp_AckRead(void)
{
    bool pinState = false;

    if(HAL_GPIO_ReadPin(dBSP_PROTO_ACK_PORT, dBSP_PROTO_ACK_PIN) == GPIO_PIN_SET)
    {
        pinState = true;
    }

    return pinState;
}

/*******************************************************************************
 * FUNCOES LOCAIS
 ******************************************************************************/

/******************************************************************************/
/**
 * @brief Inicializa o contador DWT de ciclos do Cortex-M.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
static void Bsp_DwtInit(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/******************************************************************************/
/**
 * @brief Configura o Clock do sistema para a frequencia nominal da placa.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
static void Bsp_SystemClockConfig(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

#if defined(STM32F411xE)
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 192;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);

#elif defined(STM32H723xx)
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

#elif defined(STM32F767xx)
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
#endif
}

/******************************************************************************/
/**
 * @brief Configura as portas GPIO e interrupcoes NVIC do protocolo.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
static void Bsp_GpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
#if defined(STM32F767xx)
    __HAL_RCC_GPIOB_CLK_ENABLE();
#endif

    /* Configura pino do LED */
    HAL_GPIO_WritePin(dBSP_LED_PORT, dBSP_LED_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = dBSP_LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(dBSP_LED_PORT, &GPIO_InitStruct);

#if (dBSP_IS_TRANSMITTER_NODE == 1)
    /* Transmissor (Master): DATA e CLK como saidas Open-Drain com pull-up, ACK como entrada */
    HAL_GPIO_WritePin(dBSP_PROTO_DATA_PORT, dBSP_PROTO_DATA_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(dBSP_PROTO_CLK_PORT, dBSP_PROTO_CLK_PIN, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = dBSP_PROTO_DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; // Open-Drain: imune a curtos e compativel com Multidrop
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(dBSP_PROTO_DATA_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = dBSP_PROTO_CLK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(dBSP_PROTO_CLK_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = dBSP_PROTO_ACK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(dBSP_PROTO_ACK_PORT, &GPIO_InitStruct);

#else
    /* Receptor (Slave): DATA como entrada, ACK como saida Open-Drain, CLK como EXTI */
    HAL_GPIO_WritePin(dBSP_PROTO_ACK_PORT, dBSP_PROTO_ACK_PIN, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = dBSP_PROTO_DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(dBSP_PROTO_DATA_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = dBSP_PROTO_ACK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; // Open-Drain com pull-up
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(dBSP_PROTO_ACK_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = dBSP_PROTO_CLK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(dBSP_PROTO_CLK_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(dBSP_PROTO_CLK_EXTI_IRQN, 1, 0);
    HAL_NVIC_EnableIRQ(dBSP_PROTO_CLK_EXTI_IRQN);
#endif
}

/** @} DOXYGEN GROUP TAG END OF FILE */
