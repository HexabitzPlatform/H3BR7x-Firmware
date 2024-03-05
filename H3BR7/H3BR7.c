/*
 BitzOS (BOS) V0.3.1 - Copyright (C) 2017-2024 Hexabitz
 All rights reserved

 File Name     : H3BR7.c
 Description   : Source code for module H3BR7.
 	 	 	 	 (Description_of_module)

(Description of Special module peripheral configuration):
>>USARTs 1,2,3,5 for module ports.
>>Timer6 for Seven Segment
>>

 */

/* Includes ------------------------------------------------------------------*/
#include "BOS.h"
#include "H3BR7_inputs.h"
/* Define UART variables */
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart6;

/* Exported variables */
extern FLASH_ProcessTypeDef pFlash;
extern uint8_t numOfRecordedSnippets;


Segment_Codes Digit[7] = {Empty}; //Digit[0]: LSD, Digit[6]: MSD
/* Module exported parameters ------------------------------------------------*/
module_param_t modParam[NUM_MODULE_PARAMS] ={{.paramPtr = NULL, .paramFormat =FMT_FLOAT, .paramName =""}};

/* Private variables ---------------------------------------------------------*/
uint8_t Res_it;          //A global variable to specify the index of the comma
uint8_t StartSevSeg_it;  //A global variable to specify the index of the comma
int Comma_flag=0;        //Activate a flag when a float number is shown


uint8_t  Moving_sentence_buffer[MOVING_SENTENCE_MAX_LENGTH + 6] = {0};
uint8_t  Moving_sentence_length = 0;
uint8_t  Moving_sentence_flag = 0;
uint32_t Moving_sentence_counter = 0;
uint8_t  Moving_sentence_index = 0;

/* Private function prototypes -----------------------------------------------*/
void ExecuteMonitor(void);
void MX_TIM6_Init(void);
void FLASH_Page_Eras(uint32_t Addr );

/* Create CLI commands --------------------------------------------------------*/
portBASE_TYPE CLI_SevenDisplayNumberCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE CLI_SevenDisplayNumberFCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE CLI_SevenDisplayQuantitiesCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE CLI_SevenDisplayLetterCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE CLI_SevenDisplaySentenceCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE CLI_SevenDisplayMovingSentenceCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE CLI_SevenDisplayOffCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE CLI_SetIndicatorCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE CLI_ClearIndicatorCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );

/* CLI command structure : SevenDisplayNumber */
const CLI_Command_Definition_t CLI_SevenDisplayNumberCommandDefinition =
{
	( const int8_t * ) "seven_display_number", /* The command string to type. */
	( const int8_t * ) "seven_display_number:\r\n Parameters required to execute a SevenDisplayNumber: Number , StartSevSeg \r\n\r\n",
	CLI_SevenDisplayNumberCommand, /* The function to run. */
	2 /* two parameters are expected. */
};


/* CLI command structure : SevenDisplayNumberF */
const CLI_Command_Definition_t CLI_SevenDisplayNumberFCommandDefinition =
{
	( const int8_t * ) "seven_display_numberf", /* The command string to type. */
	( const int8_t * ) "seven_display_numberf:\r\n Parameters required to execute a SevenDisplayNumberF: NumberF , Res, StartSevSeg \r\n\r\n",
	CLI_SevenDisplayNumberFCommand, /* The function to run. */
	3 /* three parameters are expected. */
};


/* CLI command structure : SevenDisplayQuantities */
const CLI_Command_Definition_t CLI_SevenDisplayQuantitiesCommandDefinition =
{
	( const int8_t * ) "seven_display_quantities", /* The command string to type. */
	( const int8_t * ) "seven_display_quantities:\r\n Parameters required to execute a SevenDisplayQuantities: NumberF, Res, Unit, StartSevSeg \r\n\r\n",
	CLI_SevenDisplayQuantitiesCommand, /* The function to run. */
	4 /* four parameters are expected. */
};

/* CLI command structure : SevenDisplayLetter */
const CLI_Command_Definition_t CLI_SevenDisplayLetterCommandDefinition =
{
	( const int8_t * ) "seven_display_letter", /* The command string to type. */
	( const int8_t * ) "seven_display_letter:\r\n Parameters required to execute a SevenDisplayLetter: Letter,StartSevSeg  \r\n\r\n",
	CLI_SevenDisplayLetterCommand, /* The function to run. */
	2 /* two parameters are expected. */
};

/* CLI command structure : SevenDisplaySentance */
const CLI_Command_Definition_t CLI_SevenDisplaySentenceCommandDefinition =
{
	( const int8_t * ) "seven_display_sentence", /* The command string to type. */
	( const int8_t * ) "seven_display_sentence:\r\nParameters required to execute a SevenDisplaySentance:  StartSevseg, Sentence \r\n\r\n",
	CLI_SevenDisplaySentenceCommand, /* The function to run. */
	2 /* two parameters are expected. */
};

/* CLI command structure : SevenDisplayMovingSentance */
const CLI_Command_Definition_t CLI_SevenDisplayMovingSentenceCommandDefinition =
{
	( const int8_t * ) "seven_display_moving_sentence", /* The command string to type. */
	( const int8_t * ) "seven_display_moving_sentence:\r\nParameters required to execute a SevenDisplayMovingSentence: Sentence \r\n\r\n",
	CLI_SevenDisplayMovingSentenceCommand, /* The function to run. */
	1 /* one parameters are expected. */
};

/* CLI command structure : SevenDisplayOff */
const CLI_Command_Definition_t CLI_SevenDisplayOffCommandDefinition =
{
	( const int8_t * ) "seven_display_off", /* The command string to type. */
	( const int8_t * ) "seven_display_off:\r\nParameters required to execute a SevenDisplayOff \r\n\r\n",
	CLI_SevenDisplayOffCommand, /* The function to run. */
	0 /* zero parameters are expected. */
};


/* CLI command structure : SetIndicator */
const CLI_Command_Definition_t CLI_SetIndicatorCommandDefinition =
{
	( const int8_t * ) "set_indicator", /* The command string to type. */
	( const int8_t * ) "set_indicator:\r\nParameters required to execute a SetIndicator: indicator number \r\n\r\n",
	CLI_SetIndicatorCommand, /* The function to run. */
	1 /* one parameters are expected. */
};


/* CLI command structure : ClearIndicator */
const CLI_Command_Definition_t CLI_ClearIndicatorCommandDefinition =
{
	( const int8_t * ) "clear_indicator", /* The command string to type. */
	( const int8_t * ) "clear_indicator:\r\nParameters required to execute a ClearIncicator: indicator number \r\n\r\n",
	CLI_ClearIndicatorCommand, /* The function to run. */
	1 /* one parameters are expected. */
};


