/*
 BitzOS (BOS) V0.4.0 - Copyright (C) 2017-2025 Hexabitz
 All rights reserved

 File Name  : H3BR7.c
 Description: Main source code for H3BR7 module, controlling a 6-digit 7-segment display and indicator LEDs.
 Peripherals: Configures USART1,2,3,5,6 for module ports (P1-P5), TIM6 for 7-segment multiplexing.
 Features: Displays integers, floating-point numbers, quantities with units, letters, sentences, and moving sentences on 7-segment.
           Supports 4 indicator LEDs, CLI commands, remote bootloader updates, and power modes (stop/standby).
*/

/* Includes ****************************************************************/
#include "BOS.h"
#include "H3BR7_inputs.h"

/* Exported Typedef ******************************************************/
/* Define UART variables */
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart6;

SegmentCodes Digit[7] ={Empty};   /* Digit[0]: LSD, Digit[6]: MSD */

extern TIM_HandleTypeDef htim6; /* Timer for 7-segment (6 peices) */

/* Module Parameters */
ModuleParam_t ModuleParam[NUM_MODULE_PARAMS] ={0};

/* Private variables ---------------------------------------------------------*/
uint8_t CommaIndex;     /* A global variable to specify the index of the comma */
uint8_t StartSevSegIndex;
uint8_t SevenSegIndex = 0;
uint8_t MovingSentenceFlag = 0;
uint8_t MovingSentenceIndex = 0;
uint8_t MovingSentenceLength = 0;
uint8_t MovingSentenceBuffer[MOVING_SENTENCE_MAX_LENGTH + 6] = {0};
uint32_t MovingSentenceCounter = 0;
int CommaFlag=0;        /* Activate a flag when a float number is shown */

/* Private function prototypes *********************************************/
void MX_TIM6_Init(void);
void Module_Peripheral_Init(void);
void SetupPortForRemoteBootloaderUpdate(uint8_t port);
void RemoteBootloaderUpdate(uint8_t src,uint8_t dst,uint8_t inport,uint8_t outport);
uint8_t ClearROtopology(void);
Module_Status Module_MessagingTask(uint16_t code, uint8_t port, uint8_t src, uint8_t dst, uint8_t shift);

/* Local function prototypes ***********************************************/
SegmentCodes GetNumberCode(uint8_t digit);
SegmentCodes GetLetterCode(char letter);
SegmentCodes ClearAllDigits(void);

/* Create CLI commands *****************************************************/
portBASE_TYPE CLI_DisplayNumberCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE CLI_DisplayQuantitiesCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE CLI_DisplaySentenceCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE CLI_DisplayMovingSentenceCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE CLI_DisplayOffCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE CLI_SetIndicatorCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE CLI_ClearIndicatorCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );

/* CLI command structure ***************************************************/
/* CLI command structure : DisplayNumber */
const CLI_Command_Definition_t CLI_DisplayNumberCommandDefinition = {
	( const int8_t * ) "display_num", /* The command string to type. */
	( const int8_t * ) "display_num:\r\n Parameters required to execute a DisplayNumber: Number, Res, StartSevSeg \r\n\r\n",
	CLI_DisplayNumberCommand, /* The function to run. */
	3 /* three parameters are expected. */
};

/***************************************************************************/
/* CLI command structure : DisplayQuantities */
const CLI_Command_Definition_t CLI_DisplayQuantitiesCommandDefinition = {
	( const int8_t * ) "display_quant", /* The command string to type. */
	( const int8_t * ) "display_quant:\r\n Parameters required to execute a DisplayQuantities: NumberF, Res, Unit, StartSevSeg \r\n\r\n",
	CLI_DisplayQuantitiesCommand, /* The function to run. */
	4 /* four parameters are expected. */
};

/***************************************************************************/
/* CLI command structure : DisplaySentance */
const CLI_Command_Definition_t CLI_DisplaySentenceCommandDefinition = {
	( const int8_t * ) "display_sent", /* The command string to type. */
	( const int8_t * ) "display_sent:\r\nParameters required to execute a DisplaySentance:Sentence,StartSevseg,  \r\n\r\n",
	CLI_DisplaySentenceCommand, /* The function to run. */
	2 /* two parameters are expected. */
};

/***************************************************************************/
/* CLI command structure : DisplayMovingSentance */
const CLI_Command_Definition_t CLI_DisplayMovingSentenceCommandDefinition = {
	( const int8_t * ) "display_mov_sent", /* The command string to type. */
	( const int8_t * ) "display_mov_sent:\r\nParameters required to execute a DisplayMovingSentence: Sentence \r\n\r\n",
	CLI_DisplayMovingSentenceCommand, /* The function to run. */
	1 /* one parameters are expected. */
};

/***************************************************************************/
/* CLI command structure : DisplayOff */
const CLI_Command_Definition_t CLI_DisplayOffCommandDefinition = {
	( const int8_t * ) "display_off", /* The command string to type. */
	( const int8_t * ) "display_off:\r\nParameters required to execute a DisplayOff \r\n\r\n",
	CLI_DisplayOffCommand, /* The function to run. */
	0 /* zero parameters are expected. */
};

/***************************************************************************/
/* CLI command structure : SetIndicator */
const CLI_Command_Definition_t CLI_SetIndicatorCommandDefinition = {
	( const int8_t * ) "set_ind", /* The command string to type. */
	( const int8_t * ) "set_ind:\r\nParameters required to execute a SetIndicator: indicator number \r\n\r\n",
	CLI_SetIndicatorCommand, /* The function to run. */
	1 /* one parameters are expected. */
};

/***************************************************************************/
/* CLI command structure : ClearIndicator */
const CLI_Command_Definition_t CLI_ClearIndicatorCommandDefinition = {
	( const int8_t * ) "clear_ind", /* The command string to type. */
	( const int8_t * ) "clear_ind:\r\nParameters required to execute a ClearIncicator: indicator number \r\n\r\n",
	CLI_ClearIndicatorCommand, /* The function to run. */
	1 /* one parameters are expected. */
};

/***************************************************************************/
/************************ Private function Definitions *********************/
/***************************************************************************/
/* @brief  System Clock Configuration
 *         This function configures the system clock as follows:
 *            - System Clock source            = PLL (HSE)
 *            - SYSCLK(Hz)                     = 64000000
 *            - HCLK(Hz)                       = 64000000
 *            - AHB Prescaler                  = 1
 *            - APB1 Prescaler                 = 1
 *            - HSE Frequency(Hz)              = 8000000
 *            - PLLM                           = 1
 *            - PLLN                           = 16
 *            - PLLP                           = 2
 *            - Flash Latency(WS)              = 2
 *            - Clock Source for UART1,UART2,UART3 = 16MHz (HSI)
 */
