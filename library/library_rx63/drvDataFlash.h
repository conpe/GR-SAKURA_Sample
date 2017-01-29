/********************************************/
/*	RX63nのE2データフラッシュ操作 	    */
/*			for RX63n @ CS+	    */
/*			Wrote by conpe_	    */
/*				2016/06/10  */
/********************************************/
// データ型：uint16_t (word)
// 容量：32kbyte = 16kword
// アドレス：0～16383 (内部ではx2)

//ブロック：32byte * 1024ブロック = 32kbyte
//イレース 32byte単位
//ライト 2byte単位

// 1byteごと読み書きする。
// RX63nのデータフラッシュは2byteで扱うので、
// 1byte書いている場合、もう1byteをブランクチェックしてもブランクとはならない

// 書き込みタスクを定期的に実行すること(10msecごと程度)

#ifndef __DATAFLASH_RX63_H__
#define __DATAFLASH_RX63_H__

#include "iodefine.h"
#include "CommonDataType.h"

#define DF_OK 0
#define DF_ERROR -1
#define DF_OUT_OF_ADDRESS -4
#define DF_BUFF_FULL -3
#define DF_BLANK 1

#define FLD_OK 0
#define FLD_ERROR -1
#define FLD_TMOUT -2
#define FLD_BUSY -3
#define FLD_OUT_OF_ADDRESS -4
#define FLD_BLANK 1
#define FLD_NOBLANK 2
#define FLD_NODATA 3

#define E2_DF_SIZE_WORD 32768
#define E2_DF_SIZE_BYTE 16384


// EEPROM初期化
int8_t drvDataFlash_init(void);
// 書き込みタスク
int8_t drvDataFlash_WriteTask(void);
// 書き込み時タスク登録
void drvDataFlash_attachWriteTask( void (*Tsk)(void));
// バッファ付き書き込み
int8_t drvDataFlash_write(uint16_t Address, uint8_t Data);
// 読み込み
int8_t drvDataFlash_read(uint16_t Address, uint8_t* Data);


// 即時書き込み
int8_t drvDataFlash_writeImmWord(uint16_t Address, uint16_t Data);

#endif