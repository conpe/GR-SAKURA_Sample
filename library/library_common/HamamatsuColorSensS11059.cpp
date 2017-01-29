/************************************************/
/*	浜松フォトニクス			*/
/*		デジタルカラーセンサS11059(I2C)	*/
/*			for RX63n @ CS+		*/
/*			Wrote by conpe_		*/
/*				2015/04/12	*/
/************************************************/

#include "HamamatsuColorSensS11059.h"


//uint8_t mhs11059_comus_t::DevId = 0x05;


/*********************
測距センサ初期化
コンストラクタ
I2C初期化
引数：	I2C
**********************/
hamamatsu_S11059::hamamatsu_S11059(I2c_t* I2Cn){
	this->I2Cn = I2Cn;
	this->I2cAddress = HMS11059_I2CADRS;	// 固定
	
	this->SensRed=0;
	this->SensGreen=0;
	this->SensBlue=0;
	this->SensIr=0;
	
	this->fMeasurement = false;
	this->fRepeatMeasurement = false;
	this->fI2cErr = false;
}
/*********************
測距センサ初期化
デストラクタ
**********************/
hamamatsu_S11059::~hamamatsu_S11059(void){
}


/*********************
測距センサ動作開始
**********************/
void hamamatsu_S11059::begin(void){
	I2Cn->begin(400, 32);
}



/*********************
計測結果取得
概要：
 
引数：
 Dist ここに返すよ
返値：
 0 正常
 -1 未計測
**********************/
int8_t hamamatsu_S11059::getLuminance(uint16_t* LuminanceRed, uint16_t* LuminanceGreen, uint16_t* LuminanceBlue){
	*LuminanceRed = this->SensRed;
	*LuminanceGreen = this->SensGreen;
	*LuminanceBlue = this->SensBlue;
	return 0;
}
int8_t hamamatsu_S11059::getLuminance(uint16_t* LuminanceRed, uint16_t* LuminanceGreen, uint16_t* LuminanceBlue, uint16_t* LuminanceIr){
	*LuminanceRed = this->SensRed;
	*LuminanceGreen = this->SensGreen;
	*LuminanceBlue = this->SensBlue;
	*LuminanceIr = this->SensIr;
	return 0;
}
int8_t hamamatsu_S11059::getLuminanceRed(uint16_t* LuminanceRed){
	*LuminanceRed = this->SensRed;
	
	return 0;
}
int8_t hamamatsu_S11059::getLuminanceGreen(uint16_t* LuminanceGreen){
	*LuminanceGreen = this->SensGreen;
	
	return 0;
}
int8_t hamamatsu_S11059::getLuminanceBlue(uint16_t* LuminanceBlue){
	*LuminanceBlue = this->SensBlue;
	
	return 0;
}
int8_t hamamatsu_S11059::getLuminanceIr(uint16_t* LuminanceIr){
	*LuminanceIr = this->SensIr;
	
	return 0;
}

uint16_t hamamatsu_S11059::getHsvH(void){
	uint16_t RgbMax;
	uint16_t RgbMin;
	color MaxCol;
	int16_t H;
	
	if(this->SensRed > this->SensGreen){
		RgbMax = this->SensRed;
		MaxCol = red;
	}else{
		RgbMax = this->SensGreen;
		MaxCol = green;
	}
	if(RgbMax < this->SensBlue){
		RgbMax = this->SensBlue;
		MaxCol = blue;
	}
	
	if(this->SensRed < this->SensGreen){
		RgbMin = this->SensRed;
	}else{
		RgbMin = this->SensGreen;
	}
	if(RgbMin > this->SensBlue){
		RgbMin = this->SensBlue;
	}
	
	
	switch(MaxCol){
	case red:
		H = (60 * (this->SensBlue-this->SensGreen))/(RgbMax-RgbMin);
		break;
	case green:
		H = 120 + (60*(this->SensRed-this->SensBlue))/(RgbMax-RgbMin);
		break;
	case blue:
		H = 240 + (60*(this->SensGreen-this->SensRed))/(RgbMax-RgbMin);
		break;
	default:
		H = 0;
		break;
	}
	
	if(H<0){
		H = H+360;
	}
	
	return (uint16_t)H;
}




/*********************
計測
概要：
 距離の取得
引数：
 なし
返値：
 AttachedIndex
 -5 すでに計測中
 -6 通信おかしい
 -1 Attach数がいっぱい(I2C_COMUS)(RIICクラスの仕様),
 -2 Comusの領域を確保失敗(RIICクラスの仕様)
**********************/
int8_t hamamatsu_S11059::updateSens(void){
	int8_t AttachedIndex;
	uint8_t TxData;
	uint16_t TxNum;
	uint16_t RxNum;
	
	//if(!fMeasurement || I2Cn->isIdle()){
	//if(!fI2cErr){
		// I2Cあたっち準備
		TxData = (uint8_t)HMS11059_REG_SENSRED;
		TxNum = 1;
		RxNum = 8;
		
		// I2Cあたーっち！
		AttachedIndex = attachI2cComu(GET_SENS, &TxData, TxNum, RxNum);
		if(0<=AttachedIndex){
			
		}else{
			fI2cErr = true;
		}
	//}else{
	//	return -6;
	//}
	
	return AttachedIndex;
}