/*-----------------------------------------------------------*/

/* -----------------------------------------------------------------------
 |												 Private Functions	 														|
 ----------------------------------------------------------------------- 
 */

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow : 
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 48000000
 *            HCLK(Hz)                       = 48000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 1
 *            HSE Frequency(Hz)              = 8000000
 *            PREDIV                         = 1
 *            PLLMUL                         = 6
 *            Flash Latency(WS)              = 1
 * @param  None
 * @retval None
 */
void SystemClock_Config(void){
	  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	  /** Configure the main internal regulator output voltage
	  */
	  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
	  /** Initializes the RCC Oscillators according to the specified parameters
	  * in the RCC_OscInitTypeDef structure.
	  */
	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
	  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
	  RCC_OscInitStruct.PLL.PLLN = 12;
	  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	  HAL_RCC_OscConfig(&RCC_OscInitStruct);

	  /** Initializes the CPU, AHB and APB buses clocks
	  */
	  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                              |RCC_CLOCKTYPE_PCLK1;
	  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	 HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

	  /** Initializes the peripherals clocks
	  */
	  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART2;
	  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);


	  HAL_NVIC_SetPriority(SysTick_IRQn,0,0);
	
}

/*-----------------------------------------------------------*/


/* --- Save array topology and Command Snippets in Flash RO --- 
 */
uint8_t SaveToRO(void){
	BOS_Status result =BOS_OK;
	HAL_StatusTypeDef FlashStatus =HAL_OK;
	uint16_t add =8, temp =0;
	uint8_t snipBuffer[sizeof(snippet_t) + 1] ={0};
	
	HAL_FLASH_Unlock();
	
	/* Erase RO area */
	FlashStatus =FLASH_WaitForLastOperation((uint32_t ) HAL_FLASH_TIMEOUT_VALUE);
	FLASH_PageErase(FLASH_BANK_1,RO_MID_ADDRESS);
	//TOBECHECKED
	FlashStatus =FLASH_WaitForLastOperation((uint32_t ) HAL_FLASH_TIMEOUT_VALUE);
	if(FlashStatus != HAL_OK){
		return pFlash.ErrorCode;
	}
	else{
		/* Operation is completed, disable the PER Bit */
		CLEAR_BIT(FLASH->CR,FLASH_CR_PER);
	}
	
	/* Save number of modules and myID */
	if(myID){
		temp =(uint16_t )(N << 8) + myID;
		//HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,RO_START_ADDRESS,temp);
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,RO_START_ADDRESS,temp);
		//TOBECHECKED
		FlashStatus =FLASH_WaitForLastOperation((uint32_t ) HAL_FLASH_TIMEOUT_VALUE);
		if(FlashStatus != HAL_OK){
			return pFlash.ErrorCode;
		}
		else{
			/* If the program operation is completed, disable the PG Bit */
			CLEAR_BIT(FLASH->CR,FLASH_CR_PG);
		}
		
		/* Save topology */
		for(uint8_t i =1; i <= N; i++){
			for(uint8_t j =0; j <= MaxNumOfPorts; j++){
				if(array[i - 1][0]){
                 HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,RO_START_ADDRESS + add,array[i - 1][j]);
				 //HALFWORD 	//TOBECHECKED
					
					FlashStatus =FLASH_WaitForLastOperation((uint32_t ) HAL_FLASH_TIMEOUT_VALUE);
					if(FlashStatus != HAL_OK){
						return pFlash.ErrorCode;
					}
					else{
						/* If the program operation is completed, disable the PG Bit */
						CLEAR_BIT(FLASH->CR,FLASH_CR_PG);
						add +=8;
					}
				}
			}
		}
	}
	
	// Save Command Snippets
	int currentAdd = RO_MID_ADDRESS;
	for(uint8_t s =0; s < numOfRecordedSnippets; s++){
		if(snippets[s].cond.conditionType){
			snipBuffer[0] =0xFE;		// A marker to separate Snippets
			memcpy((uint32_t* )&snipBuffer[1],(uint8_t* )&snippets[s],sizeof(snippet_t));
			// Copy the snippet struct buffer (20 x numOfRecordedSnippets). Note this is assuming sizeof(snippet_t) is even.
			for(uint8_t j =0; j < (sizeof(snippet_t)/4); j++){
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,currentAdd,*(uint64_t* )&snipBuffer[j*8]);
				//HALFWORD
				//TOBECHECKED
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
			// Copy the snippet commands buffer. Always an even number. Note the string termination char might be skipped
			for(uint8_t j =0; j < ((strlen(snippets[s].cmd) + 1)/4); j++){
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,currentAdd,*(uint64_t* )(snippets[s].cmd + j*4 ));
				//HALFWORD
				//TOBECHECKED
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
	
	HAL_FLASH_Lock();
	
	return result;
}

/* --- Clear array topology in SRAM and Flash RO --- 
 */
uint8_t ClearROtopology(void){
	// Clear the array 
	memset(array,0,sizeof(array));
	N =1;
	myID =0;
	
	return SaveToRO();
}
/*-----------------------------------------------------------*/

/* --- Trigger ST factory bootloader update for a remote module.
 */
void remoteBootloaderUpdate(uint8_t src,uint8_t dst,uint8_t inport,uint8_t outport){

	uint8_t myOutport =0, lastModule =0;
	int8_t *pcOutputString;

	/* 1. Get route to destination module */
	myOutport =FindRoute(myID,dst);
	if(outport && dst == myID){ /* This is a 'via port' update and I'm the last module */
		myOutport =outport;
		lastModule =myID;
	}
	else if(outport == 0){ /* This is a remote update */
		if(NumberOfHops(dst)== 1)
		lastModule = myID;
		else
		lastModule = route[NumberOfHops(dst)-1]; /* previous module = route[Number of hops - 1] */
	}

	/* 2. If this is the source of the message, show status on the CLI */
	if(src == myID){
		/* Obtain the address of the output buffer.  Note there is no mutual
		 exclusion on this buffer as it is assumed only one command console
		 interface will be used at any one time. */
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

/*-----------------------------------------------------------*/

/* --- Setup a port for remote ST factory bootloader update:
 - Set baudrate to 57600
 - Enable even parity
 - Set datasize to 9 bits
 */
void SetupPortForRemoteBootloaderUpdate(uint8_t port){
	UART_HandleTypeDef *huart =GetUart(port);

	huart->Init.BaudRate =57600;
	huart->Init.Parity = UART_PARITY_EVEN;
	huart->Init.WordLength = UART_WORDLENGTH_9B;
	HAL_UART_Init(huart);

	/* The CLI port RXNE interrupt might be disabled so enable here again to be sure */
	__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);
}

/* --- H3BR7 module initialization.
 */
void Module_Peripheral_Init(void){

	/* Array ports */
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();
	MX_USART5_UART_Init();
	MX_USART6_UART_Init();

	//Init a timer for 7-seg:
	MX_TIM6_Init();

	//seven segment GPIO Init:
	seven_seg_gpio_init();

	//Circulating DMA Channels ON All Module
	 for(int i=1;i<=NumOfPorts;i++)
		{
		  if(GetUart(i)==&huart1)
				   { index_dma[i-1]=&(DMA1_Channel1->CNDTR); }
		  else if(GetUart(i)==&huart2)
				   { index_dma[i-1]=&(DMA1_Channel2->CNDTR); }
		  else if(GetUart(i)==&huart3)
				   { index_dma[i-1]=&(DMA1_Channel3->CNDTR); }
		  else if(GetUart(i)==&huart4)
				   { index_dma[i-1]=&(DMA1_Channel4->CNDTR); }
		  else if(GetUart(i)==&huart5)
				   { index_dma[i-1]=&(DMA1_Channel5->CNDTR); }
		  else if(GetUart(i)==&huart6)
				   { index_dma[i-1]=&(DMA1_Channel6->CNDTR); }
		}


	/* Create module special task (if needed) */
}

/*-----------------------------------------------------------*/
/* --- H3BR7 message processing task.
 */
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
//	  case CODE_H3BR7_SevenDisplayNumber:
//	  Number=((int32_t )cMessage[port - 1][shift] ) + ((int32_t )cMessage[port - 1][1 + shift] << 8) + ((int32_t )cMessage[port - 1][2 + shift] << 16) + ((int32_t )cMessage[port - 1][3 + shift] << 24);
//	  StartSevSeg=(uint8_t)cMessage[port - 1][4+shift];
//	  SevenDisplayNumber(Number, StartSevSeg);
//	  break;
//
//	  case CODE_H3BR7_SevenDisplayNumberF:
//		  Number_int=((uint32_t) cMessage[port - 1][shift] + (uint32_t) (cMessage[port - 1][1+shift] <<8) + (uint32_t) (cMessage[port - 1][2+shift]<<16) + (uint32_t) (cMessage[port - 1][3+shift] <<24));
//		  NumberF = *((float*)&Number_int);
//		  Res=(uint8_t)cMessage[port - 1][4+shift];
//		  StartSevSeg=(uint8_t)cMessage[port - 1][5+shift];
//		  SevenDisplayNumberF(NumberF, Res, StartSevSeg);
//		  break;
//
//	  case CODE_H3BR7_SevenDisplayQuantities:
//		  Number_int=((uint32_t) cMessage[port - 1][shift] + (uint32_t) (cMessage[port - 1][1+shift] <<8) + (uint32_t) (cMessage[port - 1][2+shift]<<16) + (uint32_t) (cMessage[port - 1][3+shift] <<24));
//		  NumberF = *((float*)&Number_int);
//		  Res=(uint8_t)cMessage[port - 1][4+shift];
//		  Unit=(uint8_t)cMessage[port - 1][5+shift];
//    	  StartSevSeg=(uint8_t)cMessage[port - 1][6+shift];
//    	  SevenDisplayQuantities(NumberF, Res, Unit, StartSevSeg);
//		  break;
//
//	  case CODE_H3BR7_SevenDisplayLetter:
//		  StartSevSeg=(uint8_t)cMessage[port - 1][1+shift];
//		  SevenDisplayLetter((char)cMessage[port-1][shift], StartSevSeg);
//		  break;
//
//	  case CODE_H3BR7_SevenDisplaySentence:
//		  length=(uint8_t)cMessage[port - 1][shift];
//		  StartSevSeg=(uint8_t)cMessage[port - 1][1+shift];
//		  SevenDisplaySentence((char *)&cMessage[port-1][2 + shift], length, StartSevSeg);
//		  break;
//
//	  case CODE_H3BR7_SevenDisplayMovingSentence:
//		  length=(uint8_t)cMessage[port - 1][shift];
//		  SevenDisplayMovingSentence((char *)&cMessage[port-1][1 + shift], length);
//		  break;
//
//	  case CODE_H3BR7_SevenDisplayOff:
//		  SevenDisplayOff();
//		  break;
//
//	  case CODE_H3BR7_SetIndicator:
//		  indicator=(uint8_t)cMessage[port - 1][shift];
//		  SetIndicator(indicator);
//		  break;
//
//	  case CODE_H3BR7_ClearIndicator:
//		  indicator=(uint8_t)cMessage[port - 1][shift];
//		  ClearIndicator(indicator);
//		  break;
//	  default:
//			result =H3BR7_ERR_UnknownMessage;
//			break;
	}


	return result;
}

