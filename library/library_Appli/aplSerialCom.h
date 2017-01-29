/**************************************************
aplSerialCom.h
	シリアル通信処理
**************************************************/


#ifndef __APL_SERIALCOM_H__
#define __APL_SERIALCOM_H__


#include "SCI.h"
#define Serial_t Sci_t

#include "aplSerialComSpec.h"

// ヘッダ読んでない場合
#ifndef __APL_SERIALCOM_SPEC_H__
// シリアル通信コマンド処理テーブル
typedef struct ST_SERIAL_COM_COMMAND_TABLE_T{
	uint16_t ComId;			// コマンドID
	uint8_t DataLength;		// データ長
	int8_t (*Func)(uint8_t*);	// 処理関数(失敗 or 成功を返す)
} ST_SERIAL_COM_COMMAND_TABLE_T;

extern const ST_SERIAL_COM_COMMAND_TABLE_T stSerialComCommandTable[]
#endif



#define FUNC_RET_OK 0		// OKを返す
#define FUNC_RET_NG -1		// NGを返す
#define FUNC_RET_NO_RET 1	// 何も返さない




enum SerialComMode_t{
	SELCOM_MODE_COMMAND,	// コマンドモード(コマンドでやり取り)
	SELCOM_MODE_THROUGH,	// パススルーモード(送受信いずれも自由)
};

class SerialCom_t{
public:
	// 初期化
	static int8_t init(Serial_t *SerialObj, SerialComMode_t InitialMode);
	
	// 処理タスク
	static void task(void);
	
	// モードセット
	static void setMode(SerialComMode_t NewMode);
	// モードセット(シリアル通信IF)
	static int8_t setModeSerialCom(uint8_t *RcvData);
	
	// 受信時タスク登録
	static void attachRcvTask(void (*Tsk)(void));
	// 受信時タスク解除
	static void dettachRcvTask(void);
	
	
	
	
	
	
private:
	static Serial_t *SerialObj;		// シリアル通信ドライバオブジェクト
	static SerialComMode_t Mode;		// 処理モード
	static void (*AddedRcvTask)(void);	// 受信時処理タスク
	static const ST_SERIAL_COM_COMMAND_TABLE_T stSerialComDefauldCommandTable[];	// シリアル通信デフォルトコマンド
	void SerialCom(void);	// インスタンス化させないためにprivate
	// チェックサム関係
	static uint8_t calcCheckSum(uint8_t* RcvData);		// チェックサム計算
	static uint8_t isValidCheckSum(uint8_t* RcvData);	// チェックサムあってるか確認
	static void setCheckSum(uint8_t* RcvData);		// チェックサムセット
	
	
};
#endif
