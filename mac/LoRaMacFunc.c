/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-07-13     Rick       the first version
 */
#include "LoRaMacFunc.h"
#include "sample.h"
#include "LoRaMacConfig.h"
#include "rtthread.h"

#if( OVER_THE_AIR_ACTIVATION == 0 )
uint8_t FNwkSIntKey[] = LORAWAN_F_NWK_S_INT_KEY;
uint8_t SNwkSIntKey[] = LORAWAN_S_NWK_S_INT_KEY;
uint8_t NwkSEncKey[] = LORAWAN_NWK_S_ENC_KEY;
uint8_t AppSKey[] = LORAWAN_APP_S_KEY;
uint32_t DevAddr = LORAWAN_DEVICE_ADDRESS;
#else
uint8_t AppKey[] = LORAWAN_APP_KEY;
uint8_t NwkKey[] = LORAWAN_NWK_KEY;
uint8_t devEui[] = LORAWAN_DEVICE_EUI;
uint8_t joinEui[] = LORAWAN_JOIN_EUI;
#endif
uint8_t AppPort = LORAWAN_APP_PORT;
uint8_t AppDataSize = 1;
uint8_t AppDataBuffer[LORAWAN_APP_DATA_MAX_SIZE];
uint8_t IsTxConfirmed = LORAWAN_CONFIRMED_MSG_ON;
uint32_t TxDutyCycleTime;
bool NextTx = true;

TimerEvent_t TxNextPacketTimer;

typedef enum
{
    LORAMAC_HANDLER_UNCONFIRMED_MSG = 0,
    LORAMAC_HANDLER_CONFIRMED_MSG = !LORAMAC_HANDLER_UNCONFIRMED_MSG
}LoRaMacHandlerMsgTypes_t;

extern enum eDeviceState
{
    DEVICE_STATE_RESTORE,
    DEVICE_STATE_START,
    DEVICE_STATE_JOIN,
    DEVICE_STATE_SEND,
    DEVICE_STATE_CYCLE,
    DEVICE_STATE_RXC,
    DEVICE_STATE_SLEEP
}DeviceState;

typedef struct LoRaMacHandlerAppData_s
{
    LoRaMacHandlerMsgTypes_t MsgType;
    uint8_t Port;
    uint8_t BufferSize;
    uint8_t *Buffer;
}LoRaMacHandlerAppData_t;

LoRaMacHandlerAppData_t AppData =
{
    .MsgType = LORAMAC_HANDLER_UNCONFIRMED_MSG,
    .Buffer = NULL,
    .BufferSize = 0,
    .Port = 0
};
const char* MacStatusStrings[] =
{
    "OK",                            // LORAMAC_STATUS_OK
    "Busy",                          // LORAMAC_STATUS_BUSY
    "Service unknown",               // LORAMAC_STATUS_SERVICE_UNKNOWN
    "Parameter invalid",             // LORAMAC_STATUS_PARAMETER_INVALID
    "Frequency invalid",             // LORAMAC_STATUS_FREQUENCY_INVALID
    "Datarate invalid",              // LORAMAC_STATUS_DATARATE_INVALID
    "Frequency or datarate invalid", // LORAMAC_STATUS_FREQ_AND_DR_INVALID
    "No network joined",             // LORAMAC_STATUS_NO_NETWORK_JOINED
    "Length error",                  // LORAMAC_STATUS_LENGTH_ERROR
    "Region not supported",          // LORAMAC_STATUS_REGION_NOT_SUPPORTED
    "Skipped APP data",              // LORAMAC_STATUS_SKIPPED_APP_DATA
    "Duty-cycle restricted",         // LORAMAC_STATUS_DUTYCYCLE_RESTRICTED
    "No channel found",              // LORAMAC_STATUS_NO_CHANNEL_FOUND
    "No free channel found",         // LORAMAC_STATUS_NO_FREE_CHANNEL_FOUND
    "Busy beacon reserved time",     // LORAMAC_STATUS_BUSY_BEACON_RESERVED_TIME
    "Busy ping-slot window time",    // LORAMAC_STATUS_BUSY_PING_SLOT_WINDOW_TIME
    "Busy uplink collision",         // LORAMAC_STATUS_BUSY_UPLINK_COLLISION
    "Crypto error",                  // LORAMAC_STATUS_CRYPTO_ERROR
    "FCnt handler error",            // LORAMAC_STATUS_FCNT_HANDLER_ERROR
    "MAC command error",             // LORAMAC_STATUS_MAC_COMMAD_ERROR
    "ClassB error",                  // LORAMAC_STATUS_CLASS_B_ERROR
    "Confirm queue error",           // LORAMAC_STATUS_CONFIRM_QUEUE_ERROR
    "Multicast group undefined",     // LORAMAC_STATUS_MC_GROUP_UNDEFINED
    "Unknown error",                 // LORAMAC_STATUS_ERROR
};