/*-----------------------------------------------------------*/
/* --- Get the port for a given UART. 
 */
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

/*-----------------------------------------------------------*/

/* --- Register this module CLI Commands
 */
void RegisterModuleCLICommands(void){
	    FreeRTOS_CLIRegisterCommand(&CLI_SevenDisplayNumberCommandDefinition);
		FreeRTOS_CLIRegisterCommand(&CLI_SevenDisplayNumberFCommandDefinition);
		FreeRTOS_CLIRegisterCommand(&CLI_SevenDisplayQuantitiesCommandDefinition);
		FreeRTOS_CLIRegisterCommand(&CLI_SevenDisplayLetterCommandDefinition);
		FreeRTOS_CLIRegisterCommand(&CLI_SevenDisplaySentenceCommandDefinition);
		FreeRTOS_CLIRegisterCommand(&CLI_SevenDisplayMovingSentenceCommandDefinition);
		FreeRTOS_CLIRegisterCommand(&CLI_SevenDisplayOffCommandDefinition);
		FreeRTOS_CLIRegisterCommand(&CLI_SetIndicatorCommandDefinition);
		FreeRTOS_CLIRegisterCommand(&CLI_ClearIndicatorCommandDefinition);



}

/*-----------------------------------------------------------*/


/* Module special task function (if needed) */
//void Module_Special_Task(void *argument){
//
//	/* Infinite loop */
//	uint8_t cases; // Test variable.
//	for(;;){
//		/*  */
//		switch(cases){
//
//
//			default:
//				osDelay(10);
//				break;
//		}
//
//		taskYIELD();
//	}
//
//}


/*-----------------------------------------------------------*/
Segment_Codes get_number_code(uint8_t digit)
{
	Module_Status status = H3BR7_OK;

	Segment_Codes code;
	switch(digit)
	{
	case 0:
		code = zero_number;
		break;
	case 1:
		code = one_number;
		break;
	case 2:
		code = two_number;
		break;
	case 3:
		code = three_number;
		break;
	case 4:
		code = four_number;
		break;
	case 5:
		code = five_number;
		break;
	case 6:
		code = six_number;
		break;
	case 7:
		code = seven_number;
		break;
	case 8:
		code = eight_number;
		break;
	case 9:
		code = nine_number;
		break;

	default:
		code = Empty;
		break;

	}
	return code;
}

