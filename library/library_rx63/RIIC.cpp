/********************************************/
/*		RIICモジュール使用 I2C通信			*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/02/26		*/
/********************************************/

#include "RIIC.h"

// RIIC module 
// Rx63n is master. 

//絶対beginするんだぞっ

//【更新予定】
// 送信バッファを持たないようにしたい(通信オブジェクトに持たせる)
// 通信相手によって違うバッファを使う仕組みほしいな？


I2c_t I2C0(I2C_RIIC0);	//
//I2c_t I2C1(I2C_RIIC1);	//
I2c_t I2C2(I2C_RIIC2);	//
//I2c_t I2C3(I2C_RIIC3);	//



/****************************
 I2C コンストラクタ
概要：
引数：
 モジュールNo.
返値：
 なし
****************************/
I2c_t::I2c_t(i2c_module I2cModule)
{
	_ModuleNum = I2cModule;		
	fBegun = false;				// 未begin()
	
	//レジスタ
	switch(I2cModule){
	case I2C_RIIC0:
		I2Creg = &RIIC0;
		break;
	case I2C_RIIC1:
		I2Creg = &RIIC1;
		break;
	case I2C_RIIC2:
		I2Creg = &RIIC2;
		break;
	case I2C_RIIC3:
		I2Creg = &RIIC3;
		break;
	default:
		break;
	}
	
	NextAttachIndex = 0;
	CurrentComuIndex = 0;
	
	
}

/****************************
 I2C デストラクタ
概要：
 確保したメモリを開放する
引数：
 なし
返値：
 なし
****************************/
I2c_t::~I2c_t(void){
	if(TxBuff) delete TxBuff;
	if(ComusBuff) delete ComusBuff;
}


/****************************
 I2C通信初期化
概要：
 RIICモジュールの初期化を行う。
 初回のみ。
引数：
	uint16_t Baud_kbps	ボーレート[kbps]単位
	uint16_t TxBuffNum	送信バッファの数
返値：
 送受信バッファ、通信内容バッファの領域を確保できなければ-1
****************************/
int8_t I2c_t::begin(uint16_t Baud_kbps, uint16_t TxBuffNum, uint16_t ComusNum){
	
	if(!fBegun){	// 初回のみ
		fBegun = true;
		
		//if(Status!=I2C_IDLE){
			// 通信止める
			
			// ステータス更新
			Status = I2C_IDLE;
		//}
		
		if(Baud_kbps<=100){
			Baud_kbps = 100;
		}else{
			Baud_kbps = 400;
		}
		
		this->Baud_kbps = Baud_kbps;
		
		// RIIC setting
		initRegister(Baud_kbps);
		
		// pin mode setting
		setPinModeI2C();
		
		//Buffer
		TxBuff = new RingBuffer<uint8_t>(TxBuffNum);
		if(TxBuff==NULL){
			return -1;
		}
		/*
		// 受信バッファは通信オブジェクトで持つため、RIICクラスとしては持たない。
		if(RxBuff) delete RxBuff;
		RxBuff = new RingBuffer<uint8_t>(BuffNum);
		if(RxBuff==NULL){
			return -1;
		}
		*/
		
		// 通信内容バッファ
		this->ComusNum = ComusNum;
		ComusBuff = new RingBuffer<I2c_comu_t*>(this->ComusNum);
		if(ComusBuff==NULL){
			return -1;
		}
	}else{
		return 1;	// begin済
	}
	
	return 0;
}
int8_t I2c_t::begin(uint16_t Baud_kbps, uint16_t TxBuffNum){
	return begin(Baud_kbps, TxBuffNum, I2C_COMUSNUM_DEFAULT);
}
int8_t I2c_t::begin(uint16_t Baud_kbps){
	return begin(Baud_kbps, I2C_BUFFER_SIZE_DEFAULT, I2C_COMUSNUM_DEFAULT);
}
int8_t I2c_t::begin(void){
	return begin(I2C_BAUDRATE_DEFAULT, I2C_BUFFER_SIZE_DEFAULT, I2C_COMUSNUM_DEFAULT);
}



