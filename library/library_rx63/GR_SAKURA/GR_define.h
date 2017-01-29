/**************************************************
GR_define.h
	GR-SAKURAのポート番号とかを規定するよ。
	共通で使う簡単な関数も。
	
	LED x4
	SW 	x1
**************************************************/



#ifndef __GR_DEF_H__
#define __GR_DEF_H__

#include "portReg.h"


#define PCLK 48000000	// 周辺モジュールクロック[Hz]


#define GR_LED0 PA0
#define GR_LED1 PA1
#define GR_LED2 PA2
#define GR_LED3 PA6

#define GR_SW	PA7

void GR_begin(void);

// クロック
uint32_t getPCLK(void);

// 起動時からの時間
void GR_cnt(void);		// 時間カウント (1ms間隔で呼び出すこと)
//uint32_t getTime_ms(uint32_t BaseTime_ms = 0);	// 時間取得 [ms]
uint32_t getTime_ms(void);	// 時間取得 [ms]
void resetTime(void);		// 時間を0クリア

#endif
