/**************************************************
aplSerialCom.cpp
	シリアル通信処理
**************************************************/

#include "aplSerialCom.h"

// ヘッダ
#define HEADER_RCV 0xFBBF
#define HEADER_RET_OK 0xBFFB
#define HEADER_RET_NG 0xCFFC

#define SERIAL_COM_LENGTH 13


void (*SerialCom_t::AddedRcvTask)(void) = NULL;
Serial_t* SerialCom_t::SerialObj;
SerialComMode_t SerialCom_t::Mode = SELCOM_MODE_COMMAND;


#ifndef __APL_SERIALCOM_SPEC_H__
// ユーザー設定が無い場合
const ST_SERIAL_COM_COMMAND_TABLE_T stSerialComCommandTable[] = {
	{	0x0000,		0,	NULL	},	// 
};
#endif

// シリアル通信デフォルトコマンド処理テーブル
const ST_SERIAL_COM_COMMAND_TABLE_T SerialCom_t::stSerialComDefauldCommandTable[] = {
	/*	ComId,	DataLength,	Func	*/
	{	0xFF00,		1,	setModeSerialCom	},	// モードセット
	{	0x0000,		0,	NULL	},	// 
};


// 初期化
int8_t SerialCom_t::init(Serial_t *SerialObj, SerialComMode_t InitialMode){
	SerialCom_t::SerialObj = SerialObj;
	SerialCom_t::Mode = InitialMode;
	
	return 0;
}

// タスク処理
// コマンドモード時
//  コマンド処理 ユーザーコマンド->デフォルトコマンドの順
//  タスク登録されていればそれも実行
void SerialCom_t::task(void){
	uint16_t Sel;
	uint16_t ComId;
	uint16_t RcvDataNum;		// 受信バッファデータ数
//	uint16_t RcvDataNumCom;		// コマンドまでのデータ数
	volatile uint16_t SelRcvData=0;
	uint16_t i;
	uint8_t DataTmp;
	uint8_t RetDataNum;
	static uint8_t fHead = 0;
	static uint8_t RcvData[SERIAL_COM_LENGTH];	// ヘッダから順に覚えておく ヘッダ2byte, コマンドID 2byte, データ8byte, チェックサム1byte
	static uint8_t RcvDataPos = 0;	// 次保存する位置
	int8_t FuncRet = FUNC_RET_NO_RET;
	bool_t IsSumOk = 0;	// チェックサム
	bool_t Breaked = FALSE;
	const uint16_t RcvHeader = HEADER_RCV;
	
	
	
	// 受信データ解釈,コマンド実行
	// 受信データコピー
	
	
	// コマンド解析
	// ヘッダを探す
	// 受信数
	if(RcvDataNum = SerialObj->available()){	// 受信データあり
		for(SelRcvData=0; SelRcvData<RcvDataNum; ++SelRcvData){	// 受信した数だけ解析をすすめる
			DataTmp = SerialObj->watch(SelRcvData);
			
			if(0==fHead){	// まだヘッダ見つけてない
				if((uint8_t)((RcvHeader>>8)&0x00FF) == DataTmp){	// ヘッダ1つ目
					fHead = 1;
				}else{	// ヘッダじゃなかったのでリセット
				}
			}else if(1==fHead){	// 1つ目のヘッダ見つけてる状態
				if((uint8_t)((RcvHeader)&0x00FF) == DataTmp){	// ヘッダ2つ目
					fHead = 2;
					RcvData[0] = (uint8_t)((RcvHeader>>8)&0x00FF);
					RcvData[1] = (uint8_t)((RcvHeader)&0x00FF);
					
					RcvDataPos = 2;
				}else{	// 1つ目の次が2つ目じゃなかったのでリセット
					fHead = 0;
					RcvDataPos = 0;
				}
			}else if(2==fHead){	// ヘッダ見つけた状態で次の周きた
				RcvData[RcvDataPos] = DataTmp;
				++RcvDataPos;
				if(SERIAL_COM_LENGTH <= RcvDataPos ){	// 受信完了
					fHead = 0;
					RcvDataPos = 0;
					//チェックサム確認
					IsSumOk = isValidCheckSum(RcvData);
					if(IsSumOk){
						SelRcvData;
						break;
					}else{
						SelRcvData -= (SERIAL_COM_LENGTH-2);	// ずらして解析し直し
					}
				}
			}else{
				
			}
		}
	}
	
	if(IsSumOk){
		
		ComId = ((uint16_t)RcvData[3]<<8) | (uint16_t)RcvData[2];
		
		// 関数実行
		// ユーザーコマンド
		Sel = 0;
		
		if(SELCOM_MODE_COMMAND==Mode){	// コマンドモードならば
			while(NULL != stSerialComCommandTable[Sel].Func){				// 関数登録あり(テーブルの最後を検知)
				if(ComId == stSerialComCommandTable[Sel].ComId){			// コマンドID一致
					FuncRet = (*stSerialComCommandTable[Sel].Func)(&RcvData[4]);	// 関数実行				
					Breaked = TRUE;
					break;
				}
				++Sel;
			}
		}
		if(!Breaked){	// ユーザーコマンド実行されていないならば
			// デフォルトコマンドを実行
			Sel = 0;
			if((0xFF00==ComId)||(SELCOM_MODE_COMMAND==Mode)){		// モードセットコマンドもしくはコマンドモードならば
				while(NULL != stSerialComDefauldCommandTable[Sel].Func){			// 関数登録あり(テーブルの最後を検知)
					if(ComId == stSerialComDefauldCommandTable[Sel].ComId){			// コマンドID一致
						FuncRet = (*stSerialComDefauldCommandTable[Sel].Func)(&RcvData[4]);	// 関数実行
						Breaked = TRUE;
						break;
					}
					++Sel;
				}
			}else{
				FuncRet = FUNC_RET_NO_RET;
			}
		}
	}
	
	// コマンド応答
	if(Breaked){	// コマンド一致した
		if(FUNC_RET_OK == FuncRet){
			// ヘッダをセット
			RcvData[0] = (uint8_t)((HEADER_RET_OK>>8)&0x00FF);
			RcvData[1] = (uint8_t)((HEADER_RET_OK)&0x00FF);
			
			// 応答送信
			RetDataNum = SERIAL_COM_LENGTH;
			
		}else if(FUNC_RET_NG == FuncRet){
			// ヘッダをセット
			RcvData[0] = (uint8_t)((HEADER_RET_NG>>8)&0x00FF);
			RcvData[1] = (uint8_t)((HEADER_RET_NG)&0x00FF);
			
			// 応答送信
			RetDataNum = SERIAL_COM_LENGTH;
		}else{
			// 何も返さない
			RetDataNum = 0;
		}
		
		// チェックサム更新
		setCheckSum(RcvData);
		
		// 応答データ送信
		i = 0;
		while(i<RetDataNum){
			SerialObj->transmit(RcvData[i]);
			++i;
		}
		
		// コマンドまでの受信データ消す
		for(i = 0; i<(SelRcvData+1); ++i){
			SerialObj->read();
		}
		
	}
	
	
	// 実行タスク登録されていれば
	if(NULL != AddedRcvTask){
		//受信データあれば？
		(*AddedRcvTask)();	// 実行する
	}
	
}