/*!
 * MAC event info status strings.
 */
const char* EventInfoStatusStrings[] =
{
    "OK",                            // LORAMAC_EVENT_INFO_STATUS_OK
    "Error",                         // LORAMAC_EVENT_INFO_STATUS_ERROR
    "Tx timeout",                    // LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT
    "Rx 1 timeout",                  // LORAMAC_EVENT_INFO_STATUS_RX1_TIMEOUT
    "Rx 2 timeout",                  // LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT
    "Rx1 error",                     // LORAMAC_EVENT_INFO_STATUS_RX1_ERROR
    "Rx2 error",                     // LORAMAC_EVENT_INFO_STATUS_RX2_ERROR
    "Join failed",                   // LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL
    "Downlink repeated",             // LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED
    "Tx DR payload size error",      // LORAMAC_EVENT_INFO_STATUS_TX_DR_PAYLOAD_SIZE_ERROR
    "Downlink too many frames loss", // LORAMAC_EVENT_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS
    "Address fail",                  // LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL
    "MIC fail",                      // LORAMAC_EVENT_INFO_STATUS_MIC_FAIL
    "Multicast fail",                // LORAMAC_EVENT_INFO_STATUS_MULTICAST_FAIL
    "Beacon locked",                 // LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED
    "Beacon lost",                   // LORAMAC_EVENT_INFO_STATUS_BEACON_LOST
    "Beacon not found"               // LORAMAC_EVENT_INFO_STATUS_BEACON_NOT_FOUND
};


void PrintHexBuffer( uint8_t *buffer, uint8_t size )
{
    uint8_t newline = 0;

    for( uint8_t i = 0; i < size; i++ )
    {
        if( newline != 0 )
        {
            printf( "\r\n" );
            newline = 0;
        }

        printf( "%02X ", buffer[i] );

        if( ( ( i + 1 ) % 16 ) == 0 )
        {
            newline = 1;
        }
    }
    printf( "\r\n" );
}

void JoinNetwork( void )
{
    LoRaMacStatus_t status;
    MlmeReq_t mlmeReq;
    mlmeReq.Type = MLME_JOIN;
    mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE;

    status = LoRaMacMlmeRequest( &mlmeReq );
    if( status == LORAMAC_STATUS_OK )
    {
        rt_kprintf( "###### ===== JOINING ==== ######\r\n" );
        DeviceState = DEVICE_STATE_SLEEP;
    }
    else
    {
        DeviceState = DEVICE_STATE_CYCLE;
    }
}