/****************************
 RIICレジスタ設定
概要：
引数：
 ボーレート[kbps](100or400)
返値：
 なし
****************************/
void I2c_t::initRegister(uint16_t Baud_kbps){
	uint8_t BaudCnt;
	
	// モジュール起こす
	SYSTEM.PRCR.WORD = 0xA503u;
	switch(_ModuleNum){
	case I2C_RIIC0:
		MSTP(RIIC0) = 0u;
		break;
	case I2C_RIIC1:
		MSTP(RIIC1) = 0u;
		break;
	case I2C_RIIC2:
		MSTP(RIIC2) = 0u;
		break;
	case I2C_RIIC3:
		MSTP(RIIC3) = 0u;
		break;
	}
		
	
	// I2Cリセット
	I2Creg->ICCR1.BIT.ICE = 0;		
	I2Creg->ICCR1.BIT.IICRST = 1;	
	I2Creg->ICCR1.BIT.ICE = 1;	
	
	// ビットレート設定
	//ICMR1.CKS
	//ICBRL 0～31
	//ICBRH
	//getPCLK();
	//I2C_CLOCK_DIV128
	switch(Baud_kbps){
	case 100:	// 立ち上がり立ち下がり時間をそれぞれ1000ns, 300nsとする
		I2Creg->ICMR1.BIT.CKS = I2C_CLOCK_DIV8;
		BaudCnt = (uint8_t)((float)::getPCLK() / 8.0 * (1.0/(float)Baud_kbps/1000.0 - 0.0000013));
		// BaudCnt = 53;
		I2Creg->ICBRH.BIT.BRH = BaudCnt/2;
		I2Creg->ICBRL.BIT.BRL = (BaudCnt+1)/2;
		break;
	case 400:	// 立ち上がり立ち下がり時間をそれぞれ300nsとする
		I2Creg->ICMR1.BIT.CKS = I2C_CLOCK_DIV4;
		BaudCnt = (uint8_t)((float)::getPCLK() / 4.0 * (1.0/(float)Baud_kbps/1000.0 - 0.0000006));
		// BaudCnt = 23;
		I2Creg->ICBRH.BIT.BRH = BaudCnt/2;
		I2Creg->ICBRL.BIT.BRL = (BaudCnt+1)/2;
		break;
	}
	
	//ICMR2
	//ICMR3
	// タイムアウト機能使うなら
	/*
	ICMR2.TMWE
	TMOCNTL=00h;
	TMOCNTU=00h;
	ICFER.TMOE=1;
	*/
	
	// いろんな検出とかの機能設定
	//ICFER
	I2Creg->ICFER.BIT.MALE = 1;	// アービトレーションロスト検出
	
	
	// 割り込み設定
	//ICIER
	//受信データフル割り込み許可
	I2Creg->ICIER.BIT.RIE = 1;
	//送信データエンプティ割り込み許可
	I2Creg->ICIER.BIT.TIE = 1;	
	//送信終了割り込み許可
	I2Creg->ICIER.BIT.TEIE = 1;
	//ストップコンディション割り込み許可
	I2Creg->ICIER.BIT.SPIE = 1;	
	//NACK受信割り込み許可(スレーブいない)
	I2Creg->ICIER.BIT.NAKIE = 1;
	
	// ACKBTビットへの書き込み許可
	I2Creg->ICMR3.BIT.ACKWP = 1;
	
	// 転送中断許可
	I2Creg->ICFER.BIT.NACKE = 1;
	
	// アービトレーションロスト検出割り込み許可(EEI割り込み)
	I2Creg->ICIER.BIT.ALIE = 1;
	
	
	switch(_ModuleNum){
	case I2C_RIIC0:
		IEN(RIIC0,RXI0) = 1;
		IPR(RIIC0,RXI0) = 9;
		IEN(RIIC0,TXI0) = 1;
		IPR(RIIC0,TXI0) = 9;
		IEN(RIIC0,TEI0) = 1;
		IPR(RIIC0,TEI0) = 9;
		IEN(RIIC0,EEI0) = 1;
		IPR(RIIC0,EEI0) = 9;
		break;
	case I2C_RIIC1:
		IEN(RIIC1,RXI1) = 1;
		IPR(RIIC1,RXI1) = 9;
		IEN(RIIC1,TXI1) = 1;
		IPR(RIIC1,TXI1) = 9;
		IEN(RIIC1,TEI1) = 1;
		IPR(RIIC1,TEI1) = 9;
		IEN(RIIC1,EEI1) = 1;
		IPR(RIIC1,EEI1) = 9;
		break;
	case I2C_RIIC2:
		IEN(RIIC2,RXI2) = 1;
		IPR(RIIC2,RXI2) = 9;
		IEN(RIIC2,TXI2) = 1;
		IPR(RIIC2,TXI2) = 9;
		IEN(RIIC2,TEI2) = 1;
		IPR(RIIC2,TEI2) = 9;
		IEN(RIIC2,EEI2) = 1;
		IPR(RIIC2,EEI2) = 9;
		break;
	}
	
	// 内部リセット解除
	I2Creg->ICCR1.BIT.IICRST = 0;
	
}