/*********************
計測準備
概要：
 モジュールを起こす
 (ADCリセット, スリープ解除)
引数：	
**********************/
int8_t hamamatsu_S11059::wakeup(void){
	int8_t AttachedIndex;
	uint8_t TxData[2];
	uint16_t TxNum;
	
	if(!fI2cErr){
		// I2Cあたっち準備
		TxData[0] = (uint8_t)HMS11059_REG_CONTROL;
		TxData[1] = (uint8_t)0x8A;
		TxNum = 2;
		
		// I2Cあたーっち！
		AttachedIndex = attachI2cComu(SET_CTRL, TxData, TxNum, 0);
		if((0<=AttachedIndex) && (I2Cn->getComusNum()>=AttachedIndex)){
			
		}else{
			fI2cErr = true;
		}
		
	}else{
		return -6;
	}
	
	return AttachedIndex;
}

/*********************
計測開始
概要：
 
引数：
 (予定)ゲイン、積分時間、測定繰り返すか
**********************/
int8_t hamamatsu_S11059::startMeasurement(void){
	int8_t AttachedIndex;
	uint8_t TxData[2];
	uint16_t TxNum;
	
	if(!fI2cErr){
		if(!fMeasurement || fRepeatMeasurement){
				
			// I2Cあたっち準備
			TxData[0] = (uint8_t)HMS11059_REG_CONTROL;
			TxData[1] = (uint8_t)0x0A;	// Highゲイン, 連続変換, 積分時間22.4ms
			TxNum = 2;
			
			// I2Cあたーっち！
			AttachedIndex = attachI2cComu(SET_CTRL, TxData, TxNum, 0);
			if((0<=AttachedIndex) && (I2Cn->getComusNum()>=AttachedIndex)){
				fMeasurement = true;
				if(1){
					fRepeatMeasurement = true;
				}
			}else{
				fI2cErr = true;
			}
		}else{
			return -5;
		}
	}else{
		return -6;
	}
	
	return AttachedIndex;
}


/*********************
計測終了
概要：
 モジュールをスタンバイ状態にする
引数：	
**********************/
int8_t hamamatsu_S11059::stopMeasurement(void){
	int8_t AttachedIndex;
	uint8_t TxData[2];
	uint16_t TxNum;
	
	if(!fI2cErr){
		// I2Cあたっち準備
		TxData[0] = (uint8_t)HMS11059_REG_CONTROL;
		TxData[1] = (uint8_t)0xC0;
		TxNum = 2;
		
		// I2Cあたーっち！
		AttachedIndex = attachI2cComu(SET_CTRL, TxData, TxNum, 0);
		if((0<=AttachedIndex) && (I2Cn->getComusNum()>=AttachedIndex)){
		}else{
			fI2cErr = true;
		}
	}else{
		return -6;
	}
	
	return AttachedIndex;
}










/*********************
I2Cクラスに通信をアタッチする
概要：
 通信を開始する。
 終わるとRxCallbackが呼ばれる。
引数：

返値：
 AttachedIndex
**********************/
int8_t hamamatsu_S11059::attachI2cComu(mhs11059_comu_content ComuType, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum){
	int8_t AttachedIndex = 0;
	
	mhs11059_comus_t* NewComu;
	
	NewComu = new mhs11059_comus_t(I2cAddress, TxData, TxNum, RxNum);
		if(NULL==NewComu) __heap_chk_fail();
	if(NULL == NewComu){
		return -1;	// ヒープ足りない
	}
	if(TxNum>0){
		if(NULL == NewComu->TxData){
			delete NewComu;
			return -1;
		}
	}
	if(RxNum>0){
		if(NULL == NewComu->RxData){
			delete NewComu;
			return -1;
		}
	}
	
	NewComu->MHS11059 = this;
	NewComu->ComuType = ComuType;
	
	AttachedIndex = I2Cn->attach(NewComu);		// 送受信登録
	if(-1==AttachedIndex){
		// 登録失敗したらなかったことに
		delete NewComu;
	}else{
		LastAttachComu = NewComu;
		fLastAttachComuFin = false;
	}
	
	return AttachedIndex;
}


/*********************
I2C読み終わったので捕獲
概要：
引数：	
**********************/
int8_t hamamatsu_S11059::fetchI2cRcvData(const mhs11059_comus_t* Comu){
	uint8_t* RcvData = Comu->RxData;
	
	if(NULL!=Comu){
		// 通信ちゃんと出来てる？
		fI2cErr = Comu->Err;
		
		// 通信完了フラグ立てる
		if(LastAttachComu == Comu){
			fLastAttachComuFin = true;
		}
		
		if(!fI2cErr){
			
			// 通信内容によって受信データを処理する
			switch(Comu->ComuType){
			case GET_SENS:	// センサ値取得
				SensRed = ((uint16_t)RcvData[0]<<8 | ((uint16_t)RcvData[1]&0x00FF));
				SensGreen = ((uint16_t)RcvData[2]<<8 | ((uint16_t)RcvData[3]&0x00FF));
				SensBlue = ((uint16_t)RcvData[4]<<8 | ((uint16_t)RcvData[5]&0x00FF));
				SensIr = ((uint16_t)RcvData[6]<<8 | ((uint16_t)RcvData[7]&0x00FF));
				
				if(fRepeatMeasurement){
					fMeasurement = false;
				}
				break;
			case SET_CTRL:
				// none
				break;
			case SET_MANUALTIMING:
				// none
				break;
			}
		}else{
			fMeasurement = false;
		}
		
		return 0;
	}else{
		return -1;
	}
}
