/******************************************************************************/
/**
 * @file    GpioProtocol.c
 * @brief   Implementacao do protocolo digital proprietario via GPIO e NVIC.
 * @author  Rodrigues - Formacao em Sistemas Embarcados Virtus-CC
 * @note    Estilo de codificacao inspirado nas normas do Laboratorio ASSERT (IFPB).
 * @{
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "GpioProtocol.h"
#include "Bsp.h"
#include <string.h>

/*******************************************************************************
 * DEFINES LOCAIS (fixos, apenas auxiliar para calculos)
 ******************************************************************************/
#define dNUMBER_OF_DATA_BITS              8
#define dTIMEOUT_STEP_US                  2
#define dRETRY_BACKOFF_US                 20
#define dVALIDATION_PACKET_SIZE           100

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
/// Estados de recepcao da maquina de estados interna do EXTI
typedef enum
{
    eRX_STATE_IDLE = 0,
    eRX_STATE_DATA_BITS,
    eRX_STATE_PARITY_BIT,
    eRX_STATE_STOP_BIT
} rxState_t;

/// Estrutura que encapsula todas as variaveis estaticas do modulo
static struct
{
    protoRole_t   currentRole;
    protoStats_t  stats;
    u8            rxBuffer[dGPIO_PROTOCOL_MAX_PACKET_SIZE];
    volatile u16  rxIndex;
    volatile bool rxPacketReady;
    volatile rxState_t rxState;
    volatile u8   rxCurrentByte;
    volatile u8   rxBitCount;
    volatile u8   rxCalculatedParity;
} gpioProtocol;

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/
static u8 GpioProtocol_CalculateEvenParity(u8 byteVal);

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/**
 * @brief Inicializa as variaveis e a maquina de estados do protocolo.
 * @param role: papel do no (ePROTO_ROLE_TRANSMITTER ou ePROTO_ROLE_RECEIVER).
 * @retval Nenhum.
 ******************************************************************************/
void GpioProtocol_Init(protoRole_t role)
{
    gpioProtocol.currentRole = role;
    GpioProtocol_ResetStats();

    if(role == ePROTO_ROLE_TRANSMITTER)
    {
        Bsp_ClkWrite(true);
        Bsp_DataWrite(true);
    }
    else
    {
        Bsp_AckWrite(true);
        gpioProtocol.rxState = eRX_STATE_IDLE;
        gpioProtocol.rxIndex = 0;
        gpioProtocol.rxPacketReady = false;
    }
}

/******************************************************************************/
/**
 * @brief Transmite uma palavra de 8 bits envelopada com paridade e handshake.
 * @param data: byte de dados a ser enviado.
 * @retval protoStatus_t: ePROTO_OK em caso de sucesso, ou codigo de falha.
 ******************************************************************************/
