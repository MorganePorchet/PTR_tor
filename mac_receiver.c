#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>

#include "main.h"

osStatus_t sendTokenFrame(void * dataFramePointer)
{
	struct queueMsg_t queueMsg = {0};		// initialized to 0
	osStatus_t retCode;
	
	// set the type and token frame pointer
	queueMsg.type = TOKEN;
	queueMsg.anyPtr = dataFramePointer;
	
	// put the message on the queue_macS
	retCode = osMessageQueuePut(
		queue_macS_id,
		&queueMsg,
		osPriorityNormal, 
		osWaitForever);
	
	return retCode;
}

void MacReceiver(void *argument)
{
	struct queueMsg_t queueMsg;
	uint8_t * qPtr;
	
	osStatus_t retCode;
	
	for (;;)
	{
		
		// read Queue
		retCode = osMessageQueueGet(
			queue_macR_id,
			&queueMsg,
			NULL,
			osWaitForever);
		
		CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
		
		qPtr = queueMsg.anyPtr;
		
		if(queueMsg.type == FROM_PHY)
		{
			// it's a TOKEN 
			if (qPtr[0] == TOKEN_TAG)
			{
				retCode = sendTokenFrame(queueMsg.anyPtr);
				CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
			}
		}
	}
}
