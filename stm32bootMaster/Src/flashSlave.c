/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    flashSlave.c
  * @brief   FSM for flashing slave firmaware
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 Envox d.o.o.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "flashSlave.h"

uint8_t CMD_GET = 0x00;
uint8_t CMD_GET_VERSION = 0x01;
uint8_t CMD_ID = 0x02;
uint8_t CMD_READ_MEMORY = 0x11;
uint8_t CMD_GO = 0x21;
uint8_t CMD_WRITE_MEMORY = 0x31;
uint8_t CMD_ERASE = 0x43;
uint8_t CMD_EXTENDED_ERASE = 0x44;
uint8_t CMD_WRITE_PROTECT = 0x63;
uint8_t CMD_WRITE_UNPROTECT = 0x73;
uint8_t CMD_READOUT_PROTECT = 0x82;
uint8_t CMD_READOUT_UNPROTECT = 0x92;
uint8_t ENTER_BOOTLOADER = 0x7F;
uint8_t CRC_MASK = 0xFF;


long maxTimeout = 2000;
long timeout;
int flashStatus = STAND_BY;
uint8_t crc; 

void sendDataAndCRC(uint8_t data){
					uint8_t sendData[1];
					sendData[0] = data;
					HAL_UART_Transmit_IT(&huart2, sendData, 1);
					HAL_Delay(1);
				  sendData[0]  = CRC_MASK ^ data;
					HAL_UART_Transmit_IT(&huart2, sendData, 1);
					HAL_Delay(1);
}

void sendDataNoCRC(uint8_t data){
					uint8_t sendData[1];
					sendData[0] = data;
					HAL_UART_Transmit_IT(&huart2, sendData, 1);
					HAL_Delay(1);
}

void resetSlave( void ){
// not implemented -- I do manual reset
	HAL_Delay(3000);
}

uint8_t flashSlaveFSM( ){
    switch (flashStatus)
    {
        case STAND_BY:
          // statements
					resetSlave();
					
					flashStatus = SEND_START;
          break;
        case WAIT_FOR_RESPONSE:
					if(rxReady){
						if(gotACK) {
							flashStatus = GOT_ACK;
							rxReady = 0;
							gotACK = 0;
							gotNACK = 0;
						}
						else if (gotNACK){
							flashStatus = GOT_NACK;
							rxReady = 0;
							gotACK = 0;
							gotNACK = 0;
						}
					}
					if(HAL_GetTick()- timeout > maxTimeout){
						flashStatus = STAND_BY;
					}
          break;
        case SEND_START:
          // statements
					sendDataNoCRC(ENTER_BOOTLOADER);
					timeout = HAL_GetTick();
					flashStatus = WAIT_FOR_RESPONSE;				
          break;
        case GOT_ACK:
					HAL_Delay(100);
					flashStatus = SEND_GET;
          break;
        case GOT_NACK:
					HAL_Delay(100);
					flashStatus = SEND_GET;
          break;
        case RESPONSE_TIMEOUT:
          // statements
          break;
        case SEND_GET:
          // statements
					sendDataAndCRC(CMD_ID);
					timeout = HAL_GetTick();
					flashStatus = WAIT_FOR_RESPONSE;
          break;
				
        default:
					flashStatus = STAND_BY;
          // default statements
    }
		return flashStatus;
}