/*-----------------------------------------------------------*/
Segment_Codes get_letter_code(char letter){
	Module_Status status = H3BR7_OK;

	Segment_Codes letter_code;
	switch(letter){

	case 'A':
		letter_code=A_letter;
		break;
	case 'B':
		letter_code=B_letter;
		break;
	case'C':
		letter_code=C_letter;
		break;
	case'D':
		letter_code=D_letter;
		break;
	case'E':
		letter_code=E_letter;
		break;
	case'F':
		letter_code=F_letter;
		break;
	case'G':
		letter_code=G_letter;
		break;
	case'H':
		letter_code=H_letter;
		break;
	case'I':
		letter_code=I_letter;
		break;
	case'J':
		letter_code=J_letter;
		break;
	case'K':
		letter_code=K_letter;
		break;
	case'L':
		letter_code=L_letter;
		break;
	case'M':
		letter_code=M_letter;
		break;
	case'N':
		letter_code=N_letter;
		break;
	case'O':
		letter_code=O_letter;
		break;
	case'P':
		letter_code=P_letter;
		break;
	case'Q':
		letter_code=Q_letter;
		break;
	case'R':
		letter_code=R_letter;
		break;
	case'S':
		letter_code=S_letter;
		break;
	case'T':
		letter_code=T_letter;
		break;
	case'U':
		letter_code=U_letter;
		break;
	case'V':
		letter_code=V_letter;
		break;
	case'W':
		letter_code=W_letter;
		break;
	case'X':
		letter_code=X_letter;
		break;
	case'Y':
		letter_code=Y_letter;
		break;
	case'Z':
		letter_code=Z_letter;
		break;

	case 'a':
		letter_code=a_letter;
		break;
	case 'b':
		letter_code=b_letter;
		break;
	case'c':
		letter_code=c_letter;
		break;
	case'd':
		letter_code=d_letter;
		break;
	case'e':
		letter_code=e_letter;
		break;
	case'f':
		letter_code=f_letter;
		break;
	case'g':
		letter_code=g_letter;
		break;
	case'h':
		letter_code=h_letter;
		break;
	case'i':
		letter_code=i_letter;
		break;
	case'j':
		letter_code=j_letter;
		break;
	case'k':
		letter_code=k_letter;
		break;
	case'l':
		letter_code=l_letter;
		break;
	case'm':
		letter_code=m_letter;
		break;
	case'n':
		letter_code=n_letter;
		break;
	case'o':
		letter_code=o_letter;
		break;
	case'p':
		letter_code=p_letter;
		break;
	case'q':
		letter_code=q_letter;
		break;
	case'r':
		letter_code=r_letter;
		break;
	case's':
		letter_code=s_letter;
		break;
	case't':
		letter_code=t_letter;
		break;
	case'u':
		letter_code=u_letter;
		break;
	case'v':
		letter_code=v_letter;
		break;
	case'w':
		letter_code=w_letter;
		break;
	case'x':
		letter_code=x_letter;
		break;
	case'y':
		letter_code=y_letter;
		break;
	case'z':
		letter_code=z_letter;
		break;

	default: break;

	}
	return letter_code;
}
/*-----------------------------------------------------------*/

Segment_Codes clear_all_digits(void){
	Module_Status status = H3BR7_OK;

	for(int i=0;i<7;i++) Digit[i] = Empty;
	Comma_flag = 0;
	Moving_sentence_flag = 0;
	Moving_sentence_counter = 0;

}

/*-----------------------------------------------------------*/

/* -----------------------------------------------------------------------
 |								  APIs							          | 																 	|
/* -----------------------------------------------------------------------
 */