void SystemClock_Config(void){
	RCC_OscInitTypeDef RCC_OscInitStruct ={0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct ={0};

	/** Configure the main internal regulator output voltage */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE; // Enable both HSI and HSE oscillators
	RCC_OscInitStruct.HSEState = RCC_HSE_ON; // Enable HSE (External High-Speed Oscillator)
	RCC_OscInitStruct.HSIState = RCC_HSI_ON; // Enable HSI (Internal High-Speed Oscillator)
	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1; // No division on HSI
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT; // Default calibration value for HSI
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON; // Enable PLL
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE; // Set PLL source to HSE
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1; // Prescaler for PLL input
	RCC_OscInitStruct.PLL.PLLN =16; // Multiplication factor for PLL
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // PLLP division factor
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2; // PLLQ division factor
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2; // PLLR division factor
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	/** Initializes the CPU, AHB and APB buses clocks */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // Select PLL as the system clock source
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1; // AHB Prescaler set to 1
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1; // APB1 Prescaler set to 1

	HAL_RCC_ClockConfig(&RCC_ClkInitStruct,FLASH_LATENCY_2); // Configure system clocks with flash latency of 2 WS
}

/***************************************************************************/
/* enable stop mode regarding only UART1 , UART2 , and UART3 */
BOS_Status EnableStopModebyUARTx(uint8_t port){

	UART_WakeUpTypeDef WakeUpSelection;
	UART_HandleTypeDef *huart =GetUart(port);

	if((huart->Instance == USART1) || (huart->Instance == USART2) || (huart->Instance == USART3)){

		/* make sure that no UART transfer is on-going */
		while(__HAL_UART_GET_FLAG(huart, USART_ISR_BUSY) == SET);

		/* make sure that UART is ready to receive */
		while(__HAL_UART_GET_FLAG(huart, USART_ISR_REACK) == RESET);

		/* set the wake-up event:
		 * specify wake-up on start-bit detection */
		WakeUpSelection.WakeUpEvent = UART_WAKEUP_ON_STARTBIT;
		HAL_UARTEx_StopModeWakeUpSourceConfig(huart,WakeUpSelection);

		/* Enable the UART Wake UP from stop mode Interrupt */
		__HAL_UART_ENABLE_IT(huart,UART_IT_WUF);

		/* enable MCU wake-up by LPUART */
		HAL_UARTEx_EnableStopMode(huart);

		/* enter STOP mode */
		HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON,PWR_STOPENTRY_WFI);
	}
	else
		return BOS_ERROR;

}

/***************************************************************************/
/* Enable standby mode regarding wake-up pins:
 * WKUP1: PA0  pin
 * WKUP4: PA2  pin
 * WKUP6: PB5  pin
 * WKUP2: PC13 pin
 * NRST pin
 *  */
BOS_Status EnableStandbyModebyWakeupPinx(WakeupPins_t wakeupPins){

	/* Clear the WUF FLAG */
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF);

	/* Enable the WAKEUP PIN */
	switch(wakeupPins){

		case PA0_PIN:
			HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1); /* PA0 */
			break;

		case PA2_PIN:
			HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN4); /* PA2 */
			break;

		case PB5_PIN:
			HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN6); /* PB5 */
			break;

		case PC13_PIN:
			HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2); /* PC13 */
			break;

		case NRST_PIN:
			/* do no thing*/
			break;
	}

	/* Enable SRAM content retention in Standby mode */
	HAL_PWREx_EnableSRAMRetention();

	/* Finally enter the standby mode */
	HAL_PWR_EnterSTANDBYMode();

	return BOS_OK;
}

/***************************************************************************/
/* Disable standby mode regarding wake-up pins:
 * WKUP1: PA0  pin
 * WKUP4: PA2  pin
 * WKUP6: PB5  pin
 * WKUP2: PC13 pin
 * NRST pin
 *  */
BOS_Status DisableStandbyModeWakeupPinx(WakeupPins_t wakeupPins){

	/* The standby wake-up is same as a system RESET:
	 * The entire code runs from the beginning just as if it was a RESET.
	 * The only difference between a reset and a STANDBY wake-up is that, when the MCU wakes-up,
	 * The SBF status flag in the PWR power control/status register (PWR_CSR) is set */
	if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET){
		/* clear the flag */
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

		/* Disable  Wake-up Pinx */
		switch(wakeupPins){

			case PA0_PIN:
				HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1); /* PA0 */
				break;

			case PA2_PIN:
				HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN4); /* PA2 */
				break;

			case PB5_PIN:
				HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN6); /* PB5 */
				break;

			case PC13_PIN:
				HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2); /* PC13 */
				break;

			case NRST_PIN:
				/* do no thing*/
				break;
		}

		IND_blink(1000);

	}
	else
		return BOS_OK;

}

/***************************************************************************/
/* Save Command Topology in Flash RO */
uint8_t SaveTopologyToRO(void){

	HAL_StatusTypeDef flashStatus =HAL_OK;

	/* flashAdd is initialized with 8 because the first memory room in topology page
	 * is reserved for module's ID */
	uint16_t flashAdd =8;
	uint16_t temp =0;

	/* Unlock the FLASH control register access */
	HAL_FLASH_Unlock();

	/* Erase Topology page */
	FLASH_PageErase(FLASH_BANK_2,TOPOLOGY_PAGE_NUM);

	/* Wait for an Erase operation to complete */
	flashStatus =FLASH_WaitForLastOperation((uint32_t ) HAL_FLASH_TIMEOUT_VALUE);

	if(flashStatus != HAL_OK){
		/* return FLASH error code */
		return pFlash.ErrorCode;
	}

	else{
		/* Operation is completed, disable the PER Bit */
		CLEAR_BIT(FLASH->CR,FLASH_CR_PER);
	}

	/* Save module's ID and topology */
	if(myID){

		/* Save module's ID */
		temp =(uint16_t )(N << 8) + myID;

		/* Save module's ID in Flash memory */
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,TOPOLOGY_START_ADDRESS,temp);

		/* Wait for a Write operation to complete */
		flashStatus =FLASH_WaitForLastOperation((uint32_t ) HAL_FLASH_TIMEOUT_VALUE);

		if(flashStatus != HAL_OK){
			/* return FLASH error code */
			return pFlash.ErrorCode;
		}

		else{
			/* If the program operation is completed, disable the PG Bit */
			CLEAR_BIT(FLASH->CR,FLASH_CR_PG);
		}

		/* Save topology */
		for(uint8_t row =1; row <= N; row++){
			for(uint8_t column =0; column <= MAX_NUM_OF_PORTS; column++){
				/* Check the module serial number
				 * Note: there isn't a module has serial number 0
				 */
				if(Array[row - 1][0]){
					/* Save each element in topology Array in Flash memory */
					HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,TOPOLOGY_START_ADDRESS + flashAdd,Array[row - 1][column]);
					/* Wait for a Write operation to complete */
					flashStatus =FLASH_WaitForLastOperation((uint32_t ) HAL_FLASH_TIMEOUT_VALUE);
					if(flashStatus != HAL_OK){
						/* return FLASH error code */
						return pFlash.ErrorCode;
					}
					else{
						/* If the program operation is completed, disable the PG Bit */
						CLEAR_BIT(FLASH->CR,FLASH_CR_PG);
						/* update new flash memory address */
						flashAdd +=8;
					}
				}
			}
		}
	}
	/* Lock the FLASH control register access */
	HAL_FLASH_Lock();
}

/***************************************************************************/
/* Save Command Snippets in Flash RO */
uint8_t SaveSnippetsToRO(void){
	HAL_StatusTypeDef FlashStatus =HAL_OK;
	uint8_t snipBuffer[sizeof(Snippet_t) + 1] ={0};

	/* Unlock the FLASH control register access */
	HAL_FLASH_Unlock();
	/* Erase Snippets page */
	FLASH_PageErase(FLASH_BANK_2,SNIPPETS_PAGE_NUM);
	/* Wait for an Erase operation to complete */
	FlashStatus =FLASH_WaitForLastOperation((uint32_t ) HAL_FLASH_TIMEOUT_VALUE);

	if(FlashStatus != HAL_OK){
		/* return FLASH error code */
		return pFlash.ErrorCode;
	}
	else{
		/* Operation is completed, disable the PER Bit */
		CLEAR_BIT(FLASH->CR,FLASH_CR_PER);
	}

	/* Save Command Snippets */
	int currentAdd = SNIPPETS_START_ADDRESS;
	for(uint8_t index =0; index < NumOfRecordedSnippets; index++){
		/* Check if Snippet condition is true or false */
		if(Snippets[index].Condition.ConditionType){
			/* A marker to separate Snippets */
			snipBuffer[0] =0xFE;
			memcpy((uint32_t* )&snipBuffer[1],(uint8_t* )&Snippets[index],sizeof(Snippet_t));
			/* Copy the snippet struct buffer (20 x NumOfRecordedSnippets). Note this is assuming sizeof(Snippet_t) is even */
			for(uint8_t j =0; j < (sizeof(Snippet_t) / 4); j++){
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,currentAdd,*(uint64_t* )&snipBuffer[j * 8]);
				FlashStatus =FLASH_WaitForLastOperation((uint32_t ) HAL_FLASH_TIMEOUT_VALUE);
				if(FlashStatus != HAL_OK){
					return pFlash.ErrorCode;
				}
				else{
					/* If the program operation is completed, disable the PG Bit */
					CLEAR_BIT(FLASH->CR,FLASH_CR_PG);
					currentAdd +=8;
				}
			}
			/* Copy the snippet commands buffer. Always an even number. Note the string termination char might be skipped */
			for(uint8_t j =0; j < ((strlen(Snippets[index].CMD) + 1) / 4); j++){
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,currentAdd,*(uint64_t* )(Snippets[index].CMD + j * 4));
				FlashStatus =FLASH_WaitForLastOperation((uint32_t ) HAL_FLASH_TIMEOUT_VALUE);
				if(FlashStatus != HAL_OK){
					return pFlash.ErrorCode;
				}
				else{
					/* If the program operation is completed, disable the PG Bit */
					CLEAR_BIT(FLASH->CR,FLASH_CR_PG);
					currentAdd +=8;
				}
			}
		}
	}
	/* Lock the FLASH control register access */
	HAL_FLASH_Lock();
}

