/**************************************************
GR_define.h
	GR-SAKURA�̃|�[�g�ԍ��Ƃ����K�肷���B
	
	LED x4
	SW 	x1
**************************************************/



#ifndef __GR_DEF_H__
#define __GR_DEF_H__

#include "portReg.h"


#define PCLK 48000000	// ���Ӄ��W���[���N���b�N[Hz]


#define GR_LED0 PA0
#define GR_LED1 PA1
#define GR_LED2 PA2
#define GR_LED3 PA6

#define GR_SW	PA7

void GR_begin(void);
uint32_t getPCLK(void);


#endif
