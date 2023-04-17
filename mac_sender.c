#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>

#include "main.h"

/*
* 	Create the frame for the token
*/
osStatus_t createTokenFrame(struct queueMsg_t queueMsg){
	osStatus_t retCode;
	uint8_t * msg;
	
	uint8_t msgToSend[17];
	
	// fill the message to send
	msgToSend[0] = TOKEN_TAG;
	// State st_i fields
	for (uint8_t i = 1; i < sizeof(msgToSend); i++)
	{
		msgToSend[i] = 0; 
	}
	msgToSend[gTokenInterface.myAddress] =  0xA;		// TODO: replace the harcoded chat application bit
	
	// memory allocation
	msg = osMemoryPoolAlloc(memPool, osWaitForever);
	
	queueMsg.type = TO_PHY;
	queueMsg.anyPtr = msg;
	memcpy(msg, msgToSend, sizeof(msgToSend));
	
	retCode = osMessageQueuePut(
		queue_phyS_id,
		&queueMsg,
		osPriorityNormal,
		osWaitForever);
	
	return retCode;
}

void MacSender(void *argument)
{
	struct queueMsg_t queueMsg;
	uint8_t * qPtr;
	osStatus_t retCode;
	
	for (;;)
	{
		// read QUEUE
		retCode = osMessageQueueGet(
			queue_macS_id,
			&queueMsg,
			NULL,
			osWaitForever);
		
		CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
		
		qPtr = queueMsg.anyPtr;
		
		switch(queueMsg.type)
		{
			case NEW_TOKEN:
				retCode = createTokenFrame(queueMsg);
				break;
		}
	}
}