/****************************
 I2C関係入出力ポート設定
概要：
引数：
 なし
返値：
 なし
****************************/
void I2c_t::setPinModeI2C(void){
	
	// ポート出力設定
	MPC.PWPR.BYTE = 0x00u;
	MPC.PWPR.BYTE = 0x40u;
	SYSTEM.PRCR.WORD = 0xA503u;
	switch(_ModuleNum){
	case I2C_RIIC0:
		setPinMode(PORT_SCL0, PIN_INPUT);	// output
		MPC.P12PFS.BIT.PSEL = 0x0F;			// SCL0
		PORT1.PMR.BIT.B2 = 1;				// not GPIO	
		setPinMode(PORT_SDA0, PIN_INPUT);	// output
		MPC.P13PFS.BIT.PSEL = 0x0F;			// SDA0 
		PORT1.PMR.BIT.B3 = 1;				// not GPIO	
		break;
	case I2C_RIIC1:
		break;
	case I2C_RIIC2:
		setPinMode(PORT_SCL2, PIN_INPUT);	// output
		MPC.P16PFS.BIT.PSEL = 0x0F;			// SCL2
		PORT1.PMR.BIT.B6 = 1;				// not GPIO	
		setPinMode(PORT_SDA2, PIN_INPUT);	// output
		MPC.P17PFS.BIT.PSEL = 0x0F;			// SDA2 
		PORT1.PMR.BIT.B7 = 1;				// not GPIO	
		break;
	}
	
}


/****************************
 I2C通信登録
概要：
引数：
 送信相手アドレス
 送信データ
 送信データ数
 受信データ
 受信データ数
 コールバック関数(通信終了時に呼ばれる)
返値：
 AttachIndex, 
 -1 : Attach数がいっぱい(I2C_COMUS)なら-1,
 -2 : Comusの領域を確保できなければ-2
****************************/


int8_t I2c_t::attach(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum, void (*CallBackFunc)(I2c_comu_t*)){		// 送受信
	int8_t AttachIndex;
	I2c_comu_t* Comu;
	
	if(!ComusBuff->isFull()){
		AttachIndex = ComusBuff->getWriteIndex();
		Comu = new I2c_comu_general_t(AttachIndex, DestAddress, TxData, TxNum, RxNum, CallBackFunc);
		if(Comu == NULL){	// new失敗
			return -2;
		}
		ComusBuff->add(Comu);	// 通信内容登録
		
		if(Status == I2C_IDLE){	// 送信止まってる状態だったら
			startComu();	// 送信開始
		}
	}else{
		
		// 詰まってるぽいのでI2Cリセット
		resetI2C();
		
		return -1;
	}
	
	return AttachIndex;
}
// 送信のみ
int8_t I2c_t::attach(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, void (*CallBackFunc)(I2c_comu_t* Comu)){						// 送信のみ
	return attach(DestAddress, TxData, TxNum, 0, CallBackFunc);
}
// 送信のみ(コールバック関数なし)
int8_t I2c_t::attach(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum){		// 送信のみ
	return attach(DestAddress, TxData, TxNum, 0, NULL);
}
// 通信クラスでアタッチ
// -1:バッファ詰まってる(呼び出し元でComuはdeleteしてね)
// -2:通信開始失敗
int8_t I2c_t::attach(I2c_comu_t* AttachComu){
	//int8_t AttachIndex;
	
	//AttachIndex = ComusBuff->getWriteIndex();
	if(!ComusBuff->add(AttachComu)){	// 通信内容登録
	// 登録成功
		if(I2C_IDLE == Status){	// 送信止まってる状態だったら
			if(startComu()){	// 送信開始
				// 開始失敗
				return -2;
			}
		}
	}else{
		
		// 詰まってるぽいのでI2Cリセット
		resetI2C();
		
		return -1;
	}
	
	return 0;
}

