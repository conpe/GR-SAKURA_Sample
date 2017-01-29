/********************************************/
/*		フタバのコマンドサーボ				*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/07/19		*/
/********************************************/

#include "FutabaCmdServo.h"

// 各レジスタのアドレスとデータ長
//  配列の位置はftcmdsv_comu_contentの値に相当
const uint8_t futaba_cmd_servo::ResisterAdrsLength[24][2] = {
	{0x04, 1},
	{0x05, 1},
	{0x06, 1},
	{0x07, 1},
	{0x08, 2},
	{0x0A, 2},
	{0x0E, 2},
	{0x16, 1},
	{0x17, 1},
	{0x18, 1},
	{0x19, 1},
	{0x1A, 1},
	{0x1B, 1},
	{0x1C, 2},
	{0x1E, 2},	// RAM 
	{0x20, 2},
	{0x23, 1},
	{0x24, 1},
	{0x2A, 2},
	{0x2C, 2},
	{0x2E, 2},
	{0x30, 2},
	{0x32, 2},
	{0x34, 2}
};

bool_t futaba_cmd_servo::fRcv = false;
RingBuffer<futaba_cmd_servo_comu_t*> futaba_cmd_servo::ComusBuff(FTCMDSV_COMUSNUM_DEFAULT);

futaba_cmd_servo_comu_t* futaba_cmd_servo::CurrentComu = NULL;
bool_t futaba_cmd_servo::fAttaching = false;
bool_t futaba_cmd_servo::fManaging = false;

/*********************
サーボ(SCI初期化)
引数：	SCIモジュール番号
		ボーレート (9600～230400)
**********************/
futaba_cmd_servo::futaba_cmd_servo(uint8_t ID, Sci_t* Sci, uint32_t BaudRate){
	
	futaba_cmd_servo::ID = ID;
	
	ServoSci = Sci;
	this->BaudRate = BaudRate;
	
	FailCnt = 0;
	
	fRcv = false;
	fAttaching = false;
	fManaging = false;
	
	LimitCW = FTCMDSV_LIMIT_CW;
	LimitCCW = FTCMDSV_LIMIT_CCW;
}


futaba_cmd_servo::~futaba_cmd_servo(void){
	//delete ComusBuff;
}



int8_t futaba_cmd_servo::begin(void){
	
	if(ServoSci->begin(BaudRate, true, true)){	// use tx, rx
		return -2;
	}
	//ServoSci->enableTxEndInterrupt();
	ServoSci->attachTxEndCallBackFunction(intTxEnd);
	
	return 0;
}

void futaba_cmd_servo::setLimit(int16_t cw, int16_t ccw){
	LimitCW = cw;
	LimitCCW = ccw;
}


// ROM領域

/*********************
サーボID設定
概要：
	サーボのIDを変更する。
	新しいIDを書き込み、保存する。
引数：
		newID	：新しいサーボID
**********************/
void futaba_cmd_servo::setID(uint8_t newID){
	
	
	sendShortPacket(CMDSV_SERVOID, 0x00, CMDSV_ADRS_ServoID, 1, 1, &newID);
	
	ID = newID;
	
	waitEndCommand();
	writeROM();
	waitEndCommand();
	reset();
	waitEndCommand();
}

/*********************
通信速度設定
引数：
		newBR	：新しい通信速度(CMDSV_BR_115200,...)
**********************/
void futaba_cmd_servo::setBaudRate(uint8_t newBR){

	sendShortPacket(CMDSV_BAUDRATE, 0x00, CMDSV_ADRS_BaudRate, 1, 1, &newBR);
	waitEndCommand();
	writeROM();
	waitEndCommand();
	reset();
	waitEndCommand();
}



/*********************
ROMに焼く
引数：	
**********************/
void futaba_cmd_servo::writeROM(void){
	sendShortPacket(CMDSV_WRITEROM, 0x40, 0xFF, 0, 0, &ID);	//&IDはダミー
	waitEndCommand();
}

