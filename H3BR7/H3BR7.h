/*
 BitzOS (BOS) V0.3.6 - Copyright (C) 2017-2024 Hexabitz
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

/* Exported Macros *********************************************************/
#define	MODULE_PN		_H3BR6

/* Port-related Definitions */
#define	NUM_OF_PORTS	5
#define P_PROG 			P2		/* ST factory bootloader UART */

/* Define available ports */
#define _P1
#define _P2
#define _P3
#define _P4
#define _P5

/* Define Available USARTs */
#define _USART1
#define _USART2
#define _USART3
#define _USART5
#define _USART6

/* Port-UART mapping */
#define UART_P1 &huart6
#define UART_P2 &huart2
#define UART_P3 &huart3
#define UART_P4 &huart1
#define UART_P5 &huart5

/* Module-specific Hardware Definitions ************************************/
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

/* Pins For Seven Segment*/
#define SEVEN_SEG_A_PIN 				GPIO_PIN_4
#define SEVEN_SEG_A_GPIO_PORT 			GPIOA

#define SEVEN_SEG_B_PIN 				GPIO_PIN_5
#define SEVEN_SEG_B_GPIO_PORT 			GPIOA

#define SEVEN_SEG_C_PIN 				GPIO_PIN_2
#define SEVEN_SEG_C_GPIO_PORT	 		GPIOB

#define SEVEN_SEG_D_PIN 				GPIO_PIN_1
#define SEVEN_SEG_D_GPIO_PORT 			GPIOB

#define SEVEN_SEG_E_PIN 				GPIO_PIN_0
#define SEVEN_SEG_E_GPIO_PORT 			GPIOB

#define SEVEN_SEG_F_PIN 				GPIO_PIN_7
#define SEVEN_SEG_F_GPIO_PORT 			GPIOA

#define SEVEN_SEG_G_PIN 				GPIO_PIN_6
#define SEVEN_SEG_G_GPIO_PORT 			GPIOA

#define SEVEN_SEG_DP_PIN 				GPIO_PIN_12
#define SEVEN_SEG_DP_GPIO_PORT 			GPIOB

/* Enable Pin For Seven Segment*/
#define SEVEN_SEG_ENABLE_1_PIN 			GPIO_PIN_0
#define SEVEN_SEG_ENABLE_1_GPIO_PORT 	GPIOA

#define SEVEN_SEG_ENABLE_2_PIN 			GPIO_PIN_1
#define SEVEN_SEG_ENABLE_2_GPIO_PORT 	GPIOA

#define SEVEN_SEG_ENABLE_3_PIN 			GPIO_PIN_6
#define SEVEN_SEG_ENABLE_3_GPIO_PORT 	GPIOB

#define SEVEN_SEG_ENABLE_4_PIN 			GPIO_PIN_3
#define SEVEN_SEG_ENABLE_4_GPIO_PORT 	GPIOB

#define SEVEN_SEG_ENABLE_5_PIN 			GPIO_PIN_1
#define SEVEN_SEG_ENABLE_5_GPIO_PORT 	GPIOD

#define SEVEN_SEG_ENABLE_6_PIN 			GPIO_PIN_0
#define SEVEN_SEG_ENABLE_6_GPIO_PORT 	GPIOD

/*  Pins For LEDs Indicator */
#define LED_INDICATOR1_PIN 			    GPIO_PIN_5
#define LED_INDICATOR1_GPIO_PORT 	    GPIOB

#define LED_INDICATOR2_PIN 			    GPIO_PIN_4
#define LED_INDICATOR2_GPIO_PORT 	    GPIOB

#define LED_INDICATOR3_PIN 			    GPIO_PIN_14
#define LED_INDICATOR3_GPIO_PORT 	    GPIOB

#define LED_INDICATOR4_PIN 			    GPIO_PIN_13
#define LED_INDICATOR4_GPIO_PORT 	    GPIOB

/* Indicator LED */
#define _IND_LED_PORT			        GPIOB
#define _IND_LED_PIN		        	GPIO_PIN_7

