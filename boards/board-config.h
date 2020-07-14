#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define BOARD_TCXO_WAKEUP_TIME                      5

/*!
 * Board MCU pins definitions
 */
#define RADIO_RESET                                 17

#define RADIO_MOSI                                  7
#define RADIO_MISO                                  6
#define RADIO_SCLK                                  5
#define RADIO_BUSY                                  18

#define RADIO_NSS                                   4
#define RADIO_DIO_1                                 26
#define RADIO_TCXO_POWER                            NC
#define RADIO_ANT_SWITCH_POWER                      16

#define LED_1                                       NC
#define LED_2                                       NC

#define SWCLK                                       14
#define SWDAT                                       13


#ifdef __cplusplus
}
#endif

#endif // __BOARD_CONFIG_H__