Module_Status SevenDisplayNumber(int32_t Number, uint8_t StartSevSeg)
{
	Module_Status status = H3BR7_OK;
	clear_all_digits();   //Seven segment display off

	int32_t max_value, min_value;
	uint8_t index_digit_last; // the index of the last used digit int 7-segment.
	uint8_t signal = 0; //0 for Positive numbers, and 1 for negative numbers.
	uint8_t length;


	if( !(StartSevSeg >= 0 && StartSevSeg <= 5) )
	{
		status = H3BR7_ERR_WrongParams;
		return status;
	}


	switch(StartSevSeg)
	{
	case 0:
		max_value = 999999;
		min_value = -99999;
		break;
	case 1:
		max_value = 99999;
		min_value = -9999;
		break;
	case 2:
		max_value = 9999;
		min_value = -999;
		break;
	case 3:
		max_value = 999;
		min_value = -99;
		break;
	case 4:
		max_value = 99;
		min_value = -9;
		break;
	case 5:
		max_value = 9;
		min_value = 0;
		break;
		// Case 5 is a special case.
	default: break;
	}


	if(StartSevSeg == 5 && (Number < 0 || Number > 9) )
	{
		status = H3BR7_NUMBER_IS_OUT_OF_RANGE;
		return status;
	}
   if(Number > max_value || Number < min_value)
	{
		status = H3BR7_NUMBER_IS_OUT_OF_RANGE;
		return status;
	}

	if(Number < 0)
		{
			signal = 1;
			Number *= -1;
		}


			if(Number>0 && Number<=9)
			{
				length= 1;
			}

			if(Number>9 && Number<=99)
			{
				length= 2;
			}

			if(Number>99 && Number<=999)
			{
				length= 3;
			}

			if(Number>999 && Number<=9999)
			{
				length= 4;
			}

			if(Number>9999 && Number<=99999)
			{
				length= 5;
			}

			if(Number>99999 && Number<=999999)
			{
				length= 6;
			}

	index_digit_last = length+StartSevSeg;
	if(signal==1)
	{
		Digit[index_digit_last] = Symbol_minus;
		Digit[index_digit_last+1]=Empty;
	}

	for(int i = StartSevSeg; i < 6;i++)
	{
		if(i == index_digit_last && signal == 1) continue;
		Digit[i] = get_number_code(Number % 10);
		Number /= 10;
	}



		for(int x=index_digit_last; x<6; x++)
		{
			if(signal==1 && x==index_digit_last)continue;
			Digit[x]=Empty;
		}

	return status;

}
/* ----------------------------------------------------------------------------*/
Module_Status SevenDisplayNumberF(float NumberF,uint8_t Res,uint8_t StartSevSeg)
{
	Module_Status status = H3BR7_OK;
	clear_all_digits();   //Seven segment display off

	float max_value_comma;
	float min_value_comma;
	uint8_t index_digit_last;
	uint8_t signal = 0;
	uint32_t Number_int;
	uint8_t length;
    uint8_t zero_flag = 0;

    Res_it=Res;
    StartSevSeg_it=StartSevSeg;
    Comma_flag=1;



    if((uint32_t)NumberF == 0) zero_flag = 1;

    if( !(StartSevSeg >= 0 && StartSevSeg <= 5) )
    	{
    		status = H3BR7_ERR_WrongParams;
    		Comma_flag=0;

    		return status;
    	}


	switch(StartSevSeg){
	case 0:
	   max_value_comma=99999.9;
	   min_value_comma=-9999.9;
	   break;
	case 1:
		max_value_comma=9999.9;
		min_value_comma=-999.9;
		break;
	case 2:
		max_value_comma=999.9;
		min_value_comma=-99.9;
		break;
	case 3:
		max_value_comma=99.9;
		min_value_comma=-9.9;
		break;
	case 4:
		max_value_comma=9.9;
		min_value_comma=-9;
		break;

	case 5:
		max_value_comma=9;
		min_value_comma=0;
		break;
	default:
		break;

	}

	if(StartSevSeg==4 && ((NumberF<0 || NumberF>9.9) || (NumberF>0 || NumberF<0.9)))
	{
			status = H3BR7_NUMBER_IS_OUT_OF_RANGE;
			Comma_flag=0;
			return status;
	}

	if(StartSevSeg==5 && (NumberF>9 || NumberF<0))
	{
		   status = H3BR7_NUMBER_IS_OUT_OF_RANGE;
		    Comma_flag=0;
			return status;
	}

	 if(NumberF>max_value_comma || NumberF<min_value_comma)
	 {
		  status = H3BR7_NUMBER_IS_OUT_OF_RANGE;
		  Comma_flag=0;
		  return status;
	 }

	 if(NumberF < 0 )
	 {
		  signal = 1;
		  NumberF *= -1;
	 }


   switch(Res){
   case 0:
	   	Number_int=(uint32_t)NumberF;
	   	Comma_flag=0;
	    break;
   case 1:
	    if(StartSevSeg == 5 ) Number_int=(uint32_t)NumberF;
	    else Number_int=(uint32_t)(NumberF*10);
	   break;
   case 2:
	    if(StartSevSeg == 5 ) Number_int=(uint32_t)NumberF;
	    else Number_int=(uint32_t)(NumberF*100);
	   break;
   case 3:
	    if(StartSevSeg == 5 ) Number_int=(uint32_t)NumberF;
	    else Number_int=(uint32_t)(NumberF*1000);
   	   break;
   case 4:
	    if(StartSevSeg == 5 ) Number_int=(uint32_t)NumberF;
	    else Number_int=(uint32_t)(NumberF*10000);
   	   break;
   case 5:
	    if(StartSevSeg == 5 ) Number_int=(uint32_t)NumberF;
	    else Number_int=(uint32_t)(NumberF*100000);
   	   break;
   default:
   		break;


   }


   			if(Number_int>0 && Number_int<=9)
   			{
   				length= 1;
   			}

   			if(Number_int>9 && Number_int<=99)
   			{
   				length= 2;
   			}

   			if(Number_int>99 && Number_int<=999)
   			{
   				length= 3;
   			}

   			if(Number_int>999 && Number_int<=9999)
   			{
   				length= 4;
   			}

   			if(Number_int>9999 && Number_int<=99999)
   			{
   				length= 5;
   			}

   			if(Number_int>99999 && Number_int<=999999)
   			{
   				length= 6;
   			}


   			if(zero_flag == 0) index_digit_last = length + StartSevSeg;
   			else index_digit_last = Res + 1 + StartSevSeg;
   				if(signal==1)
   				{
   					Digit[index_digit_last] = Symbol_minus;
   					Digit[index_digit_last+1]=Empty;
   				}

   				for(int i = StartSevSeg; i < 6;i++)
   					{
   						if(i == index_digit_last && signal == 1) continue;
   						Digit[i] = get_number_code(Number_int % 10);
   						Number_int /= 10;
   					}

						for(int x=index_digit_last; x<6; x++)
						{
						if(signal==1 && x==index_digit_last)continue;
						Digit[x]=Empty;
						}
	return status;
}