/*********************
サーボをリセット
引数：	
**********************/
void futaba_cmd_servo::reset(void){
	sendShortPacket(CMDSV_RESET, 0x20, 0xFF, 0, 0, &ID);	//&IDはダミー
	waitEndCommand();
}


// RAM領域

/*********************
目標位置設定
引数：
		angle	：目標角度(0.1度単位 例：90.2度→902 )
**********************/
void futaba_cmd_servo::setGoalPosition(int16_t angle){
	uint8_t ang[2];
	
	// 最大回転角度を制限
	if(LimitCW<angle){
		angle = LimitCW;
	}else if(LimitCCW>angle){
		angle = LimitCCW;
	}
	
	
	ang[0] = angle&0x00FF;
	ang[1] = (angle>>8)&0xFF;
	
	writeMemory(CMDSV_GOALPOSITION, ang);
	
	this->GoalPosition = angle;
	
}

/*********************
移動時間設定
引数：
		TimeMs	：目標時間 [ms]
**********************/
void futaba_cmd_servo::setGoalTime(uint16_t TimeMs){
	uint8_t gt[2];
	uint16_t Time = TimeMs/10;
	
	Time = Time&0x3FFF;
	
	gt[0] = Time&0x00FF;
	gt[1] = (Time>>8)&0xFF;
	
	writeMemory(CMDSV_GOALTIME, gt);
	
}


/*********************
最大トルク
引数：	
	Torque : 最大トルク [%]
**********************/
void futaba_cmd_servo::setMaxTorque(uint8_t MaxTorque){
	
	writeMemory(CMDSV_MAXTORQUE, &MaxTorque);
	
}

/*********************
トルクEnable
引数：
		enagle	：futaba_cmd_servo::TORQUE_ON, futaba_cmd_servo::TORQUE_OFF, futaba_cmd_servo::TORQUE_BRAKE
**********************/
int8_t futaba_cmd_servo::enableTorque(ftcmdsv_torque_enable Enable){
	
	return writeMemory(CMDSV_TORQUEENABLE, (uint8_t*)&Enable);
	
}





	
/*********************
ショートパケットデータ生成
 TxPacketに送るパケットデータをnewして返すので、
 使い終わったらdeleteすること。
引数：	
		**TxPacket: 送信パケット(戻り)
		Flag	：フラグたち
		Address	：メモリアドレス
		Length	：データの長さ(送信時は送信データ長, 受信時は受信データ長, コマンド送るときは0, 初期化時は0xFF)
		Cnt		: サーボの数(ショートパケットでは基本1。コマンド送るだけとかの時は0)
		*Data	：データ
戻値：	
	パケットの長さ
**********************/
int8_t futaba_cmd_servo::genShortPacket(uint8_t **TxPacket, uint8_t Flag, uint8_t Address, uint8_t Length, uint8_t Cnt, uint8_t *TrData){
	uint8_t i;
	uint8_t Sum;
	
	
	uint8_t TxNum = 7+Length*Cnt+1;
	
//	PORTD.PODR.BYTE = 0x81;
	
	*TxPacket = new uint8_t[TxNum];	// ヘッダ+データ+チェックサム
		if(NULL==TxPacket) __heap_chk_fail();
	if( NULL == *TxPacket){
		return -1;
	}
	
	(*TxPacket)[0] = 0xFA;
	(*TxPacket)[1] = 0xAF;
	(*TxPacket)[2] = this->ID;
	(*TxPacket)[3] = Flag;
	(*TxPacket)[4] = Address;
	(*TxPacket)[5] = Length;
	(*TxPacket)[6] = Cnt;
	Sum =  this->ID ^ Flag ^ Address ^ Length ^ Cnt;
	if(0!=Length*Cnt){
		for(i = 0; i<Length*Cnt; i++){
			(*TxPacket)[7+i] = *TrData;
			Sum = Sum ^ *TrData;
			TrData++;
		}
	}
	(*TxPacket)[TxNum-1] = Sum;
	
	
	return TxNum;
}
// 領域は外で確保する版
int8_t futaba_cmd_servo::genShortPacket(uint8_t *TxPacket, uint8_t Flag, uint8_t Address, uint8_t Length, uint8_t Cnt, uint8_t *TrData){
	uint8_t i;
	uint8_t Sum;
	
	uint8_t TxNum = 7+Length*Cnt+1;
	
	(TxPacket)[0] = 0xFA;
	(TxPacket)[1] = 0xAF;
	(TxPacket)[2] = this->ID;
	(TxPacket)[3] = Flag;
	(TxPacket)[4] = Address;
	(TxPacket)[5] = Length;
	(TxPacket)[6] = Cnt;
	Sum =  this->ID ^ Flag ^ Address ^ Length ^ Cnt;
	if(0!=Length*Cnt){
		for(i = 0; i<Length*Cnt; i++){
			(TxPacket)[7+i] = *TrData;
			Sum = Sum ^ *TrData;
			TrData++;
		}
	}
	(TxPacket)[TxNum-1] = Sum;
	
	
	return TxNum;
}