/****************************
 I2C通信開始
引数
 なし
返値
 0	:正常
 -1	:通信中
 -2	:通信するものが登録されていない
****************************/
int8_t I2c_t::startComu(void){
	int8_t ack = 0;
	I2c_comu_t* CurrentComuTmp;
	
	//if((Status == I2C_IDLE)&&(!isBusyBus())){	// 通信中でない
	if((Status == I2C_IDLE)){	// 通信中でない
							// isBusyBusって通信終わってちょっとしないと0にならないのが心配
		if(!ComusBuff->watch(&CurrentComuTmp)){		// バッファから通信情報取得&&バッファよめた
			// 通信開始
			if(CurrentComuTmp->RxNum > 0){	// 受信もする
				ack = transmit_receive(CurrentComuTmp->DestAddress, CurrentComuTmp->TxData, CurrentComuTmp->TxNum, CurrentComuTmp->RxData, CurrentComuTmp->RxNum );	// 送受信
			}else{
				ack = transmit(CurrentComuTmp->DestAddress, CurrentComuTmp->TxData, CurrentComuTmp->TxNum);	// 送信
			}
			if(!ack){	// 通信開始成功ならCurrentComu更新
				ComusBuff->read(&CurrentComu);
			}
		}else{
			ack = -2;	// バッファが空
		}
	}else{
		ack = -1;	// まだ通信中です
	}
	
	return ack;
}
	




/****************************
 I2C送信開始(単発モード)
引数
 送信相手アドレス
 送信データ
 送信データ数
返値
 0	: 正常
 -1	: ステータスがI2C_IDLEじゃないか、バスがビジー状態
****************************/
int8_t I2c_t::transmit(uint8_t DstAddress, uint8_t* TrData, uint8_t TrDataNum){
	int8_t ack = 0x00;
	//if((Status == I2C_IDLE)&&(!isBusyBus())){
		// ステータス更新
		Status = I2C_TRANSMIT;
		// バッファに入れていく
		ack |= TxBuff->add(DstAddress<<1);
		for(int i=0; i<TrDataNum; i++){
			ack |= TxBuff->add(*TrData++);
		}
		
		// カウントリセット
		CntTransmit = TrDataNum;
		
		// スタートビット発行要求
		reqStartCondition();	// 実際に発行されるとTDREの割り込みが入り、一連の処理が始まる。
		
	//}else{
	//	return -1;
	//}
	
	return ack;
}

/****************************
 I2C受信開始(単発モード)
引数
 受信相手アドレス
 受信データ
 受信データ数
返値
 0	: 正常
 -1	: ステータスがI2C_IDLEじゃないか、バスがビジー状態
****************************/
int8_t I2c_t::receive(uint8_t DstAddress, uint8_t* RcData, uint8_t RcDataNum){
	int8_t ack;
	//if((Status == I2C_IDLE)&&(!isBusyBus())){
		// ステータス更新
		Status = I2C_RECEIVE;
		// 送受信情報
		ack = TxBuff->add((DstAddress<<1) | 0x01);	// Readモードにする
		RcvBuffer = RcData;
		
		// カウントリセット
		CntTransmit = 1;
		CntReceive = RcDataNum;
		I2c_t::RcDataNum = RcDataNum;
		
		// スタートビット発行要求
		reqStartCondition();	// 実際に発行されるとTDREの割り込みが入り、一連の処理が始まる。
	//}else{
	//	return -1;
	//}
	return ack;
}

/****************************
 I2C送受信開始(単発モード)
引数
 通信相手アドレス
 送信データ
 送信データ数
 受信データ
 受信データ数
返値
 0	: 正常
 -1	: ステータスがI2C_IDLEじゃないか、バスがビジー状態
****************************/
int8_t I2c_t::transmit_receive(uint8_t DstAddress, uint8_t* TrData, uint8_t TrDataNum, uint8_t* RcData, uint8_t RcDataNum){
	int8_t ack = 0x00;
	//if((Status == I2C_IDLE)&&(!isBusyBus())){
		// ステータス更新
		Status = I2C_TRANSMIT_RECEIVE;
		// バッファに入れていく
		ack |= TxBuff->add(DstAddress<<1);				// 相手アドレス
		for(uint16_t i=0; i<TrDataNum; i++){
			ack |= TxBuff->add(*TrData++);				// 送信データ
		}
		RcvBuffer = RcData;
		I2c_t::DstAddress = DstAddress;
		
		
		// カウントリセット
		CntTransmit = TrDataNum;
		CntReceive = RcDataNum;
		I2c_t::RcDataNum = RcDataNum;
		
		// スタートビット発行要求
		reqStartCondition();	// 実際に発行されるとTDREの割り込みが入り、一連の処理が始まる。
	//}else{
	//	return -1;
	//}
	
	return ack;
}


