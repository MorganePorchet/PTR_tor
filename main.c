//////////////////////////////////////////////////////////////////////////////////
/// \file main.c
/// \brief Tokenring intialisation
/// \author Pascal Sartoretti (pascal dot sartoretti at hevs dot ch)
/// \version 1.0 - original for RTX5
/// \date  2018-02
//////////////////////////////////////////////////////////////////////////////////
#include "stm32f7xx_hal.h"
#include <stdio.h>
#include "main.h"									// tokenring constants							

#include "ext_led.h"							// use external leds
#include "ext_uart.h"							// use external uart (for tokenring)
#include "EventRecorder.h"				// use event recorder

//--------------------------------------------------------------------------------
// Global variable for system control
//--------------------------------------------------------------------------------
struct TOKENINTERFACE gTokenInterface;
//--------------------------------------------------------------------------------
// Memory pool
//--------------------------------------------------------------------------------
osMemoryPoolId_t memPool;
//--------------------------------------------------------------------------------
// Event flag
//--------------------------------------------------------------------------------
osEventFlagsId_t eventFlag_id;
//--------------------------------------------------------------------------------
// Queues id and attributes
//--------------------------------------------------------------------------------
osMessageQueueId_t queue_macR_id;
osMessageQueueId_t queue_phyS_id;
osMessageQueueId_t queue_dbg_id;
osMessageQueueId_t queue_macS_id;
osMessageQueueId_t queue_chatR_id;
osMessageQueueId_t queue_chatS_id;
osMessageQueueId_t queue_timeR_id;
osMessageQueueId_t queue_timeS_id;
osMessageQueueId_t queue_lcd_id;
osMessageQueueId_t queue_keyboard_id;
osMessageQueueId_t queue_usartR_id;

const osMessageQueueAttr_t queue_macR_attr = {
	.name = "MAC_RECEIVER"  	
};
const osMessageQueueAttr_t queue_macS_attr = {
	.name = "MAC_SENDER  "  	
};
const osMessageQueueAttr_t queue_phyS_attr = {
	.name = "PH_SENDER   "  	
};
const osMessageQueueAttr_t queue_dbg_attr = {
	.name = "DEBUGGER    "  	
};
const osMessageQueueAttr_t queue_chatR_attr = {
	.name = "CHAT_REC    "  	
};
const osMessageQueueAttr_t queue_timeR_attr = {
	.name = "TIME_RECEIVE"  	
};
const osMessageQueueAttr_t queue_timeS_attr = {
	.name = "TIME_SENDER "  	
};
const osMessageQueueAttr_t queue_chatS_attr = {
	.name = "CHAT_SENDER "  	
};
const osMessageQueueAttr_t queue_lcd_attr = {
	.name = "LCD MANAGER "  	
};
const osMessageQueueAttr_t queue_keyboard_attr = {
	.name = "KEYBOARD    "  	
};
const osMessageQueueAttr_t queue_usartR_attr = {
	.name = "USART_RECEIV"  	
};
//--------------------------------------------------------------------------------
// External threads id, functions and attributes
//--------------------------------------------------------------------------------
osThreadId_t phy_rec_id;
osThreadId_t phy_snd_id;
osThreadId_t mac_rec_id;
osThreadId_t mac_snd_id;
osThreadId_t time_rec_id;
osThreadId_t time_snd_id;
osThreadId_t chat_rec_id;
osThreadId_t chat_snd_id;
osThreadId_t debug_id;
osThreadId_t touch_id;
osThreadId_t lcd_id;
osThreadId_t audio_id;

extern void PhReceiver(void *argument);
extern void PhSender(void *argument);
extern void MacReceiver(void *argument);
extern void MacSender(void *argument);
extern void TimeReceiver(void *argument);
extern void TimeSender(void *argument);
extern void ChatReceiver(void *argument);
extern void ChatSender(void *argument);
extern void DebugStation(void *argument);
extern void Touch(void *argument);
extern void LCD(void *argument);
extern void AudioPlayer(void *argument);