void PrepareTxFrame( uint8_t port )
{
        AppDataBuffer[0] = 0x01;
        AppDataSize = 1;
}
void UserPrepareTxFrame( uint8_t port,uint8_t *buffer,uint8_t size )
{
        memset1(AppDataBuffer,0,sizeof(AppDataBuffer));
        memcpy1(AppDataBuffer,buffer,sizeof(AppDataBuffer));
        AppDataSize = size;
}
bool SendFrame( void )//协议栈自用发送函数
{
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;

    if( LoRaMacQueryTxPossible( AppDataSize, &txInfo ) != LORAMAC_STATUS_OK )
    {
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
        mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
    }
    else
    {
        if( IsTxConfirmed == false )
        {
            mcpsReq.Type = MCPS_UNCONFIRMED;
            mcpsReq.Req.Unconfirmed.fPort = AppPort;
            mcpsReq.Req.Unconfirmed.fBuffer = AppDataBuffer;
            mcpsReq.Req.Unconfirmed.fBufferSize = AppDataSize;
            mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
        }
        else
        {
            mcpsReq.Type = MCPS_CONFIRMED;
            mcpsReq.Req.Confirmed.fPort = AppPort;
            mcpsReq.Req.Confirmed.fBuffer = AppDataBuffer;
            mcpsReq.Req.Confirmed.fBufferSize = AppDataSize;
            mcpsReq.Req.Confirmed.NbTrials = 1;
            mcpsReq.Req.Confirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
        }
    }

    AppData.MsgType = ( mcpsReq.Type == MCPS_CONFIRMED ) ? LORAMAC_HANDLER_CONFIRMED_MSG : LORAMAC_HANDLER_UNCONFIRMED_MSG;
    AppData.Port = mcpsReq.Req.Unconfirmed.fPort;
    AppData.Buffer = mcpsReq.Req.Unconfirmed.fBuffer;
    AppData.BufferSize = mcpsReq.Req.Unconfirmed.fBufferSize;

    LoRaMacStatus_t status;
    status = LoRaMacMcpsRequest( &mcpsReq );

    if( status == LORAMAC_STATUS_OK )
    {
        return false;
    }
    return true;
}
bool DataSend( uint8_t *buffer, uint8_t size)//用户用发送函数（Unconfirmed Dataup）
{
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;
    UserPrepareTxFrame(3,buffer,size);
    if( LoRaMacQueryTxPossible( AppDataSize, &txInfo ) != LORAMAC_STATUS_OK )
    {
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
        mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
    }
    else
    {
        if( IsTxConfirmed == false )
        {
            mcpsReq.Type = MCPS_UNCONFIRMED;
            mcpsReq.Req.Unconfirmed.fPort = AppPort;
            mcpsReq.Req.Unconfirmed.fBuffer = AppDataBuffer;
            mcpsReq.Req.Unconfirmed.fBufferSize = AppDataSize;
            mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
        }
        else
        {
            mcpsReq.Type = MCPS_CONFIRMED;
            mcpsReq.Req.Confirmed.fPort = AppPort;
            mcpsReq.Req.Confirmed.fBuffer = AppDataBuffer;
            mcpsReq.Req.Confirmed.fBufferSize = AppDataSize;
            mcpsReq.Req.Confirmed.NbTrials = 3;
            mcpsReq.Req.Confirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
        }
    }

    AppData.MsgType = ( mcpsReq.Type == MCPS_CONFIRMED ) ? LORAMAC_HANDLER_CONFIRMED_MSG : LORAMAC_HANDLER_UNCONFIRMED_MSG;
    AppData.Port = mcpsReq.Req.Unconfirmed.fPort;
    AppData.Buffer = mcpsReq.Req.Unconfirmed.fBuffer;
    AppData.BufferSize = mcpsReq.Req.Unconfirmed.fBufferSize;

    LoRaMacStatus_t status;
    status = LoRaMacMcpsRequest( &mcpsReq );
    if( status == LORAMAC_STATUS_OK )
    {
        return false;
    }
    return true;
}
bool ConfirmedDataSend( uint8_t *buffer, uint8_t size, uint8_t nbtrials)//用户用发送函数（Confirmed Dataup）
{
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;

    UserPrepareTxFrame(10,buffer,size);

    if( LoRaMacQueryTxPossible( AppDataSize, &txInfo ) != LORAMAC_STATUS_OK )
    {
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
        mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
    }
    else
    {
                mcpsReq.Type = MCPS_CONFIRMED;
                mcpsReq.Req.Confirmed.fPort = AppPort;
                mcpsReq.Req.Confirmed.fBuffer = AppDataBuffer;
                mcpsReq.Req.Confirmed.fBufferSize = AppDataSize;
                mcpsReq.Req.Confirmed.NbTrials = nbtrials;
                mcpsReq.Req.Confirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
    }

    AppData.MsgType = ( mcpsReq.Type == MCPS_CONFIRMED ) ? LORAMAC_HANDLER_CONFIRMED_MSG : LORAMAC_HANDLER_UNCONFIRMED_MSG;
    AppData.Port = mcpsReq.Req.Unconfirmed.fPort;
    AppData.Buffer = mcpsReq.Req.Unconfirmed.fBuffer;
    AppData.BufferSize = mcpsReq.Req.Unconfirmed.fBufferSize;

    LoRaMacStatus_t status;
    status = LoRaMacMcpsRequest( &mcpsReq );

    if( status == LORAMAC_STATUS_OK )
    {
        return false;
    }
    return true;
}

