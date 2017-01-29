/**************************************************
GR_Ex15_OLED.h
	GR-SAKURA�p�g�����GR_Ex15�̏�ɂ��傤�Ǎڂ����ႤOLED
	�ƃ^�N�g�X�C�b�`x4�̊�p���C�u��������B
	
	�����C���X�^���X
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
	// �X�C�b�`���菈������֐�����Ȃ����Ȃ��B
	static void OLED_TxCallback(void);	// 
};




extern gr_ex_oled GrExOled;
extern OledDriverSSD1306 Oled;

#endif
