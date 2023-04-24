//////////////////////////////////////////////////////////////////////////////////
/// \file chat_sender.c
/// \brief Chat sender thread
/// \author Pascal Sartoretti (sap at hevs dot ch)
/// \version 1.0 - original
/// \date  2018-02
//////////////////////////////////////////////////////////////////////////////////
#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "main.h"
#include "ext_keyboard.h"

//////////////////////////////////////////////////////////////////////////////////
/// \brief Called on interrupt keyboard received char
/// \param The GPIO pin caused interrupt (GPIO_PIN_8)
//////////////////////////////////////////////////////////////////////////////////
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	struct queueMsg_t queueMsg;					// queue message
	osStatus_t retCode;
	 
	if(GPIO_Pin == GPIO_PIN_8)
	{
		if(ext_kbChar != 0)
		{
			queueMsg.addr = ext_kbChar;
			//----------------------------------------------------------------------------
			// QUEUE SEND								
			//----------------------------------------------------------------------------
			retCode = osMessageQueuePut(
				queue_keyboard_id,
				&queueMsg,
				osPriorityNormal,
				0);
			CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE);	
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////
// THREAD CHAT SENDER
//////////////////////////////////////////////////////////////////////////////////
void ChatSender(void *argument)
{
	struct queueMsg_t queueMsg;					// queue message
	char * msg;													// any string pointer
	char msgToSend[255];								// keep message to send
	uint8_t msgToSendPtr=0;							// counter of received bytes
	osStatus_t retCode;									// return error code
	//------------------------------------------------------------------------------
	// Initialise the keyboard
	//------------------------------------------------------------------------------
	Ext_Keyboard_Init();

	//------------------------------------------------------------------------------
	for (;;)														// loop until doomsday
	{
		//----------------------------------------------------------------------------
		// QUEUE READ										
		//----------------------------------------------------------------------------
		retCode = osMessageQueueGet( 	
			queue_keyboard_id,
			&queueMsg,
			NULL,
			osWaitForever); 	
    CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE);		
		if((gTokenInterface.connected != FALSE)
				&&(gTokenInterface.currentView == MAINDISPLAY))
		{
			// message to send ---------------------------------------------------------
			if(queueMsg.addr == 0x0D)			// a <CR> has been received
			{
				// echo LCD
				//------------------------------------------------------------------------
				// MEMORY ALLOCATION				
				//------------------------------------------------------------------------
				msg = osMemoryPoolAlloc(memPool,osWaitForever);				
				queueMsg.type = CHAR_MSG;
				queueMsg.anyPtr = msg;
				msg[0] = 0x0D;				// send a CR
				msg[1] = 0x0A;				// and a LF
				msg[2] = 0;
				//------------------------------------------------------------------------
				// QUEUE SEND								
				//------------------------------------------------------------------------
				retCode = osMessageQueuePut(
					queue_lcd_id,
					&queueMsg,
					osPriorityNormal,
					osWaitForever);
				CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE);	

				// prepare message to send
				msgToSend[msgToSendPtr] = 0;	// end of C string
				msgToSendPtr = 0;
				//------------------------------------------------------------------------
				// MEMORY ALLOCATION				
				//------------------------------------------------------------------------
				msg = osMemoryPoolAlloc(memPool,osWaitForever);				
				queueMsg.addr = gTokenInterface.destinationAddress;
				queueMsg.sapi = CHAT_SAPI;
				queueMsg.type = DATA_IND;
				queueMsg.anyPtr = msg;
				memcpy(msg,msgToSend,strlen(msgToSend)+1);
				//------------------------------------------------------------------------
				// QUEUE SEND								
				//------------------------------------------------------------------------
				retCode = osMessageQueuePut(
					queue_macS_id,
					&queueMsg,
					osPriorityNormal,
					osWaitForever);
				CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE);					
			}
			// just display char ------------------------------------------------------
			else if (msgToSendPtr < 250)	// message size limit
			{
				//------------------------------------------------------------------------
				// MEMORY ALLOCATION				
				//------------------------------------------------------------------------
				msg = osMemoryPoolAlloc(memPool,osWaitForever);				
				queueMsg.type = CHAR_MSG;
				queueMsg.anyPtr = msg;
				msgToSend[msgToSendPtr] = queueMsg.addr;
				msgToSendPtr++;
				msg[0] = queueMsg.addr;
				msg[1] = 0;
				//------------------------------------------------------------------------
				// QUEUE SEND								
				//------------------------------------------------------------------------
				retCode = osMessageQueuePut(
					queue_lcd_id,
					&queueMsg,
					osPriorityNormal,
					osWaitForever);
				CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE);	
			}
		}
	}		
}