/* Module-specific Macro Definitions ***************************************/
#define NUM_MODULE_PARAMS				 1
#define MOVING_SENTENCE_MAX_LENGTH       100
#define MOVING_SENTENCE_COUNTER_OVERFLOW 95 /* moving time: 300 ms (500 / 3.145) */


/* Module-specific Type Definition *****************************************/
/* Numbers / Letters Representations Type Definition */
typedef enum {
	Empty =0x00,

	/* Numbers Representation*/
	ZERO_NUMBER =0X3F, ONE_NUMBER =0X06, TWO_NUMBER =0X5B, THREE_NUMBER =0X4F, FOUR_NUMBER =0X66,
	FIVE_NUMBER =0X6D, SIX_NUMBER =0X7D, SEVEN_NUMBER =0X07, EIGHT_NUMBER =0X7F, NINE_NUMBER =0X6F,

	/* Small Letter Representation*/
	a_LETTER =0X5F, b_LETTER =0X7C, c_LETTER =0X58, d_LETTER =0X5E, e_LETTER =0X79, f_LETTER =0X71,
	g_LETTER =0X6F, h_LETTER =0X74, i_LETTER =0X10, j_LETTER =0X1E, k_LETTER =0X75, l_LETTER =0X38,
	m_LETTER =0X37, n_LETTER =0X54, o_LETTER =0X5C, p_LETTER =0X73, q_LETTER =0X67, r_LETTER =0X50,
	s_LETTER =0X6C, t_LETTER =0X78, u_LETTER =0X1C, v_LETTER =0X3E, w_LETTER =0X7E, x_LETTER =0X76,
	y_LETTER =0X6E, z_LETTER =0X1B,

	/* Capital Letter Representation*/
	A_LETTER =0X77, B_LETTER =0X7C, C_LETTER =0X39, D_LETTER =0X5E, E_LETTER =0X79, F_LETTER =0X71,
	G_LETTER =0X3D, H_LETTER =0X74, I_LETTER =0X10, J_LETTER =0X1E, K_LETTER =0X75, L_LETTER =0X38,
	M_LETTER =0X37, N_LETTER =0X54, O_LETTER =0X5C, P_LETTER =0X73, Q_LETTER =0X67, R_LETTER =0X50,
	S_LETTER =0X6C, T_LETTER =0X78, U_LETTER =0X1C, V_LETTER =0X3E, W_LETTER =0X7E, X_LETTER =0X76,
	Y_LETTER =0X6E, Z_LETTER =0X1B,

	SYMBOL_MINUS =0X40

} SegmentCodes;

/* Led Indicator Type Definition */
typedef enum{
	INDICATOR_1=1,
	INDICATOR_2,
	INDICATOR_3,
	INDICATOR_4,

	OFF_LED=0x00,
	ON_LED=0xFF
} IndicatorLED;

/* Module-status Type Definition */
typedef enum {
	H3BR7_OK =0,
	H3BR7_ERR_UnknownMessage,
	H3BR7_ERR_WrongParams,
	H3BR7_NUMBER_IS_OUT_OF_RANGE, // Longer than 6 Digits
	H3BR7_Out_Of_Range,
	H3BR7_ERROR =255
} Module_Status;

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

/***************************************************************************/
/***************************** General Functions ***************************/
/***************************************************************************/
Module_Status SevenDisplayNumber(int32_t Number, uint8_t StartSevSeg);
Module_Status SevenDisplayNumberF(float NumberF,uint8_t Res,uint8_t StartSevSeg);
Module_Status SevenDisplayQuantities(float NumberF, uint8_t Res,char Unit ,uint8_t StartSevSeg);
Module_Status SevenDisplayLetter(char letter , uint8_t StartSevSeg);
Module_Status SevenDisplaySentence(char *Sentance,uint16_t length,uint8_t StartSevSeg);
Module_Status SevenDisplayMovingSentence(char *Sentance,uint16_t length);
Module_Status SevenDisplayOff(void);
Module_Status SetIndicator(IndicatorLED indicator );
Module_Status ClearIndicator(IndicatorLED  indicator);

#endif /* H3BR6_H */

/***************** (C) COPYRIGHT HEXABITZ ***** END OF FILE ****************/
