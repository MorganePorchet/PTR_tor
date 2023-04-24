#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>

#include "main.h"


osMessageQueueId_t queue_storing_id;
uint8_t * storedFrame;

const osMessageQueueAttr_t queue_storing_attr = {
	.name = "MACS_STORING"
};


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

/*
 *
*/
osStatus_t createDataFrame()
{
	struct queueMsg_t queueMsg;
	osStatus_t retCode;
	union mac_control_union controlUnion;
	union mac_status_union statusUnion;
	uint8_t length;
	uint8_t * frame;
	char * qPtr;
	
	// GET the message from private queue
	retCode = osMessageQueueGet(
		queue_storing_id,
		&queueMsg,
		NULL,
		osWaitForever);
	CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);

	qPtr = (char*) queueMsg.anyPtr;

	// control
	controlUnion.controlBf.destAddr = queueMsg.addr;		// dest address
	controlUnion.controlBf.destSAPI = queueMsg.sapi;		// dest sapi
	controlUnion.controlBf.srcSAPI = queueMsg.sapi;			// src sapi
	controlUnion.controlBf.srcAddr = MYADDRESS;					// src address
	controlUnion.controlBf.nothing1 = 0;								// empty bits
	controlUnion.controlBf.nothing2 = 0;
	
	// lenght in bytes
	length = strlen(qPtr);
	
	// status
	statusUnion.status.ack = 0;
	statusUnion.status.read = 0;
	
	// checksum calculation
	uint8_t checksum = 0;
	for (uint8_t i = 0; i < strlen(qPtr); i++)
	{
		checksum = checksum + qPtr[i];
	}
	checksum = checksum%3;
	
	// checksum storing
	statusUnion.status.checksum = checksum;
	
	// memory allocation
	frame = osMemoryPoolAlloc(memPool, osWaitForever);
	frame[0] = controlUnion.controlBytes.source;
	frame[1] = controlUnion.controlBytes.dest;
	frame[2] = length;
	memcpy(&frame[3], qPtr, length);
	frame[3+length] = statusUnion.raw;
	
	// send copy of the frame to PHY_SENDER
	storedFrame = osMemoryPoolAlloc(memPool, osWaitForever)
	memcpy(storedFrame, frame, sizeof(frame)); 
	retCode = osMessageQueuePut(
		queue_phyS_id,
		frame,
		osPriorityNormal,
		osWaitForever);
	
	CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
	
	return retCode;
}

void MacSender(void *argument)
{
	struct queueMsg_t queueMsg;
	osStatus_t retCode;
	
	// create the private message queue
	queue_storing_id = osMessageQueueNew(3, sizeof(struct queueMsg_t), &queue_storing_attr);

	for (;;)
	{
		// read QUEUE
		retCode = osMessageQueueGet(
			queue_macS_id,
			&queueMsg,
			NULL,
			osWaitForever);
		
		CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
		
		switch(queueMsg.type)
		{
			case NEW_TOKEN:
				retCode = createTokenFrame();
				break;
			case TOKEN:
				// TODO: store thoken in var
				// no messages in the queue
				if (osMessageQueueGetCount(queue_storing_id) == 0)
				{
					// update the frame type
					queueMsg.type = TO_PHY;
					
					retCode = osMessageQueuePut(
						queue_phyS_id,
						&queueMsg,
						osPriorityNormal, 
						osWaitForever); 
				}
				else
				{
					// send fist message in the private queue
					retCode = createDataFrame();
				}
				break;
			case DATA_IND:
				
				if (osMessageQueueGetCount(queue_storing_id) != osMessageQueueGetCapacity(queue_storing_id))
				{

					// storing msg in the private queue
					retCode = osMessageQueuePut(
							queue_storing_id,
							&queueMsg,
							osPriorityNormal,
							osWaitForever
					); 
					CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
				}
				
				break;
		}
		
		CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
		
		
		// TODO : memory release
	}
}