/* ----------------------------------------------------------------------------*/
Module_Status SevenDisplayQuantities(float NumberF, uint8_t Res,char Unit ,uint8_t StartSevSeg){
	Module_Status status = H3BR7_OK;
	clear_all_digits();   //Seven segment display off

		float max_value_comma;
		float min_value_comma;
		uint8_t index_digit_last;
		uint8_t signal = 0;
		uint32_t Number_int;
		uint8_t length;
		uint8_t zero_flag = 0;

		Res_it=Res;
		StartSevSeg_it=StartSevSeg+1;
		Comma_flag=1;


	    if((uint32_t)NumberF == 0) zero_flag = 1;

	    if( !(StartSevSeg >= 0 && StartSevSeg <= 5) )
	    	{
	    		status = H3BR7_ERR_WrongParams;
	    		Comma_flag=0;
	    		return status;
	    	}

	    switch(StartSevSeg){
	    		case 0:
	    		    max_value_comma=9999.9;
	    		    min_value_comma=-999.9;
	    		   break;
	    		case 1:
	    			max_value_comma=999.9;
	    			min_value_comma=-99.9;
	    			break;
	    		case 2:
	    			max_value_comma=99.9;
	    			min_value_comma=-9.9;
	    			break;
	    		case 3:
	    			max_value_comma=9.9;
	    			min_value_comma=0.9;
	    			break;
	    		case 4:
	    			max_value_comma=9;
	    			min_value_comma=0;
	    			break;

	    		case 5:
	    			// Case 5 is a special case.

	    			break;
	    		default:
	    			break;

	    		}

	    if(StartSevSeg==5)
	    {
			status = H3BR7_NUMBER_IS_OUT_OF_RANGE;
			Comma_flag=0;
			return status;
	    }



		if(NumberF>max_value_comma || NumberF<min_value_comma)
		{
			status = H3BR7_NUMBER_IS_OUT_OF_RANGE;
			Comma_flag=0;
			return status;
		}

		if(NumberF < 0 )
		{
			signal = 1;
			NumberF *= -1;
		}

		switch(Res){
		   case 0:
			   	Number_int=(uint32_t)NumberF;
			   	Comma_flag=0;
			    break;
		   case 1:
			    if(StartSevSeg == 5 ) Number_int=(uint32_t)NumberF;
			    else Number_int=(uint32_t)(NumberF*10);
			   break;
		   case 2:
			    if(StartSevSeg == 5 ) Number_int=(uint32_t)NumberF;
			    else Number_int=(uint32_t)(NumberF*100);
			   break;
		   case 3:
			    if(StartSevSeg == 5 ) Number_int=(uint32_t)NumberF;
			    else Number_int=(uint32_t)(NumberF*1000);
		   	   break;
		   case 4:
			    if(StartSevSeg == 5 ) Number_int=(uint32_t)NumberF;
			    else Number_int=(uint32_t)(NumberF*10000);
		   	   break;
		   case 5:
			    if(StartSevSeg == 5 ) Number_int=(uint32_t)NumberF;
			    else Number_int=(uint32_t)(NumberF*100000);
		   	   break;
		   default:
		   		break;


		   }



	   			if(Number_int>=0 && Number_int<=9)
	   			{
	   				length= 1;
	   			}

	   			if(Number_int>9 && Number_int<=99)
	   			{
	   				length= 2;
	   			}

	   			if(Number_int>99 && Number_int<=999)
	   			{
	   				length= 3;
	   			}

	   			if(Number_int>999 && Number_int<=9999)
	   			{
	   				length= 4;
	   			}

	   			if(Number_int>9999 && Number_int<=99999)
	   			{
	   				length= 5;
	   			}

	   			if(Number_int>99999 && Number_int<=999999)
	   			{
	   				length= 6;
	   			}





	   			if(zero_flag == 0) index_digit_last = length + StartSevSeg+1;
	   			 else index_digit_last = Res + 2 + StartSevSeg;
	   				if(signal==1)
	   				{
	   					Digit[index_digit_last] = Symbol_minus;
	   					Digit[index_digit_last+1]=Empty;
	   				}

					Digit[StartSevSeg]=get_letter_code(Unit);

	   				for(int i = StartSevSeg+1; i < 6;i++)
	   					{
	   						if(i == index_digit_last && signal == 1) continue;

	   						Digit[i] = get_number_code(Number_int % 10);
	   						Number_int /= 10;
	   					}



	   				for(int x=index_digit_last; x<6; x++)
	   				{
						if(signal==1 && x==index_digit_last)continue;
						Digit[x]=Empty;
					}



		return status;

}
/* ----------------------------------------------------------------------------*/
Module_Status SevenDisplayLetter(char letter, uint8_t StartSevSeg){
	Module_Status status = H3BR7_OK;
	clear_all_digits();   //Seven segment display off

	if( !(StartSevSeg >= 0 && StartSevSeg <= 5) )
		{
		    status = H3BR7_ERR_WrongParams;
		    return status;
		 }

	 Digit[StartSevSeg]=get_letter_code(letter);


return status;


}

/* ----------------------------------------------------------------------------*/
Module_Status SevenDisplaySentence(char *Sentence,uint16_t length,uint8_t StartSevSeg){
	Module_Status status = H3BR7_OK;
	clear_all_digits();   //Seven segment display off

	uint16_t max_length;
	char letter;

	if(length == 0 || Sentence == NULL)
	{
		status = H3BR7_ERROR;
		return status;
	}


	switch(StartSevSeg){
		case 0:
			max_length=6;
			break;
		case 1:
			max_length=5;
			break;
		case 2:
			max_length=4;
			break;
		case 3:
			max_length=3;
			break;
		case 4:
			max_length=2;
		case 5:
			max_length=1;
		default:
			break;

	}


		if(length>max_length)
		{
			status=H3BR7_Out_Of_Range;
			return status;
		}

		for(int x=length - 1; x>=0; x--)
		{
			letter=Sentence[x];

			if( (Sentence[x] >= 'a' && Sentence[x] <= 'z') ||
					(Sentence[x] >= 'A' && Sentence[x] <= 'Z') )
			{
				Digit[StartSevSeg] = get_letter_code(letter);
			}

			else if(Sentence[x] >='0' && Sentence[x] <= '9')
			{
				Digit[StartSevSeg]  = get_number_code(letter - '0');
			}
			StartSevSeg++;
		}




	return status;

}
/*-----------------------------------------------------------*/
Module_Status SevenDisplayMovingSentence(char *Sentence,uint16_t length){

	Module_Status status = H3BR7_OK;
	clear_all_digits();   //Seven segment display off

	if(length == 0 || Sentence == NULL)
	{
		status = H3BR7_ERROR;
		return status;
	}

	if(length <= MOVING_SENTENCE_MAX_LENGTH)
	{
		Moving_sentence_index = 0;		Moving_sentence_flag = 1;
		Moving_sentence_length = length + 6;

		for(int i=0;i<6;i++) Moving_sentence_buffer[i] = Empty;

		for(int i=0;i<length;i++)
		{
			if( (Sentence[i] >= 'a' && Sentence[i] <= 'z') ||
					(Sentence[i] >= 'A' && Sentence[i] <= 'Z') )
			{
				Moving_sentence_buffer[i+6] = get_letter_code(Sentence[i]);
			}

			else if(Sentence[i] >='0' && Sentence[i] <= '9')
			{
				Moving_sentence_buffer[i+6] = get_number_code(Sentence[i] - '0');
			}

			else
			{
				Moving_sentence_buffer[i+6] = Empty;
			}
		}
	}

	else
	{
		status = H3BR7_Out_Of_Range;
		return status;
	}

	return status;

}
/*-----------------------------------------------------------*/
Module_Status SevenDisplayOff(void){

	Module_Status status = H3BR7_OK;
	clear_all_digits();   //Seven segment display off
	return status;

}
/*-----------------------------------------------------------*/
Module_Status SetIndicator( IndicatorLED indicator ){

	Module_Status status = H3BR7_OK;

    if(indicator==Ind1)
    {
    	HAL_GPIO_WritePin(Led_Indicator1_GPIO_Port, Led_Indicator1_Pin, GPIO_PIN_RESET);
	}

	else if(indicator==Ind2)
	{
		HAL_GPIO_WritePin(Led_Indicator2_GPIO_Port, Led_Indicator2_Pin, GPIO_PIN_RESET);
	}

	else if(indicator==Ind3)
	{
		HAL_GPIO_WritePin(Led_Indicator3_GPIO_Port, Led_Indicator3_Pin, GPIO_PIN_RESET);
	}

	else if(indicator==Ind4)
	{
		HAL_GPIO_WritePin(Led_Indicator4_GPIO_Port, Led_Indicator4_Pin, GPIO_PIN_RESET);
	}

	else
	{
		status=H3BR7_ERROR;
		return status;
	}

	return status;

}
/*-----------------------------------------------------------*/
Module_Status ClearIndicator(IndicatorLED  indicator){

	Module_Status status = H3BR7_OK;


    if(indicator==Ind1)
    {
    	HAL_GPIO_WritePin(Led_Indicator1_GPIO_Port, Led_Indicator1_Pin, GPIO_PIN_SET);
	}

	else if(indicator==Ind2)
	{
		HAL_GPIO_WritePin(Led_Indicator2_GPIO_Port, Led_Indicator2_Pin, GPIO_PIN_SET);
	}

	else if(indicator==Ind3)
	{
		HAL_GPIO_WritePin(Led_Indicator3_GPIO_Port, Led_Indicator3_Pin, GPIO_PIN_SET);
	}

	else if(indicator==Ind4)
	{
		HAL_GPIO_WritePin(Led_Indicator4_GPIO_Port, Led_Indicator4_Pin, GPIO_PIN_SET);
	}

	else
	{
		status=H3BR7_ERROR;
		return status;
	}


	return status;

}
/*-----------------------------------------------------------*/