void OnTxNextPacketTimerEvent( void )
{
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;

    TimerStop( &TxNextPacketTimer );

    mibReq.Type = MIB_NETWORK_ACTIVATION;
    status = LoRaMacMibGetRequestConfirm( &mibReq );

    if( status == LORAMAC_STATUS_OK )
    {
        if( mibReq.Param.NetworkActivation == ACTIVATION_TYPE_NONE )
        {
            JoinNetwork( );
        }
        else
        {
            DeviceState = DEVICE_STATE_SEND;
            NextTx = true;
        }
    }
}

void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
    printf( "\r\n###### ===== MCPS-Confirm ==== ######\r\n" );
    printf( "STATUS      : %s\r\n", EventInfoStatusStrings[mcpsConfirm->Status] );
    if( mcpsConfirm->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
    }
    else
    {
        switch( mcpsConfirm->McpsRequest )
        {
            case MCPS_UNCONFIRMED:
            {
                break;
            }
            case MCPS_CONFIRMED:
            {
                break;
            }
            case MCPS_PROPRIETARY:
            {
                break;
            }
            default:
                break;
        }
    }
    MibRequestConfirm_t mibGet;
    MibRequestConfirm_t mibReq;

    mibReq.Type = MIB_DEVICE_CLASS;
    LoRaMacMibGetRequestConfirm( &mibReq );

    SendDoneCallback(AppDataBuffer);

    printf( "\r\n###### ===== UPLINK FRAME %lu ==== ######\r\n", mcpsConfirm->UpLinkCounter );
    printf( "\r\n" );

    printf( "CLASS       : %c\r\n", "ABC"[mibReq.Param.Class] );
    printf( "\r\n" );
    printf( "TX PORT     : %d\r\n", AppData.Port );

    if( AppData.BufferSize != 0 )
    {
        printf( "TX TYPE     : " );
        if( AppData.MsgType == LORAMAC_HANDLER_CONFIRMED_MSG )
        {
            printf( "CONFIRMED - %s\r\n", ( mcpsConfirm->AckReceived != 0 ) ? "ACK" : "NACK" );
        }
        else
        {
            printf( "UNCONFIRMED\r\n" );
        }
        printf( "Send Buffer : " );
        PrintHexBuffer( AppData.Buffer, AppData.BufferSize );
    }
    printf( "DATA RATE   : DR_%d\r\n", mcpsConfirm->Datarate );

    mibGet.Type  = MIB_CHANNELS;
    if( LoRaMacMibGetRequestConfirm( &mibGet ) == LORAMAC_STATUS_OK )
    {
        printf( "U/L FREQ    : %lu\r\n", mibGet.Param.ChannelList[mcpsConfirm->Channel].Frequency );
    }

    printf( "TX POWER    : %d\r\n", mcpsConfirm->TxPower );

    mibGet.Type  = MIB_CHANNELS_MASK;
    if( LoRaMacMibGetRequestConfirm( &mibGet ) == LORAMAC_STATUS_OK )
    {
        printf("CHANNEL MASK: ");
#ifdef LORAWAN_DRIVER_REGION_EU433
        for( uint8_t i = 0; i < 1; i++)
#else
        for( uint8_t i = 0; i < 5; i++)
#endif
        {
            printf("%04X ", mibGet.Param.ChannelsMask[i] );
        }
        printf("\r\n");
    }

    printf( "\r\n" );
}

