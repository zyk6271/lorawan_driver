/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-07-13     Rick       the first version
 */
#include "rtthread.h"
#include "stdbool.h"
#include <stdio.h>
#include "utilities.h"
#include "LoRaMac.h"
#include "NvmCtxMgmt.h"
#include "string.h"

#ifndef RF_APPLICATION_SRC_LORAWAN_FUNC_H_
#define RF_APPLICATION_SRC_LORAWAN_FUNC_H_



bool SendFrame( void );
bool DataSend( uint8_t *buffer, uint8_t size);
bool ConfirmedDataSend( uint8_t *buffer, uint8_t size, uint8_t nbtrials);

void JoinNetwork( void );
void OnMacProcessNotify(void);
void PrepareTxFrame( uint8_t port );
void OnTxNextPacketTimerEvent( void );
void MlmeConfirm( MlmeConfirm_t *mlmeConfirm );
void McpsConfirm( McpsConfirm_t *mcpsConfirm );
void McpsIndication( McpsIndication_t *mcpsIndication );
void MlmeIndication( MlmeIndication_t *mlmeIndication );
void UserPrepareTxFrame( uint8_t port,uint8_t *buffer,uint8_t size );


#endif /* RF_APPLICATION_SRC_LORAWAN_FUNC_H_ */
