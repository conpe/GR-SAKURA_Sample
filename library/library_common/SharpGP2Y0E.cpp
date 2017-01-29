/********************************************/
/*		Sharp 測距センサ __GP2Y0E*(I2C)		*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/03/11		*/
/********************************************/



#include "SharpGP2Y0E.h"


uint8_t shgp2_comus_t::DevId = 0x02;

/*********************
測距センサ初期化
コンストラクタ
引数：	I2C
**********************/
sharp_GP2Y0E::sharp_GP2Y0E(I2c_t* I2Cn, uint8_t I2cAddress){
	sharp_GP2Y0E::I2Cn = I2Cn;
	sharp_GP2Y0E::I2cAddress = I2cAddress;
	
	
	MeasurementRequest = false;
	fMeasurement = false;
	fI2cErr = false;
	Distance_mm = SHGP2_DISTANCE_DEFAULT;
}
/*********************
デストラクタ
**********************/
sharp_GP2Y0E::~sharp_GP2Y0E(void){
	
}


/*********************
動作開始
I2C初期化
**********************/
int8_t sharp_GP2Y0E::begin(void){
	return I2Cn->begin(400, 32);
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
int8_t sharp_GP2Y0E::getDistance(int16_t* Dist){
	*Dist = Distance_mm;
	
	if(Distance_mm==SHGP2_DISTANCE_DEFAULT){
		return -1;
	}
	
	return 0;
}

/*********************
連続距離計測開始
概要：
 測定終わったらすぐ次の測定要求を出す
引数：
 
**********************/
int8_t sharp_GP2Y0E::startMeasurement(void){
	
	MeasurementRequest = 1;
	
	// 計測してないなら開始
	if(!fMeasurement){
		measure();
	}
	return 0;
}
/*********************
距離計測終了
概要：
 自身を登録
引数：	
**********************/
int8_t sharp_GP2Y0E::stopMeasurement(void){
	MeasurementRequest = 0;
	return 0;
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
int8_t sharp_GP2Y0E::measure(void){
	int8_t AttachedIndex;
	uint8_t TxData[1];
	uint16_t TxNum;
	uint16_t RxNum;
	
	//if(!fMeasurement || I2Cn->isIdle()){
	//if(!fI2cErr){
	//	if(!fMeasurement){	// 前の計測終わってない
	
			// I2Cあたっち準備
			TxData[0] = (uint8_t)SHGP2_REG_DISTANCE;
			TxNum = 1;
			RxNum = 2;
			
			// I2Cあたーっち！
			AttachedIndex = attachI2cComu(GET_DISTANCE, TxData, TxNum, RxNum);
			if(0<=AttachedIndex){
				// 成功
				fMeasurement = true;
				fI2cErr = false;
			}else{
				// 失敗
				fMeasurement = false;
				fI2cErr = true;
			}
			
	//	}else{
	//		return -5;
	//	}
	//}else{
	//	return -6;
	//}
	
	return AttachedIndex;
}



/*********************
メディアンフィルタの設定
概要：
 
引数：
 メディアンフィルタの計算に使う数
  1, 5, 7, 9のいずれか
**********************/
int8_t sharp_GP2Y0E::setMedianFilter(uint8_t MedianCalcNum){
	int8_t AttachedIndex;
	uint8_t TxData[2];
	uint16_t TxNum;
	
	// I2Cあたっち準備
	TxData[0] = (uint8_t)SHGP2_REG_MEDIANFILTER;
	switch(MedianCalcNum){
	case 7:
		TxData[1] = 0x00;
		break;
	case 5:
		TxData[1] = 0x10;
		break;
	case 9:
		TxData[1] = 0x20;
		break;
	case 1:
		TxData[1] = 0x30;
		break;
	default:
		TxData[1] = 0x30;
		break;
	}
	TxNum = 2;
	
	// I2Cあたーっち！
	AttachedIndex = attachI2cComu(SET_MEDIANFILTER, TxData, TxNum, 0);
	
	return AttachedIndex;
}


/*********************
【E-FUSE】I2Cアドレスの設定
概要：
 E-fuseなプログラミングでデバイスのI2Cアドレスを書き換える。
 電源onoffがいるので、FETかなにかでVppを制御する必要あり。
 参考:アプリケーションノートhttp://www.sharp.co.jp/products/device/doc/opto/gp2y0e02_03_appl_e.pdf
引数：
 uint8_t NewAddress 新しいI2Cアドレス(0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78)
 					データシートでは1bit左シフトしたアドレスで表記されているので注意
 pins Vpp 電源(タイミングに合わせてon/offする)

返値：
 0	正しく書けた
 -5	書いた値が違う
 その他	I2Cアタッチ失敗
**********************/
int8_t sharp_GP2Y0E::setI2cAddress(uint8_t NewAddress, pins Vpp){
	uint8_t TxData[2];
	uint8_t SetI2cAddress;
	int8_t AttachedIndex;
	
	
	
	SetI2cAddress = (NewAddress>>3)&0x0F;	// E[0]～E[3]に合わせる
	
	outPin(Vpp, SHGP2_VPP_OFF);
	
	// Stage1
	TxData[0] = 0xEC;	// Register Address
	TxData[1] = 0xFF;	// data
	
	
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	// wait for finish transmit. 
	while(true == fLastAttachComuFin);
	outPin(Vpp, SHGP2_VPP_ON);		// turn on
	
	// Stage2
	TxData[0] = 0xC8;
	TxData[1] = 0x00;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	
	// Stage3
	TxData[0] = 0xC9;
	TxData[1] = 0x45;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	
	// Stage4
	TxData[0] = 0xCD;
	TxData[1] = SetI2cAddress;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	
	// Stage5
	TxData[0] = 0xCA;
	TxData[1] = 0x01;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	volatile uint16_t i;
	for(i=0;i<30000; i++){
		// 書き込み待ち
		// 500usec
	}
	
	//ここからは新しいアドレスでアクセスしなきゃならない？
	
	// Stage6
	TxData[0] = 0xCA;
	TxData[1] = 0x00;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	
	// wait for finish transmit. 
	while(true == fLastAttachComuFin);
	outPin(Vpp, SHGP2_VPP_OFF);		// turn off
	
	// Stage7
	// Stage7.0
	TxData[0] = 0xEF;
	TxData[1] = 0x00;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	// Stage7.1
	TxData[0] = 0xC8;
	TxData[1] = 0x40;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	// Stage7.2
	TxData[0] = 0xC8;
	TxData[1] = 0x00;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	
	// Stage8
	TxData[0] = 0xEE;
	TxData[1] = 0x06;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	
	
	// Stage9 Check
	// Stage9.0
	TxData[0] = 0xEF;
	TxData[1] = 0x00;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	// Stage9.1
	TxData[0] = 0xEC;
	TxData[1] = 0xFF;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	// Stage9.2
	TxData[0] = 0xEF;
	TxData[1] = 0x03;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	// Stage9.3 read
	TxData[0] = 0x27;
	FuseRcv = new uint8_t[1];
		if(NULL==FuseRcv) __heap_chk_fail();
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 1, 1);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	//read
	// wait for finish receive. 
	while(true == fLastAttachComuFin);
	int8_t ack = (int8_t)FuseRcv[0];
	
	delete[] FuseRcv;
	
	// Stage9.4
	TxData[0] = 0xEF;
	TxData[1] = 0x00;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	// Stage9.5
	TxData[0] = 0xEC;
	TxData[1] = 0x7F;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// エラーだ…
		return AttachedIndex;
	}
	
	
	return ack;
}