/****************************
 I2Cリセット処理
****************************/
void I2c_t::resetI2C(void){
	//I2c_comu_t* Comu;
	//bool IcferMaleTmp;
	uint8_t ReClkCnt = 8;	// 追加クロック最大数
	uint16_t ReClkCntC = 400;	// 追加クロック最大数
	
	//if(0==I2Creg->ICCR1.BIT.SDAI){	// SDAラインがlowになってる
			
		Status = I2C_RESET;
		
		
		// リセット(initResister内で実施)
		//I2Creg->ICCR1.BIT.ICE = 0;		
		//I2Creg->ICCR1.BIT.IICRST = 1; // これやるとレジスタ設定しなおし要
		
		// RIIC setting
		initRegister(Baud_kbps);
		
		// pin mode setting
		setPinModeI2C();
		
		
		// バッファ開放
		/*
		// 登録元で要求だしたままだと思っちゃうので消しちゃだめでしょでしょ
		while(!ComusBuff->isEmpty()){
			ComusBuff->read(&Comu);
			delete Comu;
		}
		*/
		
		// アービトレーションロスト検出機能一旦無効化
		//IcferMaleTmp = I2Creg->ICFER.BIT.MALE;
		I2Creg->ICFER.BIT.MALE = 0;
		
		// 追加クロック出力
		while(0==I2Creg->ICCR1.BIT.SDAI){	// スレーブがlow固定にしてる間
			if(0==I2Creg->ICCR1.BIT.CLO){
				I2Creg->ICCR1.BIT.CLO = 1;		// 追加クロックを送って開放されるか試す
				// 指定回数超えたらやめる
				ReClkCnt--;	
				if(!ReClkCnt){
					break;
				}
			}
			// 指定回数超えたらやめる
			ReClkCntC--;	
			if(!ReClkCntC){
				break;
			}
		}
		
		//I2Creg->ICFER.BIT.MALE = IcferMaleTmp;
		
		
		reqStartCondition();
	//}
};





/****************************
 I2C送信割り込み
指定回数分データを送る。

呼ばれる条件：
 TDREが立った(ICDRT レジスタが空になった)
 TRSビットが立った(スタートコンディションが発行された)

****************************/
void I2c_t::isrTx(void){
	uint8_t Data;
	
	switch(Status){
	case I2C_TRANSMIT:
		if(I2Creg->ICSR2.BIT.NACKF == 0){
			if(!TxBuff->read(&Data)){	// データあるだけ送る
				I2Creg->ICDRT = Data;
				CntTransmit--;
			}
		}else{
			I2Creg->ICSR2.BIT.NACKF = 0;
			isrTxEnd();	// 送信強制終了
		}
		break;
		
	case I2C_RECEIVE:
		// 相手アドレス送る
		if(!TxBuff->read(&Data)){
			I2Creg->ICDRT = Data;
		}
		
		break;
	case I2C_TRANSMIT_RECEIVE:
		// [送信]データ送る
		if(CntTransmit>=0){
			if(I2Creg->ICSR2.BIT.NACKF == 0){
				if(!TxBuff->read(&Data)){	// データあるだけ送る
					I2Creg->ICDRT = Data;
				}
			}else{
				isrTxEnd();	// 送信強制終了
			}
			CntTransmit--;
		}else{
			
			if(StatusIn == I2C_RESTART){	// リスタートコンディション送った後に来た！
				// [受信]相手アドレス送る
				I2Creg->ICDRT = (DstAddress<<1) | 0x01;
				StatusIn = I2C_TRANSMIT_DSTADDRESS_R;
				// 受信割り込み待ち
			}
		}
		
		break;
	case I2C_RESET:
		reqStopCondition();
		break;
	}
}