/* -----------------------------------------------------------------------
 |								Commands							      |
   -----------------------------------------------------------------------
 */
portBASE_TYPE CLI_SevenDisplayNumberCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ){
	Module_Status status = H3BR7_OK;
	int32_t Number;
	uint8_t StartSevSeg;
	static int8_t *pcParameterString1;
	static int8_t *pcParameterString2;
	portBASE_TYPE xParameterStringLength1 =0;
	portBASE_TYPE xParameterStringLength2 =0;

	static const int8_t *pcOKMessage=(int8_t* )"SevenSegmentDisplay is on:\r\n %d  \n\r";
	static const int8_t *pcWrongParamsMessage =(int8_t* )"Wrong Params!\n\r";
	static const int8_t *pcWrongRangeMessage =(int8_t* )"Number is out of range!\n\r";


	(void )xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	pcParameterString1 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength1 );
	Number =(int32_t )atol((char* )pcParameterString1);

	pcParameterString2 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameterStringLength2 );
	StartSevSeg =(uint8_t )atol((char* )pcParameterString2);

	status=SevenDisplayNumber(Number,StartSevSeg);

	if(status == H3BR7_OK)
	{
		sprintf((char* )pcWriteBuffer,(char* )pcOKMessage,Number);

	}

	else if(status == H3BR7_ERR_WrongParams)
		strcpy((char* )pcWriteBuffer,(char* )pcWrongParamsMessage);

	else if(status == H3BR7_NUMBER_IS_OUT_OF_RANGE)
		strcpy((char* )pcWriteBuffer,(char* )pcWrongRangeMessage);


	return pdFALSE;
}

/* ----------------------------------------------------------------------------*/
portBASE_TYPE CLI_SevenDisplayNumberFCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ){
	Module_Status status = H3BR7_OK;
	float NumberF=0;
	uint8_t Res;
	uint8_t StartSevSeg;


	static int8_t *pcParameterString1;
	static int8_t *pcParameterString2;
	static int8_t *pcParameterString3;

	portBASE_TYPE xParameterStringLength1 =0;
	portBASE_TYPE xParameterStringLength2 =0;
	portBASE_TYPE xParameterStringLength3 =0;


	static const int8_t *pcOKMessage=(int8_t* )"SevenSegmentDisplay is on :\r\n %f \n\r";
	static const int8_t *pcWrongParamsMessage =(int8_t* )"Wrong Params!\n\r";
	static const int8_t *pcWrongRangeMessage =(int8_t* )"Number is out of range!\n\r";


	(void )xWriteBufferLen;
	configASSERT(pcWriteBuffer);


	 pcParameterString1 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength1 );
	 NumberF =(float )atof((char* )pcParameterString1);

	 pcParameterString2 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameterStringLength2 );
	 Res =(uint8_t )atol((char* )pcParameterString2);

	 pcParameterString3 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 3, &xParameterStringLength3 );
	 StartSevSeg =(uint8_t )atol((char* )pcParameterString3);

	 status=SevenDisplayNumberF(NumberF, Res, StartSevSeg);

	 if(status == H3BR7_OK)
	 {
			sprintf((char* )pcWriteBuffer,(char* )pcOKMessage,NumberF);

	 }

	 else if(status == H3BR7_ERR_WrongParams)
	 		strcpy((char* )pcWriteBuffer,(char* )pcWrongParamsMessage);

	 else if(status == H3BR7_NUMBER_IS_OUT_OF_RANGE)
	 		strcpy((char* )pcWriteBuffer,(char* )pcWrongRangeMessage);


		return pdFALSE;




}
/* ----------------------------------------------------------------------------*/
portBASE_TYPE CLI_SevenDisplayQuantitiesCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ){
	Module_Status status = H3BR7_OK;

	float NumberF=0;
	uint8_t Res;
	char Unit ;
	uint8_t StartSevSeg;


	static int8_t *pcParameterString1;
	static int8_t *pcParameterString2;
	static int8_t *pcParameterString3;
	static int8_t *pcParameterString4;

	portBASE_TYPE xParameterStringLength1 =0;
	portBASE_TYPE xParameterStringLength2 =0;
	portBASE_TYPE xParameterStringLength3 =0;
	portBASE_TYPE xParameterStringLength4 =0;

	static const int8_t *pcOKMessage=(int8_t* )"SevenSegmentDisplay is on:\r\n%f %c";
	static const int8_t *pcWrongParamsMessage =(int8_t* )"Wrong Params!\n\r";
	static const int8_t *pcWrongRangeMessage =(int8_t* )"Number is out of range!\n\r";


	(void )xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	 pcParameterString1 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength1 );
	 NumberF =(float )atof((char* )pcParameterString1);

	 pcParameterString2 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameterStringLength2 );
	 Res =(uint8_t )atol((char* )pcParameterString2);

	 pcParameterString3 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 3, &xParameterStringLength3 );
	 if(xParameterStringLength3 == 1)
	 	{
	 		Unit = pcParameterString3[0];
	 		if (! ((Unit >= 'a' && Unit <= 'z') || (Unit >= 'A' && Unit <= 'Z') ))
	 		{
	 			 status=H3BR7_ERR_WrongParams;
	 		}

	 	}
	 	else
	 	{
	 		 status=H3BR7_ERR_WrongParams;
	 	}

	 pcParameterString4 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 4, &xParameterStringLength4 );
	 StartSevSeg =(uint8_t )atol((char* )pcParameterString4);

	 status=SevenDisplayQuantities(NumberF, Res, Unit, StartSevSeg);

	 if(status == H3BR7_OK)
	 {
		 sprintf((char* )pcWriteBuffer,(char* )pcOKMessage,NumberF,Unit);

	 }

	 else if(status == H3BR7_ERR_WrongParams)
			strcpy((char* )pcWriteBuffer,(char* )pcWrongParamsMessage);

	 else if(status == H3BR7_NUMBER_IS_OUT_OF_RANGE)
		 	strcpy((char* )pcWriteBuffer,(char* )pcWrongRangeMessage);



	return pdFALSE;

}

