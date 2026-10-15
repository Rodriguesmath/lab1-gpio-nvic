/**
  ******************************************************************************
  * @file    gpio_protocol.h
  * @brief   Driver do Protocolo Digital Customizado via GPIO e NVIC (EXTI)
  *          Formação em Sistemas Embarcados Virtus-CC - Curso 2: Periféricos
  ******************************************************************************
  */

#ifndef __GPIO_PROTOCOL_H__
#define __GPIO_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* ========================================================================== */
/* CONFIGURAÇÕES GERAIS DO PROTOCOLO                                          */
/* ========================================================================== */

#define PROTOCOL_MAX_PACKET_SIZE   128  /**< Tamanho máximo do buffer de pacote */
#define PROTOCOL_DEFAULT_TIMEOUT_US 2000 /**< Timeout em microssegundos para ACK */

/**
 * @brief Papel do nó na rede (Transmissor/Master ou Receptor/Slave)
 */
typedef enum {
    PROTO_ROLE_TRANSMITTER,
    PROTO_ROLE_RECEIVER
} ProtoRole_t;

/**
 * @brief Status das operações do protocolo
 */
typedef enum {
    PROTO_OK = 0,
    PROTO_ERR_TIMEOUT,
    PROTO_ERR_PARITY,
    PROTO_ERR_FRAME,
    PROTO_ERR_BUSY,
    PROTO_ERR_NACK
} ProtoStatus_t;

/**
 * @brief Estatísticas de transmissão para validação e benchmark
 */
typedef struct {
    uint32_t packets_sent;
    uint32_t packets_received;
    uint32_t bytes_transferred;
    uint32_t parity_errors;
    uint32_t frame_errors;
    uint32_t timeout_errors;
    uint32_t retransmissions;
    uint32_t transfer_time_us;
} ProtoStats_t;

/* ========================================================================== */
/* PROTÓTIPOS DAS FUNÇÕES PÚBLICAS                                            */
/* ========================================================================== */

/**
 * @brief Inicializa o módulo do protocolo
 * @param role Papel deste nó (PROTO_ROLE_TRANSMITTER ou PROTO_ROLE_RECEIVER)
 */
void GPIO_Proto_Init(ProtoRole_t role);

/**
 * @brief Transmite um byte de 8 bits pelo protocolo com paridade e handshake ACK
 * @param data Byte a ser transmitido
 * @return PROTO_OK em caso de sucesso ou código de erro
 */
ProtoStatus_t GPIO_Proto_SendByte(uint8_t data);

/**
 * @brief Transmite um array de bytes (ex: 100 inteiros de 8 bits)
 * @param pData Ponteiro para o buffer de dados
 * @param length Quantidade de bytes a enviar
 * @return PROTO_OK se todos os bytes foram confirmados com ACK
 */
ProtoStatus_t GPIO_Proto_SendPacket(const uint8_t *pData, uint16_t length);

/**
 * @brief Handler chamado dentro de HAL_GPIO_EXTI_Callback quando ocorre borda no CLK
 * @param GPIO_Pin Pino que gerou a interrupção externa
 */
void GPIO_Proto_EXTI_Handler(uint16_t GPIO_Pin);

/**
 * @brief Verifica se um novo pacote de dados foi recebido por completo
 * @return true se há pacote pronto no buffer de recepção
 */
bool GPIO_Proto_IsPacketReady(void);

/**
 * @brief Obtém os dados do pacote recebido e limpa a flag de pronto
 * @param pDest Buffer de destino
 * @param max_len Tamanho máximo do buffer de destino
 * @return Quantidade de bytes copiados
 */
uint16_t GPIO_Proto_GetRxPacket(uint8_t *pDest, uint16_t max_len);

/**
 * @brief Retorna as estatísticas do protocolo para benchmark
 */
const ProtoStats_t* GPIO_Proto_GetStats(void);

/**
 * @brief Reseta as estatísticas de benchmark
 */
void GPIO_Proto_ResetStats(void);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_PROTOCOL_H__ */
