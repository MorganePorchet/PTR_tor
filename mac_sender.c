#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>

#include "main.h"

/*
* 	Create the frame for the token
*/
osStatus_t createTokenFrame(void){
	struct queueMsg_t queueMsg;
	osStatus_t retCode;
	uint8_t * msg;
	
	// fill the message to send
	uint8_t msgToSend[TOKENSIZE-2];		// TOKENSIZE(in PHY layer) - 2 to have the size in MAC layer
	msgToSend[0] = TOKEN_TAG;
	// State st_i fields
	for (uint8_t i = 1; i < sizeof(msgToSend); i++)
	{
		msgToSend[i] = 0; 
	}
	msgToSend[MYADDRESS] =  0xA;		// TODO: replace the harcoded chat application bit
	
	// memory allocation
	msg = osMemoryPoolAlloc(memPool, osWaitForever);
	
	// set the type and data frame pointer
	queueMsg.type = TO_PHY;
	queueMsg.anyPtr = msg;
	// update content of the data frame pointer
	memcpy(msg, msgToSend, sizeof(msgToSend));
	
	// put the token in the queue_phyS_id
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
				retCode = createTokenFrame();
				break;
			case TOKEN:
				if (osMessageQueueGetCount(queue_macS_id) == 0)
				{
					// update the frame type
					queueMsg.type = TO_PHY;
					
					retCode = osMessageQueuePut(
						queue_phyS_id,
						&queueMsg,
						osPriorityNormal, 
						osWaitForever); 
				}
				break;
		}
		
		CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
		
		
		// TODO : memory release
	}
}