/***************************************************************************/
/* Clear Array topology in SRAM and Flash RO */
uint8_t ClearROtopology(void){
	/* Clear the Array */
	memset(Array,0,sizeof(Array));
	N =1;
	myID =0;
	
	return SaveTopologyToRO();
}

/***************************************************************************/
/* Trigger ST factory bootloader update for a remote module */
void RemoteBootloaderUpdate(uint8_t src,uint8_t dst,uint8_t inport,uint8_t outport){

	uint8_t myOutport =0, lastModule =0;
	int8_t *pcOutputString;

	/* 1. Get Route to destination module */
	myOutport =FindRoute(myID,dst);
	if(outport && dst == myID){ /* This is a 'via port' update and I'm the last module */
		myOutport =outport;
		lastModule =myID;
	}
	else if(outport == 0){ /* This is a remote update */
		if(NumberOfHops(dst)== 1)
		lastModule = myID;
		else
		lastModule = Route[NumberOfHops(dst)-1]; /* previous module = Route[Number of hops - 1] */
	}

	/* 2. If this is the source of the message, show status on the CLI */
	if(src == myID){
		/* Obtain the address of the output buffer.  Note there is no mutual
		 * exclusion on this buffer as it is assumed only one command console
		 * interface will be used at any one time. */
		pcOutputString =FreeRTOS_CLIGetOutputBuffer();

		if(outport == 0)		// This is a remote module update
			sprintf((char* )pcOutputString,pcRemoteBootloaderUpdateMessage,dst);
		else
			// This is a 'via port' remote update
			sprintf((char* )pcOutputString,pcRemoteBootloaderUpdateViaPortMessage,dst,outport);

		strcat((char* )pcOutputString,pcRemoteBootloaderUpdateWarningMessage);
		writePxITMutex(inport,(char* )pcOutputString,strlen((char* )pcOutputString),cmd50ms);
		Delay_ms(100);
	}

	/* 3. Setup my inport and outport for bootloader update */
	SetupPortForRemoteBootloaderUpdate(inport);
	SetupPortForRemoteBootloaderUpdate(myOutport);

	/* 5. Build a DMA stream between my inport and outport */
	StartScastDMAStream(inport,myID,myOutport,myID,BIDIRECTIONAL,0xFFFFFFFF,0xFFFFFFFF,false);
}

/***************************************************************************/
/* Setup a port for remote ST factory bootloader update:
 * Set baudrate to 57600
 * Enable even parity
 * Set datasize to 9 bits
 */
void SetupPortForRemoteBootloaderUpdate(uint8_t port){

	UART_HandleTypeDef *huart =GetUart(port);

	HAL_UART_DeInit(huart);

	huart->Init.Parity = UART_PARITY_EVEN;
	huart->Init.WordLength = UART_WORDLENGTH_9B;
	HAL_UART_Init(huart);

	/* The CLI port RXNE interrupt might be disabled so enable here again to be sure */
	__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);

}

/***************************************************************************/
/* H3BR7 module initialization */
void Module_Peripheral_Init(void){

	/* Array ports */
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();
	MX_USART5_UART_Init();
	MX_USART6_UART_Init();

	MX_TIM6_Init();
	SevenSegGPIOInit();

	/* Circulating DMA Channels ON All Module */
	 for(int i=1;i<=NUM_OF_PORTS;i++)
		{
		  if(GetUart(i)==&huart1)
				   { dmaIndex[i-1]=&(DMA1_Channel1->CNDTR); }
		  else if(GetUart(i)==&huart2)
				   { dmaIndex[i-1]=&(DMA1_Channel2->CNDTR); }
		  else if(GetUart(i)==&huart3)
				   { dmaIndex[i-1]=&(DMA1_Channel3->CNDTR); }
		  else if(GetUart(i)==&huart4)
				   { dmaIndex[i-1]=&(DMA1_Channel4->CNDTR); }
		  else if(GetUart(i)==&huart5)
				   { dmaIndex[i-1]=&(DMA1_Channel5->CNDTR); }
		  else if(GetUart(i)==&huart6)
				   { dmaIndex[i-1]=&(DMA1_Channel6->CNDTR); }
		}
}

/***************************************************************************/
/* H3BR7 message processing task */
Module_Status Module_MessagingTask(uint16_t code,uint8_t port,uint8_t src,uint8_t dst,uint8_t shift){
	Module_Status result = H3BR7_OK;

	int32_t Number=0;
	uint8_t StartSevSeg=0;

	uint32_t Number_int;
	float NumberF;
	uint8_t Res=0;
	char Unit;

    uint8_t length;

    IndicatorLED indicator;

	switch(code){
	  case CODE_H3BRX_SEVEN_DISPLAY_NUMBER:
	  Number=((int32_t )cMessage[port - 1][shift] ) + ((int32_t )cMessage[port - 1][1 + shift] << 8) + ((int32_t )cMessage[port - 1][2 + shift] << 16) + ((int32_t )cMessage[port - 1][3 + shift] << 24);
	  StartSevSeg=(uint8_t)cMessage[port - 1][4+shift];
//	  SevenDisplayNumber(Number, StartSevSeg);
	  break;

	  case CODE_H3BRX_SEVEN_DISPLAY_NUMBER_F:
		  Number_int=((uint32_t) cMessage[port - 1][shift] + (uint32_t) (cMessage[port - 1][1+shift] <<8) + (uint32_t) (cMessage[port - 1][2+shift]<<16) + (uint32_t) (cMessage[port - 1][3+shift] <<24));
		  NumberF = *((float*)&Number_int);
		  Res=(uint8_t)cMessage[port - 1][4+shift];
		  StartSevSeg=(uint8_t)cMessage[port - 1][5+shift];
//		  SevenDisplayNumberF(NumberF, Res, StartSevSeg);
		  break;

	  case CODE_H3BRX_SEVEN_DISPLAY_QUANTITIES:
		  Number_int=((uint32_t) cMessage[port - 1][shift] + (uint32_t) (cMessage[port - 1][1+shift] <<8) + (uint32_t) (cMessage[port - 1][2+shift]<<16) + (uint32_t) (cMessage[port - 1][3+shift] <<24));
		  NumberF = *((float*)&Number_int);
		  Res=(uint8_t)cMessage[port - 1][4+shift];
		  Unit=(uint8_t)cMessage[port - 1][5+shift];
    	  StartSevSeg=(uint8_t)cMessage[port - 1][6+shift];
//    	  SevenDisplayQuantities(NumberF, Res, Unit, StartSevSeg);
		  break;

	  case CODE_H3BRX_SEVEN_DISPLAY_LETTER:
//		  StartSevSeg=(uint8_t)cMessage[port - 1][1+shift];
//		  SevenDisplayLetter((char)cMessage[port-1][shift], StartSevSeg);
		  break;

	  case CODE_H3BRX_SEVEN_DISPLAY_SENTENCE:
		  length=(uint8_t)cMessage[port - 1][shift];
		  StartSevSeg=(uint8_t)cMessage[port - 1][1+shift];
//		  SevenDisplaySentence((char *)&cMessage[port-1][2 + shift], length, StartSevSeg);
		  break;

	  case CODE_H3BRX_SEVEN_DISPLAY_MOVING_SENTENCE:
		  length=(uint8_t)cMessage[port - 1][shift];
//		  SevenDisplayMovingSentence((char *)&cMessage[port-1][1 + shift], length);
		  break;

	  case CODE_H3BRX_SEVEN_DISPLAY_OFF:
//		  SevenDisplayOff();
		  break;

	  case CODE_H3BRX_SET_INDICATOR:
		  indicator=(uint8_t)cMessage[port - 1][shift];
		  SetIndicator(indicator);
		  break;

	  case CODE_H3BRX_CLEAR_INDICATOR:
		  indicator=(uint8_t)cMessage[port - 1][shift];
		  ClearIndicator(indicator);
		  break;
	  default:
			result =H3BR7_ERR_UNKNOWNMESSAGE;
			break;
	}


	return result;
}

