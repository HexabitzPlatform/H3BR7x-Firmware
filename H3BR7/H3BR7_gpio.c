/*
 BitzOS (BOS) V0.4.0 - Copyright (C) 2017-2025 Hexabitz
 All rights reserved

 File Name     : H3BR7_gpio.c
 Description   : Source code provides code for the configuration of all used GPIO pins .

 */

/* Includes ****************************************************************/
#include "BOS.h"

/***************************************************************************/
/* Configure GPIO **********************************************************/
/***************************************************************************/

/* Pinout Configuration */
void GPIO_Init(void){
	/* GPIO Ports Clock Enable */
	__GPIOC_CLK_ENABLE();
	__GPIOA_CLK_ENABLE();
	__GPIOD_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	__GPIOF_CLK_ENABLE(); /* for HSE and Boot0 */
	
	IND_LED_Init();
}

/***************************************************************************/
/* Configure indicator LED */
void IND_LED_Init(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = _IND_LED_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(_IND_LED_PORT,&GPIO_InitStruct);
}

/***************************************************************************/
/* Seven Segment GPIO Init */
void SevenSegGPIOInit(void){

	GPIO_InitTypeDef GPIO_InitStruct ={0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA,SEVEN_SEG_A_PIN | SEVEN_SEG_F_PIN | SEVEN_SEG_B_PIN | SEVEN_SEG_C_PIN | SEVEN_SEG_G_PIN,GPIO_PIN_RESET);

	HAL_GPIO_WritePin(GPIOA,SEVEN_SEG_ENABLE_4_PIN | SEVEN_SEG_ENABLE_2_PIN | SEVEN_SEG_ENABLE_6_PIN,GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB,SEVEN_SEG_DP_PIN | SEVEN_SEG_D_PIN | SEVEN_SEG_E_PIN,GPIO_PIN_RESET);

	HAL_GPIO_WritePin(GPIOB,SEVEN_SEG_ENABLE_1_PIN | SEVEN_SEG_ENABLE_3_PIN | SEVEN_SEG_ENABLE_5_PIN,GPIO_PIN_SET);

	HAL_GPIO_WritePin(GPIOB,LED_INDICATOR1_PIN | LED_INDICATOR2_PIN | LED_INDICATOR3_PIN | LED_INDICATOR4_PIN,GPIO_PIN_SET);

	/*Configure GPIO pins : A_Pin F_Pin B_Pin
	 G_Pin CA1_Pin CA2_Pin  */
	GPIO_InitStruct.Pin =SEVEN_SEG_A_PIN | SEVEN_SEG_F_PIN | SEVEN_SEG_B_PIN | SEVEN_SEG_G_PIN | SEVEN_SEG_ENABLE_1_PIN | SEVEN_SEG_ENABLE_2_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);

	/*Configure GPIO pins : DP_Pin C_Pin D_Pin E_Pin CA3_Pin
	 CA4_Pin  LEDs(1,2,3,4)  */
	GPIO_InitStruct.Pin =SEVEN_SEG_DP_PIN | SEVEN_SEG_D_PIN | SEVEN_SEG_E_PIN | SEVEN_SEG_ENABLE_1_PIN | SEVEN_SEG_C_PIN | SEVEN_SEG_ENABLE_3_PIN | SEVEN_SEG_ENABLE_4_PIN | LED_INDICATOR1_PIN | LED_INDICATOR2_PIN | LED_INDICATOR3_PIN | LED_INDICATOR4_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);

	/*Configure GPIO pin Output Level */

	/*Configure GPIO pins : CA5_Pin CA6_Pin  */

	GPIO_InitStruct.Pin =SEVEN_SEG_ENABLE_5_PIN | SEVEN_SEG_ENABLE_6_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD,&GPIO_InitStruct);
}

/***************************************************************************/
/* Check for factory reset condition:
 - P1 TXD is connected to last port RXD  */
