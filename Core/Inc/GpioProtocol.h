/******************************************************************************/
/**
 * @file    GpioProtocol.h
 * @brief   Protocolo Digital Customizado via GPIO e NVIC (EXTI).
 * @author  Rodrigues - Formacao em Sistemas Embarcados Virtus-CC
 * @note    Estilo de codificacao inspirado nas normas do Laboratorio ASSERT (IFPB).
 * @{
 ******************************************************************************/
#ifndef _GPIO_PROTOCOL_H_
#define _GPIO_PROTOCOL_H_

/*******************************************************************************
 * INCLUDES NECESSARIOS
 ******************************************************************************/
#include "AssertTypes.h"

/*******************************************************************************
 * CONFIGURACOES
 ******************************************************************************/
/// Tamanho maximo do pacote de dados em bytes
#define dGPIO_PROTOCOL_MAX_PACKET_SIZE    128
// [bytes]

/// Timeout em microssegundos para recepcao do handshake ACK
#define dGPIO_PROTOCOL_TIMEOUT_US         2000
// [us]

/// Tempo de bit em microssegundos (~200 kbps de taxa bruta)
#define dGPIO_PROTOCOL_BIT_TIME_US        5
// [us]

/// Duracao do pulso de ACK gerado pelo receptor
#define dGPIO_PROTOCOL_ACK_PULSE_US       4
// [us]

/// Numero maximo de retransmissoes por byte em caso de erro
#define dGPIO_PROTOCOL_MAX_RETRIES        3

/*******************************************************************************
 * DEFINES PUBLICOS
 ******************************************************************************/

/*******************************************************************************
 * TIPOS DE DADOS PUBLICOS
 ******************************************************************************/
/// Papel do dispositivo no barramento digital
typedef enum
{
    ePROTO_ROLE_TRANSMITTER,
    ePROTO_ROLE_RECEIVER
} protoRole_t;

/// Codigos de status e retorno das operacoes do protocolo
typedef enum
{
    ePROTO_OK = 0,
    ePROTO_ERR_TIMEOUT,
    ePROTO_ERR_PARITY,
    ePROTO_ERR_FRAME,
    ePROTO_ERR_BUSY,
    ePROTO_ERR_NACK
} protoStatus_t;

/// Metricas e estatisticas de comunicacao para analise de desempenho
typedef struct
{
    u32 packetsSent;
    u32 packetsReceived;
    u32 bytesTransferred;
    u32 parityErrors;
    u32 frameErrors;
    u32 timeoutErrors;
    u32 retransmissions;
    u32 transferTimeUs;
} protoStats_t;

/*******************************************************************************
 * PROTOTIPOS PUBLICOS
 ******************************************************************************/

/******************************************************************************/
/**
 * @brief Inicializa as variaveis e a maquina de estados do protocolo.
 * @param role: papel do no (ePROTO_ROLE_TRANSMITTER ou ePROTO_ROLE_RECEIVER).
 * @retval Nenhum.
 ******************************************************************************/
void GpioProtocol_Init(protoRole_t role);

/******************************************************************************/
/**
 * @brief Transmite uma palavra de 8 bits envelopada com paridade e handshake.
 * @param data: byte de dados a ser enviado.
 * @retval protoStatus_t: ePROTO_OK em caso de sucesso, ou codigo de falha.
 ******************************************************************************/
protoStatus_t GpioProtocol_SendByte(u8 data);

/******************************************************************************/
/**
 * @brief Transmite um bloco/vetor de bytes sequencialmente com retransmissao.
 * @param pData: ponteiro para o vetor de dados.
 * @param length: quantidade de bytes a transmitir.
 * @retval protoStatus_t: status final do envio do pacote.
 ******************************************************************************/
protoStatus_t GpioProtocol_SendPacket(const u8 *pData, u16 length);

/******************************************************************************/
/**
 * @brief Rotina de servico disparada pela interrupcao externa do pino de CLK.
 * @param gpioPin: pino do microcontrolador que acionou o evento EXTI.
 * @retval Nenhum.
 ******************************************************************************/
void GpioProtocol_ExtiHandler(u16 gpioPin);

/******************************************************************************/
/**
 * @brief Indica se um pacote completo de dados esta disponivel para leitura.
 * @param Nenhum.
 * @retval bool: true caso o buffer contenha um pacote integro.
 ******************************************************************************/
bool GpioProtocol_IsPacketReady(void);

/******************************************************************************/
/**
 * @brief Copia o pacote recebido para o buffer de destino da aplicacao.
 * @param pDest: ponteiro de destino para onde os dados serao copiados.
 * @param maxLen: tamanho maximo do buffer de destino.
 * @retval u16: quantidade efetiva de bytes copiados.
 ******************************************************************************/
u16 GpioProtocol_GetRxPacket(u8 *pDest, u16 maxLen);

/******************************************************************************/
/**
 * @brief Acessa a estrutura de estatisticas e benchmark do protocolo.
 * @param Nenhum.
 * @retval const protoStats_t*: ponteiro somente-leitura para as estatisticas.
 ******************************************************************************/
const protoStats_t* GpioProtocol_GetStats(void);

/******************************************************************************/
/**
 * @brief Zera todos os contadores de estatisticas e erros.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void GpioProtocol_ResetStats(void);

#endif /* _GPIO_PROTOCOL_H_ */
/** @} DOXYGEN GROUP TAG END OF FILE */