/***************************************************************************/
/* Get the port for a given UART */
uint8_t GetPort(UART_HandleTypeDef *huart){

	if(huart->Instance == USART6)
		return P1;
	else if(huart->Instance == USART2)
		return P2;
	else if(huart->Instance == USART3)
		return P3;
	else if(huart->Instance == USART1)
		return P4;
	else if(huart->Instance == USART5)
		return P5;
	
	return 0;
}

/***************************************************************************/
/* Register this module CLI Commands */
void RegisterModuleCLICommands(void){
	    FreeRTOS_CLIRegisterCommand(&CLI_DisplayNumberCommandDefinition);
		FreeRTOS_CLIRegisterCommand(&CLI_DisplayQuantitiesCommandDefinition);
		FreeRTOS_CLIRegisterCommand(&CLI_DisplaySentenceCommandDefinition);
		FreeRTOS_CLIRegisterCommand(&CLI_DisplayMovingSentenceCommandDefinition);
		FreeRTOS_CLIRegisterCommand(&CLI_DisplayOffCommandDefinition);
		FreeRTOS_CLIRegisterCommand(&CLI_SetIndicatorCommandDefinition);
		FreeRTOS_CLIRegisterCommand(&CLI_ClearIndicatorCommandDefinition);
}

/***************************************************************************/
/* This functions is useful only for input (sensors) modules.
 * @brief: Samples a module parameter value based on parameter index.
 * @param paramIndex: Index of the parameter (1-based index).
 * @param value: Pointer to store the sampled float value.
 * @retval: Module_Status indicating success or failure.
 */
Module_Status GetModuleParameter(uint8_t paramIndex,float *value){
	Module_Status status =BOS_OK;

	switch(paramIndex){

		/* Invalid parameter index */
		default:
			status =BOS_ERR_WrongParam;
			break;
	}

	return status;
}

/***************************************************************************/
/****************************** Local Functions ****************************/
/***************************************************************************/
/*
 * HAL_TIM_PeriodElapsedCallback: Callback function triggered when the timer period elapses
 * htim: Pointer to the timer handle structure (TIM_HandleTypeDef) for the timer instance
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	// Check if the callback is triggered by Timer 6
	if(htim == &htim6){
		// Disable all Seven Segment digits (set enable pins high, assuming active-low)
		HAL_GPIO_WritePin(SEVEN_SEG_ENABLE_1_GPIO_PORT,SEVEN_SEG_ENABLE_1_PIN,GPIO_PIN_SET);
		HAL_GPIO_WritePin(SEVEN_SEG_ENABLE_2_GPIO_PORT,SEVEN_SEG_ENABLE_2_PIN,GPIO_PIN_SET);
		HAL_GPIO_WritePin(SEVEN_SEG_ENABLE_3_GPIO_PORT,SEVEN_SEG_ENABLE_3_PIN,GPIO_PIN_SET);
		HAL_GPIO_WritePin(SEVEN_SEG_ENABLE_4_GPIO_PORT,SEVEN_SEG_ENABLE_4_PIN,GPIO_PIN_SET);
		HAL_GPIO_WritePin(SEVEN_SEG_ENABLE_5_GPIO_PORT,SEVEN_SEG_ENABLE_5_PIN,GPIO_PIN_SET);
		HAL_GPIO_WritePin(SEVEN_SEG_ENABLE_6_GPIO_PORT,SEVEN_SEG_ENABLE_6_PIN,GPIO_PIN_SET);
		// Set segment pins (A-G) based on the current digit's pattern in Digit array
		HAL_GPIO_WritePin(SEVEN_SEG_A_GPIO_PORT,SEVEN_SEG_A_PIN,Digit[SevenSegIndex] & 0b00000001);
		HAL_GPIO_WritePin(SEVEN_SEG_B_GPIO_PORT,SEVEN_SEG_B_PIN,Digit[SevenSegIndex] & 0b00000010);
		HAL_GPIO_WritePin(SEVEN_SEG_C_GPIO_PORT,SEVEN_SEG_C_PIN,Digit[SevenSegIndex] & 0b00000100);
		HAL_GPIO_WritePin(SEVEN_SEG_D_GPIO_PORT,SEVEN_SEG_D_PIN,Digit[SevenSegIndex] & 0b00001000);
		HAL_GPIO_WritePin(SEVEN_SEG_E_GPIO_PORT,SEVEN_SEG_E_PIN,Digit[SevenSegIndex] & 0b00010000);
		HAL_GPIO_WritePin(SEVEN_SEG_F_GPIO_PORT,SEVEN_SEG_F_PIN,Digit[SevenSegIndex] & 0b00100000);
		HAL_GPIO_WritePin(SEVEN_SEG_G_GPIO_PORT,SEVEN_SEG_G_PIN,Digit[SevenSegIndex] & 0b01000000);
		HAL_GPIO_WritePin(SEVEN_SEG_DP_GPIO_PORT,SEVEN_SEG_DP_PIN,0);
		// Enable decimal point if current digit is at CommaIndex and CommaFlag is set
		if(SevenSegIndex == StartSevSegIndex + CommaIndex && CommaFlag == 1){
			HAL_GPIO_WritePin(SEVEN_SEG_DP_GPIO_PORT,SEVEN_SEG_DP_PIN,1);
		}
		// Enable the appropriate digit based on SevenSegIndex (active-low)
		switch(SevenSegIndex){
			case 0:
				HAL_GPIO_WritePin(SEVEN_SEG_ENABLE_1_GPIO_PORT,SEVEN_SEG_ENABLE_1_PIN,GPIO_PIN_RESET);
				break;

			case 1:
				HAL_GPIO_WritePin(SEVEN_SEG_ENABLE_2_GPIO_PORT,SEVEN_SEG_ENABLE_2_PIN,GPIO_PIN_RESET);
				break;

			case 2:
				HAL_GPIO_WritePin(SEVEN_SEG_ENABLE_3_GPIO_PORT,SEVEN_SEG_ENABLE_3_PIN,GPIO_PIN_RESET);
				break;

			case 3:
				HAL_GPIO_WritePin(SEVEN_SEG_ENABLE_4_GPIO_PORT,SEVEN_SEG_ENABLE_4_PIN,GPIO_PIN_RESET);
				break;

			case 4:
				HAL_GPIO_WritePin(SEVEN_SEG_ENABLE_5_GPIO_PORT,SEVEN_SEG_ENABLE_5_PIN,GPIO_PIN_RESET);
				break;

			case 5:
				HAL_GPIO_WritePin(SEVEN_SEG_ENABLE_6_GPIO_PORT,SEVEN_SEG_ENABLE_6_PIN,GPIO_PIN_RESET);
				break;
				// Special case: Set decimal point based on bit 7 of Digit[6]
			case 6:
				HAL_GPIO_WritePin(SEVEN_SEG_DP_GPIO_PORT,SEVEN_SEG_DP_PIN,Digit[6] & 0b10000000);
				break;

			default:
				break;

		}
		// Increment SevenSegIndex to move to the next digit
		SevenSegIndex++;
		if(SevenSegIndex > 7)
			SevenSegIndex =0;// Reset index after exceeding 7 (6 digits + special case)

		// Processing moving sentence if enabled
		if(MovingSentenceFlag == 1){

			MovingSentenceCounter++;
		// Update display when counter reaches overflow
			if(MovingSentenceCounter == MOVING_SENTENCE_COUNTER_OVERFLOW){
				MovingSentenceCounter =0;
				uint8_t temp;
		// Update Digit array with next set of characters from MovingSentenceBuffer
				for(int i =0; i < 6; i++){
					temp =MovingSentenceIndex + i;
					if(temp == MovingSentenceLength){
						temp -=MovingSentenceLength;// Wrap around if index exceeds length
					}
					Digit[5 - i] =MovingSentenceBuffer[temp];// Fill digits in reverse order
				}
				MovingSentenceIndex++;
				if(MovingSentenceIndex == MovingSentenceLength){
					MovingSentenceIndex =0;// Reset index to loop sentence
				}
			}
		}
	}
}

/***************************************************************************/
/*
 * GetNumberCode: Function to retrieve the display code for a given digit
 * digit: The numerical digit (0-9) to get the corresponding display code for
 */
