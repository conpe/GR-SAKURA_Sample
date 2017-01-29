/**************************************************
aplDataFlash.h
	データフラッシュ読み書き
**************************************************/


#ifndef __APL_MEMCTRL_H__
#define __APL_MEMCTRL_H__


#include "drvDataFlash.h"
#include "aplMemCtrlSpec.h"


typedef union UN_FL_DATA{
	float Val;
	uint8_t Bit[4];
} UN_FL_DATA;

typedef union UN_DB_DATA{
	float Val;
	uint8_t Bit[8];
} UN_DB_DATA;



// E2データフラッシュ初期化
int8_t initMem(void);

// E2データフラッシュから読む
int8_t readMem(uint16_t Id, void* ReadData);

// E2データフラッシュへ書く
int8_t writeMem(uint16_t Id, void* Data);
int8_t writeMemUB(uint16_t Id, uint8_t Data);
int8_t writeMemUH(uint16_t Id, uint16_t Data);
int8_t writeMemUW(uint16_t Id, uint32_t Data);
int8_t writeMemFL(uint16_t Id, float Data);
int8_t writeMemDB(uint16_t Id, double Data);

// 定数初期化
int8_t clearAllMem(void);

// シリアル通信インターフェース
int8_t MemRcvCom_Write(uint8_t* RcvData);	// E2データフラッシュ書き込み
int8_t MemRcvCom_Read(uint8_t* Data);		// E2データフラッシュ読み込み
int8_t MemRcvCom_clearAllMem(uint8_t* Data);	// E2データフラッシュ初期化

#endif
