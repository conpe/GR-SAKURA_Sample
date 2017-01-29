/********************************************/
/*		InvenSense MPUxxxx					*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/03/31		*/
/********************************************/



#include "InvensenseMPU.h"


uint8_t ismpu_comus_t::DevId = 0x04;

// LSB
static const float IsMpuAccelScaleFactor[4] = {16384, 8192, 4096, 2048};	// LSB/g
static const float IsMpuGyroScaleFactor[4] = {131, 65.5, 32.8, 16.4};		// LSB/(deg/s)



/*********************
測距センサ(I2C初期化)
コンストラクタ
引数：	I2C
**********************/
invensense_MPU::invensense_MPU(I2c_t* I2Cn, uint8_t I2cAddress){
	invensense_MPU::I2Cn = I2Cn;
	invensense_MPU::I2cAddress = I2cAddress;
	
	
	MeasurementRequest = false;
	fMeasurement = false;
	fI2cErr = true;				// 通信できるまではエラー扱い
	Accel[0] = ISMPU_ACCEL_DEFAULT;
	Accel[1] = ISMPU_ACCEL_DEFAULT;
	Accel[2] = ISMPU_ACCEL_DEFAULT;
	Gyro[0] = ISMPU_GYRO_DEFAULT;
	Gyro[1] = ISMPU_GYRO_DEFAULT;
	Gyro[2] = ISMPU_GYRO_DEFAULT;
	Temp = ISMPU_TEMP_DEFAULT;
	
	setAccelOffset(0.0F, 0.0F, 0.0F);
	setGyroOffset(0.0F, 0.0F, 0.0F);
	
	invAxis[ISMPU_IDX_ACC_X] = 0;
	invAxis[ISMPU_IDX_ACC_X] = 1;
	invAxis[ISMPU_IDX_ACC_X] = 2;
	invAxis[ISMPU_IDX_GYRO_X] = 3;
	invAxis[ISMPU_IDX_GYRO_X] = 4;
	invAxis[ISMPU_IDX_GYRO_X] = 5;
	invAxis[ISMPU_IDX_MAG_X] = 6;
	invAxis[ISMPU_IDX_MAG_X] = 7;
	invAxis[ISMPU_IDX_MAG_X] = 8;
	
	GyroFs = 0;
	AccelFs = 0;
	
}
/*********************
測距センサ
デストラクタ
**********************/
invensense_MPU::~invensense_MPU(void){
}


/*********************
セットアップ
概要：
 モジュール起こす
 ISMPU_REG_PWR_MGMT_1に0x00を書く
引数：
 
**********************/
int8_t invensense_MPU::begin(void){
	int8_t AttachedIndex;
	uint8_t TxData[2];
	uint16_t TxNum;
	
	
	// I2C開始
	I2Cn->begin(400, 32);
	
	
	// I2Cあたっち準備
	TxData[0] = (uint8_t)ISMPU_REG_PWR_MGMT_1;
	TxData[1] = 0x00;
	TxNum = 2;
	
	// I2Cあたーっち！
	AttachedIndex = attachI2cComu(SET_CONFIG, TxData, TxNum, 0);
	if((0<=AttachedIndex) && (I2Cn->getComusNum()>=AttachedIndex)){
		//fMeasurement = true;
	}

	return AttachedIndex;
}

/*********************
計測結果取得
概要：
 各計測結果を返す
引数：

**********************/
int8_t invensense_MPU::getAccel(float* AccX, float* AccY, float* AccZ ){
	*AccX = invensense_MPU::Accel[0];
	*AccY = invensense_MPU::Accel[1];
	*AccZ = invensense_MPU::Accel[2];
	
	return 0;
}
int8_t invensense_MPU::getGyro(float* GyroX, float* GyroY, float* GyroZ){
	*GyroX = invensense_MPU::Gyro[0];
	*GyroY = invensense_MPU::Gyro[1];
	*GyroZ = invensense_MPU::Gyro[2];
	
	return 0;
}
int8_t invensense_MPU::getTemp(float* Temp){
	*Temp = invensense_MPU::Temp;
	
	return 0;
}





