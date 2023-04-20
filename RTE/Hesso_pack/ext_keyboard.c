
#include "stm32f7xx_hal.h"
#include "ext_keyboard.h"

#define TRUE 1
#define FALSE 0
#define NORMAL       0
#define SHIFT	     1
#define CONTROL      2
#define ALT          3
#define TABULATOR    0x09
#define BACKSPACE    0x08
#define ESCAPE       0x1B
#define CR           0x0D
#define F1           0x10
#define F2           0x11
#define F3           0x12
#define F4           0x13
#define F5           0x14
#define F6           0x15
#define F7           0x16
#define F8           0x17
#define F9           0x18
#define F10          0x19
#define F11          0x1A
#define F12          0x1B

static SPI_HandleTypeDef keyboard;								// SPI keyboard handle
uint8_t ext_kbChar;																// exported to global use
uint8_t scan;
static uint32_t newChar=0;												// internal flag
uint8_t release;
uint8_t state;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
uint8_t ScanCodeAnalyse(uint8_t scan)
{
      switch(scan)
      {
       /*---------------------------------------------------------------------------*/
       case 0xF0: release = TRUE; return 0   ; /* release key code                  */
       /*---------------------------------------------------------------------------*/
       case 0x14: if (release == FALSE)
                     state = CONTROL;
                  else
                  {
                     state = NORMAL;
                     release = FALSE;
                  }
                  return 0x00;
       /*---------------------------------------------------------------------------*/
       case 0x11: if (release == FALSE)
                     state = ALT;
                  else
                  {
                     state = NORMAL;
                     release = FALSE;
                  }
                  return 0x00;
       /*---------------------------------------------------------------------------*/
			 case 0x12:
			 case 0x58:
       case 0x59: if (release == FALSE)
                     state = SHIFT;
                  else
                  {
                     state = NORMAL;
                     release = FALSE;
                  }
                  return 0x00;
      }
       /*---------------------------------------------------------------------------*/
      if (release == TRUE)
      {
      	release = FALSE;
      	return 0x00;
      }
       /*---------------------------------------------------------------------------*/
       /*---------------------------------------------------------------------------*/
      switch(scan)
      {
       /*---------------------------------------------------------------------------*/
       case 0x05: return F1;	// 'F1' to 'F12'
       /*---------------------------------------------------------------------------*/
       case 0x06: return F2;
       /*---------------------------------------------------------------------------*/
       case 0x04: return F3;
       /*---------------------------------------------------------------------------*/
       case 0x0c: return F4;
       /*---------------------------------------------------------------------------*/
       case 0x03: return F5;
       /*---------------------------------------------------------------------------*/
       case 0x0B: return F6;
       /*---------------------------------------------------------------------------*/
       case 0x83: return F7;
       /*---------------------------------------------------------------------------*/
       case 0x0A: return F8;
       /*---------------------------------------------------------------------------*/
       case 0x01: return F9;
       /*---------------------------------------------------------------------------*/
       case 0x09: return F10;
       /*---------------------------------------------------------------------------*/
       case 0x78: return F11;
       /*---------------------------------------------------------------------------*/
       case 0x07: return F12;
       /*---------------------------------------------------------------------------*/
       case 0x16: switch (state) /* key '1' */
                  {
                   case NORMAL    : return '1';
                   case CONTROL   : return 0;
                   case SHIFT     : return '!';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x1E: switch (state) /* key '2' */
                  {
                   case NORMAL    : return '2';
                   case CONTROL   : return 0;
                   case SHIFT     : return '@';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x26: switch (state) /* key '3' */
                  {
                   case NORMAL    : return '3';
                   case CONTROL   : return 0;
                   case SHIFT     : return '#'; 
                  }
       /*---------------------------------------------------------------------------*/
       case 0x25: switch (state) /* key '4' */
                  {
                   case NORMAL    : return '4';
                   case CONTROL   : return 0;
                   case SHIFT     : return '$';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x2E: switch (state) /* key '5' */
                  {
                   case NORMAL    : return '5';
                   case CONTROL   : return 0;
                   case SHIFT     : return '%';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x36: switch (state) /* key '6' */
                  {
                   case NORMAL    : return '6';
                   case CONTROL   : return 0;
                   case SHIFT     : return '^';
                  }
       /*---------------------------------------------------------------------------*/
			 case 0x6C:
			 case 0x3D: switch (state) /* key '7' */
                  {
                   case NORMAL    : return '7';
                   case CONTROL   : return 0;
                   case SHIFT     : return '&';
                  }
       /*---------------------------------------------------------------------------*/
			 case 0x75:
       case 0x3E: switch (state) /* key '8' */
                  {
                   case NORMAL    : return '8';
                   case CONTROL   : return 0;
                   case SHIFT     : return '*';
                  }
       /*---------------------------------------------------------------------------*/
			 case 0x7D:
       case 0x46: switch (state) /* key '9' */
                  {
                   case NORMAL    : return '9';
                   case CONTROL   : return 0;
                   case SHIFT     : return '(';
                  }
       /*---------------------------------------------------------------------------*/
			 case 0x7C:
       case 0x45: switch (state) /* key '0' */
                  {
                   case NORMAL    : return '0';
                   case CONTROL   : return 0;
                   case SHIFT     : return ')';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x7E: switch (state) /* key '*' */
                  {
                   case NORMAL    : return '*';
                   case CONTROL   : return F11;
                   case SHIFT     : return '?';
                   default        : return 0xBF; /* ? vertical flipped */
                  }
       /*---------------------------------------------------------------------------*/
       case 0x4E: switch (state) /* key '-' */
                  {
                   case NORMAL    : return '-';
                   case CONTROL   : return 0;
                   case SHIFT     : return '_';
                   default        : return 0x00;
                  }
       /*---------------------------------------------------------------------------*/
       case 0x65: return 0x1B;  /* End button -> Escape command */
       /*---------------------------------------------------------------------------*/
       case 0x15: switch (state) /* key 'Q' */
                  {
                   case SHIFT     : return 'Q';
                   case CONTROL   : return 0x11;
                   default        : return 'q';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x1D: switch (state) /* key 'W' */
                  {
                   case SHIFT     : return 'W';
                   case CONTROL   : return 0x17;
                   default        : return 'w';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x24: switch (state) /* key 'E' */
                  {
                   case SHIFT     : return 'E';
                   case CONTROL   : return 0x05;
                   default        : return 'e';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x2D: switch (state) /* key 'R' */
                  {
                   case SHIFT     : return 'R';
                   case CONTROL   : return 0x12;
                   default        : return 'r';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x2C: switch (state) /* key 'T' */
                  {
                   case SHIFT     : return 'T';
                   case CONTROL   : return 0x14;
                   default        : return 't';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x35: switch (state) /* key 'Z' */
                  {
                   case SHIFT     : return 'Y';
                   case CONTROL   : return 0x1A;
                   default        : return 'y';
                  }
       /*---------------------------------------------------------------------------*/
				 case 0x6B:
       case 0x3C: switch (state) /* key 'U' */
                  {
                   case SHIFT     : return 'U';
                   case CONTROL   : return 0x15;
                   default        : return 'u';
                  }
       /*---------------------------------------------------------------------------*/
				 case 0x73:
       case 0x43: switch (state) /* key 'I' */
                  {
                   case SHIFT     : return 'I';
                   case CONTROL   : return 0x09;
                   default        : return 'i';
                  }
       /*---------------------------------------------------------------------------*/
				 case 0x74:
       case 0x44: switch (state) /* key 'O' */
                  {
                   case SHIFT     : return 'O';
                   case CONTROL   : return 0x0F;
                   default        : return 'o';
                  }
       /*---------------------------------------------------------------------------*/
				case 0x7B:
       case 0x4D: switch (state) /* key 'P' */
                  {
                   case SHIFT     : return 'P';
                   case CONTROL   : return 0x10;
                   default        : return 'p';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x5B: switch (state) /* key 'è' */
                  {
                   case SHIFT     : return '}';
                   case ALT       : return 0;
                   default        : return ']';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x66: return 0x08;  /* Backspace */
       /*---------------------------------------------------------------------------*/
       case 0x0D: return 0x09;  /* Horizontal tabulator */
       /*---------------------------------------------------------------------------*/
       case 0x1C: switch (state) /* key 'A' */
                  {
                   case SHIFT     : return 'A';
                   case CONTROL   : return 0x01;
                   default        : return 'a';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x1B: switch (state) /* key 'S' */
                  {
                   case SHIFT     : return 'S';
                   case CONTROL   : return 0x13;
                   default        : return 's';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x23: switch (state) /* key 'D' */
                  {
                   case SHIFT     : return 'D';
                   case CONTROL   : return 0x04;
                   default        : return 'd';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x2B: switch (state) /* key 'F' */
                  {
                   case SHIFT     : return 'F';
                   case CONTROL   : return 0x06;
                   default        : return 'f';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x34: switch (state) /* key 'G' */
                  {
                   case SHIFT     : return 'G';
                   case CONTROL   : return 0x07;
                   default        : return 'g';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x33: switch (state) /* key 'H' */
                  {
                   case SHIFT     : return 'H';
                   case CONTROL   : return 0x08;
                   default        : return 'h';
                  }
       /*---------------------------------------------------------------------------*/
				 case 0x69:
       case 0x3B: switch (state) /* key 'J' */
                  {
                   case SHIFT     : return 'J';
                   case CONTROL   : return 0x0A;
                   default        : return 'j';
                  }
       /*---------------------------------------------------------------------------*/
				 case 0x72:
       case 0x42: switch (state) /* key 'K' */
                  {
                   case SHIFT     : return 'K';
                   case CONTROL   : return 0x0B;
                   default        : return 'k';
                  }
       /*---------------------------------------------------------------------------*/
				 case 0x7A:
       case 0x4B: switch (state) /* key 'L' */
                  {
                   case SHIFT     : return 'L';
                   case CONTROL   : return 0x0C;
                   default        : return 'l';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x54: switch (state) /* key '[' */
                  {
                   case SHIFT     : return '{';
                   case ALT       : return 0; /* beta symbol */
                   default        : return '[';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x52: switch (state) /* key ''' */
                  {
                   case SHIFT     : return '"';
                   case ALT       : return 0;
                   default        : return 0x27;
                  }
       /*---------------------------------------------------------------------------*/
       case 0x1A: switch (state) /* key 'Y' */
                  {
                   case SHIFT     : return 'Z';
                   case CONTROL   : return 0x19;
                   default        : return 'z';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x22: switch (state) /* key 'X' */
                  {
                   case SHIFT     : return 'X';
                   case CONTROL   : return 0x18;
                   default        : return 'x';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x21: switch (state) /* key 'C' */
                  {
                   case SHIFT     : return 'C';
                   case CONTROL   : return 0x03;
                   default        : return 'c';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x2A: switch (state) /* key 'V' */
                  {
                   case SHIFT     : return 'V';
                   case CONTROL   : return 0x16;
                   default        : return 'v';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x32: switch (state) /* key 'B' */
                  {
                   case SHIFT     : return 'B';
                   case CONTROL   : return 0x02;
                   default        : return 'b';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x31: switch (state) /* key 'N' */
                  {
                   case SHIFT     : return 'N';
                   case CONTROL   : return 0x0E;
                   default        : return 'n';
                  }
       /*---------------------------------------------------------------------------*/
				 case 0x70:
       case 0x3A: switch (state) /* key 'M' */
                  {
                   case SHIFT     : return 'M';
                   case CONTROL   : return 0x0D;
                   default        : return 'm';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x41: switch (state) /* key ',' */
                  {
                   case SHIFT     : return '<';
                   default        : return ',';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x49: switch (state) /* key '.' */
                  {
                   case SHIFT     : return '>';
                   default        : return '.';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x5A: return 0x0D;  /* Carriage return (Enter) */
       /*---------------------------------------------------------------------------*/
       case 0x29: return ' ';  /* Spacebar */
       /*---------------------------------------------------------------------------*/
       case 0x63: switch (state) /* key 'UP ARROW' */
                  {
                   case SHIFT     : return 'U';
                   default        : return 'u';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x60: switch (state) /* key 'DOWN ARROW' */
                  {
                   case SHIFT     : return 'D';
                   default        : return 'd';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x61: switch (state) /* key 'LEFT ARROW' */
                  {
                   case SHIFT     : return 'L';
                   default        : return 'l';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x6A: switch (state) /* key 'RIGHT ARROW' */
                  {
                   case SHIFT     : return 'R';
                   default        : return 'r';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x0E: switch (state) /* key 'apostroph' */
                  {
                   case SHIFT     : return '~';
                   default        : return '`';
                  }
       /*---------------------------------------------------------------------------*/
				 case 0x79:
       case 0x4C: switch (state) /* key ';' */
                  {
										case SHIFT     : return ':';
                   default        : return ';';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x4A: switch (state) /* key '/' */
                  {
										case SHIFT     : return '?';
                   default        : return '/';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x5D: switch (state) /* key '\' */
                  {
										case SHIFT     : return '|';
                   default        : return '\\';
                  }
       /*---------------------------------------------------------------------------*/
       case 0x55: switch (state) /* key '=' */
                  {
										case SHIFT     : return '+';
                   default        : return '=';
                  }
      }	
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void EXTI9_5_IRQHandler(void)
{
	// read keyboard data and store it on global variable
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_0, GPIO_PIN_RESET);	// SPI CS activate
	HAL_SPI_Receive(&keyboard,&scan,1,100);					// SPI read
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_0, GPIO_PIN_SET);		// SPI CS desactivate
	// process irq
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);									// clear irq and callback
	ext_kbChar = ScanCodeAnalyse(scan);
	if(ext_kbChar != 0)
	{
		newChar = 1;
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Ext_Keyboard_Init(void)
{
	volatile uint8_t dummy;
  GPIO_InitTypeDef GPIO_InitStruct;

	/* Peripheral clock enable */
	__HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_SPI2_CLK_ENABLE();
  
  /**SPI2 GPIO Configuration    
  PI1      ------>  SPI2_SCK
  PB14     ------>  SPI2_MISO
  PB15     ------>  SPI2_MOSI 
	PI0      ------>  /SPI_CB_SS
	PF8      ------>  /SPI_INT
  */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	GPIO_InitStruct.Pin   = GPIO_PIN_0;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);	
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  keyboard.Instance = SPI2;
  keyboard.Init.Mode = SPI_MODE_MASTER;
  keyboard.Init.Direction = SPI_DIRECTION_2LINES;
  keyboard.Init.DataSize = SPI_DATASIZE_8BIT;
  keyboard.Init.CLKPolarity = SPI_POLARITY_HIGH;
  keyboard.Init.CLKPhase = SPI_PHASE_1EDGE;
  keyboard.Init.NSS = SPI_NSS_SOFT;
  keyboard.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;	//3.333MHz -> (Fosc/4) / 16
  keyboard.Init.FirstBit = SPI_FIRSTBIT_MSB;
  keyboard.Init.TIMode = SPI_TIMODE_DISABLE;
  keyboard.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  keyboard.Init.CRCPolynomial = 7;
  keyboard.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  keyboard.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
	HAL_SPI_Init(&keyboard);
	// dummy read to clear any glitch
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_SPI_Receive(&keyboard,(uint8_t *)&dummy,1,10);
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_0, GPIO_PIN_SET);
	// enable interrupts on keyboard ISR
  HAL_NVIC_SetPriority(EXTI9_5_IRQn,5,5);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
uint8_t Ext_Keyboard_Read(void)
{
	while(newChar == 0)										// wait new char coming
	{}
	newChar = 0;													// clear it
	return ext_kbChar;										// return read char
}



