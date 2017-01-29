/********************************************/
/*			OLED(SSD1306)�\��				*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/02/25		*/
/********************************************/

/*
�y�g�����z

OLED�ւ̃f�[�^�A�R�}���h���M�؂�ւ����s�����߂ɁASPI���M���ɃR�[���o�b�N����K�v����B
�Ώۂ�OLED�C���X�^���X��OledDriverSSD1306::callbackTxIsr()���Ăяo���O���[�o���Ȋ֐��ւ̃|�C���^���Abegin()�ɗ^���Ă��������B


�y�X�V�����z
2015.03.03	�R�[���o�b�N�����Bbegin�ɃR�[���o�b�N����֐���^����悤�ɂ��āA
			�����SPI���M�̊��荞�ݎ��ɌĂяo�����悤�ɃZ�b�g�B
2015.03.07	�R�[���o�b�N�֐�����CD�o�͂���悤��
2015.11.28	print���\�b�h�A���I�Ɋm�ۂ��Ă��瑼�̂Ƃ��N�H���Ă��̂ŐÓI�Ɋm�ۂ���悤�ɕύX�B

�y�ύX�\��z
�R�[���o�b�N�֐��̎d�l�čl

*/

#ifndef OLED_DRIVER_SSD_HPP_
#define OLED_DRIVER_SSD_HPP_

#include "CommonDataType.h"
#include "RSPI.h"
#include "portReg.h"
#include <stdio.h>
#include <stdarg.h>


// for debug
//#include <rxduino.h>
#include "GR_define.h"

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_LINE 8

#define OLED_BUFFER_SIZE_DEFAULT 512		// SPI�o�b�t�@�̐�
#define OLED_DC_BUFFER_SIZE_DEFAULT OLED_BUFFER_SIZE_DEFAULT	// DC�o�b�t�@�̐�(SPI�Ɠ����ɂ���)

#define OLED_FONT_NUM 192		// �t�H���g�̐�

#define OLED_MAX_STRING 64		// ��x�ɏo���ő啶����




enum write_mode{
	OLED_WRITE_COMMAND = 0,
	OLED_WRITE_DATA = 1
};

enum ctrl_mode{
	OLED_SPI,
	OLED_I2C,
	OLED_PARALLEL
};


class OledDriverSSD1306
{
public :
	/////////////////////////////////////////////////
	// initialize
	/////////////////////////////////////////////////
	OledDriverSSD1306(void);
	~OledDriverSSD1306(void);
	int8_t begin(spi_module SpiModule, spi_pin PinMosi, spi_pin PinRspck, spi_pin pinSsl, pins PinDC, void (*Callback)());	// with SPI
	
	// util
	void initRegister(void);
	void onDisplay(void){writeCommand(0xaf);};
	void offDisplay(void){writeCommand(0xae);};
	
	/////////////////////////////////////////////////
	// Data/Command transmit
	/////////////////////////////////////////////////
	void setDC(write_mode WriteMode);		// set Data or Command
	write_mode getDC(void){return _DC;};		// set Data or Command
	int8_t writeCommand(uint8_t Command);
	int8_t writeData(uint8_t Data);
	
	void clearDisplay(void);
	void clearLine(void);
	
	/////////////////////////////////////////////////
	// Cursol management 
	/////////////////////////////////////////////////
	void setCur(uint8_t R, uint8_t C);
	void setCurNextLine(void);	// Line feed
	void setCurNextCol(void);	// 
	void setCurNewLine(uint8_t Row);	// Set cursol and Clear the line.
	void setCurNewLine(uint8_t Row, uint8_t Col);
	
	/////////////////////////////////////////////////
	// Charactor
	/////////////////////////////////////////////////
	static const uint8_t Font7h5w[192][5];
	int8_t writeChar(char ch);
	int8_t print(char ch);
	int8_t print(int num);
	int8_t print(const char *control, ...);
	int8_t println(const char *control, ...);
	
	/////////////////////////////////////////////////
	// CallBackFunction
	/////////////////////////////////////////////////
	void callbackTxIsr(void);
	
	/////////////////////////////////////////////////
	// info
	/////////////////////////////////////////////////
	uint16_t getOledWidth(void){return OLED_WIDTH;};
	uint16_t getOledHeight(void){return OLED_HEIGHT;};
	uint16_t getOledLine(void){return OLED_LINE;};
	uint16_t getOledHeightPerLine(void){return OLED_HEIGHT/OLED_LINE;};
	
	
	/////////////////////////////////////////////////
	// debug
	/////////////////////////////////////////////////
	pins getPinDC(void){return _PinDC;};
	
private :
	ctrl_mode _CtrlMode;	// SPI or I2C or Parallel
	Spi_t *SPIn;		// SPI module registers
	spi_ssl _SpiCs;		// SPI SSL
	pins _PinDC;		// D/C pin 
	write_mode _DC;		// now D or C
	uint8_t _CurRow;
	uint8_t _CurCol;
	
	RingBuffer<uint8_t> *DCBuff;
	write_mode lastDC;
	
	/////////////////////////////////////////////////
	// SSD1306 initialize
	/////////////////////////////////////////////////
	//void initRegister(void);
	
	/////////////////////////////////////////////////
	// SSD1306 transmit
	/////////////////////////////////////////////////
	int8_t write(uint8_t Data);				// transmit data
	
	/////////////////////////////////////////////////
	// Cursol management 
	/////////////////////////////////////////////////
	void incCur(void);	// �o���Ă�J�[�\���ʒu�X�V
	
	
	/////////////////////////////////////////////////
	// Display data management
	/////////////////////////////////////////////////
	
	
	
};

#endif

