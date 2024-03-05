/*
 BitzOS (BOS) V0.3.1 - Copyright (C) 2017-2024 Hexabitz
 All rights reserved
 
 File Name     : H3BR7.h
 Description   : Header file for module H3BR7.
 	 	 	 	 (Description_of_module)

(Description of Special module peripheral configuration):
>>
>>
>>

 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef H3BR7_H
#define H3BR7_H

/* Includes ------------------------------------------------------------------*/
#include "BOS.h"
#include "H3BR7_MemoryMap.h"
#include "H3BR7_uart.h"
#include "H3BR7_gpio.h"
#include "H3BR7_dma.h"
#include "H3BR7_inputs.h"
#include "H3BR7_eeprom.h"
/* Exported definitions -------------------------------------------------------*/

#define	modulePN		_H3BR7


/* Port-related definitions */
#define	NumOfPorts			5

#define P_PROG 				P2						/* ST factory bootloader UART */

/* Define available ports */
#define _P1 
#define _P2 
#define _P3 
#define _P4 
#define _P5 


/* Define available USARTs */
#define _Usart1 1
#define _Usart2 1
#define _Usart3 1
#define _Usart5 1
#define _Usart6	1


/* Port-UART mapping */

#define P1uart &huart6
#define P2uart &huart2
#define P3uart &huart3
#define P4uart &huart1
#define P5uart &huart5


/* Port Definitions */
#define	USART1_TX_PIN		GPIO_PIN_9
#define	USART1_RX_PIN		GPIO_PIN_10
#define	USART1_TX_PORT		GPIOA
#define	USART1_RX_PORT		GPIOA
#define	USART1_AF			GPIO_AF1_USART1

#define	USART2_TX_PIN		GPIO_PIN_2
#define	USART2_RX_PIN		GPIO_PIN_3
#define	USART2_TX_PORT		GPIOA
#define	USART2_RX_PORT		GPIOA
#define	USART2_AF			GPIO_AF1_USART2

#define	USART3_TX_PIN		GPIO_PIN_10
#define	USART3_RX_PIN		GPIO_PIN_11
#define	USART3_TX_PORT		GPIOB
#define	USART3_RX_PORT		GPIOB
#define	USART3_AF			GPIO_AF4_USART3



#define	USART5_TX_PIN		GPIO_PIN_3
#define	USART5_RX_PIN		GPIO_PIN_2
#define	USART5_TX_PORT		GPIOD
#define	USART5_RX_PORT		GPIOD
#define	USART5_AF			GPIO_AF3_USART5

#define	USART6_TX_PIN		GPIO_PIN_8
#define	USART6_RX_PIN		GPIO_PIN_9
#define	USART6_TX_PORT		GPIOB
#define	USART6_RX_PORT		GPIOB
#define	USART6_AF			GPIO_AF8_USART6

/* Module-specific Definitions */

#define NUM_MODULE_PARAMS						1


/*  Pins For SevenSegment*/
#define Seven_seg_a_Pin 				GPIO_PIN_4
#define Seven_seg_a_GPIO_Port 			GPIOA

#define Seven_seg_b_Pin 				GPIO_PIN_5
#define Seven_seg_b_GPIO_Port 			GPIOA

#define Seven_seg_c_Pin 				GPIO_PIN_2
#define Seven_seg_c_GPIO_Port	 		GPIOB

#define Seven_seg_d_Pin 				GPIO_PIN_1
#define Seven_seg_d_GPIO_Port 			GPIOB

#define Seven_seg_e_Pin 				GPIO_PIN_0
#define Seven_seg_e_GPIO_Port 			GPIOB

#define Seven_seg_f_Pin 				GPIO_PIN_7
#define Seven_seg_f_GPIO_Port 			GPIOA

#define Seven_seg_g_Pin 				GPIO_PIN_6
#define Seven_seg_g_GPIO_Port 			GPIOA

#define Seven_seg_DP_Pin 				GPIO_PIN_12
#define Seven_seg_DP_GPIO_Port 			GPIOB

/* Enable Pin For SevenSegment*/
#define Seven_seg_Enable_1_Pin 			GPIO_PIN_0
#define Seven_seg_Enable_1_GPIO_Port 	GPIOD

#define Seven_seg_Enable_2_Pin 			GPIO_PIN_1
#define Seven_seg_Enable_2_GPIO_Port 	GPIOD

#define Seven_seg_Enable_3_Pin 			GPIO_PIN_3
#define Seven_seg_Enable_3_GPIO_Port 	GPIOB

#define Seven_seg_Enable_4_Pin 			GPIO_PIN_6
#define Seven_seg_Enable_4_GPIO_Port 	GPIOB

#define Seven_seg_Enable_5_Pin 			GPIO_PIN_1
#define Seven_seg_Enable_5_GPIO_Port 	GPIOA

#define Seven_seg_Enable_6_Pin 			GPIO_PIN_0
#define Seven_seg_Enable_6_GPIO_Port 	GPIOA
/*  Pins For LEDs Indicator */

#define Led_Indicator1_Pin 			    GPIO_PIN_5
#define Led_Indicator1_GPIO_Port 	    GPIOB

#define Led_Indicator2_Pin 			    GPIO_PIN_4
#define Led_Indicator2_GPIO_Port 	    GPIOB

#define Led_Indicator3_Pin 			    GPIO_PIN_14
#define Led_Indicator3_GPIO_Port 	    GPIOB

#define Led_Indicator4_Pin 			    GPIO_PIN_13
#define Led_Indicator4_GPIO_Port 	    GPIOB

/* Module EEPROM Variables */

// Module Addressing Space 500 - 599
#define _EE_MODULE							500		

