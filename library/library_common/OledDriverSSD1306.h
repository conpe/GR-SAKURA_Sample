/********************************************/
/*			OLED(SSD1306)表示				*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/02/25		*/
/********************************************/

/*
【使い方】

OLEDへのデータ、コマンド送信切り替えを行うために、SPI送信時にコールバックする必要あり。
対象のOLEDインスタンスのOledDriverSSD1306::callbackTxIsr()を呼び出すグローバルな関数へのポインタを、begin()に与えてください。


【更新履歴】
2015.03.03	コールバック実装。beginにコールバックする関数を与えるようにして、
			それをSPI送信の割り込み時に呼び出されるようにセット。
2015.03.07	コールバック関数内でCD出力するように
2015.11.28	printメソッド、動的に確保してたら他のとこ侵食してたので静的に確保するように変更。

【変更予定】
コールバック関数の仕様再考

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

#define OLED_BUFFER_SIZE_DEFAULT 512		// SPIバッファの数
#define OLED_DC_BUFFER_SIZE_DEFAULT OLED_BUFFER_SIZE_DEFAULT	// DCバッファの数(SPIと同じにする)

#define OLED_FONT_NUM 192		// フォントの数

#define OLED_MAX_STRING 64		// 一度に出す最大文字列数




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
	void incCur(void);	// 覚えてるカーソル位置更新
	
	
	/////////////////////////////////////////////////
	// Display data management
	/////////////////////////////////////////////////
	
	
	
};

#endif

