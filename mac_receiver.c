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

osStatus_t send_TO_PHY_frame(void * dataFramePointer)
{
	struct queueMsg_t queueMsg;
	osStatus_t retCode;
	
	// update elements
	queueMsg.type = TO_PHY;
	queueMsg.anyPtr = dataFramePointer;

	// resend frame to PHY_SENDER
	retCode = osMessageQueuePut(
		queue_phyS_id,
		&queueMsg,
		osPriorityNormal,
		osWaitForever);
	
	return retCode;
}

osStatus_t send_DATABACK_frame(void * dataFramePointer)
{
	struct queueMsg_t queueMsg;
	uint8_t * qPtr;
	osStatus_t retCode;
	
	qPtr = dataFramePointer;
	
	// update elements
	queueMsg.type = DATABACK;
	queueMsg.anyPtr = dataFramePointer;
	queueMsg.addr = qPtr[0];
	queueMsg.sapi = qPtr[1];

	// resend frame to MAC_SENDER
	retCode = osMessageQueuePut(
		queue_macS_id,
		&queueMsg,
		osPriorityNormal,
		osWaitForever);
	
	return retCode;
}

osStatus_t send_DATA_IND_frame(void * dataFramePointer)
{
	osStatus_t retCode;
	
	return retCode;
}
void MacReceiver(void *argument)
{
	struct queueMsg_t queueMsg;
	union mac_control_union controlUnion;
	union mac_status_union statusUnion;
	uint8_t * qPtr;
	uint8_t length;
	uint8_t checksum;
	bool sapiState = false;
	
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
			// it's a message
			else
			{
				controlUnion.controlBytes.dest = qPtr[1];	
				controlUnion.controlBytes.source = qPtr[0];
				
				// DESTINATION ADDRESS CHECK
				if (controlUnion.controlBf.destAddr == MYADDRESS)
				{
					length = qPtr[2];
					statusUnion.raw = qPtr[3+length];
					
					// CHECKSUM
					checksum = Checksum(qPtr);
					
					// update the READ and ACK bit depending on the checksum and sapi
					if (checksum == statusUnion.status.checksum)
					{
						statusUnion.status.ack = 1;
						
						// CHECK SAPI
						// 	- the sapi must be either chat or time
						//	- I have to be listening to the sepcific sapi
						if ((controlUnion.controlBf.destSAPI == CHAT_SAPI && gTokenInterface.connected) || 
							(controlUnion.controlBf.destSAPI == TIME_SAPI && gTokenInterface.broadcastTime))
						{
							statusUnion.status.read = 1;
							sapiState = true;
						}
						else
						{
							statusUnion.status.read = 0;
							sapiState = false;
						}
						
						if (sapiState)
						{
							// SOURCE ADDRESS CHECK
							if (controlUnion.controlBf.srcAddr == MYADDRESS)
							{
								// DATA_IND
//								retCode = send_DATA_IND_frame(queueMsg.anyPtr);
//								CheckRetCode(retCode, __LINE__, __FILE__, CONTINUE);
								
								// DATABACK
								retCode = send_DATABACK_frame(queueMsg.anyPtr);
							}
							else
							{
								// TO_PHY
								retCode = send_TO_PHY_frame(queueMsg.anyPtr);
							}
						}
						else
						{
							// DATABACK
							retCode = send_DATABACK_frame(queueMsg.anyPtr);
						}
					}
					else
					{
						statusUnion.status.ack = 0;
						statusUnion.status.read = 0;
						
						// DATABACK	
						retCode = send_DATABACK_frame(queueMsg.anyPtr);
					}
				}
				else
				{
					// SOURCE ADDRESS CHECK
					if (controlUnion.controlBf.srcAddr == MYADDRESS)
					{
						// DATABACK
						retCode = send_DATABACK_frame(queueMsg.anyPtr);
					}
					else
					{
						// send frame to PHY_SENDER
						retCode = send_TO_PHY_frame(queueMsg.anyPtr);
					}
				}
				
				
				
				/*
				// Source Address = MYADDRESS
				if (controlUnion.controlBf.srcAddr == MYADDRESS)
				{
					// Dest Address = MYADDRESS -> have sent message to myself
					if (controlUnion.controlBf.destAddr == MYADDRESS)
					{
						length = qPtr[3];
						statusUnion.raw = qPtr[3+length];
						
						// checksum
						checksum = Checksum(qPtr);
						
						// update the READ and ACK bit depending on the checksum
						if (checksum == statusUnion.status.checksum)
						{
							statusUnion.status.read = 1;
							statusUnion.status.ack = 1;
							
							// DATA_IND -> TODO
						}
						else
						{
							statusUnion.status.read = 1;
							statusUnion.status.ack = 0;							
						}
						
						// send frame to PHY_SENDER
						// change type
						queueMsg.type = TO_PHY;
						// update status
						qPtr[3+length] = statusUnion.raw;
						
						// resend frame to PHY_SENDER
						retCode = osMessageQueuePut(
							queue_phyS_id,
							&queueMsg,
							osPriorityNormal,
							osWaitForever);
					}
					else 
					{
						// send frame to MAC_SENDER
						// change type
						queueMsg.type = DATABACK;

						// resend frame to PHY_SENDER
						retCode = osMessageQueuePut(
							queue_macS_id,
							&queueMsg,
							osPriorityNormal,
							osWaitForever);
					}
				}
				// it's not the message I sent
				else
				{
					// the frame is for me
					if (controlUnion.controlBf.destAddr == MYADDRESS)
					{
						length = qPtr[3];
						statusUnion.raw = qPtr[3+length];
						
						// checksum
						checksum = Checksum(qPtr);
						
						// update the READ and ACK bit depending on the checksum
						if (checksum == statusUnion.status.checksum)
						{
							statusUnion.status.read = 1;
							statusUnion.status.ack = 1;
							
							// DATA_IND -> TODO
						}
						else
						{
							statusUnion.status.read = 1;
							statusUnion.status.ack = 0;							
						}
						
						// send frame to PHY_SENDER
						// change type
						queueMsg.type = TO_PHY;
						// update status
						qPtr[3+length] = statusUnion.raw;
						
						// resend frame to PHY_SENDER
						retCode = osMessageQueuePut(
							queue_phyS_id,
							&queueMsg,
							osPriorityNormal,
							osWaitForever);
					}
					// the message is not for me
					else 
					{
						// send frame to PHY_SENDER
						// change type
						queueMsg.type = TO_PHY;

						// resend frame to PHY_SENDER
						retCode = osMessageQueuePut(
							queue_phyS_id,
							&queueMsg,
							osPriorityNormal,
							osWaitForever);
					}
				}
				*/
			}
		}
	}
}
