/**************************************************
GR_Ex15_OLED.h
	GR-SAKURA用拡張基板GR_Ex15の上にちょうど載っちゃうOLED
	とタクトスイッチx4の基板用ライブラリだよ。
	
	生成インスタンス
	GrExOled
	Oled

**************************************************/



#ifndef __GR_EX15_OLED_H__
#define __GR_EX15_OLED_H__


#include "GR_Ex15.h"
#include "OledDriverSSD1306.h"


#define OLED_SW0 GREX_IO23
#define OLED_SW1 GREX_IO22
#define OLED_SW2 GREX_IO21
#define OLED_SW3 GREX_IO20

#define OLED_SPI_MODULE RSPI_SPI1
#define OLED_PIN_DC PE7


class gr_ex_oled{
public:
	void begin(void);
	// スイッチ周り処理する関数いらないかなぁ。
	static void OLED_TxCallback(void);	// 
};




extern gr_ex_oled GrExOled;
extern OledDriverSSD1306 Oled;

#endif