/****************************
 I2C送信完了割り込み
すべてのデータの送信が終わった時に呼ばれる。
ストップコンディションもしくはリスタートコンディションを発行する。
****************************/
void I2c_t::isrTxEnd(void){
	I2Creg->ICSR2.BIT.TEND = 0;
	
	switch(Status){
	case I2C_TRANSMIT:
			I2Creg->ICSR2.BIT.STOP = 0;
			reqStopCondition();
		break;
	case I2C_TRANSMIT_RECEIVE:
		if(!I2Creg->ICSR2.BIT.NACKF){
			
			if(CurrentComu->Err == false){
				reqRestartCondition();
				StatusIn = I2C_RESTART;
			}else{
				I2Creg->ICSR2.BIT.STOP = 0;
				reqStopCondition();
			}
		}
		break;
	}
}
/****************************
 I2Cストップコンディション完了割り込み
****************************/
void I2c_t::isrStop(void){
	
	I2Creg->ICMR3.BIT.RDRFS = 0;
	I2Creg->ICMR3.BIT.ACKBT = 0;
	I2Creg->ICSR2.BIT.NACKF = 0;
	I2Creg->ICSR2.BIT.STOP = 0;
	I2Creg->ICFER.BIT.MALE = 1;	// I2C_RESETのときMALEオフにしてるのでここでオン
	//if(I2C_RESET != Status){
	//void (I2c_comu_t::*pFunc)() = &I2c_comu_t::callBack;
	
	//if(NULL != (CurrentComu->*pFunc())){
		
	if(NULL != CurrentComu){
		// コールバック関数実行
		CurrentComu->callBack();
		// 通信内容消す
		delete CurrentComu;
		CurrentComu = NULL;
	}
	TxBuff->clear();
	Status = I2C_IDLE;
	// 次の通信開始
	startComu();
	
}


/****************************
 I2C受信割り込み
****************************/
void I2c_t::isrRx(void){
	
	if(CurrentComu->Err == false){
		
		if(!I2Creg->ICSR2.BIT.NACKF){	// このチェックは初回だけかも！
			if((CntReceive<=2) && !I2Creg->ICMR3.BIT.WAIT){
				I2Creg->ICMR3.BIT.WAIT = 1;
			}
			if(CntReceive == 1){
				I2Creg->ICMR3.BIT.RDRFS = 1;	//RDRF(受信データフル)フラグをSCLクロックの8クロック目の立ち上がり時に“1”にする
			}
			
			if(CntReceive == 0){
				I2Creg->ICSR2.BIT.STOP = 0;	// ストップコンディション出てるフラグクリア
				reqStopCondition();
				RcvBuffer[RcDataNum-1] = I2Creg->ICDRR;	// 最終データ受信

				I2Creg->ICMR3.BIT.ACKBT = 1;	// （NACK送信））
				I2Creg->ICMR3.BIT.WAIT = 0;
				// -> ストップビット発行待ち
				StatusIn = I2C_READ_END;
			}else{
				if(RcDataNum==CntReceive){		// 初回特典
					0 == I2Creg->ICDRR;	// ダミーリード(何か評価しないと読んだことにならないみたい。最適化で消えないか要注意)
				}else{	// データ受信
					RcvBuffer[RcDataNum-CntReceive-1] = I2Creg->ICDRR;

					StatusIn = I2C_READ_DATA;
				}
			}
			
			CntReceive--;
			
		}else{
			// スレーブいないのでストップコンディション出して終了
			I2Creg->ICSR2.BIT.STOP = 0;
			reqStopCondition();
			I2Creg->ICDRR;
			
			CurrentComu->Err = true;
		}
	}
	
}



/****************************
 I2C スレーブいない
****************************/
void I2c_t::isrNack(void){
	StatusIn = I2C_NACK;
	
	CurrentComu->Err = true;
	
	//I2Creg->ICFER.BIT.NACKE = 0;
	I2Creg->ICSR2.BIT.NACKF = 0;
	I2Creg->ICSR2.BIT.STOP = 0;
	reqStopCondition();

}

/****************************
 I2C アービトレーションロスト検出
 SDAラインがスレーブに抑えられちゃってる
****************************/
void I2c_t::isrArbitrationLost(void){
	
	CurrentComu->Err = true;
	resetI2C();
	
}


// interrupt
#pragma section IntPRG

