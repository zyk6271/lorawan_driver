
/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-06-03     Rick       the first version
 */
#ifndef APPLICATIONS_LORAWAN_RUN_H_
#define APPLICATIONS_LORAWAN_RUN_H_
#include "stdbool.h"

void LoRaTaskInit(void);
void SendDoneCallback(uint8_t *buffer);
void ReceiveDoneCallback(uint8_t *buffer);

#endif /* APPLICATIONS_LORARUN_H_ */