/*********************
I2Cクラスに通信をアタッチする
概要：
 通信を開始する。
 終わるとfetchI2cRcvDataが呼ばれる。
 [例外 : I2Cバッファいっぱい]
  newした通信クラスをdeleteして、なかったことにする。-1を返す。
 
引数：

返値：
 AttachedIndex
**********************/
int8_t sharp_GP2Y0E::attachI2cComu(shgp2_comu_content ComuType, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum){
	int8_t AttachedIndex = 0;
	
	shgp2_comus_t* NewComu;
	
	NewComu = new shgp2_comus_t(I2cAddress, TxData, TxNum, RxNum);
		if(NULL==NewComu) __heap_chk_fail();
	if(NULL == NewComu){
		return -1;	// ヒープ足りない？
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
	
	NewComu->SHGP2 = this;
	NewComu->ComuType = ComuType;
	
	AttachedIndex = I2Cn->attach(NewComu);		// 送受信登録
	if(-1 == AttachedIndex){
		// 登録失敗したらなかったことに
		delete NewComu;
	}else{
		fLastAttachComuFin = false;
		LastAttachComu = NewComu;
	}
	
	return AttachedIndex;
}




/*********************
I2C読み終わったので捕獲
概要：
引数：	
**********************/
int8_t sharp_GP2Y0E::fetchI2cRcvData(const shgp2_comus_t* Comu){
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
			case GET_DISTANCE:	// 距離取得
				Distance_mm = (int16_t)((((uint16_t)RcvData[0])<<4)|(((uint16_t)RcvData[1])&0x000F))*10/16/4;
				
				if(Distance_mm == SHGP2_FAIL_DISTANCE_MM){
					Distance_mm = SHGP2_DISTANCE_DEFAULT;
				}
			
				fMeasurement = false;
					
				//更に計測する
				if(MeasurementRequest){
					measure();
				}
				
				break;
			case SET_MEDIANFILTER:
				// none
				break;
				
			case SET_FUSE_I2CADDRESS:
				if(Comu->RxNum>0){
					FuseRcv[0] = RcvData[0];
				}
				break;
			}
		}else{
			Distance_mm = SHGP2_DISTANCE_DEFAULT;
			fMeasurement = false;
		}
		
		return 0;
	}else{
		return -1;
	}
}

