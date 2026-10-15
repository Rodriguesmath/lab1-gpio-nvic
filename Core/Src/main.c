/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Programa Principal - Lab GPIO e NVIC STM32
  *                   Formação em Sistemas Embarcados Virtus-CC - Curso 2
  *                   Comunicação Digital Proprietária Ponto-a-Ponto via GPIO
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio_protocol.h"
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
/* Array de teste mínimo exigido no roteiro (100 inteiros de 8 bits) */
static uint8_t tx_test_payload[100];
static uint8_t rx_test_payload[100];

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  /* USER CODE BEGIN 2 */
  /* Inicializa o array de validação de 100 bytes (0x00 a 0x63) */
  for (int i = 0; i < 100; i++) {
    tx_test_payload[i] = (uint8_t)(i & 0xFF);
  }

  /* Inicializa o protocolo customizado com o papel definido */
#if (IS_TRANSMITTER_NODE == 1)
  GPIO_Proto_Init(PROTO_ROLE_TRANSMITTER);
#else
  GPIO_Proto_Init(PROTO_ROLE_RECEIVER);
#endif
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
#if (IS_TRANSMITTER_NODE == 1)
    /* ---------------------------------------------------------------------- */
    /* MÓDULO A: TRANSMISSOR (MASTER)                                         */
    /* Envia o pacote de 100 bytes a cada 1 segundo e mede o tempo            */
    /* ---------------------------------------------------------------------- */
    HAL_Delay(1000);

    /* Dispara a transmissão dos 100 bytes */
    ProtoStatus_t status = GPIO_Proto_SendPacket(tx_test_payload, 100);

    if (status == PROTO_OK) {
      /* Pisca o LED indicando transmissão bem sucedida com ACK */
      HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
      
      const ProtoStats_t *stats = GPIO_Proto_GetStats();
      (void)stats; /* Disponível no Live Watch do depurador para benchmark */
    } else {
      /* Erro de transmissão ou timeout de ACK */
    }

#else
    /* ---------------------------------------------------------------------- */
    /* MÓDULO B: RECEPTOR (SLAVE)                                             */
    /* Aguarda a interrupção externa do NVIC montar o pacote de 100 bytes     */
    /* ---------------------------------------------------------------------- */
    if (GPIO_Proto_IsPacketReady()) {
      uint16_t bytes_received = GPIO_Proto_GetRxPacket(rx_test_payload, 100);

      /* Validação de integridade do payload recebido */
      bool payload_ok = true;
      if (bytes_received == 100) {
        for (int i = 0; i < 100; i++) {
          if (rx_test_payload[i] != tx_test_payload[i]) {
            payload_ok = false;
            break;
          }
        }
      } else {
        payload_ok = false;
      }

      if (payload_ok) {
        /* Pacote íntegro recebido: pisca o LED de sucesso */
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
      }
    }
#endif
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief Callback de Interrupção Externa do NVIC (EXTI)
  * Chamada na borda de subida do pino CLK (PA1) no Módulo Receptor
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == PROTO_CLK_Pin) {
    GPIO_Proto_EXTI_Handler(GPIO_Pin);
  }
}

/**
  * @brief System Clock Configuration (Configuração padrão HSI/HSE)
  */
void SystemClock_Config(void)
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

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);

#elif defined(STM32H723xx)
  /* H723 Clock Initialization: HSI 64MHz default */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

#elif defined(STM32F767xx)
  /* F767 Clock Initialization: HSI 16MHz default */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
#endif
}

/**
  * @brief GPIO Initialization Function
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Enable GPIO Clocks */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
#if defined(STM32F767xx)
  __HAL_RCC_GPIOB_CLK_ENABLE();
#endif

  /* 1. Configuração do LED On-Board */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

#if (IS_TRANSMITTER_NODE == 1)
  /* ======================================================================== */
  /* CONFIGURAÇÃO DO TRANSMISSOR: DATA=Saída, CLK=Saída, ACK=Entrada         */
  /* ======================================================================== */
  HAL_GPIO_WritePin(PROTO_DATA_GPIO_Port, PROTO_DATA_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(PROTO_CLK_GPIO_Port, PROTO_CLK_Pin, GPIO_PIN_SET);

  /* DATA Pin: Output Push-Pull Alta Velocidade */
  GPIO_InitStruct.Pin = PROTO_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(PROTO_DATA_GPIO_Port, &GPIO_InitStruct);

  /* CLK Pin: Output Push-Pull Alta Velocidade */
  GPIO_InitStruct.Pin = PROTO_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(PROTO_CLK_GPIO_Port, &GPIO_InitStruct);

  /* ACK Pin: Input com Pull-Up */
  GPIO_InitStruct.Pin = PROTO_ACK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(PROTO_ACK_GPIO_Port, &GPIO_InitStruct);

#else
  /* ======================================================================== */
  /* CONFIGURAÇÃO DO RECEPTOR: DATA=Entrada, CLK=EXTI Borda Subida, ACK=Saída */
  /* ======================================================================== */
  HAL_GPIO_WritePin(PROTO_ACK_GPIO_Port, PROTO_ACK_Pin, GPIO_PIN_SET);

  /* DATA Pin: Entrada com Pull-Up */
  GPIO_InitStruct.Pin = PROTO_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(PROTO_DATA_GPIO_Port, &GPIO_InitStruct);

  /* ACK Pin: Saída Push-Pull */
  GPIO_InitStruct.Pin = PROTO_ACK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(PROTO_ACK_GPIO_Port, &GPIO_InitStruct);

  /* CLK Pin: Interrupção Externa no NVIC (EXTI) por Borda de Subida */
  GPIO_InitStruct.Pin = PROTO_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(PROTO_CLK_GPIO_Port, &GPIO_InitStruct);

  /* Habilita a Interrupção no NVIC com prioridade alta */
  HAL_NVIC_SetPriority(PROTO_CLK_EXTI_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(PROTO_CLK_EXTI_IRQn);
#endif
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) {
  }
}
