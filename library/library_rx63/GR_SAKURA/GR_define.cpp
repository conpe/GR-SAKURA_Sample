
#include "GR_define.h"

void GR_begin(void){
	setPinMode(GR_LED0, PIN_OUTPUT);
	setPinMode(GR_LED1, PIN_OUTPUT);
	setPinMode(GR_LED2, PIN_OUTPUT);
	setPinMode(GR_LED3, PIN_OUTPUT);
	//outPin(GR_LED0, 0);
	//outPin(GR_LED1, 0);
	//outPin(GR_LED2, 0);
	//outPin(GR_LED3, 0);
	setPinMode(GR_SW, PIN_INPUT);
}

uint32_t getPCLK(void){
	return (uint32_t)PCLK;
}


// ŽžŠÔŠÖŒW
static uint32_t GR_Time_msec = 0;

void GR_cnt(void){
	GR_Time_msec++;
}
/*
uint32_t getTime_ms(uint32_t BaseTime_ms){
	return GR_Time_msec - BaseTime_ms;
}*/
uint32_t getTime_ms(void){
	return GR_Time_msec;
}

void resetTime(void){
	GR_Time_msec = 0;
}

