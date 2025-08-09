/*
 BitzOS (BOS) V0.4.0 - Copyright (C) 2017-2025 Hexabitz
 All rights reserved

 File Name     : main.c
 Description   : Main program body.
 */

/* Includes ****************************************************************/
#include "BOS.h"

/* Private variables *******************************************************/

/* Private Function Prototypes *********************************************/

/* Main Function ***********************************************************/
int main(void){

	/* Initialize Module &  BitzOS */
	Module_Init();

	/* Don't place your code here */
	for(;;){
	}
}

/***************************************************************************/
/* User Task */
void UserTask(void *argument){

	/* put your code here, to run repeatedly. */
	while(1){
		DisplayNumber(-27.9,1, 1);
		Delay_ms(1000);
		DisplayOff();
		SetIndicator(INDICATOR_2);
		SetIndicator(INDICATOR_3);
		Delay_ms(1000);
		DisplayNumber(-24,0, 1);
		Delay_ms(1000);
		DisplayQuantities(153.4 ,1 ,'S',2);
		Delay_ms(1000);
		SetIndicator(INDICATOR_1);
		SetIndicator(INDICATOR_4);
		Delay_ms(1000);
		DisplaySentence("Hello", 5, 1);
		ClearIndicator(INDICATOR_3);
		Delay_ms(1000);
		DisplayMovingSentence("Hexabitz platform", 17);
		ClearIndicator(INDICATOR_4);
		Delay_ms(10000);
		ClearIndicator(INDICATOR_1);
		ClearIndicator(INDICATOR_2);
		Delay_ms(1000);


}
}

/***************************************************************************/
/***************** (C) COPYRIGHT HEXABITZ ***** END OF FILE ****************/