/*!
 * \brief   MCPS-Indication event function
 *
 * \param   [IN] mcpsIndication - Pointer to the indication structure,
 *               containing indication attributes.
 */
void McpsIndication( McpsIndication_t *mcpsIndication )
{
    printf( "\r\n###### ===== MCPS-Indication ==== ######\r\n" );
    printf( "STATUS      : %s\r\n", EventInfoStatusStrings[mcpsIndication->Status] );
    if( mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
        return;
    }

    switch( mcpsIndication->McpsIndication )
    {
        case MCPS_UNCONFIRMED:
        {
            break;
        }
        case MCPS_CONFIRMED:
        {
            break;
        }
        case MCPS_PROPRIETARY:
        {
            break;
        }
        case MCPS_MULTICAST:
        {
            break;
        }
        default:
            break;
    }

    if( mcpsIndication->FramePending == true )
    {
        OnTxNextPacketTimerEvent( );
    }

    if( mcpsIndication->RxData == true )
    {
        ReceiveDoneCallback(mcpsIndication->Buffer);
    }



    const char *slotStrings[] = { "1", "2", "C", "C Multicast", "B Ping-Slot", "B Multicast Ping-Slot" };

    printf( "\r\n###### ===== DOWNLINK FRAME %lu ==== ######\r\n", mcpsIndication->DownLinkCounter );

    printf( "RX WINDOW   : %s\r\n", slotStrings[mcpsIndication->RxSlot] );

    printf( "RX PORT     : %d\r\n", mcpsIndication->Port );

    if( mcpsIndication->BufferSize != 0 )
    {
        printf( "RX DATA     : \r\n" );
        PrintHexBuffer( mcpsIndication->Buffer, mcpsIndication->BufferSize );
    }

    printf( "\r\n" );
    printf( "DATA RATE   : DR_%d\r\n", mcpsIndication->RxDatarate );
    printf( "RX RSSI     : %d\r\n", mcpsIndication->Rssi );
    printf( "RX SNR      : %d\r\n", mcpsIndication->Snr );

    printf( "\r\n" );
}

void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
    printf( "\r\n###### ===== MLME-Confirm ==== ######\r\n" );
    printf( "STATUS      : %s\r\n", EventInfoStatusStrings[mlmeConfirm->Status] );
    if( mlmeConfirm->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
    }
    switch( mlmeConfirm->MlmeRequest )
    {
        case MLME_JOIN:
        {
            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
                MibRequestConfirm_t mibGet;
                printf( "###### ===== JOINED ==== ######\r\n" );
                printf( "\r\nOTAA\r\n\r\n" );
                mibGet.Type = MIB_DEV_ADDR;
                LoRaMacMibGetRequestConfirm( &mibGet );
                printf( "DevAddr     : %08lX\r\n", mibGet.Param.DevAddr );
                mibGet.Type = MIB_CHANNELS_DATARATE;
                LoRaMacMibGetRequestConfirm( &mibGet );
                printf( "DATA RATE   : DR_%d\r\n", mibGet.Param.ChannelsDatarate );
                printf( "\r\nWatting for Link Check\r\n" );

                DeviceState = DEVICE_STATE_SEND;
            }
            else
            {
                JoinNetwork( );
            }
            break;
        }
        case MLME_LINK_CHECK:
        {
            if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
            {
            }
            break;
        }
        default:
            break;
    }
}

/*!
 * \brief   MLME-Indication event function
 *
 * \param   [IN] mlmeIndication - Pointer to the indication structure.
 */
void MlmeIndication( MlmeIndication_t *mlmeIndication )
{
    if( mlmeIndication->Status != LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED )
    {
        printf( "\r\n###### ===== MLME-Indication ==== ######\r\n" );
        printf( "STATUS      : %s\r\n", EventInfoStatusStrings[mlmeIndication->Status] );
    }
    if( mlmeIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK )
    {
    }
    switch( mlmeIndication->MlmeIndication )
    {
        case MLME_SCHEDULE_UPLINK:
        {
            OnTxNextPacketTimerEvent( );
            break;
        }
        default:
            break;
    }
}
void OnMacProcessNotify(void)
{
}