SegmentCodes GetNumberCode(uint8_t digit){
	Module_Status status =H3BR7_OK;

	SegmentCodes code;
	switch(digit){
		case 0:
			code =ZERO_NUMBER;
			break;

		case 1:
			code =ONE_NUMBER;
			break;

		case 2:
			code =TWO_NUMBER;
			break;

		case 3:
			code =THREE_NUMBER;
			break;

		case 4:
			code =FOUR_NUMBER;
			break;

		case 5:
			code =FIVE_NUMBER;
			break;

		case 6:
			code =SIX_NUMBER;
			break;

		case 7:
			code =SEVEN_NUMBER;
			break;

		case 8:
			code =EIGHT_NUMBER;
			break;

		case 9:
			code =NINE_NUMBER;
			break;

		default:
			code =Empty;
			break;

	}
	return code;
}

/***************************************************************************/
/*
 * GetLetterCode: Function to retrieve the display code for a given letter
 * letter: The character to get the corresponding display code for
 */
SegmentCodes GetLetterCode(char letter){
	Module_Status status =H3BR7_OK;

	SegmentCodes letter_code;
	switch(letter){

		/* Capital Letters *********************************************************/
		case 'A':
			letter_code =A_LETTER;
			break;

		case 'B':
			letter_code =B_LETTER;
			break;

		case 'C':
			letter_code =C_LETTER;
			break;

		case 'D':
			letter_code =D_LETTER;
			break;

		case 'E':
			letter_code =E_LETTER;
			break;

		case 'F':
			letter_code =F_LETTER;
			break;

		case 'G':
			letter_code =G_LETTER;
			break;

		case 'H':
			letter_code =H_LETTER;
			break;

		case 'I':
			letter_code =I_LETTER;
			break;

		case 'J':
			letter_code =J_LETTER;
			break;

		case 'K':
			letter_code =K_LETTER;
			break;

		case 'L':
			letter_code =L_LETTER;
			break;

		case 'M':
			letter_code =M_LETTER;
			break;

		case 'N':
			letter_code =N_LETTER;
			break;

		case 'O':
			letter_code =O_LETTER;
			break;

		case 'P':
			letter_code =P_LETTER;
			break;

		case 'Q':
			letter_code =Q_LETTER;
			break;

		case 'R':
			letter_code =R_LETTER;
			break;

		case 'S':
			letter_code =S_LETTER;
			break;

		case 'T':
			letter_code =T_LETTER;
			break;

		case 'U':
			letter_code =U_LETTER;
			break;

		case 'V':
			letter_code =V_LETTER;
			break;

		case 'W':
			letter_code =W_LETTER;
			break;

		case 'X':
			letter_code =X_LETTER;
			break;

		case 'Y':
			letter_code =Y_LETTER;
			break;

		case 'Z':
			letter_code =Z_LETTER;
			break;

			/* Small Letters ***********************************************************/
		case 'a':
			letter_code =a_LETTER;
			break;

		case 'b':
			letter_code =b_LETTER;
			break;

		case 'c':
			letter_code =c_LETTER;
			break;

		case 'd':
			letter_code =d_LETTER;
			break;

		case 'e':
			letter_code =e_LETTER;
			break;

		case 'f':
			letter_code =f_LETTER;
			break;

		case 'g':
			letter_code =g_LETTER;
			break;

		case 'h':
			letter_code =h_LETTER;
			break;

		case 'i':
			letter_code =i_LETTER;
			break;

		case 'j':
			letter_code =j_LETTER;
			break;

		case 'k':
			letter_code =k_LETTER;
			break;

		case 'l':
			letter_code =l_LETTER;
			break;

		case 'm':
			letter_code =m_LETTER;
			break;

		case 'n':
			letter_code =n_LETTER;
			break;

		case 'o':
			letter_code =o_LETTER;
			break;

		case 'p':
			letter_code =p_LETTER;
			break;

		case 'q':
			letter_code =q_LETTER;
			break;

		case 'r':
			letter_code =r_LETTER;
			break;

		case 's':
			letter_code =s_LETTER;
			break;

		case 't':
			letter_code =t_LETTER;
			break;

		case 'u':
			letter_code =u_LETTER;
			break;

		case 'v':
			letter_code =v_LETTER;
			break;

		case 'w':
			letter_code =w_LETTER;
			break;

		case 'x':
			letter_code =x_LETTER;
			break;

		case 'y':
			letter_code =y_LETTER;
			break;

		case 'z':
			letter_code =z_LETTER;
			break;

		default:
			break;

	}
	return letter_code;
}


/***************************************************************************/
/*
 * ClearAllDigits: Function to clear all digits on the Seven Segment display
 */
SegmentCodes ClearAllDigits(void){
	Module_Status status =H3BR7_OK;

	for(int i =0; i < 6; i++)
		Digit[i] =Empty;

	CommaFlag =0;
	MovingSentenceFlag =0;
	MovingSentenceCounter =0;
}

/***************************************************************************/
/***************************** General Functions ***************************/
/***************************************************************************/
/*
 * DisplayNumber: Function to display a number on the Seven Segment display
 * Number: The decimal number to be displayed (float)
 * Res: Resolution (number of decimal places)
 * StartSevSeg: Starting position on the Seven Segment display (1-6)
 */