#pragma interrupt (Excep_RIIC0_TXI0(enable, vect=VECT(RIIC0,TXI0)))
void Excep_RIIC0_TXI0(void)
{
	I2C0.isrTx();
}
#pragma interrupt (Excep_RIIC0_RXI0(enable, vect=VECT(RIIC0,RXI0)))
void Excep_RIIC0_RXI0(void){
	I2C0.isrRx();
}
#pragma interrupt (Excep_RIIC0_TEI0(enable, vect=VECT(RIIC0,TEI0)))
void Excep_RIIC0_TEI0(void){
	I2C0.isrTxEnd();
}
#pragma interrupt (Excep_RIIC0_EEI0(enable, vect=VECT(RIIC0,EEI0)))
void Excep_RIIC0_EEI0(void){
	//IEN(RIIC0,EEI0) = 0;	// 自身が重ねて入らないようにする
	
	if(RIIC0.ICSR2.BIT.STOP){	// ストップコンディション
		I2C0.isrStop();
	}
	if(RIIC0.ICSR2.BIT.NACKF){	// NACK受信(スレーブいない)
		RIIC0.ICSR2.BIT.NACKF = 0;
		I2C0.isrNack();
	}
	if(RIIC0.ICSR2.BIT.AL){
		RIIC0.ICSR2.BIT.AL = 0;
		I2C0.isrArbitrationLost();	// アービトレーションロストなのでリセットする
	}
	
	//IEN(RIIC0,EEI0) = 1;
}
#pragma interrupt (Excep_RIIC2_TXI2(enable, vect=VECT(RIIC2,TXI2)))
void Excep_RIIC2_TXI2(void)
{
	I2C2.isrTx();
}
#pragma interrupt (Excep_RIIC2_RXI2(enable, vect=VECT(RIIC2,RXI2)))
void Excep_RIIC2_RXI2(void){
	I2C2.isrRx();
}
#pragma interrupt (Excep_RIIC2_TEI2(enable, vect=VECT(RIIC2,TEI2)))
void Excep_RIIC2_TEI2(void){
	I2C2.isrTxEnd();
}
#pragma interrupt (Excep_RIIC2_EEI2(enable, vect=VECT(RIIC2,EEI2)))
void Excep_RIIC2_EEI2(void){
	if(RIIC2.ICSR2.BIT.STOP){
		I2C2.isrStop();
	}
	if(RIIC2.ICSR2.BIT.NACKF){	// NACK受信(スレーブいない)
		RIIC2.ICSR2.BIT.NACKF = 0;
		I2C2.isrNack();
	}
	if(RIIC2.ICSR2.BIT.AL){
		RIIC2.ICSR2.BIT.AL = 0;
		I2C2.isrArbitrationLost();	// アービトレーションロストなのでリセットする
	}
}


#pragma section 



/****************************
 通信データクラス コンストラクタ
 送受信
****************************/
I2c_comu_t::I2c_comu_t(void){
	
}

I2c_comu_t::I2c_comu_t(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum){
	setI2c(AttachIndex, DestAddress, TxData, TxNum, RxNum);
}
I2c_comu_t::I2c_comu_t(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum){
	setI2c(0, DestAddress, TxData, TxNum, RxNum);
}


/****************************
 通信データクラス コンストラクタ
 送信のみ
  受信バッファはNULL, 受信数は0とする。
****************************/
I2c_comu_t::I2c_comu_t(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum){
	setI2c(AttachIndex, DestAddress, TxData, TxNum, 0);
}

/****************************
 通信データクラス デストラクタ
****************************/
I2c_comu_t::~I2c_comu_t(void){
	
	if(TxData){
		delete[] TxData;
	}
	if(RxData){
		delete[] RxData;
	}
}


void I2c_comu_t::setI2c(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum){

	this->AttachIndex = AttachIndex;
	this->DestAddress = DestAddress;
	this->TxNum = TxNum;
	this->RxNum = RxNum;
	if(TxNum>0){
		if(TxNum<0x10){		// 基本的に同じ数だけメモリ確保(断片化対策)
			TxNum = 0x10;
		}
		this->TxData = new uint8_t[TxNum];	// newできなかった時の処理を呼び出し元で書くこと
	}else{
		this->TxData = NULL;
	}
	
	if(RxNum>0){
		if(RxNum<0x10){
			RxNum = 0x10;
		}
		this->RxData = new uint8_t[RxNum];
	}else{
		this->RxData = NULL;
	}
	
	// 送信データをコピーする。
	if(NULL!=this->TxData){	// バッファ確保できた時だけ
		memcpy(this->TxData, TxData, this->TxNum);
	}
	
	this->Err = false;
}