/*********************
ショートパケット送信
　一つのサーボにデータを送る
　サーボ通信クラスをnewしてattachSciComuに投げる
引数：	
		Flag	：フラグたち
		Address	：メモリアドレス
		Length	：1サーボあたりのデータの長さ(送受共)
		Cnt		：サーボの数
		*Data	：データ sizeof(Data) = Length*Cnt
**********************/
int8_t futaba_cmd_servo::sendShortPacket(ftcmdsv_comu_content ComuType, uint8_t Flag, uint8_t Address, uint8_t Length, uint8_t Cnt, uint8_t *TrData){
	uint8_t TxData[10];	// 最大で10バイトまでしか送らないので。Cnt=1の場合。
	int8_t TxNum;
	futaba_cmd_servo_comu_t* NewComu;
	uint8_t RxNum;
	int8_t ack;
	
	
	if(fAttaching){
		return -1;
	}
	
	fAttaching = true;
	
	if(Flag == 0x0F){	// 受信
		RxNum = Length;
	}else{
		RxNum = 0;
	}
	
	TxNum = genShortPacket(TxData, Flag, Address, Length, Cnt, TrData);	// 送信データ生成
	
	if(TxNum<0){
		fAttaching = false;
		return -0x10;
	}
	
	NewComu = new futaba_cmd_servo_comu_t(this, ComuType, TxData, (uint8_t)TxNum, RxNum);
		if(NULL==NewComu) __heap_chk_fail();
	if(NULL == NewComu){
		// 通信パケット領域確保できなかった
		fAttaching = false;
		return -0x20;
	}
	if(0<TxNum){
		if(NULL == NewComu->TxData){
			// 送信バッファ確保できなかった
			fAttaching = false;
			delete NewComu;
			return -0x21;
		}
	}
	if(0<RxNum){
		if(NULL == NewComu->RxData){
			//受信ありなのに受信バッファ確保できなかった
			fAttaching = false;
			delete NewComu;
			return -0x22;
		}
	}
	
	ack = attachComu(NewComu);	// 送信バッファに登録
	if(ack){	// 登録失敗したらなかったことに
		delete NewComu;
	}
	
	fAttaching = false;
	return ack;
}

/*********************
データ書く
ショートパケット生成して通信バッファに突っ込む
引数：	ID		：サーボID
		Flag	：フラグたち 基本ゼロ。ROM書いたりリセットするときに使う
		Address	：メモリアドレス
		Cnt		：送るサーボ数 基本1で。
		Length	：データの長さ
		*Data	：データ
戻値：
	エラー情報
**********************/
int8_t futaba_cmd_servo::writeMemory(ftcmdsv_comu_content ComuType, uint8_t *TrData){
	uint8_t Flag, Address, Length, Cnt;
	
	Flag = 0;	// 送信のみ
	Address = ResisterAdrsLength[ComuType][0];
	Length = ResisterAdrsLength[ComuType][1];
	Cnt = 1;
	
	return sendShortPacket(ComuType, Flag, Address, Length, Cnt, TrData);
}


