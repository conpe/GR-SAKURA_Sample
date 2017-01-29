/**************************************************
	aplSerialComSpec.h
	シリアル通信コマンド管理
				2016/09/18 22:14:35
**************************************************/

#ifndef __APL_SERIALCOM_SPEC_H__
#define __APL_SERIALCOM_SPEC_H__


#include "CommonDataType.h"

// シリアル通信コマンド処理テーブル
typedef struct ST_SERIAL_COM_COMMAND_TABLE_T{
	uint16_t ComId;			// コマンドID
	uint8_t DataLength;		// データ長
	int8_t (*Func)(uint8_t*);	// 処理関数(失敗 or 成功を返す)
} ST_SERIAL_COM_COMMAND_TABLE_T;

extern const ST_SERIAL_COM_COMMAND_TABLE_T stSerialComCommandTable[];

#endif