/*****************************************
モードセット
概要：
	動作モードを設定
引数：
	新しい動作モード
返値：
	なし
******************************************/
void SerialCom_t::setMode(SerialComMode_t NewMode){
	Mode = NewMode;
}

/*****************************************
書き込み時実行タスク登録
概要：
	定期的に呼び出す書き込み処理で実行するタスクを登録
引数：
	実行関数ポインタ
返値：
	なし
******************************************/
int8_t SerialCom_t::setModeSerialCom(uint8_t *RcvData){
	if((SELCOM_MODE_COMMAND == RcvData[0])||(SELCOM_MODE_THROUGH == RcvData[0])){
		setMode((SerialComMode_t)RcvData[0]);
		
		return FUNC_RET_OK;
	}else{
		return FUNC_RET_NG;
	}
}

/*****************************************
受信内容処理タスク登録
概要：
	定期的に呼び出す書き込み処理で実行するタスクを登録
引数：
	実行関数ポインタ
返値：
	なし
******************************************/
void SerialCom_t::attachRcvTask( void (*Tsk)(void)){
	AddedRcvTask = Tsk;
}

/*****************************************
受信内容処理タスク登録解除
概要：
	定期的に呼び出す書き込み処理で実行するタスクを解除
引数：
	なし
返値：
	なし
******************************************/
void SerialCom_t::dettachRcvTask(void){
	AddedRcvTask = NULL;
}

/*****************************************
チェックサム確認
概要：
	受信データのSERIAL_COM_LENGTH-1バイトを足した値を返す
引数：
	なし
返値：
	uint8_t チェックサム
******************************************/
uint8_t SerialCom_t::calcCheckSum(uint8_t* RcvData){
	uint8_t DataSum = 0;
	uint8_t i;
	
	for(i=0;i<SERIAL_COM_LENGTH-1;++i){
		DataSum += RcvData[i];
	}
	
	return DataSum;
}

/*****************************************
チェックサムあってるか確認
概要：
	
引数：
	なし
返値：
	なし
******************************************/
uint8_t SerialCom_t::isValidCheckSum(uint8_t* RcvData){
	return RcvData[SERIAL_COM_LENGTH-1] == calcCheckSum(RcvData);
}

/*****************************************
チェックサムをセット
概要：
	最終バイトにチェックサムをセット
引数：
	
返値：
	なし
******************************************/
void SerialCom_t::setCheckSum(uint8_t* RcvData){
	RcvData[SERIAL_COM_LENGTH-1] = calcCheckSum(RcvData);
}