protoStatus_t GpioProtocol_SendByte(u8 data)
{
    u8 parity = GpioProtocol_CalculateEvenParity(data);
    u32 timeoutCounter = 0;
    bool ackReceived = false;

    // 1. START BIT (Linha de dados levada a nivel LOW)
    Bsp_DataWrite(false);
    Bsp_DelayUs(1);
    Bsp_ClkWrite(false);
    Bsp_DelayUs(dGPIO_PROTOCOL_BIT_TIME_US / 2);
    Bsp_ClkWrite(true); // Borda de subida de sincronismo
    Bsp_DelayUs(dGPIO_PROTOCOL_BIT_TIME_US / 2);

    // 2. DADOS (8 BITS - LSB primeiro)
    for(u8 i = 0; i < dNUMBER_OF_DATA_BITS; i++)
    {
        bool bitValue = ((data >> i) & 0x01) ? true : false;
        Bsp_DataWrite(bitValue);
        Bsp_DelayUs(1);
        Bsp_ClkWrite(false);
        Bsp_DelayUs(dGPIO_PROTOCOL_BIT_TIME_US / 2);
        Bsp_ClkWrite(true); // Borda de subida de amostragem
        Bsp_DelayUs(dGPIO_PROTOCOL_BIT_TIME_US / 2);
    }

    // 3. PARITY BIT
    Bsp_DataWrite((parity == 1) ? true : false);
    Bsp_DelayUs(1);
    Bsp_ClkWrite(false);
    Bsp_DelayUs(dGPIO_PROTOCOL_BIT_TIME_US / 2);
    Bsp_ClkWrite(true);
    Bsp_DelayUs(dGPIO_PROTOCOL_BIT_TIME_US / 2);

    // 4. STOP BIT (Linha de dados retorna para HIGH)
    Bsp_DataWrite(true);
    Bsp_DelayUs(1);
    Bsp_ClkWrite(false);
    Bsp_DelayUs(dGPIO_PROTOCOL_BIT_TIME_US / 2);
    Bsp_ClkWrite(true);
    Bsp_DelayUs(dGPIO_PROTOCOL_BIT_TIME_US / 2);

    // 5. HANDSHAKE (Aguardar pulso de ACK em nivel LOW vindo do receptor)
    while(timeoutCounter < dGPIO_PROTOCOL_TIMEOUT_US)
    {
        if(Bsp_AckRead() == false)
        {
            ackReceived = true;
            break;
        }
        Bsp_DelayUs(dTIMEOUT_STEP_US);
        timeoutCounter += dTIMEOUT_STEP_US;
    }

    if(ackReceived == false)
    {
        gpioProtocol.stats.timeoutErrors++;
        return ePROTO_ERR_TIMEOUT;
    }

    // Aguarda o receptor liberar a linha de ACK para HIGH
    timeoutCounter = 0;
    while((Bsp_AckRead() == false) && (timeoutCounter < 1000))
    {
        Bsp_DelayUs(dTIMEOUT_STEP_US);
        timeoutCounter += dTIMEOUT_STEP_US;
    }

    gpioProtocol.stats.bytesTransferred++;
    return ePROTO_OK;
}

/******************************************************************************/
/**
 * @brief Transmite um bloco/vetor de bytes sequencialmente com retransmissao.
 * @param pData: ponteiro para o vetor de dados.
 * @param length: quantidade de bytes a transmitir.
 * @retval protoStatus_t: status final do envio do pacote.
 ******************************************************************************/
protoStatus_t GpioProtocol_SendPacket(const u8 *pData, u16 length)
{
    if((pData == NULL) || (length == 0))
    {
        return ePROTO_ERR_FRAME;
    }

    u32 startCycles = DWT->CYCCNT;
    protoStatus_t status = ePROTO_OK;

    for(u16 i = 0; i < length; i++)
    {
        u8 retries = 0;
        do
        {
            status = GpioProtocol_SendByte(pData[i]);
            if(status != ePROTO_OK)
            {
                retries++;
                gpioProtocol.stats.retransmissions++;
                Bsp_DelayUs(dRETRY_BACKOFF_US);
            }
        } while((status != ePROTO_OK) && (retries < dGPIO_PROTOCOL_MAX_RETRIES));

        if(status != ePROTO_OK)
        {
            break;
        }
    }

    u32 endCycles = DWT->CYCCNT;
    gpioProtocol.stats.transferTimeUs = (endCycles - startCycles) / (SystemCoreClock / 1000000UL);

    if(status == ePROTO_OK)
    {
        gpioProtocol.stats.packetsSent++;
    }

    return status;
}

/******************************************************************************/
/**
 * @brief Rotina de servico disparada pela interrupcao externa do pino de CLK.
 * @param gpioPin: pino do microcontrolador que acionou o evento EXTI.
 * @retval Nenhum.
 ******************************************************************************/