/*********************
連続距離計測開始
概要：
 測定終わったらすぐ次の測定要求を出す
引数：
 
**********************/
int8_t invensense_MPU::startMeasurement(void){
	
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
int8_t invensense_MPU::stopMeasurement(void){
	MeasurementRequest = 0;
	return 0;
}






/*********************
ジャイロセンサのレンジ設定
概要：
 ISMPU_REG_GYRO_CONFIGの3,4ビット目で設定
引数：
 0: +-250
 1: +-500
 2: +-1000
 3: +-2000
**********************/
int8_t invensense_MPU::setGyroRange(uint8_t Fs){
	int8_t AttachedIndex;
	uint8_t TxData[2];
	uint16_t TxNum;
	
	// I2Cあたっち準備
	TxData[0] = (uint8_t)ISMPU_REG_GYRO_CONFIG;
	TxData[1] = (Fs&0x03)<<3;
	TxNum = 2;
	
	AttachedIndex = attachI2cComu(SET_CONFIG, TxData, TxNum, 0);
	if((0<=AttachedIndex) && (I2Cn->getComusNum()>=AttachedIndex)){
		GyroFs = Fs;
	}
	
	return AttachedIndex;
}

/*********************
加速度センサセンサのレンジ設定
概要：
 ISMPU_REG_ACCEL_CONFIGの3,4ビット目で設定
引数：
 0: +-2g
 1: +-4g
 2: +-8g
 3: +-16g
**********************/
int8_t invensense_MPU::setAccelRange(uint8_t Fs){
	int8_t AttachedIndex;
	uint8_t TxData[2];
	uint16_t TxNum;
	
	// I2Cあたっち準備
	TxData[0] = (uint8_t)ISMPU_REG_ACCEL_CONFIG;
	TxData[1] = (Fs&0x03)<<3;
	TxNum = 2;
	
	AttachedIndex = attachI2cComu(SET_CONFIG, TxData, TxNum, 0);
	if((0<=AttachedIndex) && (I2Cn->getComusNum()>=AttachedIndex)){	//I2Cn->getComusNum()より大きい時ってなんなんだ？
		AccelFs = Fs;
	}
	
	return AttachedIndex;
}

/*********************
計測
概要：
 加速度、角速度、温度の計測
 ISMPU_REG_ACCEL_XOUT_Hから14バイトを読む
引数：
 なし
返値：
 AttachedIndex
 -5 すでに計測中
 -6 通信おかしい
 -1 Attach数がいっぱい(I2C_COMUS)(RIICクラスの仕様),
 -2 Comusの領域を確保失敗(RIICクラスの仕様)
**********************/
int8_t invensense_MPU::measure(void){
	int8_t AttachedIndex;
	uint8_t TxData[1];
	uint16_t TxNum;
	uint16_t RxNum;
	
	//if(!fMeasurement || I2Cn->isIdle()){
	//if(!fI2cErr){
		//if(!fMeasurement){
			
			// I2Cあたっち準備
			TxData[0] = (uint8_t)ISMPU_REG_ACCEL_XOUT_H;
			TxNum = 1;
			RxNum = 14;
			
			// I2Cあたーっち！
			AttachedIndex = attachI2cComu(MEASURE, TxData, TxNum, RxNum);
			if((0<=AttachedIndex) && (I2Cn->getComusNum()>=AttachedIndex)){
				fMeasurement = true;
			}
			
		//}else{
		//	return -5;
		//}
	//}else{
		//return -6;
	//}
	
	return AttachedIndex;
}

// 軸方向設定
void invensense_MPU::setAxis(uint8_t AccX, uint8_t AccY, uint8_t AccZ, uint8_t GyroX, uint8_t GyroY, uint8_t GyroZ, uint8_t MagX, uint8_t MagY, uint8_t MagZ){
	invAxis[ISMPU_IDX_ACC_X] = AccX;
	invAxis[ISMPU_IDX_ACC_X] = AccX;
	invAxis[ISMPU_IDX_ACC_X] = AccX;
	invAxis[ISMPU_IDX_GYRO_X] = GyroX;
	invAxis[ISMPU_IDX_GYRO_Y] = GyroY;
	invAxis[ISMPU_IDX_GYRO_Z] = GyroZ;
	invAxis[ISMPU_IDX_MAG_X] = MagX;
	invAxis[ISMPU_IDX_MAG_Y] = MagY;
	invAxis[ISMPU_IDX_MAG_Z] = MagZ;
}
	
// オフセット設定
void invensense_MPU::setAccelOffset(float Xofs, float Yofs, float Zofs, bool_t fAdd){
	if(fAdd){
		AccelOffset[0] += Xofs;
		AccelOffset[1] += Yofs;
		AccelOffset[2] += Zofs;
	}else{
		AccelOffset[0] = Xofs;
		AccelOffset[1] = Yofs;
		AccelOffset[2] = Zofs;
	}
};
void invensense_MPU::setGyroOffset(float Xofs, float Yofs, float Zofs, bool_t fAdd){
	if(fAdd){
		GyroOffset[0] += Xofs;
		GyroOffset[1] += Yofs;
		GyroOffset[2] += Zofs;
	}else{
		GyroOffset[0] = Xofs;
		GyroOffset[1] = Yofs;
		GyroOffset[2] = Zofs;
	}
};
	
	
	
// 姿勢推定
	
	
/*********************
姿勢推定リセット
概要：
引数：
 なし
返値：
**********************/
void invensense_MPU::resetCalcAttitudeEstimation(float Roll, float Pitch, float Yaw ){
	SumRoll = Roll;
	SumPitch = Pitch;
	SumYaw = Yaw;
}

/*********************
姿勢推定更新
概要：
引数：
 なし
返値：
**********************/
void invensense_MPU::updateCalcAttitudeEstimation(void){
	uint32_t Now_ms = getTime_ms();
	float Interval_s = (float)(Now_ms - TimeLastUpdate_ms)/1000.0F;
	
	TimeLastUpdate_ms = Now_ms;
	
	SumRoll += (Interval_s * Gyro[0]);
	SumPitch += (Interval_s * Gyro[1]);
	SumYaw += (Interval_s * Gyro[2]);
	
}



/*********************
I2Cクラスに通信をアタッチする
概要：
 通信を開始する。
 終わるとfetchI2cRcvDataが呼ばれる。
引数：

返値：
 AttachedIndex
**********************/
int8_t invensense_MPU::attachI2cComu(ismpu_comu_content ComuType, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum){
	int8_t AttachedIndex = 0;
	
	ismpu_comus_t* NewComu;
	
	NewComu = new ismpu_comus_t(I2cAddress, TxData, TxNum, RxNum);
		if(NULL==NewComu) __heap_chk_fail();
	if(NULL == NewComu){
		return -1;	// ヒープ足りない？
	}
	NewComu->ISMPU = this;
	NewComu->ComuType = ComuType;
	
	AttachedIndex = I2Cn->attach(NewComu);		// 送受信登録
	if(AttachedIndex<0){	// 登録失敗したらなかったことに
		fI2cErr = true;
		delete NewComu;
	}
	LastAttachComu = NewComu;
	fLastAttachComuFin = false;
	
	return AttachedIndex;
}


/*********************
I2C読み終わったので捕獲
概要：
引数：	
**********************/
int8_t invensense_MPU::fetchI2cRcvData(const ismpu_comus_t* Comu){
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
			case SET_CONFIG:
				break;
			case MEASURE:	// 計測値取得
				Accel[0] = (int16_t)((uint16_t)RcvData[0]<<8 | ((uint16_t)RcvData[1]&0x00FF)) / IsMpuAccelScaleFactor[AccelFs];
				Accel[1] = (int16_t)((uint16_t)RcvData[2]<<8 | ((uint16_t)RcvData[3]&0x00FF)) / IsMpuAccelScaleFactor[AccelFs];
				Accel[2] = (int16_t)((uint16_t)RcvData[4]<<8 | ((uint16_t)RcvData[5]&0x00FF)) / IsMpuAccelScaleFactor[AccelFs];
				Temp = (int16_t)(((uint16_t)RcvData[6]<<8)&0xFF00 | ((uint16_t)RcvData[7]&0x00FF)) / ISMPU_TEMP_SCALE + ISMPU_TEMP_OFFSET;
				Gyro[0] = (int16_t)((uint16_t)RcvData[8]<<8 | ((uint16_t)RcvData[9]&0x00FF)) / IsMpuGyroScaleFactor[GyroFs];
				Gyro[1] = (int16_t)((uint16_t)RcvData[10]<<8 | ((uint16_t)RcvData[11]&0x00FF)) / IsMpuGyroScaleFactor[GyroFs];
				Gyro[2] = (int16_t)((uint16_t)RcvData[12]<<8 | ((uint16_t)RcvData[13]&0x00FF)) / IsMpuGyroScaleFactor[GyroFs];
				
				// 軸方向
				if(invAxis[ISMPU_IDX_ACC_X])	Accel[0] = -Accel[0];
				if(invAxis[ISMPU_IDX_ACC_Y])	Accel[1] = -Accel[1];
				if(invAxis[ISMPU_IDX_ACC_Z])	Accel[2] = -Accel[2];
				if(invAxis[ISMPU_IDX_GYRO_X])	Gyro[0] = -Gyro[0];
				if(invAxis[ISMPU_IDX_GYRO_Y])	Gyro[1] = -Gyro[1];
				if(invAxis[ISMPU_IDX_GYRO_Z])	Gyro[2] = -Gyro[2];
				
				// オフセット
				Accel[0] -= AccelOffset[0];
				Accel[1] -= AccelOffset[1];
				Accel[2] -= AccelOffset[2];
				Gyro[0] -= GyroOffset[0];
				Gyro[1] -= GyroOffset[1];
				Gyro[2] -= GyroOffset[2];
				
				
				
				fMeasurement = false;
				
				updateCalcAttitudeEstimation();
				
				//更に計測する
				/*
				if(MeasurementRequest){
					measure();
				}
				*/
				
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

