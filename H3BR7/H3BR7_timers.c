/*
 BitzOS (BOS) V0.3.4 - Copyright (C) 2017-2024 Hexabitz
 All rights reserved

 File Name     : H3BR7_timers.c
 Description   : Peripheral timers setup source file.

 Required MCU resources :

 >> Timer 16 for micro-sec delay.
 >> Timer 17 for milli-sec delay.

 */

/* Includes ------------------------------------------------------------------*/
#include "BOS.h"

/*----------------------------------------------------------------------------*/
/* Configure Timers                                                              */
/*----------------------------------------------------------------------------*/

/* Variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim16; /* micro-second delay counter */
TIM_HandleTypeDef htim17; /* milli-second delay counter */
TIM_HandleTypeDef htim6; /* Timer for 7-segment*/

/*  Micro-seconds timebase init function - TIM14 (16-bit)
 */
void TIM_USEC_Init(void){
	  __TIM16_CLK_ENABLE();

	  htim16.Instance = TIM16;
	  htim16.Init.Prescaler = 47;
	  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim16.Init.Period = 0XFFFF;
	  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  htim16.Init.RepetitionCounter = 0;
	  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  HAL_TIM_Base_Init(&htim16);

	  HAL_TIM_Base_Start(&htim16);

}

/*-----------------------------------------------------------*/

/*  Milli-seconds timebase init function - TIM15 (16-bit)
 */
void TIM_MSEC_Init(void){

	  __TIM17_CLK_ENABLE();
	  htim17.Instance = TIM17;
	  htim17.Init.Prescaler = 47999;
	  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim17.Init.Period = 0xFFFF;
	  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  htim17.Init.RepetitionCounter = 0;
	  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  HAL_TIM_Base_Init(&htim17);

	  HAL_TIM_Base_Start(&htim17);
}

/*-----------------------------------------------------------*/
/* Timer for 7-segment */
void MX_TIM6_Init(void)
{

	TIM_MasterConfigTypeDef sMasterConfig;

	/* Peripheral clock enable */
	__TIM6_CLK_ENABLE();

	/* Peripheral configuration */
	htim6.Instance = TIM6;
	htim6.Init.Prescaler =999;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period =150;// old one 959 --> 20 ms
	HAL_TIM_Base_Init(&htim6);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim6,&sMasterConfig);

	HAL_NVIC_SetPriority(TIM6_DAC_LPTIM1_IRQn,0,0);
	HAL_NVIC_EnableIRQ(TIM6_DAC_LPTIM1_IRQn);

	HAL_TIM_Base_Start_IT(&htim6);

}

/*-----------------------------------------------------------*/
/* --- Load and start micro-second delay counter --- 
 */
void StartMicroDelay(uint16_t Delay){
	uint32_t t0 =0;
	
	portENTER_CRITICAL();
	
	if(Delay){
		t0 =htim16.Instance->CNT;
		
		while(htim16.Instance->CNT - t0 <= Delay){};
	}

	portEXIT_CRITICAL();
}

/*-----------------------------------------------------------*/

/* --- Load and start milli-second delay counter --- 
 */
void StartMilliDelay(uint16_t Delay){
	uint32_t t0 =0;
	
	portENTER_CRITICAL();
	
	if(Delay){
		t0 =htim17.Instance->CNT;
		
		while(htim17.Instance->CNT - t0 <= Delay){};
	}

	portEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

/************************ (C) COPYRIGHT HEXABITZ *****END OF FILE****/
