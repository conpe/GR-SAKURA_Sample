/**************************************************
	aplSerialComSpec.cpp
	シリアル通信コマンド管理
				2016/09/18 22:14:35
**************************************************/

#include "aplSerialComSpec.h"

// 呼び出す関数の宣言
#include "aplSerialCom.h"
#include "aplMemCtrl.h"
#include "inrof2016_lib.h"


// シリアル通信コマンド処理テーブル
const ST_SERIAL_COM_COMMAND_TABLE_T stSerialComCommandTable[] = {
	/*	ComId,	DataLength,	Func	*/
	{	0x1000,		0,	inrof2016::SerCom_resetBallSensors},
	{	0xF001,		2,	MemRcvCom_Read	},	// EEPROM読み込み
	{	0xF002,		8,	MemRcvCom_Write	},	// EEPROM書き込み
	{	0xF003,		0,	MemRcvCom_clearAllMem	},	// EEPROM全初期化
	{	0x0000,		0,	NULL	},	// 
};


