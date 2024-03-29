/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : flashSlave.h
  * @brief          : Header for flashSlave.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 Envox d.o.o.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASHSLAVE_H
#define __FLASHSLAVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "usart.h"

typedef enum
{
	STAND_BY,
	SEND_START,
	WAIT_FOR_RESPONSE,
	GOT_ACK,
	GOT_NACK,
	RESPONSE_TIMEOUT,
	DATA_READY,
	WAIT_FOR_NEXT_TRANSMIT,
	SEND_GET
} 	flash_status_t;

typedef enum
{
	IDLE,
	RESET_TIME,
	DELAY_BEFORE_NEXT_TRANSMIT,
	SEND_DATA,
	WAIT_FOR_ANSWER,
	ACK_RECEIVED,
	NACK_RECEIVED,
	TIMEOUT,
	DATA_IS_READY,
	DATA_SEND_ERROR
} send_data_status_t;

extern int sendDataStatus;
extern int flashStatus;
extern uint8_t flashSlaveFSM( void );
extern void byteFromSlave( void );
uint8_t sendData(uint8_t *sendData, uint16_t sendSize, uint8_t sendCRC);

#ifdef __cplusplus
}
#endif

#endif /* __FLASHSLAVE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