//* Module_Status Type Definition */
typedef enum {
	H3BR7_OK =0,
	H3BR7_ERR_UnknownMessage,
	H3BR7_ERR_WrongParams,
	H3BR7_NUMBER_IS_OUT_OF_RANGE, // Longer than 6 Digits
	H3BR7_Out_Of_Range,
	H3BR7_ERROR =255
} Module_Status;


typedef enum{
	zero_number=0x3f, one_number=0x06, two_number=0x5b, three_number=0x4f, four_number=0x66, five_number=0x6d, six_number=0x7d, seven_number=0x07, eight_number=0x7f ,nine_number=0x6f,

	a_letter=0x5f, b_letter=0x7C, c_letter=0x58, d_letter=0x5E, e_letter=0x79, f_letter=0x71, g_letter=0x6f, h_letter=0x74, i_letter=0x10, j_letter=0x1E,k_letter=0x75,

	l_letter=0x38  , m_letter=0x37,n_letter=0x54, o_letter=0x5C, p_letter=0x73, q_letter=0x67, r_letter=0x50, s_letter=0x6c, t_letter=0x78, u_letter=0x1c,

	v_letter=0x3e, w_letter=0x7e ,x_letter=0x76,y_letter=0x6E, z_letter=0x1B,

	A_letter=0x77, B_letter=0x7C , C_letter=0x39 , D_letter=0x5E, E_letter=0x79 , F_letter=0x71 , G_letter=0x3d , H_letter=0x74, I_letter=0x10 , J_letter=0x1E ,K_letter=0x75,

	L_letter=0x38  , M_letter=0x37,N_letter=0x54 , O_letter=0x5C , P_letter=0x73 ,  Q_letter=0x67 , R_letter=0x50,  S_letter=0x6c  ,T_letter=0x78, U_letter=0x1c,

	V_letter=0x3e ,W_letter=0x7e , X_letter=0x76 ,Y_letter=0x6E , Z_letter=0x1B  ,

	Empty = 0x00,

	Symbol_minus=0x40

} Segment_Codes;

typedef enum{
	Ind1=1,
	Ind2,
	Ind3,
	Ind4,
} IndicatorLED;

/* -------------------------------------------------------------------------------*\
 */
extern uint8_t Res_it;
extern uint8_t StartSevSeg_it;
extern int Comma_flag;


#define MOVING_SENTENCE_MAX_LENGTH 100
#define MOVING_SENTENCE_COUNTER_OVERFLOW 95 // moving time: 300 ms (500 / 3.145)

extern uint8_t  Moving_sentence_buffer[MOVING_SENTENCE_MAX_LENGTH + 6];
extern uint8_t  Moving_sentence_length;
extern uint8_t  Moving_sentence_flag;
extern uint32_t Moving_sentence_counter;
extern uint8_t  Moving_sentence_index;



extern Segment_Codes Digit[7]; //Digit[0]: LSD, Digit[5]: MSD
extern Segment_Codes get_number_code(uint8_t digit);
extern Segment_Codes get_letter_code(char letter);
extern Segment_Codes clear_all_digits(void);

/* Indicator LED */
#define _IND_LED_PORT			GPIOB
#define _IND_LED_PIN			GPIO_PIN_7

/* Export UART variables */
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart6;

/* Define UART Init prototypes */
extern void MX_USART1_UART_Init(void);
extern void MX_USART2_UART_Init(void);
extern void MX_USART3_UART_Init(void);
extern void MX_USART5_UART_Init(void);
extern void MX_USART6_UART_Init(void);
extern void SystemClock_Config(void);
extern void ExecuteMonitor(void);

/* -----------------------------------------------------------------------
 |								  APIs							          |  																 	|
/* -----------------------------------------------------------------------
 */
extern Module_Status SevenDisplayNumber(int32_t Number, uint8_t StartSevSeg);
extern Module_Status SevenDisplayNumberF(float NumberF,uint8_t Res,uint8_t StartSevSeg);
extern Module_Status SevenDisplayQuantities(float NumberF, uint8_t Res,char Unit ,uint8_t StartSevSeg);
extern Module_Status SevenDisplayLetter(char letter , uint8_t StartSevSeg);
extern Module_Status SevenDisplaySentence(char *Sentance,uint16_t length,uint8_t StartSevSeg);
extern Module_Status SevenDisplayMovingSentence(char *Sentance,uint16_t length);
extern Module_Status SevenDisplayOff(void);
extern Module_Status SetIndicator(IndicatorLED indicator );
extern Module_Status ClearIndicator(IndicatorLED  indicator);

void SetupPortForRemoteBootloaderUpdate(uint8_t port);
void remoteBootloaderUpdate(uint8_t src,uint8_t dst,uint8_t inport,uint8_t outport);

/* -----------------------------------------------------------------------
 |								Commands							      |															 	|
/* -----------------------------------------------------------------------
 */
extern const CLI_Command_Definition_t CLI_SevenDisplayNumberCommandCommandDefinition;
extern const CLI_Command_Definition_t CLI_SevenDisplayNumberFCommandDefinition;
extern const CLI_Command_Definition_t CLI_SevenDisplayQuantitiesCommandDefinition;
extern const CLI_Command_Definition_t CLI_SevenDisplayLetterCommandDefinition;
extern const CLI_Command_Definition_t CLI_SevenDisplaySentanceCommandDefinition;
extern const CLI_Command_Definition_t CLI_SevenDisplayMovingSentanceCommandDefinition;
extern const CLI_Command_Definition_t CLI_SevenDisplayOffCommandDefinition;



#endif /* H3BR7_H */

/************************ (C) COPYRIGHT HEXABITZ *****END OF FILE****/
