/* USER CODE BEGIN Header */
/******************************************************************************/
/**
 * @file    main.c
 * @brief   Aplicacao principal do Lab GPIO e NVIC STM32.
 * @author  Rodrigues - Formacao em Sistemas Embarcados Virtus-CC
 * @note    Estilo de codificacao inspirado nas normas do Laboratorio ASSERT (IFPB).
 ******************************************************************************/
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "Bsp.h"
#include "GpioProtocol.h"
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define dTEST_ARRAY_SIZE          100
#define dTX_CYCLE_DELAY_MS        1000

/* Private variables ---------------------------------------------------------*/
#if (dBSP_IS_TRANSMITTER_NODE == 1)
static u8 txTestPayload[dTEST_ARRAY_SIZE];
#else
static u8 rxTestPayload[dTEST_ARRAY_SIZE];
#endif

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* Inicializacao do Hardware de baixo nivel via camada BSP */
    Bsp_Init();

    /* USER CODE BEGIN 2 */
    // Inicializa o array de validacao de 100 inteiros de 8 bits
    for(u16 i = 0; i < dTEST_ARRAY_SIZE; i++)
    {
        txTestPayload[i] = (u8)(i & 0xFF);
    }

#if (dBSP_IS_TRANSMITTER_NODE == 1)
    GpioProtocol_Init(ePROTO_ROLE_TRANSMITTER);
#else
    GpioProtocol_Init(ePROTO_ROLE_RECEIVER);
#endif
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while(1)
    {
#if (dBSP_IS_TRANSMITTER_NODE == 1)
        // ---------------------------------------------------------------------
        // MODULO A: TRANSMISSOR (MASTER)
        // Dispara o pacote de 100 bytes periodicamente
        // ---------------------------------------------------------------------
        HAL_Delay(dTX_CYCLE_DELAY_MS);

        protoStatus_t status = GpioProtocol_SendPacket(txTestPayload, dTEST_ARRAY_SIZE);

        if(status == ePROTO_OK)
        {
            Bsp_LedToggle();
            const protoStats_t *pStats = GpioProtocol_GetStats();
            (void)pStats; // Disponivel para inspecao no Live Watch
        }

#else
        // ---------------------------------------------------------------------
        // MODULO B: RECEPTOR (SLAVE)
        // Processa o pacote recebido via interrupcao EXTI
        // ---------------------------------------------------------------------
        if(GpioProtocol_IsPacketReady() == true)
        {
            u16 bytesReceived = GpioProtocol_GetRxPacket(rxTestPayload, dTEST_ARRAY_SIZE);
            bool isPayloadValid = true;

            if(bytesReceived == dTEST_ARRAY_SIZE)
            {
                for(u16 i = 0; i < dTEST_ARRAY_SIZE; i++)
                {
                    if(rxTestPayload[i] != txTestPayload[i])
                    {
                        isPayloadValid = false;
                        break;
                    }
                }
            }
            else
            {
                isPayloadValid = false;
            }

            if(isPayloadValid == true)
            {
                // Sucesso: pisca o LED indicando entrega correta de 100 bytes
                Bsp_LedToggle();
            }
        }
#endif
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
  * @brief Callback de Interrupcao Externa do NVIC (EXTI)
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == dBSP_PROTO_CLK_PIN)
    {
        GpioProtocol_ExtiHandler((u16)GPIO_Pin);
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while(1)
    {
    }
}