Module_Status DisplayNumber(float Number,uint8_t Res,uint8_t StartSevSeg){

	Module_Status status =H3BR7_OK;

	ClearAllDigits(); /* Seven segment display off */

	float max_value=0;
	float min_value=0;
	uint8_t index_digit_last=0;
	uint8_t signal =0;
	uint32_t Number_int=0;
	uint8_t length=0;
	uint8_t zero_flag =0;
	CommaIndex =Res;

/* This step is very important to set the StartSevSeg from the customer in the range 1-6.
 *But within the code, we Processing it in the range 0-5.
 *Don't omit it.
*/
	if (StartSevSeg==0) {
		status =H3BR7_ERR_WRONGPARAMS;
		CommaFlag =0;
		return status;
	}

	StartSevSeg=StartSevSeg-1;

	StartSevSegIndex =StartSevSeg;
	CommaFlag =1;

	if(!(StartSevSeg >= 0 && StartSevSeg <= 5)){
		status =H3BR7_ERR_WRONGPARAMS;
		CommaFlag =0;
		return status;
	}

	if((uint32_t )Number == 0)
		zero_flag =1;

		switch(StartSevSeg){
			case 0:
				if (Res == 0) {
					max_value =999999;
					min_value =-99999;
				}
				else
				{
					max_value =99999.9;
					min_value =-9999.9;
				}
				break;

			case 1:
				if (Res == 0) {
					max_value =99999;
					min_value =-9999;
				}
				else
				{
					max_value =9999.9;
					min_value =-999.9;
				}
				break;

			case 2:
				if (Res == 0) {
					max_value =9999;
					min_value =-999;
				}
				else
				{
					max_value =999.9;
					min_value =-99.9;
				}
				break;

			case 3:
				if (Res == 0) {
					max_value =999;
					min_value =-99;
				}
				else
				{
					max_value =99.9;
					min_value =-9.9;
				}
				break;

			case 4:
				if (Res == 0) {
					max_value =99;
					min_value =-9;
				}
				else
				{
					max_value =9.9;
					min_value =-9;
				}
				break;

			case 5:
				if (Res == 0) {
					max_value =9;
					min_value =0;
				}
				else
				{
					max_value =9;
					min_value =0;
				}
				break;

			default:
				break;
		}

	if(Number < 0){
		signal =1;
		Number *=-1;
	}

	switch(Res){
		case 0:
			Number_int =(uint32_t )Number;
			CommaFlag =0;
			break;

		case 1:
			if(StartSevSeg == 5)
				Number_int =(uint32_t )Number;
			else
				Number_int =(uint32_t )(Number * 10);
			break;

		case 2:
			if(StartSevSeg == 5)
				Number_int =(uint32_t )Number;
			else
				Number_int =(uint32_t )(Number * 100);
			break;

		case 3:
			if(StartSevSeg == 5)
				Number_int =(uint32_t )Number;
			else
				Number_int =(uint32_t )(Number * 1000);
			break;

		case 4:
			if(StartSevSeg == 5)
				Number_int =(uint32_t )Number;
			else
				Number_int =(uint32_t )(Number * 10000);
			break;

		case 5:
			if(StartSevSeg == 5)
				Number_int =(uint32_t )Number;
			else
				Number_int =(uint32_t )(Number * 100000);
			break;

		default:
			break;

	}

	if(Number_int > 0 && Number_int <= 9){
		length =1;
	}

	if(Number_int > 9 && Number_int <= 99){
		length =2;
	}

	if(Number_int > 99 && Number_int <= 999){
		length =3;
	}

	if(Number_int > 999 && Number_int <= 9999){
		length =4;
	}

	if(Number_int > 9999 && Number_int <= 99999){
		length =5;
	}

	if(Number_int > 99999 && Number_int <= 999999){
		length =6;
	}

	if(zero_flag == 0)
		index_digit_last =length + StartSevSeg;
	else
		index_digit_last =Res + 1 + StartSevSeg;

	if(signal == 1){
		Digit[index_digit_last] =SYMBOL_MINUS;
		Digit[index_digit_last + 1] =Empty;
	}



	for(int i =StartSevSeg; i < 6; i++){
		if(i == index_digit_last && signal == 1)
			continue;
		Digit[i] =GetNumberCode(Number_int % 10);
		Number_int /=10;
	}

	for(int x =index_digit_last; x < 6; x++){
		if(signal == 1 && x == index_digit_last)
			continue;
		Digit[x] =Empty;
	}
	HAL_Delay(10);
	return status;
}

/***************************************************************************/
/*
 *DisplayQuantities
 *Number: number to be displayed {int or float}
 *Res: Resolution (number of decimal places)
 *Unit: Unit of measurement (character representing the unit)
 *StartSevSeg: Starting position on the Seven Segment display {1-6}
 */
Module_Status DisplayQuantities(float Number,uint8_t Res,char Unit,uint8_t StartSevSeg){
	Module_Status status =H3BR7_OK;

	ClearAllDigits();

	float max_value;
	float min_value;
	uint8_t index_digit_last;
	uint8_t signal =0;
	uint32_t Number_int;
	uint8_t length;
	uint8_t zero_flag =0;
	CommaIndex =Res;

/* This step is very important to set the StartSevSeg from the customer in the range 1-6.
 *But within the code, we Processing it in the range 0-5.
 *Don't omit it.
*/
		if (StartSevSeg==0) {
			status =H3BR7_ERR_WRONGPARAMS;
			CommaFlag =0;
			return status;
		}
	StartSevSeg=StartSevSeg-1;

	//added +1 to StartSevSeg because the function contains a byte that points to Unit.
	StartSevSegIndex =StartSevSeg+1;
	CommaFlag =1;

	if((uint32_t )Number == 0)
		zero_flag =1;

	if(!(StartSevSeg >= 0 && StartSevSeg <= 5)){
		status =H3BR7_ERR_WRONGPARAMS;
		CommaFlag =0;
		return status;
	}
	switch(StartSevSeg){
		case 0:
			if (Res == 0) {
				max_value =99999;
				min_value =-9999;
			}
			else
			{
				max_value =9999.9;
				min_value =-999.9;
			}
			break;

		case 1:
			if (Res == 0) {
				max_value =9999;
				min_value =-999;
			}
			else
			{
				max_value =999.9;
				min_value =-99.9;
			}
			break;

		case 2:
			if (Res == 0) {
				max_value =999;
				min_value =-99;
			}
			else
			{
				max_value =99.9;
				min_value =-9.9;
			}
			break;

		case 3:
			if (Res == 0) {
				max_value =99;
				min_value =-9;
			}
			else
			{
				max_value =9.9;
				min_value =-9;
			}
			break;

		case 4:
			if (Res == 0) {
				max_value =9;
				min_value =0;
			}
			else
			{
				max_value =9;
				min_value =0;
			}
			break;

		default:
			break;
	}

	if(Number < 0){
		signal =1;
		Number *=-1;
	}

	switch(Res){
		case 0:
			Number_int =(uint32_t )Number;
			CommaFlag =0;
			break;

		case 1:
			if(StartSevSeg == 5)
				Number_int =(uint32_t )Number;
			else
				Number_int =(uint32_t )(Number * 10);
			break;

		case 2:
			if(StartSevSeg == 5)
				Number_int =(uint32_t )Number;
			else
				Number_int =(uint32_t )(Number * 100);
			break;

		case 3:
			if(StartSevSeg == 5)
				Number_int =(uint32_t )Number;
			else
				Number_int =(uint32_t )(Number * 1000);
			break;

		case 4:
			if(StartSevSeg == 5)
				Number_int =(uint32_t )Number;
			else
				Number_int =(uint32_t )(Number * 10000);
			break;

		case 5:
			if(StartSevSeg == 5)
				Number_int =(uint32_t )Number;
			else
				Number_int =(uint32_t )(Number * 100000);
			break;

		default:
			break;

	}

	if(Number_int >= 0 && Number_int <= 9){
		length =1;
	}

	if(Number_int > 9 && Number_int <= 99){
		length =2;
	}

	if(Number_int > 99 && Number_int <= 999){
		length =3;
	}

	if(Number_int > 999 && Number_int <= 9999){
		length =4;
	}

	if(Number_int > 9999 && Number_int <= 99999){
		length =5;
	}

	if(Number_int > 99999 && Number_int <= 999999){
		length =6;
	}

	if(zero_flag == 0)
		index_digit_last =length + StartSevSeg + 1;
	else
		index_digit_last =Res + 2 + StartSevSeg;
	if(signal == 1){
		Digit[index_digit_last] =SYMBOL_MINUS;
		Digit[index_digit_last + 1] =Empty;
	}

	Digit[StartSevSeg] =GetLetterCode(Unit);

	for(int i =StartSevSeg + 1; i < 6; i++){
		if(i == index_digit_last && signal == 1)
			continue;

		Digit[i] =GetNumberCode(Number_int % 10);
		Number_int /=10;
	}

	for(int x =index_digit_last; x < 6; x++){
		if(signal == 1 && x == index_digit_last)
			continue;
		Digit[x] =Empty;
	}

	HAL_Delay(10);
	return status;

}

