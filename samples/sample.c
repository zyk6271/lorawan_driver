#include <rtthread.h>
#include <stdio.h>
#include "utilities.h"
#include "LoRaMac.h"
#include "NvmCtxMgmt.h"
#include "string.h"
#include "sample.h"
#include "LoRaMacFunc.h"
#include "LoRaMacConfig.h"
#include <stdlib.h>

#if( OVER_THE_AIR_ACTIVATION == 0 )
extern uint8_t FNwkSIntKey[];
extern uint8_t SNwkSIntKey[];
extern uint8_t NwkSEncKey[];
extern uint8_t AppSKey[];
#else
extern uint8_t AppKey[];
extern uint8_t NwkKey[];
extern uint8_t devEui[];
extern uint8_t joinEui[];
#endif
extern uint32_t DevAddr;
extern uint8_t AppPort;
extern uint8_t AppDataSize;
extern uint8_t AppDataBuffer[LORAWAN_APP_DATA_MAX_SIZE];
extern uint8_t IsTxConfirmed;
extern uint32_t TxDutyCycleTime;
extern bool NextTx;
extern TimerEvent_t TxNextPacketTimer;

bool InitFlag=0;
bool CycleFlag=0;
bool TxNextPacketTimerFlag=0;
uint32_t CycleTime=APP_TX_DUTYCYCLE;

enum eDeviceState
{
    DEVICE_STATE_RESTORE,
    DEVICE_STATE_START,
    DEVICE_STATE_JOIN,
    DEVICE_STATE_SEND,
    DEVICE_STATE_CYCLE,
    DEVICE_STATE_RXC,
    DEVICE_STATE_SLEEP
}DeviceState;

