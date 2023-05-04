//////////////////////////////////////////////////////////////////////////////////
/// \file main.h
/// \brief Tokenring definitions
/// \author Pascal Sartoretti (pascal dot sartoretti at hevs dot ch)
/// \version 1.0 - original for RTX5
/// \date  2018-02
//////////////////////////////////////////////////////////////////////////////////
#include "stm32f7xx_hal.h"

#include <stdlib.h>		
#include <stdio.h>    
#include <stdbool.h>
#include "gfx.h"
#include "gui.h"

//--------------------------------------------------------------------------------
// Constants to change the system behaviour
//--------------------------------------------------------------------------------
#define DEBUG_MODE				1					// mode is physical line (0) or debug (1)
#define MYADDRESS   			0					// your address choice (table number)
#define MAX_BLOCK_SIZE 		100				// size max for a frame

//--------------------------------------------------------------------------------
// Constants to NOT change for the system working
//--------------------------------------------------------------------------------
#define CHAT_SAPI 				0x01  	 	// sapi chat application number (0-7)
#define TIME_SAPI 				0x03   		// sapi time application number (0-7)
#define BROADCAST_ADDRESS	0x0F			// broadcast address
#define TOKEN_TAG					0xFF			// tag of tokenring frame
#define TOKENSIZE					19				// size of a token frame
#define STX 							0x02			// any frame start char
#define ETX								0x03			// any frame end char
#define CONTINUE					0x0				// for check return code halt

//--------------------------------------------------------------------------------
// identifiers used in more the one file (thread)
//--------------------------------------------------------------------------------
extern GListener 	gl;
extern osMemoryPoolId_t memPool;
extern osThreadId_t phy_rec_id;
extern osMessageQueueId_t	queue_macR_id;
extern osMessageQueueId_t	queue_phyS_id;
extern osMessageQueueId_t	queue_dbg_id;
extern osMessageQueueId_t	queue_macS_id;
extern osMessageQueueId_t  queue_chatR_id;
extern osMessageQueueId_t  queue_timeR_id;
extern osMessageQueueId_t  queue_lcd_id;
extern osMessageQueueId_t  queue_timeS_id;
extern osMessageQueueId_t  queue_chatS_id;
extern osMessageQueueId_t  queue_keyboard_id;
extern osMessageQueueId_t  queue_usartR_id;
extern osEventFlagsId_t  	eventFlag_id;
//--------------------------------------------------------------------------------
// functions used in more than one file
//--------------------------------------------------------------------------------
uint8_t Checksum(uint8_t * frame);
void CheckRetCode(uint32_t retCode,uint32_t lineNumber,char * fileName,uint8_t mode);
void DebugFrame(char * stringP);
void DebugMacFrame(uint8_t preChar,uint8_t * stringP);

//--------------------------------------------------------------------------------
// structure for system usage
//--------------------------------------------------------------------------------
struct TOKENINTERFACE
{
	uint8_t		myAddress;						///< my current address
	uint8_t		currentView;					///< the current view on LCD
	bool_t		debugOnline;					///< is debug station ON
	bool_t		connected;						///< are we connected
	bool_t		broadcastTime;				///< is broadcast time active
	bool_t		needReceiveCRCError;	///< did debug has to receive error
	bool_t		needSendCRCError;			///< didi debug has to send error
	uint32_t	debugSAPI;						///< current debug SAPI
	uint32_t	debugAddress;					///< current debug address
	bool_t		debugMsgToSend;				///< did debug have to send a message
	uint32_t	destinationAddress;		///< current destination address
	uint8_t		station_list[15];			///< 0 to 15 
};
extern struct TOKENINTERFACE gTokenInterface;

//--------------------------------------------------------------------------------
// events usage
//--------------------------------------------------------------------------------
#define	RS232_TX_EVENT 			0x0001			// ready for next byte to send		
#define	BROADCAST_TIME_EVT 	0x0002			// time to send or not
#define AUDIO_MSG_EVT	 			0x0020			// audio message to play
#define AUDIO_ERROR_EVT 		0x0040			// audio error to play
#define AUDIO_CLOCK_EVT 		0x0080			// audio clock to play

//--------------------------------------------------------------------------------
// types of messages transmitted in the queues
//--------------------------------------------------------------------------------
enum msgType_e
{
	TOUCH_EVENT,						///< touch has been pressed
	NEW_TOKEN,							///< a new token is requested
	START,									///< a start is requested (connected)
	STOP,										///< a stop is requested (disconnected)
	TOKEN_LIST,							///< token list sent to LCD
	MAC_ERROR,							///< error message to LCD
	TOKEN,									///< a token message
	DATA_IND,								///< a data sent between mac and app layers
	DATABACK,								///< a message is coming back
	TIME_MSG,								///< a time message is sent to LCD
	CHAR_MSG,								///< a single char is sent to the LCD
	CHAT_MSG,								///< a chat message is sent to the LCD
	FROM_PHY,								///< a message arriving from physical layer
	TO_PHY									///< a message sent to physical layer
};

//--------------------------------------------------------------------------------
// The queue message structure
//--------------------------------------------------------------------------------
struct queueMsg_t
{
	enum msgType_e	type;		///< the type of message
	void * anyPtr;					///< the pointer to message (if any)
	uint8_t	addr;						///< the source or destination address
	uint8_t sapi;						///< the source or destination SAPI
};

// bit field for control
struct mac_control
{
	uint8_t destSAPI : 3;		// LSB
	uint8_t destAddr : 4;
	uint8_t nothing1 : 1;
	uint8_t srcSAPI : 3;
	uint8_t srcAddr : 4;
	uint8_t nothing2 : 1;		// MSB
};

struct mac_control_byte
{
	uint8_t dest;
	uint8_t source;
};

// bit field for status
struct mac_status
{
	uint8_t ack : 1;				// LSB
	uint8_t read : 1;
	uint8_t checksum : 6;		// MSB
};

union mac_status_union
{
	struct mac_status status;
	uint8_t raw;
};

union mac_control_union
{
	struct mac_control controlBf;
	struct mac_control_byte controlBytes;
	uint16_t raw;

};