/* ----------------------------------------------------------------------------*/
portBASE_TYPE CLI_SevenDisplayLetterCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ){
	Module_Status status = H3BR7_OK;

	char letter=0;
	uint8_t StartSevSeg=0;

	static int8_t *pcParameterString1;
	static int8_t *pcParameterString2;

	portBASE_TYPE xParameterStringLength1 =0;
	portBASE_TYPE xParameterStringLength2 =0;

	static const int8_t *pcOKMessage=(int8_t* )"SevenSegmentDisplay is on: \r\n %c";
	static const int8_t *pcWrongParamsMessage =(int8_t* )"WrongParams!\n\r";
	(void )xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	 pcParameterString1 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength1 );

	if(xParameterStringLength1 == 1)
	{
		letter = pcParameterString1[0];
		if (! ((letter >= 'a' && letter <= 'z') || (letter >= 'A' && letter <= 'Z') ))
		{
			 status=H3BR7_ERR_WrongParams;
		}

	}
	else
	{
		 status=H3BR7_ERR_WrongParams;
	}

	 pcParameterString2 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameterStringLength2 );
	 StartSevSeg =(uint8_t )atol((char* )pcParameterString2);

	 status=SevenDisplayLetter((char)letter, StartSevSeg);

	 if(status == H3BR7_OK)
	 {
		 sprintf((char* )pcWriteBuffer,(char* )pcOKMessage,letter);

	 }

	 else if(status == H3BR7_ERR_WrongParams)
	 {
		strcpy((char* )pcWriteBuffer,(char* )pcWrongParamsMessage);
	 }


	return pdFALSE;

}

/* ----------------------------------------------------------------------------*/
portBASE_TYPE CLI_SevenDisplaySentenceCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ){
	Module_Status status = H3BR7_OK;
//	char Sentance[6] = {0};
	char* Sentence = NULL;
//	const int8_t* ptr;
	uint8_t StartSevSeg;


	static int8_t *pcParameterString1;
	static int8_t *pcParameterString2;

	portBASE_TYPE xParameterStringLength1 =0;
	portBASE_TYPE xParameterStringLength2 =0;


	static const int8_t *pcOKMessage=(int8_t* )"SevenSegmentDisplay is on:\r\n %s \n\r";
	static const int8_t *pcErrorParamsMessage =(int8_t* )"Error Params!\n\r";
	static const int8_t *pcWrongRangeMessage =(int8_t* )"Number is out of range!\n\r";

	(void )xWriteBufferLen;
	configASSERT(pcWriteBuffer);





//	 pcParameterString1 =(char *)FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength1 );
//	 length =(uint16_t )atol((char* )pcParameterString1);


	 pcParameterString1 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength1 );
	 StartSevSeg =(uint8_t )atol((char* )pcParameterString1);

	 pcParameterString2 =(int8_t* )FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameterStringLength2 );
	 Sentence =(char* )pcParameterString2;


//	 ptr = pcCommandString;
//	 for (int i=0;i< 25 + xParameterStringLength1 + xParameterStringLength2;i++) ptr++;
//	 int i =0;
//	 while(*ptr != 0x00)
//	 {
//		 Sentance[i] = *ptr;
//		 i++;
//		 ptr++;
//	 }

	 status=SevenDisplaySentence(Sentence, xParameterStringLength2, StartSevSeg);


	 if(status == H3BR7_OK)
	 {
		    sprintf((char* )pcWriteBuffer,(char* )pcOKMessage,Sentence);
	 }
	 else if(status == H3BR7_ERROR)
			strcpy((char* )pcWriteBuffer,(char* )pcErrorParamsMessage);

	 else if(status == H3BR7_Out_Of_Range)
			strcpy((char* )pcWriteBuffer,(char* )pcWrongRangeMessage);


	return pdFALSE;


}

/* ----------------------------------------------------------------------------*/
portBASE_TYPE CLI_SevenDisplayMovingSentenceCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ){
	Module_Status status = H3BR7_OK;
//	char Sentance[MOVING_SENTENCE_MAX_LENGTH] = {0};
	char* Sentence = NULL;
//	const int8_t* ptr;
	uint16_t length;

	static int8_t *pcParameterString1;

	portBASE_TYPE xParameterStringLength1 =0;


	static const int8_t *pcOKMessage=(int8_t* )"SevenSegmentDisplay is on:\r\n %s \n\r";
	static const int8_t *pcWrongParamsMessage =(int8_t* )"Wrong Params!\n\r";
	static const int8_t *pcWrongRangeMessage =(int8_t* )"Number is out of range!\n\r";

	(void )xWriteBufferLen;
	configASSERT(pcWriteBuffer);





//	 pcParameterString1 =(char *)FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength1 );
//	 length =(uint16_t )atol((char* )pcParameterString1);


	 pcParameterString1 =(char *)FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength1 );
	 Sentence = (char* )pcParameterString1;


//	 ptr = pcCommandString;
//	 for (int i=0;i< 25 + 7 + xParameterStringLength1;i++) ptr++;
//	 int i =0;
//	 while(*ptr != 0x00)
//	 {
//		 Sentance[i] = *ptr;
//		 i++;
//		 ptr++;
//	 }

	 status=SevenDisplayMovingSentence(Sentence, xParameterStringLength1);


	 if(status == H3BR7_OK)
	 {
		 sprintf((char* )pcWriteBuffer,(char* )pcOKMessage,Sentence);

	 }

	 else if(status == H3BR7_ERR_WrongParams)
		strcpy((char* )pcWriteBuffer,(char* )pcWrongParamsMessage);

	 else if(status == H3BR7_Out_Of_Range)
		strcpy((char* )pcWriteBuffer,(char* )pcWrongRangeMessage);


	return pdFALSE;
}

/* ----------------------------------------------------------------------------*/
portBASE_TYPE CLI_SevenDisplayOffCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString ){
	Module_Status status = H3BR7_OK;

	static const int8_t *pcOKMessage=(int8_t* )"SevenSegmentDisplay is off \n\r";
	static const int8_t *pcErrorsMessage =(int8_t* )"Error Params!\n\r";

		(void )xWriteBufferLen;
		configASSERT(pcWriteBuffer);

	 	status=SevenDisplayOff();

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

	static const int8_t *pcOKMessage=(int8_t* )"SetIndicator is on \r\n  \n\r";
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

	static const int8_t *pcOKMessage=(int8_t* )"SetIndicator is on \r\n  \n\r";
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
