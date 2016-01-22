/********************************************/
/*		InvenSense MPUxxxx					*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/03/31		*/
/********************************************/

//【使い方】
// はじめに計測開始のためsetup()を呼ぶ。
// センサから計測値を取得するため、定期的にmeasure()を呼ぶ(I2C送受信開始)。
// getAccel(), getGyro(), getTemp()で計測値を返す。


// デフォルト設定
// I2Cアドレス 0x68 or 0x69 (AD0ピンの設定による)
// 

//【更新履歴】
// 2015.03.31 新規作成
// 2015.06.29 I2C通信内容クラス対応


#ifndef __INVENSENSEISMPU_H__
#define __INVENSENSEISMPU_H__

#include "RIIC.h"
#include "CommonDataType.h"

// デフォルト設定
#define ISMPU_ADRS_DEFAULT 0x68
#define ISMPU_ACCEL_DEFAULT 0
#define ISMPU_GYRO_DEFAULT 0
#define ISMPU_TEMP_DEFAULT 0

#define ISMPU_TEMP_SCALE 	340		// LSB/deg
#define ISMPU_TEMP_OFFSET	36.53	//	deg

// レジスタ
#define  ISMPU_REG_GYRO_CONFIG	0x1B	
#define  ISMPU_REG_ACCEL_CONFIG	0x1C
#define  ISMPU_REG_PWR_MGMT_1	0x6B
#define  ISMPU_REG_ACCEL_XOUT_H	0x3B
#define  ISMPU_REG_ACCEL_XOUT_L	0x3C
#define  ISMPU_REG_ACCEL_YOUT_H	0x3D
#define  ISMPU_REG_ACCEL_YOUT_L	0x3E
#define  ISMPU_REG_ACCEL_ZOUT_H	0x3F
#define  ISMPU_REG_ACCEL_ZOUT_L	0x40
#define  ISMPU_REG_TEMP_OUT_H	0x41
#define  ISMPU_REG_TEMP_OUT_L	0x42
#define  ISMPU_REG_GYRO_XOUT_H	0x43
#define  ISMPU_REG_GYRO_XOUT_L	0x44
#define  ISMPU_REG_GYRO_YOUT_H	0x45
#define  ISMPU_REG_GYRO_YOUT_L	0x46
#define  ISMPU_REG_GYRO_ZOUT_H	0x47
#define  ISMPU_REG_GYRO_ZOUT_L	0x48


enum ismpu_comu_content{
	MEASURE,
	SET_CONFIG,		// 特にコールバックで処理しない設定系はこれで
};

class ismpu_comus_t;

class invensense_MPU{
private:
	I2c_t *I2Cn;
	uint8_t I2cAddress;		// I2Cアドレス
	uint8_t *RcvBuff;
	float Accel[3];		// 加速度x y z  g
	float Gyro[3];		// 角速度x y z  deg/s
	float Temp;		// 温度  deg
	uint8_t GyroFs;		// ジャイロセンサフルスケール設定値
	uint8_t AccelFs;	// 加速度センサフルスケール設定値
	
	
	bool_t MeasurementRequest;	// 計測要求
	bool_t fMeasurement;	// 計測中フラグ
	bool_t fI2cErr;			// I2C通信エラー ->送受信やめる
	
	// I2Cクラスに通信内容をアタッチ
	int8_t attachI2cComu(ismpu_comu_content ComuType, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum);
	ismpu_comus_t* LastAttachComu;	// 最後に通信要求した通信
	bool_t fLastAttachComuFin;		// 最後に通信要求した通信が終わったかフラグ
public:
	// コンストラクタ
	invensense_MPU(I2c_t* I2Cn, uint8_t I2cAddress);	// アドレスは 0x68 or 0x69 (AD0端子によって決まる)
	~invensense_MPU(void);
	

	// バースト計測開始
	int8_t startMeasurement(void);
	// バースト計測停止
	int8_t stopMeasurement(void);
	
	
	// 計測結果取得
	// 加速度センサ
	int8_t getAccel(float* AccX, float* AccY, float* AccZ );
	float getAccelX(void){return Accel[0];};
	float getAccelY(void){return Accel[1];};
	float getAccelZ(void){return Accel[2];};
	// ジャイロセンサ
	int8_t getGyro(float* GyroX, float* GyroY, float* GyroZ);
	float getGyroX(void){return Gyro[0];};
	float getGyroY(void){return Gyro[1];};
	float getGyroZ(void){return Gyro[2];};
	// 温度センサ
	int8_t getTemp(float* Temp);
	float getTemp(void){return Temp;};
	
	// 各種通信
	// モジュールセットアップ
	int8_t setup(void);
	// 測定開始
	int8_t measure(void);
	// ジャイロセンサのレンジ設定
	int8_t setGyroRange(uint8_t Fs);
	// 加速度センサのレンジ設定
	int8_t setAccelRange(uint8_t Fs);
	

	// debug
	i2c_status getStatus(void){return I2Cn->getStatus();};
	i2c_statusin getStatusIn(void){return I2Cn->getStatusIn();};
	
	
	// 割り込み用
	// I2C受信データを取得して通信内容ごとに処理するよ
	int8_t fetchI2cRcvData(const ismpu_comus_t* Comu);
};

class ismpu_comus_t:public I2c_comu_t{
public:
	ismpu_comus_t(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum)	: I2c_comu_t(DestAddress, TxData, TxNum, RxNum){};	// 送受信
	
	invensense_MPU* ISMPU;
	ismpu_comu_content ComuType;

	void callBack(void){ISMPU->fetchI2cRcvData(this);};
	
};

#endif