/*********************
データ読む
引数：	ID		：サーボID
		Address	：メモリアドレス
		RcvNum	：受信データ長
		*RcData	：データ返すとこ
**********************/
void futaba_cmd_servo::readMemory(uint8_t Address, uint8_t RcvNum, uint8_t *RcData){
	
	uint8_t i;
	uint8_t RcDataTmp;
	
	// 送信終了まで待つ
	while(!ServoSci->isIdle());
	
	
	//送信ピンをハイインピーダンスに
	//ここまでに自分で送信したことによるバッファオーバーランエラーを解除
	// -> とりあえずなしで
	
	//ここまでの受信データ捨てる
	ServoSci->clearRxBuff();

	while(RcDataTmp!=0xFD){
		ServoSci->receive(&RcDataTmp);
	}
	while(RcDataTmp!=0xDF){
		ServoSci->receive(&RcDataTmp);
	}
		
	while(ServoSci->receive(&RcDataTmp));	//ID
	while(ServoSci->receive(&RcDataTmp));	//Flag
	while(ServoSci->receive(&RcDataTmp));	//Address
	while(ServoSci->receive(&RcDataTmp));	//Length
	while(ServoSci->receive(&RcDataTmp));	//Cnt
			
	for(i=0; i<RcvNum; i++){
		while(ServoSci->receive(RcData));
		RcData++;
	}
			

};

// パケット通信版
int8_t futaba_cmd_servo::reqReadMemory(ftcmdsv_comu_content ComuType){
	uint8_t Flag, Address, RcvNum, Cnt;
	
	Flag = 0x0F;	// 送信のみ
	Address = ResisterAdrsLength[ComuType][0];
	RcvNum = ResisterAdrsLength[ComuType][1];
	Cnt = 0;
	
	//受信要求
	return sendShortPacket(ComuType, Flag, Address, RcvNum, Cnt, NULL);
}


int8_t futaba_cmd_servo::attachComu(futaba_cmd_servo_comu_t* NewComu){
	
	
	
	if(!ComusBuff.add(NewComu)){	// バッファに追加
		manageComuStart();	//新規追加したので送信してみるよ
		FailCnt = 0;
	}else{
		// ComusBuffがいっぱい。
		// もしかしたら受信待ちで止まってるかも
		// カウンタがマックスいったらリセット処理する。
		FailCnt++;
		//FailCnt = 0;
		if(FTCMDSV_FAILCNT_MAX<FailCnt){
			
			
			fRcv = false;
			endRcv();
			
			// バッファ消す
			futaba_cmd_servo_comu_t* ComuTmp;
			
			while(BUFFER_READ_OK == ComusBuff.read(&ComuTmp)){
				if(NULL!=ComuTmp){
					delete ComuTmp;
				}
			}
			
			if(NULL!=CurrentComu){
				delete CurrentComu;
				CurrentComu = NULL;
			}
			
			FailCnt = 0;	// 解消したとしよう。
			
			if(BUFFER_ADD_OK == ComusBuff.add(NewComu)){
				manageComuStart();	// 送信してみる
				return 0;
			}
			
			
		}
		return -1;	// 追加失敗
	}
	
	return 0;
}



/*********************
コマンド送信後に待つ
引数：	
**********************/
void futaba_cmd_servo::waitEndCommand(void){
	
	volatile uint16_t i;
	
	for(i=0;i<60000;i++);
	
}




