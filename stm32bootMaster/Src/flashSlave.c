/* USER CODE BEGIN Header */
/**
  ******************************************************************************
    @file    flashSlave.c
    @brief   FSM for flashing slave firmaware
  ******************************************************************************
    @attention

    <h2><center>&copy; Copyright (c) 2019 Envox d.o.o.
    All rights reserved.</center></h2>

    This software component is licensed under BSD 3-Clause license,
    the "License"; You may not use this file except in compliance with the
    License. You may obtain a copy of the License at:
                           opensource.org/licenses/BSD-3-Clause

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

long maxTimeout = 1000;
long timeoutNextMax = 200;
long timeout,timeoutNext;
int flashStatus = STAND_BY;
int sendDataStatus = IDLE;
uint8_t crc;
uint8_t bootloader = 0;
uint8_t bytes;
uint8_t nackCounter;
uint8_t maxNackAllowed = 5;

uint8_t sendData(uint8_t *sendData, uint16_t sendSize, uint8_t sendCRC){
	HAL_UART_Transmit(&huart2, sendData, sendSize, 20);
	if(sendCRC){
		/*
		for(int i=0;i<sendSize;i++){
			crc = CRC_MASK ^ sendData[i];
		}
		*/
		crc = CRC_MASK ^ sendData[0];
		HAL_UART_Transmit(&huart2, &crc, 1, 20);
	}
	return 1;
}

void sendDataAndCRC(uint8_t data) {
  uint8_t sendData[1];
  sendData[0] = data;
  HAL_UART_Transmit(&huart2, sendData, 1, 20);
  sendData[0]  = CRC_MASK ^ data;
  HAL_UART_Transmit(&huart2, sendData, 1, 20);
}

void sendDataNoCRC(uint8_t data) {
  uint8_t sendData[1];
  sendData[0] = data;
  HAL_UART_Transmit(&huart2, sendData, 1, 20);
}

void resetSlave( void ) {
  // not implemented -- I do manual reset
	timeoutNext = HAL_GetTick();
  flashStatus = WAIT_FOR_NEXT_TRANSMIT;
}

uint8_t flashSlaveFSM( ) {
  switch (flashStatus)
  {
    case STAND_BY:
      // statements
      resetSlave();
      flashStatus = SEND_START;
      break;
    case WAIT_FOR_RESPONSE:
			if (bootloader){
				bytes = 4;
			}
			else{
				bytes = 1;
			}
			if(HAL_UART_Receive(&huart2, rxData, bytes, 10)){
        byteFromSlave();
      }
      if (rxReady) {
        if (gotACK) {
          flashStatus = GOT_ACK;
          rxReady = 0;
          gotACK = 0;
          gotNACK = 0;
        }
        else if (gotNACK) {
          flashStatus = GOT_NACK;
          rxReady = 0;
          gotACK = 0;
          gotNACK = 0;
        }
      }
      if (HAL_GetTick() - timeout > maxTimeout) {
				flashStatus = RESPONSE_TIMEOUT;
      }
      break;
    case SEND_START:
      sendDataNoCRC(ENTER_BOOTLOADER);
      timeout = HAL_GetTick();
      flashStatus = WAIT_FOR_RESPONSE;
      break;
    case GOT_ACK:
      flashStatus = DATA_READY;
      break;
    case GOT_NACK:
      flashStatus = DATA_READY;
      break;
    case RESPONSE_TIMEOUT:
			  flashStatus = STAND_BY;
      break;
    case DATA_READY:
      timeout = HAL_GetTick();
			timeoutNext = HAL_GetTick();
      flashStatus = WAIT_FOR_NEXT_TRANSMIT;
      break;
    case WAIT_FOR_NEXT_TRANSMIT:
			if((HAL_GetTick() - timeoutNext) > timeoutNextMax){
				flashStatus = SEND_GET;
			}
		break;
    case SEND_GET:
			bootloader = 1;
      sendDataAndCRC(CMD_GET_VERSION);
      timeout = HAL_GetTick();
		  timeoutNext = HAL_GetTick();
      flashStatus = WAIT_FOR_RESPONSE;
      break;
    default:
      flashStatus = STAND_BY;
  }
  return flashStatus;
}

void byteFromSlave( void ) {
  // Clear Rx_Buffer before receiving new data
  if (rxIndex == 0) {
    memset(rxBuffer, '\0', 200);
  }
  // Clear Rx_Buffer before receiving new data
  if (rxIndex >= 200) {
    memset(rxBuffer, '\0', 200);
    rxIndex = 0;
  }
  rxBuffer[rxIndex] = rxData[0];
  rxIndex++;
  if (rxData[0] == ACK || rxData[0] == NACK) {
    rxReady = 1;
    if (rxData[0] == ACK) {
      gotACK = 1;
      rxIndex = 0;
    }
    if (rxData[0] == NACK) {
      gotNACK = 1;
      rxIndex = 0;
    }
  }
}

uint8_t sendDataToBootloader(uint8_t *transmitData, uint8_t *receiveData, uint16_t sendSize, uint16_t receiveSize, uint8_t sendCRC){
  switch (sendDataStatus)
  {
    case IDLE:
      // Just wait here if data is not needed
			nackCounter = 0;
      break;
		case RESET_TIME:
			// Reset timmer counter so we can wait for next transmit
			timeoutNext = HAL_GetTick();
			sendDataStatus = DELAY_BEFORE_NEXT_TRANSMIT;
			break;
    case DELAY_BEFORE_NEXT_TRANSMIT:
			// Wait untils we can start next transmition
			if((HAL_GetTick() - timeoutNext) > timeoutNextMax){
				sendDataStatus = SEND_DATA;
      }
      break;
    case SEND_DATA:
				// Send data to slave board
				sendData(transmitData, sendSize, sendCRC);
				sendDataStatus = WAIT_FOR_ANSWER;
      break;
    case WAIT_FOR_ANSWER:
      // Wait for the slave board answer
			if(HAL_UART_Receive(&huart2, receiveData, receiveSize, 10)){
        byteFromSlave();
      }
      if (rxReady) {
        if (gotACK) {
          sendDataStatus = ACK_RECEIVED;
        }
        else if (gotNACK) {
          sendDataStatus = NACK_RECEIVED;
        }
				rxReady = 0;
				gotACK = 0;
				gotNACK = 0;
      }
			// Constantly check for global timeout
      if (HAL_GetTick() - timeout > maxTimeout) {
				sendDataStatus = TIMEOUT;
      }
      break;
    case ACK_RECEIVED:
      // We got answer and data is ready
			sendDataStatus = DATA_IS_READY;
      break;
    case NACK_RECEIVED:
      // We got answer but not data 
			nackCounter++;
			if(nackCounter>maxNackAllowed){
				sendDataStatus = IDLE;
				return 0;
			}
			else{
				sendDataStatus = RESET_TIME;
			}
      break;
    case TIMEOUT:
			sendDataStatus = IDLE;			
			return 2;
			// Answer did not arrive timeoutCounter++;
    case DATA_IS_READY:
      // Data is ready for readout
			sendDataStatus = IDLE;
			return 1;
    case DATA_SEND_ERROR:
      // statements
			sendDataStatus = IDLE;		
			return 3;	
		default:
			sendDataStatus = DATA_SEND_ERROR;
	}
	return 0;
}