#ifdef BSP_USING_LoRaWAN_Example
void LoRaWANEntry( void *parameter)
{
    LoRaMacPrimitives_t macPrimitives;
    LoRaMacCallback_t macCallbacks;
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;

    macPrimitives.MacMcpsConfirm = McpsConfirm;
    macPrimitives.MacMcpsIndication = McpsIndication;
    macPrimitives.MacMlmeConfirm = MlmeConfirm;
    macPrimitives.MacMlmeIndication = MlmeIndication;
    macCallbacks.MacProcessNotify = OnMacProcessNotify;
    macCallbacks.NvmContextChange = NvmCtxMgmtEvent;
    LoRaMacInitialization( &macPrimitives, &macCallbacks, ACTIVE_REGION );

    while( 1 )
    {

        if( Radio.IrqProcess != NULL )
        {
            Radio.IrqProcess( );
        }

        LoRaMacProcess( );

        switch( DeviceState )
        {
            case DEVICE_STATE_RESTORE:
            {
                mibReq.Type = MIB_NETWORK_ACTIVATION;
                mibReq.Param.NetworkActivation = ACTIVATION_TYPE_OTAA;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_PUBLIC_NETWORK;
                mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_ADR;
                mibReq.Param.AdrEnable = LORAWAN_ADR_ON;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_SYSTEM_MAX_RX_ERROR;
                mibReq.Param.SystemMaxRxError = 20;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_NET_ID;
                mibReq.Param.NetID = LORAWAN_NETWORK_ID;
                LoRaMacMibSetRequestConfirm( &mibReq );

#if( OVER_THE_AIR_ACTIVATION == 0 )
                mibReq.Type = MIB_F_NWK_S_INT_KEY;
                mibReq.Param.FNwkSIntKey = FNwkSIntKey;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_S_NWK_S_INT_KEY;
                mibReq.Param.SNwkSIntKey = SNwkSIntKey;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_NWK_S_ENC_KEY;
                mibReq.Param.NwkSEncKey = NwkSEncKey;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_APP_S_KEY;
                mibReq.Param.AppSKey = AppSKey;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_DEV_ADDR;
                mibReq.Param.DevAddr = DevAddr;
                LoRaMacMibSetRequestConfirm( &mibReq );
#else
                mibReq.Type = MIB_APP_KEY;
                mibReq.Param.AppKey = AppKey;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_NWK_KEY;
                mibReq.Param.NwkKey = NwkKey;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_DEV_EUI;
                mibReq.Param.DevEui = devEui;
                LoRaMacMibSetRequestConfirm( &mibReq );

                mibReq.Type = MIB_JOIN_EUI;
                mibReq.Param.JoinEui = joinEui;
                LoRaMacMibSetRequestConfirm( &mibReq );
#endif

                DeviceState = DEVICE_STATE_START;
                break;
            }

            case DEVICE_STATE_START:
            {
                if(TxNextPacketTimerFlag==0){TxNextPacketTimerFlag=1;TimerInit(&TxNextPacketTimer,OnTxNextPacketTimerEvent);}

                LoRaMacStart();

                mibReq.Type = MIB_NETWORK_ACTIVATION;
                status = LoRaMacMibGetRequestConfirm( &mibReq );

                if( status == LORAMAC_STATUS_OK )
                {
                    if( mibReq.Param.NetworkActivation == ACTIVATION_TYPE_NONE )
                    {
                        DeviceState = DEVICE_STATE_JOIN;
                    }
                    else
                    {
                        DeviceState = DEVICE_STATE_SEND;
                        NextTx = true;
                    }
                }

                DeviceState = DEVICE_STATE_JOIN;

                break;
            }
            case DEVICE_STATE_JOIN:
            {
                mibReq.Type = MIB_DEV_EUI;
#if( OVER_THE_AIR_ACTIVATION == 0 )
                rt_kprintf( "\n\r\n" );
                rt_kprintf( "###### ===== JOINED ==== ######\r\n" );
                rt_kprintf( "Mode        : ABP\r\n\r\n" );
                rt_kprintf( "DevAddr     : %08lX\r\n", DevAddr );
                rt_kprintf( "NwkSKey     : %02X", FNwkSIntKey[0] );
                for( int i = 1; i < 16; i++ )
                {
                    rt_kprintf( " %02X", FNwkSIntKey[i] );
                }
                rt_kprintf( "\r\n" );
                rt_kprintf( "AppSKey     : %02X", AppSKey[0] );
                for( int i = 1; i < 16; i++ )
                {
                rt_kprintf( " %02X", AppSKey[i] );
                }
                rt_kprintf( "\n\r\n" );
                rt_kprintf( "Watting for Link Check\r\n" );

                mibReq.Type = MIB_NETWORK_ACTIVATION;
                mibReq.Param.NetworkActivation = ACTIVATION_TYPE_ABP;
                LoRaMacMibSetRequestConfirm( &mibReq );

                DeviceState = DEVICE_STATE_SEND;
#else
                rt_kprintf( "\n\r\n" );
                rt_kprintf( "Mode        : OTAA\r\n\r\n" );
                LoRaMacMibGetRequestConfirm( &mibReq );
                rt_kprintf( "DevEui      : %02X", mibReq.Param.DevEui[0] );
                for( int i = 1; i < 8; i++ )
                {
                    rt_kprintf( "-%02X", mibReq.Param.DevEui[i] );
                }
                rt_kprintf( "\r\n" );
                mibReq.Type = MIB_JOIN_EUI;
                LoRaMacMibGetRequestConfirm( &mibReq );
                rt_kprintf( "AppEui      : %02X", mibReq.Param.JoinEui[0] );
                for( int i = 1; i < 8; i++ )
                {
                    rt_kprintf( "-%02X", mibReq.Param.JoinEui[i] );
                }
                rt_kprintf( "\r\n" );
                rt_kprintf( "AppKey      : %02X", NwkKey[0] );
                for( int i = 1; i < 16; i++ )
                {
                    rt_kprintf( " %02X", NwkKey[i] );
                }
                rt_kprintf( "\n\r\n" );
                JoinNetwork( );
#endif
           break;
            }
            case DEVICE_STATE_SEND:
            {
                if( NextTx == true )
                {
                    if(DEVICE_CLASS==0x02)
                    {
                        mibReq.Type = MIB_DEVICE_CLASS;
                        LoRaMacMibGetRequestConfirm( &mibReq );
                        if( mibReq.Param.Class!= CLASS_C )
                        {
                            mibReq.Param.Class = CLASS_C;
                            LoRaMacMibSetRequestConfirm( &mibReq );
                        }
                    }
                    PrepareTxFrame( AppPort );
                    NextTx = SendFrame( );
                }
                if(CycleFlag)DeviceState = DEVICE_STATE_CYCLE;
                else DeviceState = DEVICE_STATE_SLEEP;
                break;
            }
            case DEVICE_STATE_CYCLE:
            {
                DeviceState = DEVICE_STATE_SLEEP;
                TimerSetValue( &TxNextPacketTimer,CycleTime );
                TimerStart( &TxNextPacketTimer );
                break;
            }
            case DEVICE_STATE_RXC:
            {
                DeviceState = DEVICE_STATE_SLEEP;
                break;
            }
            case DEVICE_STATE_SLEEP:
            {
                break;
            }
            default:
            {
                DeviceState = DEVICE_STATE_RESTORE;
                break;
            }
          }
        rt_thread_mdelay(50);
    }
}
void SendDoneCallback(uint8_t *buffer)      //发送完成回调函数
{
    printf("\r\nSend Done\r\n");
}
void ReceiveDoneCallback(uint8_t *buffer)   //接收完成回调函数
{
    printf("\r\nReceive Done\r\n");
}