uint8_t IsFactoryReset(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	uint32_t P1_TX_Port, P1_RX_Port, P_last_TX_Port, P_last_RX_Port;
	uint16_t P1_TX_Pin, P1_RX_Pin, P_last_TX_Pin, P_last_RX_Pin;

	/* Enable all GPIO Ports Clocks */
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	__GPIOD_CLK_ENABLE();
	
	/* Get GPIOs */
	GetPortGPIOs(P1,&P1_TX_Port,&P1_TX_Pin,&P1_RX_Port,&P1_RX_Pin);
	GetPortGPIOs(P_LAST,&P_last_TX_Port,&P_last_TX_Pin,&P_last_RX_Port,&P_last_RX_Pin);
	
	/* TXD of first port */
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Pin =P1_TX_Pin;
	HAL_GPIO_Init((GPIO_TypeDef* )P1_TX_Port,&GPIO_InitStruct);
	
	/* RXD of last port */
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Pin =P_last_RX_Pin;
	HAL_GPIO_Init((GPIO_TypeDef* )P_last_RX_Port,&GPIO_InitStruct);
	
	/* Check for factory reset conditions */
	HAL_GPIO_WritePin((GPIO_TypeDef* )P1_TX_Port,P1_TX_Pin,GPIO_PIN_RESET);
	Delay_ms_no_rtos(5);
	if(HAL_GPIO_ReadPin((GPIO_TypeDef* )P_last_RX_Port,P_last_RX_Pin) == RESET){
		HAL_GPIO_WritePin((GPIO_TypeDef* )P1_TX_Port,P1_TX_Pin,GPIO_PIN_SET);
		Delay_ms_no_rtos(5);
		if(HAL_GPIO_ReadPin((GPIO_TypeDef* )P_last_RX_Port,P_last_RX_Pin) == SET){
			return 1;
		}
	}
	
	/* Clear flag for formated EEPROM if it was already set */
	/* Flag address (STM32F09x) - Last 4 words of SRAM */
	*((unsigned long* )0x20007FF0) =0xFFFFFFFF;
	
	return 0;
}

/***************************************************************************/
/* Get GPIO pins and ports of this array port */
BOS_Status GetPortGPIOs(uint8_t port,uint32_t *TX_Port,uint16_t *TX_Pin,uint32_t *RX_Port,uint16_t *RX_Pin){
	BOS_Status result =BOS_OK;
	
	/* Get port UART */
	UART_HandleTypeDef *huart =GetUart(port);
	
	if(huart == &huart1){
#ifdef _USART1
		*TX_Port =(uint32_t ) USART1_TX_PORT;
		*TX_Pin = USART1_TX_PIN;
		*RX_Port =(uint32_t ) USART1_RX_PORT;
		*RX_Pin = USART1_RX_PIN;
#endif
	}
#ifdef _USART2
	else if(huart == &huart2){
		*TX_Port =(uint32_t ) USART2_TX_PORT;
		*TX_Pin = USART2_TX_PIN;
		*RX_Port =(uint32_t ) USART2_RX_PORT;
		*RX_Pin = USART2_RX_PIN;
	}
#endif
#ifdef _USART3
	else if(huart == &huart3){
		*TX_Port =(uint32_t ) USART3_TX_PORT;
		*TX_Pin = USART3_TX_PIN;
		*RX_Port =(uint32_t ) USART3_RX_PORT;
		*RX_Pin = USART3_RX_PIN;
	}
#endif
#ifdef _USART4
	else if(huart == &huart4){
		*TX_Port =(uint32_t ) USART4_TX_PORT;
		*TX_Pin = USART4_TX_PIN;
		*RX_Port =(uint32_t ) USART4_RX_PORT;
		*RX_Pin = USART4_RX_PIN;
	}
#endif
#ifdef _USART5
	else if(huart == &huart5){
		*TX_Port =(uint32_t ) USART5_TX_PORT;
		*TX_Pin = USART5_TX_PIN;
		*RX_Port =(uint32_t ) USART5_RX_PORT;
		*RX_Pin = USART5_RX_PIN;
	}
#endif
#ifdef _USART6
	else if(huart == &huart6){
		*TX_Port =(uint32_t ) USART6_TX_PORT;
		*TX_Pin = USART6_TX_PIN;
		*RX_Port =(uint32_t ) USART6_RX_PORT;
		*RX_Pin = USART6_RX_PIN;
	}
#endif
#ifdef _USART7
	else if (huart == &huart7) 
	{		
		*TX_Port = (uint32_t)USART7_TX_PORT;
		*TX_Pin = USART7_TX_PIN;
		*RX_Port = (uint32_t)USART7_RX_PORT;
		*RX_Pin = USART7_RX_PIN;
	} 
#endif
#ifdef _USART8
	else if (huart == &huart8) 
	{	
		*TX_Port = (uint32_t)USART8_TX_PORT;
		*TX_Pin = USART8_TX_PIN;
		*RX_Port = (uint32_t)USART8_RX_PORT;
		*RX_Pin = USART8_RX_PIN;
	} 
#endif
	else
		result =BOS_ERROR;
	
	return result;
}

/***************************************************************************/
/***************** (C) COPYRIGHT HEXABITZ ***** END OF FILE ****************/