const osThreadAttr_t audio_attr = {
  .stack_size = 512,
	.priority = osPriorityAboveNormal,
	.name = "AUDIO"
};

const osThreadAttr_t debug_attr = {
  .stack_size = 512,
	.priority = osPriorityNormal,
	.name = "DBG"
};

const osThreadAttr_t phy_rec_attr = {
  .stack_size = 512,
	.priority = osPriorityNormal,
	.name = "PHY_R"
};

const osThreadAttr_t phy_snd_attr = {
  .stack_size = 512,
	.priority = osPriorityNormal,
	.name = "PH_S"
};

const osThreadAttr_t mac_rec_attr = {
  .stack_size = 512,
	.priority = osPriorityNormal,
	.name = "MAC_R"
};

const osThreadAttr_t mac_snd_attr = {
  .stack_size = 512,
	.priority = osPriorityNormal,
  .name = "MAC_S" 	
};

const osThreadAttr_t time_rec_attr = {
  .stack_size = 512,
	.priority = osPriorityNormal,
  .name = "TIME_RECEIVER"  	
};

const osThreadAttr_t time_snd_attr = {
  .stack_size = 512,
	.priority = osPriorityNormal,
	.name = "TIME_SENDER"
};

const osThreadAttr_t chat_rec_attr = {
  .stack_size = 512,
	.priority = osPriorityNormal,
  .name = "CHAT_RECEIVER"  	
};

const osThreadAttr_t chat_snd_attr = {
  .stack_size = 512,
	.priority = osPriorityNormal,
	.name = "CHAT_SENDER"  	
};

const osThreadAttr_t touch_attr = {
  .stack_size = 512,
	.priority = osPriorityNormal,
  .name = "TOUCH"
};

const osThreadAttr_t lcd_attr = {
  .stack_size = 4096,
	.priority = osPriorityNormal,
	.name = "LCD"  	
};

const osThreadAttr_t tester_attr = {
  .stack_size = 256,
	.priority = osPriorityNormal,
	.name = "TESTER"  	
};

//////////////////////////////////////////////////////////////////////////////////
/// \brief Display a frame of type PHY
/// \param stringP pointer to physical frame to display
/// It is used by PHY_SENDER and PHY_RECEIVER
//////////////////////////////////////////////////////////////////////////////////
void DebugFrame(char * stringP)
{
  uint8_t  temp;                          			// used for temp calculation    
  uint8_t  byte;                          			// used for temp byte memory    
  uint8_t  table[16] = {'0','1','2','3',
                     '4','5','6','7',
                     '8','9','A','B',
                     'C','D','E','F'};
  uint32_t size;                          			// get size of frame            
  uint32_t i;                             			// temporary counter            

										 
  if ((uint8_t)stringP[1] == TOKEN_TAG)     // is it a TOKEN frame         
  {
    size = TOKENSIZE;                  			// size is TOKENSIZE            
  }
  else
  {
    size = stringP[3] + 6;             			// calculate size of frame      
  }
  i = 0;
  putchar('[');                    			    // display an open bracket      
  for(i=0;i<size;i++)
  {
    byte = stringP[i];                 			// get byte from frame          
    temp = (byte & 0xF0) >> 4;
    putchar(table[temp]);           			  // display 2 hex digits         
     temp = (byte & 0x0F);
    putchar(table[temp]);
    putchar(' ');                   			  // insert a space               
  }
  putchar(']');                     			  // display a closed bracket     
  putchar(0x0D);                    			  // with a <CR>                  
  putchar(0x0A);                    			  // with a <CR>                  
}

