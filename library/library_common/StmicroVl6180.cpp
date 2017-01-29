/********************************************/
/*	STmicro VL6180 (I2C)		    */
/*			for RX63n @ CS+     */
/*			Wrote by conpe_	    */
/*			2016/05/23	    */
/********************************************/



#include "StmicroVl6180.h"


//uint8_t stvl6_comus_t::DevId = 0x03;

/*********************
測距センサ初期化
コンストラクタ
引数：	I2C
**********************/
stmicro_VL6810::stmicro_VL6810(I2c_t* I2Cn){
	this->I2Cn = I2Cn;
	this->I2cAddress = STVL6_ADRS_DEFAULT;
	this->I2cAddressPre = STVL6_ADRS_DEFAULT;
	init();
}
stmicro_VL6810::stmicro_VL6810(I2c_t* I2Cn, uint8_t I2cAddress){
	this->I2Cn = I2Cn;
	this->I2cAddress = STVL6_ADRS_DEFAULT;
	this->I2cAddressPre = I2cAddress;
	init();
	// Address変更処理要
	
}



/*********************
デストラクタ
**********************/
stmicro_VL6810::~stmicro_VL6810(void){
	
}


/*********************
フラグとか初期化
**********************/
void stmicro_VL6810::init(void){
	MeasurementRequest = false;
	//fMeasurement = false;
	fI2cErr = false;
	Distance_mm = STVL6_DISTANCE_DEFAULT;
	cntget = 0;
}