/*********************
// staticな人たち
**********************/
/*********************
通信処理
次のパケットを送信する
**********************/
int8_t futaba_cmd_servo::manageComuStart(void){
	uint8_t i;
	futaba_cmd_servo_comu_t* CurrentComuTmp;
	
	
	if(!fRcv){	//受信待ちでなければ
		
		
		// 受信待ち中でないのでパケット送信する
		if(0==ComusBuff.watch(&CurrentComuTmp)){		// 通信オブジェクトを一つ読む
			
			if(ServoSci->getTxBuffFreeSpace() >= CurrentComuTmp->TxNum){		// 送信したい数以上のSCI送信バッファが開いてる
			
				if(fManaging){
					return -1;
				}
				fManaging = true;
	
				// 受信ありならフラグ立てておく
				if(CurrentComuTmp->RxNum > 0){		// 受信数が正なら受信する
					fRcv = true;
				}else{
					
				}
				// SCIにデータ送る
				if(0<CurrentComuTmp->TxNum){
					// 送信完了割り込みdisable
					//ServoSci->disableTxEndInterrupt();
					ServoSci->detachTxEndCallBackFunction();
					for(i=0; i<CurrentComuTmp->TxNum; i++){
						if(i>=CurrentComuTmp->TxNum-1){
							//ServoSci->enableTxEndInterrupt();	// 最後の一回で有効にする
							ServoSci->attachTxEndCallBackFunction(intTxEnd);
						}
						ServoSci->transmit(CurrentComuTmp->TxData[i]);
					}
				}
				//ServoSci->disableTxEndInterrupt();
				
				// CurrentComu更新
				if(NULL!=CurrentComu){
					delete CurrentComu;				// 前まで通信してたオブジェクトをメモリから削除
					CurrentComu = NULL;
				}
				
				ComusBuff.read(&CurrentComu);	// 通信オブジェクトを一つ取り出してカレントに
				
				fManaging = false;
				
				if(!fRcv){
					return manageComuStart();	//通信オブジェクトある限り繰り返す。受信時は受信待ちになるので繰り返さない。
				}
			}
		}
	}else{
	}
	
	return 0;
}


// 送信完了
void futaba_cmd_servo::intTxEnd(void){
	if(NULL != CurrentComu){
		if(fRcv){
			CurrentComu->FTCMDSV->startRcv();	// 受信開始
		}else{
			CurrentComu->FTCMDSV->manageComuStart();
		}
	}else{
		
	}
}

// 受信開始
void futaba_cmd_servo::startRcv(void){
	
	// 送信完了割り込みdisable
	//ServoSci->disableTxEndInterrupt();
	ServoSci->detachTxEndCallBackFunction();
	
	// 受信割り込みon // デフォでon
	// 受信バッファ消す
	ServoSci->clearRxBuff();
	// コールバック登録
	ServoSci->attachRxCallBackFunction(intRx);
	
}
// 受信終わり
void futaba_cmd_servo::endRcv(void){
	// 受信割り込みoff // デフォでon
	// コールバック取り消し
	ServoSci->detachRxCallBackFunction();
	
}



// 受信
void futaba_cmd_servo::intRx(RingBuffer<uint8_t> * RxBuff){
	if(fRcv){
		if(NULL!=CurrentComu){
			CurrentComu->FTCMDSV->manageComuReceive(RxBuff);	// 受信処理
		}
	}
	
}

