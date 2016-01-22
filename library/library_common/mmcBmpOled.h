/********************************************/
/*			OLED(SSD1306)表示 Bmpファイル	*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/04/03		*/
/********************************************/

/*
【概要】
白黒のビットマップをOLEDに表示する。
48x128ピクセル。白黒。
通常はOLEDの16～63行目に書く。


【使い方】

OLEDへのデータ、コマンド送信切り替えを行うために、SPI送信時にコールバックする必要あり。
OledDriverSSD1306::callbackTxIsr()を呼び出すグローバルな関数へのポインタを、begin()に与えてください。


【更新履歴】
2015.03.03	コールバック実装。beginにコールバックする関数を与えるようにして、
			それをSPI送信の割り込み時に呼び出されるようにセット。
2015.03.07	コールバック関数内でCD出力するように
			
*/

#ifndef OLED_DRIVER_SSD_BMP_HPP_
#define OLED_DRIVER_SSD_BMP_HPP_

#include "CommonDataType.h"
#include "RSPI.h"
#include "portReg.h"
#include <stdio.h>
#include <stdarg.h>
#include <sdmmc.h>

#include "OledDriverSSD1306.h"

// for debug
//#include <rxduino.h>
#include "GR_define.h"



class mmcBmpOled
{
public :
	/////////////////////////////////////////////////
	// initialize
	/////////////////////////////////////////////////
	mmcBmpOled(OledDriverSSD1306 *OledDriver, SDMMC *MMC);
	~mmcBmpOled(void);
	
	// bmp描画
	// SDカードから白黒Bmpファイル読んでOLEDに出力する。
	// OLED一行分読む (8*128/8 = 128バイト) → OLEDに出力する を繰り返す。
	// 縦の領域をライン単位で指定して表示する
	int8_t outBmp(const char *FileName, const uint8_t StartLineNum, const uint8_t WriteLineNum);
	// メイン部分に表示する
	int8_t outBmpMain(const char *FileName);
	// 上の帯部分に出力する
	int8_t outBmpHeader(const char *FileName);
	
	
private :
	OledDriverSSD1306* OledDriver;
	SDMMC* MMC;
	File* BmpFile;
	uint8_t DisplayLinePix;
	uint8_t DisplayLineNum;
	uint8_t DisplayWidth;
	
	
};

#endif

