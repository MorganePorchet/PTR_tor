#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>

#include "main.h"


osMessageQueueId_t queue_storing_id;
uint8_t * storedFrame;
uint8_t counterDataBack;
struct queueMsg_t * storedToken;

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
	
	msgToSend[MYADDRESS+1] =  0xA;		// this is the default state when a token is created
																		// chat = 1, time = 1
	
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
osStatus_t createDataFrame(void)
{
	struct queueMsg_t queueMsg;
	struct queueMsg_t msgToSend;
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
	if (controlUnion.controlBf.destAddr == BROADCAST_ADDRESS)
	{
		// broadcast
		statusUnion.status.ack = 1;
		statusUnion.status.read = 1;
	} 
	else 
	{
		statusUnion.status.ack = 0;
		statusUnion.status.read = 0;
	}
	
	// memory allocation
	frame = osMemoryPoolAlloc(memPool, osWaitForever);
	frame[0] = controlUnion.controlBytes.source;
	frame[1] = controlUnion.controlBytes.dest;
	frame[2] = length;
	memcpy(&frame[3], qPtr, length);

	// checksum calculation and storing
	statusUnion.status.checksum = Checksum(frame);
	
	// update the status bits
	frame[3+length] = statusUnion.raw;
	
	// make copy of the frame
	storedFrame = osMemoryPoolAlloc(memPool, osWaitForever);
	memcpy(storedFrame, frame, length+4); 
	
	// fills the message to send
	msgToSend.type = TO_PHY;
	msgToSend.anyPtr = frame;
	
	// send frame to PHY_SENDER
	retCode = osMessageQueuePut(
		queue_phyS_id,
		&msgToSend,
		osPriorityNormal,
		osWaitForever);
		
	CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);	
	
	counterDataBack = 0;
	
	// release memory allocated by CHAT_SENDER
	retCode = osMemoryPoolFree(memPool, queueMsg.anyPtr);
	
	return retCode;
}

osStatus_t send_MAC_ERROR(){
	struct queueMsg_t queueMsg;
	union mac_control_union controlUnion;
	osStatus_t retCode;
	uint8_t length;
	uint8_t * qPtr;
	char * strPtr;
	
	// memory alloc for string
	strPtr = osMemoryPoolAlloc(memPool, osWaitForever);
	if (strPtr != NULL)
	{
		qPtr = storedFrame;
		
		// get length, source byte and string
		length = qPtr[2];
		controlUnion.controlBytes.source = qPtr[0];
		
		// for loop from 3 to the end of the string [3+length]
		for(uint8_t i = 0; i < length; i++){
			strPtr[i] = (char) qPtr[3+i];
		}
	
		strPtr[length] = '\0';  // update it to the right position
		
		// fill the msg to send
		queueMsg.type = MAC_ERROR;
		queueMsg.addr = controlUnion.controlBf.srcAddr;
		queueMsg.anyPtr = strPtr;
		
		// put frame in the lcd queue
		retCode = osMessageQueuePut(
			queue_lcd_id,
			&queueMsg,
			osPriorityNormal,
			osWaitForever);

		CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);

		// free the memory for storedFrame
		retCode = osMemoryPoolFree(memPool, storedFrame);
		
		return retCode;
	}
	else 
	{
		// free the memory for storedFrame
		retCode = osMemoryPoolFree(memPool, storedFrame);

		CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
		
		return NULL;
	}
}

osStatus_t manageDataBack(void * dataFramePointer){
	struct queueMsg_t msgToSend;
	union mac_status_union statusUnion;
	osStatus_t retCode;
	uint8_t length;
	uint8_t * qPtr;
	uint8_t * msg;
	
	qPtr = dataFramePointer;
	
	length = qPtr[2];
	statusUnion.raw = qPtr[3+length];
	
	// check read bit
	if (statusUnion.status.read == 1)
	{
		// resend message to PHY_SENDER
		if(statusUnion.status.ack == 0 && counterDataBack == 0){
			counterDataBack++;
			
			msg = osMemoryPoolAlloc(memPool, osWaitForever);
			
			// fills the message to send
			msgToSend.type = TO_PHY;
			msgToSend.anyPtr = msg;	
			memcpy(msg, storedFrame, sizeof(storedFrame));

			// send frame to PHY_SENDER
			retCode = osMessageQueuePut(
				queue_phyS_id,
				&msgToSend,
				osPriorityNormal,
				osWaitForever);
			
			CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
			
		}
		else 
		{
			counterDataBack = 0;
			
			// send the token
			retCode = osMessageQueuePut(
				queue_phyS_id,
				storedToken,
				osPriorityNormal,
				osWaitForever);
			
			CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
			
			
			storedToken->anyPtr = NULL;
			// free the memory for storedFrame
			retCode = osMemoryPoolFree(memPool, storedFrame);

			CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
		}
		
		// free the memory allocated by PHY_RECEIVER
		retCode = osMemoryPoolFree(memPool, dataFramePointer);
	}
	else
	{
		retCode = send_MAC_ERROR();
	}
	return retCode;
}

osStatus_t send_TOKEN_LIST() 
{
	struct queueMsg_t msgToSend;
	osStatus_t retCode;
	uint8_t * qPtr;
	
	qPtr = storedToken->anyPtr;
	
	// update our state
	if(gTokenInterface.connected)
	{
		qPtr[MYADDRESS+1] = 0xA;
	}
	else
	{
		qPtr[MYADDRESS+1] = 0x4;		
	}

	// update the station list
	for(uint8_t i = 1; i < TOKENSIZE-2; i++)		// TOKENSIZE(in PHY layer) - 2 to have the size in MAC layer
	{
		gTokenInterface.station_list[i-1] = qPtr[i];
	}
	
	// send TOKEN_LIST frame
	msgToSend.type = TOKEN_LIST;
	
	retCode = osMessageQueuePut(
			queue_lcd_id,
			&msgToSend,
			osPriorityNormal,
			osWaitForever);
		
		return retCode;
}
void MacSender(void *argument)
{
	struct queueMsg_t queueMsg;
	osStatus_t retCode;
	
	// create the private message queue
	queue_storing_id = osMessageQueueNew(3, sizeof(struct queueMsg_t), &queue_storing_attr);
	
	// memory allocation for the token
	storedToken = osMemoryPoolAlloc(memPool, osWaitForever);
	
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
				// store the token frame header
				memcpy(storedToken, &queueMsg, sizeof(queueMsg));
			
				// update TOKEN_LIST on the LCD
				retCode = send_TOKEN_LIST();
				CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
			
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
				
				if (osMessageQueueGetCount(queue_storing_id) < osMessageQueueGetCapacity(queue_storing_id))
				{

					// storing msg in the private queue
					retCode = osMessageQueuePut(
							queue_storing_id,
							&queueMsg,
							osPriorityNormal,
							osWaitForever
					); 
				}
				break;
				
			case DATABACK:
				retCode = manageDataBack(queueMsg.anyPtr);
				break;
			
			case START:
				gTokenInterface.connected = true;
				// update TOKEN and send TOKEN_LIST
				retCode = send_TOKEN_LIST();
				break;
			
			case STOP:
				gTokenInterface.connected = false;
				// update TOKEN and send TOKEN_LIST
				retCode = send_TOKEN_LIST();
				break;			
		}
		
		CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
		
	}
}