// 受信データのパケットを解析
void futaba_cmd_servo::manageComuReceive(RingBuffer<uint8_t> * RxBuff){
	uint8_t RcDataTmp;
			
	while(!RxBuff->isEmpty()){	// バッファなくなるまで繰り返し
		if(CurrentComu->ReceivedNum==0){
			ServoSci->receive(&RcDataTmp);	// 0xFD // ←これでもbuffから読むので結局一緒では
			//RxBuff->read();
			CurrentComu->ReceivedNum++;
		}else if(CurrentComu->ReceivedNum==1){
			ServoSci->receive(&RcDataTmp);	// 0xDF
			//RxBuff->read();
			CurrentComu->ReceivedNum++;
			CurrentComu->Sum = 0;
		}else if(CurrentComu->ReceivedNum==2){
			ServoSci->receive(&RcDataTmp);	//ID
			//RxBuff->read();
			CurrentComu->ReceivedNum++;
			CurrentComu->Sum ^= RcDataTmp;
		}else if(CurrentComu->ReceivedNum==3){
			ServoSci->receive(&RcDataTmp);	//Flag
			//RxBuff->read();
			CurrentComu->ReceivedNum++;
			CurrentComu->Sum ^= RcDataTmp;
		}else if(CurrentComu->ReceivedNum==4){
			ServoSci->receive(&RcDataTmp);	//Address
			//RxBuff->read();
			CurrentComu->ReceivedNum++;
			CurrentComu->Sum ^= RcDataTmp;
		}else if(CurrentComu->ReceivedNum==5){
			ServoSci->receive(&RcDataTmp);	//Length
			//RxBuff->read();
			CurrentComu->ReceivedNum++;
			CurrentComu->Sum ^= RcDataTmp;
		}else if(CurrentComu->ReceivedNum==6){
			ServoSci->receive(&RcDataTmp);	//Cnt
			//RxBuff->read();
			CurrentComu->ReceivedNum++;
			CurrentComu->Sum ^= RcDataTmp;
		}else if(CurrentComu->ReceivedNum < (7+CurrentComu->RxNum)){	// 受信内容
			ServoSci->receive(&RcDataTmp);
			//RxBuff->read(&RcDataTmp);
			CurrentComu->RxData[CurrentComu->ReceivedNum-7] = RcDataTmp;
			
			CurrentComu->ReceivedNum++;
			CurrentComu->Sum ^= RcDataTmp;
			
		}else{	// チェックサム
			ServoSci->receive(&RcDataTmp);	// Sum
			//RxBuff->read();
			// 受信データ処理
			if(CurrentComu->Sum == RcDataTmp){	// 受信オッケー
				fetchRcvData(CurrentComu);
			}
			// 次の通信開始
			endRcv();		// 受信モード終了
			fRcv = false;		// 受信中フラグ取り下げ
			manageComuStart();	//送信データたまってたら送る
		}
	}
	
}

void futaba_cmd_servo::fetchRcvData(futaba_cmd_servo_comu_t* Comu){
	uint16_t retdata_uint16;
	
	switch(Comu->ComuType){
	case CMDSV_PRESENTPOSITION:
		
		retdata_uint16 = ( (((int16_t)Comu->RxData[1])<<8) | (int16_t)Comu->RxData[0]);
		
		if( retdata_uint16 > 65535/2){
			ResisterData.PresentPosition = (int16_t)(retdata_uint16 - 65536);
		}else{
			ResisterData.PresentPosition = (int16_t)retdata_uint16;
		}
		break;
	case CMDSV_PRESENTTIME:
		ResisterData.PresentTime = ( (((uint16_t)Comu->RxData[1])<<8) | (uint16_t)Comu->RxData[0]);
		break;
	case CMDSV_PRESENTSPEED:
	
		retdata_uint16 = ( (((int16_t)Comu->RxData[1])<<8) | (int16_t)Comu->RxData[0]);
		
		if( retdata_uint16 > 65535/2){
			ResisterData.PresentSpeed = (int16_t)(retdata_uint16 - 65536);
		}else{
			ResisterData.PresentSpeed = (int16_t)retdata_uint16;
		}
		break;
	case CMDSV_PRESENTCURRENT:
		ResisterData.PresentCurrent = ( (((uint16_t)Comu->RxData[1])<<8) | (uint16_t)Comu->RxData[0]);
		break;
	case CMDSV_PRESENTVOLTS:
		ResisterData.PresentVolts = ( (((uint16_t)Comu->RxData[1])<<8) | (uint16_t)Comu->RxData[0]);
		break;
	case CMDSV_PRESENTTEMPERATURE:
	
		retdata_uint16 = ( (((int16_t)Comu->RxData[1])<<8) | (int16_t)Comu->RxData[0]);
		
		if( retdata_uint16 > 65535/2){
			ResisterData.PresentTemperature = (int16_t)(retdata_uint16 - 65536);
		}else{
			ResisterData.PresentTemperature = (int16_t)retdata_uint16;
		}
		break;
		
		
		
	}
	
}