void GpioProtocol_ExtiHandler(u16 gpioPin)
{
    if(gpioPin != dBSP_PROTO_CLK_PIN)
    {
        return;
    }

    bool bitVal = Bsp_DataRead();

    switch(gpioProtocol.rxState)
    {
        case eRX_STATE_IDLE:
            if(bitVal == false)
            {
                gpioProtocol.rxState = eRX_STATE_DATA_BITS;
                gpioProtocol.rxCurrentByte = 0;
                gpioProtocol.rxBitCount = 0;
            }
            break;

        case eRX_STATE_DATA_BITS:
            if(bitVal == true)
            {
                gpioProtocol.rxCurrentByte |= (1 << gpioProtocol.rxBitCount);
            }
            gpioProtocol.rxBitCount++;
            if(gpioProtocol.rxBitCount >= dNUMBER_OF_DATA_BITS)
            {
                gpioProtocol.rxState = eRX_STATE_PARITY_BIT;
            }
            break;

        case eRX_STATE_PARITY_BIT:
            gpioProtocol.rxCalculatedParity = GpioProtocol_CalculateEvenParity(gpioProtocol.rxCurrentByte);
            if((bitVal ? 1 : 0) == gpioProtocol.rxCalculatedParity)
            {
                gpioProtocol.rxState = eRX_STATE_STOP_BIT;
            }
            else
            {
                gpioProtocol.stats.parityErrors++;
                gpioProtocol.rxState = eRX_STATE_IDLE;
            }
            break;

        case eRX_STATE_STOP_BIT:
            if(bitVal == true)
            {
                if(gpioProtocol.rxIndex < dGPIO_PROTOCOL_MAX_PACKET_SIZE)
                {
                    gpioProtocol.rxBuffer[gpioProtocol.rxIndex] = gpioProtocol.rxCurrentByte;
                    gpioProtocol.rxIndex++;
                    gpioProtocol.stats.bytesTransferred++;
                }

                // Pulso de Handshake ACK (nivel LOW)
                Bsp_AckWrite(false);
                Bsp_DelayUs(dGPIO_PROTOCOL_ACK_PULSE_US);
                Bsp_AckWrite(true);

                if(gpioProtocol.rxIndex >= dVALIDATION_PACKET_SIZE)
                {
                    gpioProtocol.rxPacketReady = true;
                    gpioProtocol.stats.packetsReceived++;
                }
            }
            else
            {
                gpioProtocol.stats.frameErrors++;
            }
            gpioProtocol.rxState = eRX_STATE_IDLE;
            break;

        default:
            gpioProtocol.rxState = eRX_STATE_IDLE;
            break;
    }
}

/******************************************************************************/
/**
 * @brief Indica se um pacote completo de dados esta disponivel para leitura.
 * @param Nenhum.
 * @retval bool: true caso o buffer contenha um pacote integro.
 ******************************************************************************/
bool GpioProtocol_IsPacketReady(void)
{
    return gpioProtocol.rxPacketReady;
}

/******************************************************************************/
/**
 * @brief Copia o pacote recebido para o buffer de destino da aplicacao.
 * @param pDest: ponteiro de destino para onde os dados serao copiados.
 * @param maxLen: tamanho maximo do buffer de destino.
 * @retval u16: quantidade efetiva de bytes copiados.
 ******************************************************************************/
u16 GpioProtocol_GetRxPacket(u8 *pDest, u16 maxLen)
{
    if((gpioProtocol.rxPacketReady == false) || (pDest == NULL))
    {
        return 0;
    }

    u16 count = (gpioProtocol.rxIndex < maxLen) ? gpioProtocol.rxIndex : maxLen;
    memcpy(pDest, gpioProtocol.rxBuffer, count);

    gpioProtocol.rxIndex = 0;
    gpioProtocol.rxPacketReady = false;
    return count;
}

/******************************************************************************/
/**
 * @brief Acessa a estrutura de estatisticas e benchmark do protocolo.
 * @param Nenhum.
 * @retval const protoStats_t*: ponteiro somente-leitura para as estatisticas.
 ******************************************************************************/
const protoStats_t* GpioProtocol_GetStats(void)
{
    return &gpioProtocol.stats;
}

/******************************************************************************/
/**
 * @brief Zera todos os contadores de estatisticas e erros.
 * @param Nenhum.
 * @retval Nenhum.
 ******************************************************************************/
void GpioProtocol_ResetStats(void)
{
    memset(&gpioProtocol.stats, 0, sizeof(protoStats_t));
}

/*******************************************************************************
 * FUNCOES LOCAIS
 ******************************************************************************/

/******************************************************************************/
/**
 * @brief Calcula a paridade par de um byte (Even Parity).
 * @param byteVal: byte a ser avaliado.
 * @retval u8: 1 se a soma de bits 1 for impar (precisa de 1 para ser par), 0 caso contrario.
 ******************************************************************************/
static u8 GpioProtocol_CalculateEvenParity(u8 byteVal)
{
    byteVal ^= (byteVal >> 4);
    byteVal ^= (byteVal >> 2);
    byteVal ^= (byteVal >> 1);
    return (byteVal & 0x01);
}

/** @} DOXYGEN GROUP TAG END OF FILE */