//////////////////////////////////////////////////////////////////////////////////
/// \brief Display a frame of type MAC to the PC serial port for debug
/// \param preChar the char to write before the message
/// \param strinP pointer to the message of type MAC !!!
/// It is used by PHY_SENDER and DEBUG
//////////////////////////////////////////////////////////////////////////////////
void DebugMacFrame(uint8_t preChar,uint8_t * stringP)
{
	uint8_t  temp;                          // used for temp calculation
	uint8_t  byte;                          // used for temp byte memory
	uint8_t  table[16] = {'0','1','2','3',
                     '4','5','6','7',
                     '8','9','A','B',
                     'C','D','E','F'};
	uint32_t size;                         	// get size of frame
	uint32_t i;                           	// temporary counter

  if (stringP[0] == TOKEN_TAG)           	// is it a TOKEN frame
  {
    size = TOKENSIZE - 2;         				//size is TOKENSIZE
  }
  else
  {
    size = stringP[2] + 4;             		// calculate size of frame
  }
  i = 0;
  putchar(preChar);
  putchar('-');
  putchar('[');                    				// display an open bracket
  for(i=0;i<size;i++)
  {
    byte = stringP[i];                 		// get byte from frame
    temp = (byte & 0xF0) >> 4;
    putchar(table[temp]);           			// display 2 hex digits
     temp = (byte & 0x0F);
    putchar(table[temp]);
    putchar(' ');                   			// insert a space
  }
  putchar(']');                     			// display a closed bracket
  putchar('\r');                    			// with a <CR>
  putchar('\n');                    			// and a <LF> to flush buffer
}

//////////////////////////////////////////////////////////////////////////////////
/// \brief Check an os call return code and display error if any
/// \param retCode return code to control
/// \param lineNumber the line number where was the error
/// \param fileName the filename where is the error
/// \param mode If not 0 (CONTINUE), the program stays in this function
//////////////////////////////////////////////////////////////////////////////////
void CheckRetCode(uint32_t retCode,uint32_t lineNumber,char * fileName,uint8_t mode)
{
  if(retCode != osOK )											// if an error occcurs					
  {
		printf("At line : %d\r\n",lineNumber);	// display line error						
		printf("On file : %s \r\n",fileName);
		printf("Error   : %d\r\n",retCode);			// displays error number				
		if (mode != CONTINUE)										// if mode is not CONTINUE (0)
    {
			while(1){}														// stays here forever						
    }
  }
}


uint8_t Checksum(uint8_t * frame)
{
	uint8_t checksum = 0;
	uint8_t length = 0;
	
	// get length of data
	length = frame[2];
	
	for (uint8_t i = 0; i < length+3; i++)
	{
		checksum = checksum + frame[i];
	}
	return checksum;
}

//////////////////////////////////////////////////////////////////////////////////
/// \brief Configure the clock @ 216MHz and peripheral clocks
//////////////////////////////////////////////////////////////////////////////////
static void SystemClock_Config (void) {
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef RCC_ExCLKInitStruct;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Activate the OverDrive to reach the 216 MHz Frequency */
  HAL_PWREx_EnableOverDrive();

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);

  HAL_RCCEx_GetPeriphCLKConfig(&RCC_ExCLKInitStruct);
  RCC_ExCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
  RCC_ExCLKInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLI2S;
  RCC_ExCLKInitStruct.PLLI2S.PLLI2SP = 8;
  RCC_ExCLKInitStruct.PLLI2S.PLLI2SN = 344;
  RCC_ExCLKInitStruct.PLLI2S.PLLI2SQ = 7;
  RCC_ExCLKInitStruct.PLLI2SDivQ = 1;
  HAL_RCCEx_PeriphCLKConfig(&RCC_ExCLKInitStruct);
}


//////////////////////////////////////////////////////////////////////////////////
/// \brief Used by HAL for delays (have to return system ticks)
//////////////////////////////////////////////////////////////////////////////////
uint32_t HAL_GetTick(void)
{
	return osKernelGetTickCount();
}