#define CMD_LORAWAN_INIT_INDEX       0
#define CMD_LORAWAN_SEND_INDEX       1
#define CMD_LORAWAN_RESTART_INDEX    2
#define CMD_LORAWAN_CYCLE_INDEX      3

const char* LoRaWAN_Help_Info[] =
{
    [CMD_LORAWAN_INIT_INDEX]          = "LoRaWAN Driver init                    -- lorawan init",
    [CMD_LORAWAN_SEND_INDEX]          = "LoRaWAN DataSend with no ack           -- lorawan send hello,world",
    [CMD_LORAWAN_RESTART_INDEX]       = "LoRaWAN Restart to init                -- lorawan restart",
    [CMD_LORAWAN_CYCLE_INDEX]         = "LoRaWAN Cycle Send                     -- lorawan cycle",
};

/* LoRaWAN Test function */
static int lorawan(int argc, char ** argv)
{
    size_t i = 0;

    if (argc < 2)
    {   // parameter error
        rt_kprintf("\r\n");
        rt_kprintf("Instructions:                          -- Example:\n");
        for (i = 0; i < sizeof(LoRaWAN_Help_Info) / sizeof(char*); i++) {
            rt_kprintf("%s", LoRaWAN_Help_Info[i]);
            rt_kprintf("\r\n");
        }
        rt_kprintf("\r\n");
    }
    else
    {
        const char *cmd = argv[1];

        if (!rt_strcmp(cmd, "init"))
        {
            rt_kprintf("LoRaWAN start init...\n");
            static rt_thread_t tid1 = RT_NULL;
            tid1 = rt_thread_create("LoRaWAN",LoRaWANEntry, RT_NULL,2048,5, 20);
            if (tid1 != RT_NULL)
            {
                InitFlag=1;
                rt_thread_startup(tid1);
                rt_kprintf("\r\nLoRaWAN init success...\r\n");
            }
            else rt_kprintf("\r\nLoRaWAN init fail\r\n");
        }
        else if(!rt_strcmp(cmd, "send"))
        {
            if(InitFlag==1&&CycleFlag==0)
            {
                DataSend((uint8_t *)argv[2],strlen(argv[2]));
                rt_kprintf("\r\nStarting to Send Data...\r\n");}
            else if(InitFlag==1&&CycleFlag==1)
            {
                rt_kprintf("\r\nPlease Run lorawan restart to exit cycle\r\n");
            }
            else rt_kprintf("\r\nPlease Run lorawan init First\r\n");
        }
        else if(!rt_strcmp(cmd, "restart"))
        {
            if(InitFlag)
            {
                CycleFlag=0;
                DeviceState = DEVICE_STATE_RESTORE;
                rt_kprintf("\r\nStarting to Restart...\r\n");
            }
            else rt_kprintf("\r\nPlease Run lorawan init First\r\n");
        }
        else if(!rt_strcmp(cmd, "cycle"))
        {
            if(InitFlag)
            {
                CycleFlag=1;
                CycleTime = atol(argv[2]);
                DeviceState = DEVICE_STATE_CYCLE;
                rt_kprintf("\r\nStarting to Cycle Send with period %d ms...\r\n",CycleTime);
            }
            else
            {
                rt_kprintf("\r\nPlease Run lorawan init First\r\n");
            }
        }
        else rt_kprintf("\r\nInput Wrong,Please Check again!\r\n");
    }
    return 1;
}
MSH_CMD_EXPORT(lorawan, lorawan driver test);
#endif