/*********************
動作開始
 I2C初期化
 レジスタ初期化
**********************/
int8_t stmicro_VL6810::begin(void){
	int8_t ret=0;
	
	I2Cn->begin(400, 15, 64);
	
	if(this->I2cAddressPre != STVL6_ADRS_DEFAULT){	// デフォルトアドレスじゃなければ
		ret |= setI2cAddress(this->I2cAddressPre);	// 設定する
	}
	
	ret |= setReg((register_e)0x0207, 0x01);
	ret |= setReg((register_e)0x0208, 0x01);
	ret |= setReg((register_e)0x0096, 0x00);
	ret |= setReg((register_e)0x0097, 0xfd);
	ret |= setReg((register_e)0x00e3, 0x00);
	ret |= setReg((register_e)0x00e4, 0x04);
	ret |= setReg((register_e)0x00e5, 0x02);
	ret |= setReg((register_e)0x00e6, 0x01);
	ret |= setReg((register_e)0x00e7, 0x03);
	ret |= setReg((register_e)0x00f5, 0x02);
	ret |= setReg((register_e)0x00d9, 0x05);
	ret |= setReg((register_e)0x00db, 0xce);
	ret |= setReg((register_e)0x00dc, 0x03);
	ret |= setReg((register_e)0x00dd, 0xf8);
	ret |= setReg((register_e)0x009f, 0x00);
	ret |= setReg((register_e)0x00a3, 0x3c);
	ret |= setReg((register_e)0x00b7, 0x00);
	ret |= setReg((register_e)0x00bb, 0x3c);
	ret |= setReg((register_e)0x00b2, 0x09);
	ret |= setReg((register_e)0x00ca, 0x09);  
	ret |= setReg((register_e)0x0198, 0x01);
	ret |= setReg((register_e)0x01b0, 0x17);
	ret |= setReg((register_e)0x01ad, 0x00);
	ret |= setReg((register_e)0x00ff, 0x05);
	ret |= setReg((register_e)0x0100, 0x05);
	ret |= setReg((register_e)0x0199, 0x05);
	ret |= setReg((register_e)0x01a6, 0x1b);
	ret |= setReg((register_e)0x01ac, 0x3e);
	ret |= setReg((register_e)0x01a7, 0x1f);
	ret |= setReg((register_e)0x0030, 0x00);
	
	//ret |= setReg((register_e)0x0011, 0x10);
	ret |= setReg((register_e)0x010a, 0x30);
	//ret |= setReg((register_e)0x003f, 0x46);
	//ret |= setReg((register_e)0x0031, 0xff);
	//ret |= setReg((register_e)0x0040, 0x63);
	//ret |= setReg((register_e)0x002e, 0x01);
	 
	/*
	ret |= setReg((register_e)0x001b, 0x09);
	ret |= setReg((register_e)0x003e, 0x31);
	ret |= setReg((register_e)0x0014, 0x24); 
	*/
	
	
	// 計測開始
	ret |= setMeasurePeriod(20);		// 計測周期20msec
	ret |= setReg(SYSRANGE_START, 0x03);	// 連続計測
	
	return ret;
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
int8_t stmicro_VL6810::getDistance(int16_t* Dist){
	*Dist = Distance_mm;
	
	if(cntget<10){
		cntget++;
	}else if(cntget>10){
		
	}else{
	//	begin();	// 起動後しばらくしてからやるためここで！
		cntget++;
	}
	
	return 0;
}

/*********************
連続距離計測開始
概要：
 測定終わったらすぐ次の計測結果取得要求を出す
引数：
 
**********************/
int8_t stmicro_VL6810::startMeasurement(void){
	
	MeasurementRequest = 1;
	
	//setReg(VL6180X_REG_SYSRANGE_INTERMEASUREMENT_PERIOD, 4);	// 計測周期50msec
	//setReg(VL6180X_REG_SYSRANGE_START, 0x03);	// 連続計測
	
	return 0;
}
/*********************
距離計測終了
概要：
 自身を登録
引数：	
**********************/
int8_t stmicro_VL6810::stopMeasurement(void){
	MeasurementRequest = 0;
	
	//setReg(VL6180X_REG_SYSRANGE_START, 0x03);	// 連続計測
	
	return 0;
}

/*********************
計測周期設定
概要：
引数：	uint16_t msec 10～2550
**********************/
int8_t stmicro_VL6810::setMeasurePeriod(uint16_t msec){
	// 範囲
	if(msec<10){
		msec = 10;
	}
	if(msec>(2540+10)){
		msec = 2540+10;
	}
	// 送る値へ変換
	msec = (msec-10)/10;
	
	return setReg(SYSRANGE_INTERMEASUREMENT_PERIOD, (uint8_t)(msec&0x00FF));	// 計測周期50msec
}


/*********************
割り込みクリア
概要：
引数：	
**********************/
int8_t stmicro_VL6810::clearInterrupt(void){
	return setReg(SYSTEM__INTERRUPT_CLEAR, 0x07);
}

/*********************
I2Cアドレス設定
概要：
引数：	
**********************/
int8_t stmicro_VL6810::setI2cAddress(uint8_t NewAdrs){
	int8_t ret;	
	
	ret = setReg(I2C_SLAVE__DEVICE_ADDRESS, NewAdrs);
	if(ret >= 0){	// 正しく送れていれば
		this->I2cAddress = NewAdrs;
	}
	return ret;
}


/*********************
計測
概要：距離の取得要求
引数：なし
返値：
**********************/
int8_t stmicro_VL6810::measure(void){
	return reqDat(RESULT_RANGE_RAW, 1);
}


/*********************
復活の儀式
概要：計測が止まっちゃった時に再設定とかしてみる
	ひとまずこれで計測再開する。
	ただし、2度送らないとダメ
引数：なし
返値：
**********************/
int8_t stmicro_VL6810::reset(void){
	int8_t ret=0;
	
	//ret |= setMeasurePeriod(20);		// 計測周期20msec
	//ret |= setReg(SYSRANGE_START, 0x03);	// 連続計測
	begin();
	
	return ret;
}




/*********************
レジスタへセット
概要：I2C送信開始
引数：なし
返値：
**********************/
int8_t stmicro_VL6810::setReg(register_e ComuReg, uint8_t SetData){
	int8_t AttachedIndex;
	
	AttachedIndex = attachI2cComu(ComuReg, SetData, 0);
	if(0 <= AttachedIndex){
		// 成功
		//fMeasurement = true;
		fI2cErr = false;
	}else{
		// 失敗
		//fMeasurement = false;
		fI2cErr = true;
	}
	return AttachedIndex;
}

/*********************
レジスタから読み込み
概要：I2C送受信開始
引数：なし
返値：
**********************/
int8_t stmicro_VL6810::reqDat(register_e ComuReg, uint16_t ReqNum){
	int8_t AttachedIndex;
	
	AttachedIndex = attachI2cComu(ComuReg, 0, ReqNum);
	if(0<=AttachedIndex){
		// 成功
		//fMeasurement = true;
		fI2cErr = false;
	}else{
		// 失敗
		//fMeasurement = false;
		fI2cErr = true;
	}
	return AttachedIndex;
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
int8_t stmicro_VL6810::attachI2cComu(register_e ComuReg, uint8_t SetData, uint16_t RxNum){
	int8_t AttachedIndex = 0;
	uint8_t TxNum;
	stvl6_comus_t* NewComu;
	uint8_t TxData[3];
	
	TxData[0] = (uint8_t)(((uint16_t)ComuReg)>>8)&0x00FF;
	TxData[1] = (uint8_t)((uint16_t)ComuReg)&0x00FF;
	TxData[2] = SetData;
	
	if(RxNum>0){
		TxNum = 2;	// アドレスのみ送る
	}else{
		TxNum = 3;
	}
	
	
	NewComu = new stvl6_comus_t(I2cAddress, TxData, TxNum, RxNum);
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
	
	NewComu->STVL6 = this;
	NewComu->ComuReg = ComuReg;
	
	
	// 送受信登録
	AttachedIndex = I2Cn->attach(NewComu);
	if(-1 == AttachedIndex){
		// 登録失敗したらなかったことに
		delete NewComu;
	}else{
	}
	
	return AttachedIndex;
}




/*********************
I2C読み終わったので捕獲
概要：
引数：	
**********************/
int8_t stmicro_VL6810::fetchI2cRcvData(const stvl6_comus_t* Comu){
	if(NULL!=Comu){
		// 通信ちゃんと出来てる？
		fI2cErr = Comu->Err;
		
		if(!fI2cErr){
			
			// 通信内容によって受信データを処理する
			switch(Comu->ComuReg){
			case RESULT_RANGE_RAW:	// 距離取得
				Distance_mm = *Comu->RxData;
				if(STVL6_FAIL_DISTANCE_MM == Distance_mm){
					Distance_mm = STVL6_DISTANCE_DEFAULT;
				}
				
			//	clearInterrupt();
				
			
				//更に計測する
				if(MeasurementRequest){
					measure();
				}
				
				break;
			case SYSRANGE_START:
				break;
			case RESULT_RANGE_STATUS:
				RangeStatus = (*Comu->RxData)>>4;
				break;
			case RESULT_RANGE_RETURN_SIGNAL_COUNT:
				RangeReturnSignalCount = *(uint32_t*)Comu->RxData;
				break;
			default:
				/* do nothing */
				break;
			}
		}else{
			Distance_mm = STVL6_DISTANCE_DEFAULT;
		}
	
		return 0;
	}else{
		return -1;
	}
}