/***************************************************************************/
/*
 * DisplaySentence: Function to display a sentence on the Seven Segment display
 * Sentence: Pointer to the character array containing the sentence to be displayed
 * length: Length of the sentence (number of characters)
 * StartSevSeg: Starting position on the Seven Segment display (1-6)
 */
Module_Status DisplaySentence(char *Sentence,uint16_t length,uint8_t StartSevSeg){
	Module_Status status =H3BR7_OK;

	ClearAllDigits();

	uint16_t max_length;
	char letter;

	if(length == 0 || Sentence == NULL){
		status =H3BR7_ERR_WRONGPARAMS;
		return status;
	}
	/* This step is very important to set the StartSevSeg from the customer in the range 1-6.
	 *But within the code, we Processing it in the range 0-5.
	 *Don't omit it.
	*/
	if (StartSevSeg==0) {
		status =H3BR7_ERR_WRONGPARAMS;
		return status;
		}
		StartSevSeg=StartSevSeg-1;

	switch(StartSevSeg){
		case 0:
			max_length =6;
			break;

		case 1:
			max_length =5;
			break;

		case 2:
			max_length =4;
			break;

		case 3:
			max_length =3;
			break;

		case 4:
			max_length =2;
			break;

		case 5:
			max_length =1;
			break;

		default:
			break;

	}

	if(length > max_length){
		status =H3BR7_OUT_OF_RANGE;
		return status;
	}

	for(int x =length - 1; x >= 0; x--){
		letter =Sentence[x];

		if((Sentence[x] >= 'a' && Sentence[x] <= 'z') || (Sentence[x] >= 'A' && Sentence[x] <= 'Z')){
			Digit[StartSevSeg] =GetLetterCode(letter);
		}

		else if(Sentence[x] >= '0' && Sentence[x] <= '9'){
			Digit[StartSevSeg] =GetNumberCode(letter - '0');
		}
		StartSevSeg++;
	}

	HAL_Delay(10);

	return status;

}

/***************************************************************************/
/*
 * DisplayMovingSentence: Function to display a moving sentence on the Seven Segment display
 * Sentence: Pointer to the character array containing the sentence to be displayed
 * length: Length of the sentence (number of characters)
 */
Module_Status DisplayMovingSentence(char *Sentence,uint16_t length){
	Module_Status status =H3BR7_OK;

	ClearAllDigits();

	if(length == 0 || Sentence == NULL){
		status =H3BR7_ERROR;
		return status;
	}

	if(length <= MOVING_SENTENCE_MAX_LENGTH){
		MovingSentenceIndex =0;
		MovingSentenceFlag =1;
		MovingSentenceLength =length + 6;

		for(int i =0; i < 6; i++)
			MovingSentenceBuffer[i] =Empty;

		for(int i =0; i < length; i++){
			if((Sentence[i] >= 'a' && Sentence[i] <= 'z') || (Sentence[i] >= 'A' && Sentence[i] <= 'Z')){
				MovingSentenceBuffer[i + 6] =GetLetterCode(Sentence[i]);
			}

			else if(Sentence[i] >= '0' && Sentence[i] <= '9'){
				MovingSentenceBuffer[i + 6] =GetNumberCode(Sentence[i] - '0');
			}

			else{
				MovingSentenceBuffer[i + 6] =Empty;
			}
		}
	}

	else{
		status =H3BR7_OUT_OF_RANGE;
		return status;
	}
	HAL_Delay(10);
	return status;

}

/***************************************************************************/
/*
 * DisplayOff: Function to turn off the Seven Segment display
 */
Module_Status DisplayOff(void){
	Module_Status status =H3BR7_OK;

	ClearAllDigits();

	return status;

}

/***************************************************************************/
/*
 * SetIndicator: Function to set the state of an indicator LED
 * indicator: The specific LED to be controlled{INDICATOR_1, INDICATOR_2, INDICATOR_3, INDICATOR_4}
 */
Module_Status SetIndicator(IndicatorLED indicator){

	Module_Status status =H3BR7_OK;

	if(indicator == INDICATOR_1){
		HAL_GPIO_WritePin(LED_INDICATOR1_GPIO_PORT,LED_INDICATOR1_PIN,GPIO_PIN_RESET);
	}

	else if(indicator == INDICATOR_2){
		HAL_GPIO_WritePin(LED_INDICATOR2_GPIO_PORT,LED_INDICATOR2_PIN,GPIO_PIN_RESET);
	}

	else if(indicator == INDICATOR_3){
		HAL_GPIO_WritePin(LED_INDICATOR3_GPIO_PORT,LED_INDICATOR3_PIN,GPIO_PIN_RESET);
	}

	else if(indicator == INDICATOR_4){
		HAL_GPIO_WritePin(LED_INDICATOR4_GPIO_PORT,LED_INDICATOR4_PIN,GPIO_PIN_RESET);
	}

	else{
		status =H3BR7_ERROR;
		return status;
	}

	return status;

}

/***************************************************************************/
/*
 * ClearIndicator: Function to turn off a specific indicator LED
 * indicator: The specific LED to be cleared{INDICATOR_1, INDICATOR_2, INDICATOR_3, INDICATOR_4}
 */
Module_Status ClearIndicator(IndicatorLED indicator){

	Module_Status status =H3BR7_OK;

	if(indicator == INDICATOR_1){
		HAL_GPIO_WritePin(LED_INDICATOR1_GPIO_PORT,LED_INDICATOR1_PIN,GPIO_PIN_SET);
	}

	else if(indicator == INDICATOR_2){
		HAL_GPIO_WritePin(LED_INDICATOR2_GPIO_PORT,LED_INDICATOR2_PIN,GPIO_PIN_SET);
	}

	else if(indicator == INDICATOR_3){
		HAL_GPIO_WritePin(LED_INDICATOR3_GPIO_PORT,LED_INDICATOR3_PIN,GPIO_PIN_SET);
	}

	else if(indicator == INDICATOR_4){
		HAL_GPIO_WritePin(LED_INDICATOR4_GPIO_PORT,LED_INDICATOR4_PIN,GPIO_PIN_SET);
	}

	else{
		status =H3BR7_ERROR;
		return status;
	}

	return status;

}

/***************************************************************************/
/********************************* Commands ********************************/
/***************************************************************************/
portBASE_TYPE CLI_DisplayNumberCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ){
	Module_Status status = H3BR7_OK;
	float Number;
	uint8_t Res;
	uint8_t StartSevSeg;
	static int8_t *pcParameterString1;
	static int8_t *pcParameterString2;
	static int8_t *pcParameterString3;
	portBASE_TYPE xParameterStringLength1 =0;
	portBASE_TYPE xParameterStringLength2 =0;
	portBASE_TYPE xParameterStringLength3 =0;
	static const int8_t *formatString[50];
	static const int8_t *pcOKMessage=(int8_t* )"SevenSegmentDisplay is on:\r\n %%0.%df \n\r";
	static const int8_t *pcWrongParamsMessage =(int8_t* )"Wrong Params!\n\r";


	(void )xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	 pcParameterString1 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength1 );
	 Number =(float )atof((char* )pcParameterString1);

	 pcParameterString2 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameterStringLength2 );
	 Res =(uint8_t )atol((char* )pcParameterString2);

	 pcParameterString3 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 3, &xParameterStringLength3 );
	 StartSevSeg =(uint8_t )atol((char* )pcParameterString3);


	status=DisplayNumber(Number,Res,StartSevSeg);

	if(status == H3BR7_OK)
	{
		sprintf((char* )formatString,(char* )pcOKMessage, Res);
		sprintf((char* )pcWriteBuffer,(char* )formatString,Number);

	}

	else if(status == H3BR7_ERR_WRONGPARAMS)
		strcpy((char* )pcWriteBuffer,(char* )pcWrongParamsMessage);



	return pdFALSE;
}