//////////////////////////////////////////////////////////////////////////////////
/// \brief Init all and start RTX5
//////////////////////////////////////////////////////////////////////////////////
int main(void)
{	
	SystemClock_Config();
	
	EventRecorderInitialize(EventRecordAll,0);
	EventRecorderDisable(EventRecordAPI, 0xF1, 0xF1); // remove Kernel messages
	EventRecorderDisable(EventRecordAll, 0xF4, 0xF4); // remove EventFlag messages
	EventRecorderStart();
	
	osKernelInitialize();
	
	Ext_LED_Init();												// Initialise leds
	Ext_UART_Init(9600);									// Initialise UART @ 9600 bauds

	//------------------------------------------------------------------------------
	// Default configuration station
	//------------------------------------------------------------------------------
	gTokenInterface.myAddress = MYADDRESS;	// constant from main.h
	gTokenInterface.connected = TRUE;				// default is station connected
	Ext_LED_PWM(8,100);											// display connected
	gTokenInterface.debugAddress = 9;				// default values
	gTokenInterface.debugSAPI = 1;
	gTokenInterface.debugOnline = TRUE;
	gTokenInterface.destinationAddress = 1;
	 
	//------------------------------------------------------------------------------
	// Create memory pool
	//------------------------------------------------------------------------------
	memPool = osMemoryPoolNew(8,MAX_BLOCK_SIZE,NULL);	
	//------------------------------------------------------------------------------
	// Create event flag
	//------------------------------------------------------------------------------
	eventFlag_id = osEventFlagsNew(NULL);
	//------------------------------------------------------------------------------
	// Create queues
	//------------------------------------------------------------------------------	
	queue_macR_id = osMessageQueueNew(2,sizeof(struct queueMsg_t),&queue_macR_attr); 	
	queue_phyS_id = osMessageQueueNew(2,sizeof(struct queueMsg_t),&queue_phyS_attr); 	
	queue_macS_id = osMessageQueueNew(2,sizeof(struct queueMsg_t),&queue_macS_attr); 	
	queue_dbg_id = osMessageQueueNew(2,sizeof(struct queueMsg_t),&queue_dbg_attr); 	
	queue_chatR_id = osMessageQueueNew(2,sizeof(struct queueMsg_t),&queue_chatR_attr); 	
	queue_chatS_id = osMessageQueueNew(2,sizeof(struct queueMsg_t),&queue_chatS_attr); 	
	queue_timeR_id = osMessageQueueNew(2,sizeof(struct queueMsg_t),&queue_timeR_attr); 	
	queue_timeS_id = osMessageQueueNew(2,sizeof(struct queueMsg_t),&queue_timeS_attr); 	
	queue_lcd_id = osMessageQueueNew(4,sizeof(struct queueMsg_t),&queue_lcd_attr); 	
	queue_keyboard_id = osMessageQueueNew(4,sizeof(struct queueMsg_t),&queue_keyboard_attr); 	
	queue_usartR_id = osMessageQueueNew(4,sizeof(struct queueMsg_t),&queue_usartR_attr); 	

	//------------------------------------------------------------------------------
	// Create Threads
	//------------------------------------------------------------------------------
  // audio_id = osThreadNew(AudioPlayer, NULL, &audio_attr);
  debug_id = osThreadNew(DebugStation, NULL, &debug_attr);
  phy_rec_id = osThreadNew(PhReceiver, NULL, &phy_rec_attr);
  phy_snd_id = osThreadNew(PhSender, NULL, &phy_snd_attr);
  mac_rec_id = osThreadNew(MacReceiver, NULL, &mac_rec_attr);
  mac_snd_id = osThreadNew(MacSender, NULL, &mac_snd_attr);
  time_rec_id = osThreadNew(TimeReceiver, NULL, &time_rec_attr);
  time_snd_id = osThreadNew(TimeSender, NULL, &time_snd_attr);
  chat_rec_id = osThreadNew(ChatReceiver, NULL, &chat_rec_attr);
  chat_snd_id = osThreadNew(ChatSender, NULL, &chat_snd_attr);
  touch_id = osThreadNew(Touch, NULL, &touch_attr);
  lcd_id = osThreadNew(LCD, NULL, &lcd_attr);
	
	//------------------------------------------------------------------------------
	// Start kernel and never returns
	//------------------------------------------------------------------------------
  osKernelStart();
}