/* ----------------------------------------------------------------------------*/
portBASE_TYPE CLI_DisplayQuantitiesCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ){
	Module_Status status = H3BR7_OK;

	float Number=0;
	uint8_t Res;
	char Unit ;
	uint8_t StartSevSeg;


	static int8_t *pcParameterString1;
	static int8_t *pcParameterString2;
	static int8_t *pcParameterString3;
	static int8_t *pcParameterString4;
	static const int8_t *formatString[50];

	portBASE_TYPE xParameterStringLength1 =0;
	portBASE_TYPE xParameterStringLength2 =0;
	portBASE_TYPE xParameterStringLength3 =0;
	portBASE_TYPE xParameterStringLength4 =0;

	static const int8_t *pcOKMessage=(int8_t* )"SevenSegmentDisplay is on:\r\n %%0.%df %%c\n\r";
	static const int8_t *pcWrongParamsMessage =(int8_t* )"Wrong Params!\n\r";


	(void )xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	 pcParameterString1 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength1 );
	 Number =(float )atof((char* )pcParameterString1);

	 pcParameterString2 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameterStringLength2 );
	 Res =(uint8_t )atol((char* )pcParameterString2);

	 pcParameterString3 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 3, &xParameterStringLength3 );
	 if(xParameterStringLength3 == 1)
	 	{
	 		Unit = pcParameterString3[0];
	 		if (! ((Unit >= 'a' && Unit <= 'z') || (Unit >= 'A' && Unit <= 'Z') ))
	 		{
	 			 status=H3BR7_ERR_WRONGPARAMS;
	 		}

	 	}
	 	else
	 	{
	 		 status=H3BR7_ERR_WRONGPARAMS;
	 	}

	 pcParameterString4 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 4, &xParameterStringLength4 );
	 StartSevSeg =(uint8_t )atol((char* )pcParameterString4);

	 status=DisplayQuantities(Number, Res, Unit, StartSevSeg);

	 if(status == H3BR7_OK)
	 {
		 sprintf((char* )formatString,(char* )pcOKMessage, Res);
		 sprintf((char* )pcWriteBuffer,(char* )formatString,Number,Unit);

	 }

	 else if(status == H3BR7_ERR_WRONGPARAMS)
			strcpy((char* )pcWriteBuffer,(char* )pcWrongParamsMessage);


	return pdFALSE;

}

/* ----------------------------------------------------------------------------*/
portBASE_TYPE CLI_DisplaySentenceCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ){
	Module_Status status = H3BR7_OK;
	char* Sentence = NULL;
	uint8_t StartSevSeg;

	static int8_t *pcParameterString1;
	static int8_t *pcParameterString2;

	portBASE_TYPE xParameterStringLength1 =0;
	portBASE_TYPE xParameterStringLength2 =0;


	static const int8_t *pcOKMessage=(int8_t* )"SevenSegmentDisplay is on:\r\n %s \n\r";
	static const int8_t *pcErrorParamsMessage =(int8_t* )"Error Params!\n\r";
	static const int8_t *pcWrongRangeParamsMessage =(int8_t* )"Number is out of range!\n\r";

	(void )xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	 pcParameterString1 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength1 );
	 Sentence =(char* )pcParameterString1;

	 pcParameterString2 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameterStringLength2 );
	 StartSevSeg =(uint8_t )atol((char* )pcParameterString2);



	 status=DisplaySentence(Sentence, xParameterStringLength1, StartSevSeg);

	 if(status == H3BR7_OK)
	 {
		    sprintf((char* )pcWriteBuffer,(char* )pcOKMessage,Sentence);
	 }
	 else if(status == H3BR7_OUT_OF_RANGE)
			strcpy((char* )pcWriteBuffer,(char* )pcWrongRangeParamsMessage);

	 else if(status == H3BR7_ERR_WRONGPARAMS)
			strcpy((char* )pcWriteBuffer,(char* )pcErrorParamsMessage);



	return pdFALSE;

}

/* ----------------------------------------------------------------------------*/
portBASE_TYPE CLI_DisplayMovingSentenceCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ){
	Module_Status status = H3BR7_OK;
	char* Sentence = NULL;
	uint16_t length;

	static int8_t *pcParameterString1;

	portBASE_TYPE xParameterStringLength1 =0;


	static const int8_t *pcOKMessage=(int8_t* )"SevenSegmentDisplay is on:\r\n %s \n\r";
	static const int8_t *pcWrongRangeMessage =(int8_t* )"Number is out of range!\n\r";

	(void )xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	 pcParameterString1 =(char *)FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength1 );
	 Sentence = (char* )pcParameterString1;

	 status=DisplayMovingSentence(Sentence, xParameterStringLength1);


	 if(status == H3BR7_OK)
	 {
		 sprintf((char* )pcWriteBuffer,(char* )pcOKMessage,Sentence);

	 }

	 else if(status == H3BR7_OUT_OF_RANGE)
		strcpy((char* )pcWriteBuffer,(char* )pcWrongRangeMessage);


	return pdFALSE;
}

/* ----------------------------------------------------------------------------*/
portBASE_TYPE CLI_DisplayOffCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ){
	Module_Status status = H3BR7_OK;

	static const int8_t *pcOKMessage=(int8_t* )"SevenSegmentDisplay is off \n\r";
	static const int8_t *pcErrorsMessage =(int8_t* )"Error Params!\n\r";

		(void )xWriteBufferLen;
		configASSERT(pcWriteBuffer);

	 	status=DisplayOff();

	 if(status == H3BR7_OK)
	 {
			 sprintf((char* )pcWriteBuffer,(char* )pcOKMessage);

	 }

	 else if(status == H3BR7_ERROR)
			strcpy((char* )pcWriteBuffer,(char* )pcErrorsMessage);


	return pdFALSE;

}

/*-----------------------------------------------------------*/
portBASE_TYPE CLI_SetIndicatorCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ){
	Module_Status status = H3BR7_OK;
	IndicatorLED indicator;

	static int8_t *pcParameterString1;

	portBASE_TYPE xParameterStringLength1 =0;

	static const int8_t *pcOKMessage=(int8_t* )"SetIndicator %d is on \r\n  \n\r";
	static const int8_t *pcWrongParamsMessage =(int8_t* )"Wrong Params!\n\r";


	(void )xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	pcParameterString1 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength1 );
	indicator =(IndicatorLED )atol((char* )pcParameterString1);



	status=SetIndicator(indicator);

	if(status == H3BR7_OK)
	{
		sprintf((char* )pcWriteBuffer,(char* )pcOKMessage,indicator);

	}

	else if(status == H3BR7_ERROR)
		strcpy((char* )pcWriteBuffer,(char* )pcWrongParamsMessage);


	return pdFALSE;
}

/*-----------------------------------------------------------*/
portBASE_TYPE CLI_ClearIndicatorCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ){
	Module_Status status = H3BR7_OK;
	IndicatorLED indicator;

	static int8_t *pcParameterString1;

	portBASE_TYPE xParameterStringLength1 =0;

	static const int8_t *pcOKMessage=(int8_t* )"SetIndicator %d is off \r\n  \n\r";
	static const int8_t *pcWrongParamsMessage =(int8_t* )"Wrong Params!\n\r";


	(void )xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	pcParameterString1 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength1 );
	indicator =(IndicatorLED )atol((char* )pcParameterString1);



	status=ClearIndicator(indicator);

	if(status == H3BR7_OK)
	{
		sprintf((char* )pcWriteBuffer,(char* )pcOKMessage,indicator);

	}

	else if(status == H3BR7_ERROR)
		strcpy((char* )pcWriteBuffer,(char* )pcWrongParamsMessage);


	return pdFALSE;
}

/*-----------------------------------------------------------*/

/************************ (C) COPYRIGHT HEXABITZ *****END OF FILE****/
