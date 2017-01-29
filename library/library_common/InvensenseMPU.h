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

// 角度はdeg

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
extern uint32_t getTime_ms(void);	// 時間計測

// デフォルト設定
#define ISMPU_ADRS_DEFAULT 0x68		// I2Cアドレス(慣性センサ)
#define ISMPU_AK_ADRS_DEFAULT 0x0C	// I2Cアドレス(磁気センサ)
#define ISMPU_ACCEL_DEFAULT 0.0F
#define ISMPU_GYRO_DEFAULT 0.0F
#define ISMPU_TEMP_DEFAULT 0.0F

#define ISMPU_TEMP_SCALE 	333.87F		// LSB/deg
#define ISMPU_TEMP_OFFSET	21.0F		// deg


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

// 軸インデックス
#define ISMPU_IDX_ACC_X		0
#define ISMPU_IDX_ACC_Y		1
#define ISMPU_IDX_ACC_Z		2
#define ISMPU_IDX_GYRO_X	3
#define ISMPU_IDX_GYRO_Y	4
#define ISMPU_IDX_GYRO_Z	5
#define ISMPU_IDX_MAG_X		6
#define ISMPU_IDX_MAG_Y		7
#define ISMPU_IDX_MAG_Z		8



enum ismpu_comu_content{
	MEASURE,
	SET_CONFIG,		// 特にコールバックで処理しない設定系はこれで
};

class ismpu_comus_t;

class invensense_MPU{
public:
	// コンストラクタ
	invensense_MPU(I2c_t* I2Cn, uint8_t I2cAddress);	// アドレスは 0x68 or 0x69 (AD0端子によって決まる)
	~invensense_MPU(void);
	

	// バースト計測開始
	int8_t startMeasurement(void);
	// バースト計測停止
	int8_t stopMeasurement(void);
	// モジュールセットアップ
	int8_t begin(void);
	
	
	// 計測結果取得
	// 加速度センサ
	int8_t getAccel(float* AccX, float* AccY, float* AccZ );	
	float getAccelX(void){return Accel[0];};
	float getAccelY(void){return Accel[1];};
	float getAccelZ(void){return Accel[2];};
	// ジャイロセンサ [deg/s]
	int8_t getGyro(float* GyroX, float* GyroY, float* GyroZ);
	float getGyroX(void){return Gyro[0];};
	float getGyroY(void){return Gyro[1];};
	float getGyroZ(void){return Gyro[2];};
	// 温度センサ
	int8_t getTemp(float* Temp);
	float getTemp(void){return Temp;};
	// 磁気センサ
	// 6byte＋さらに1byte読まないと連続変換しないので注意
	
	
	// 設定
	void setAxis(bool_t AccX=0, bool_t AccY=0, bool_t AccZ=0, bool_t GyroX=0, bool_t GyroY=0, bool_t GyroZ=0, bool_t MagX=0, bool_t MagY=0, bool_t MagZ=0);	// 軸方向 0:デフォルト, 1: 逆
	void setAccelOffset(float Xofs, float Yofs, float Zofs, bool_t fAdd = 0);
	void setGyroOffset(float Xofs, float Yofs, float Zofs, bool_t fAdd = 0);
	
	
	// 姿勢推定
	// Attitude Estimation
	float getAeRoll(void){return SumRoll;};
	float getAePitch(void){return SumPitch;};
	float getAeYaw(void){return SumYaw;};
	void resetCalcAttitudeEstimation(float Roll = 0.0F, float Pitch = 0.0F, float Yaw = 0.0F);	// deg
	
	
	// 各種通信
	// 測定値要求
	int8_t measure(void);		// 周期的に呼ぶ
	// ジャイロセンサのレンジ設定
	int8_t setGyroRange(uint8_t Fs);
	// 加速度センサのレンジ設定
	int8_t setAccelRange(uint8_t Fs);
	
	// ステータス
	bool_t isComuErr(void){return fI2cErr;};
	// debug
	i2c_status getStatus(void){return I2Cn->getStatus();};
	i2c_statusin getStatusIn(void){return I2Cn->getStatusIn();};
	
	
	// 割り込み用
	// I2C受信データを取得して通信内容ごとに処理するよ
	int8_t fetchI2cRcvData(const ismpu_comus_t* Comu);
	
private:
	I2c_t *I2Cn;
	uint8_t I2cAddress;		// I2Cアドレス
	uint8_t *RcvBuff;
	float Accel[3];		// 加速度x y z  g
	float Gyro[3];		// 角速度x y z  deg/s
	float AccelOffset[3];		// 加速度x y z オフセット  g
	float GyroOffset[3];		// 角速度x y z オフセット deg/s
	float Temp;		// 温度  deg
	uint8_t GyroFs;		// ジャイロセンサフルスケール設定値
	uint8_t AccelFs;	// 加速度センサフルスケール設定値
	
	bool_t invAxis[9];	// 軸方向逆にするフラグ
	
	bool_t MeasurementRequest;	// 計測要求
	bool_t fMeasurement;		// 計測中フラグ
	bool_t fI2cErr;			// I2C通信エラー ->送受信やめる
	
	// 姿勢推定
	uint32_t TimeLastUpdate_ms;	// 前回更新時間
	float SumRoll;
	float SumPitch;
	float SumYaw;
	void updateCalcAttitudeEstimation(void);
	
	
	// I2Cクラスに通信内容をアタッチ
	int8_t attachI2cComu(ismpu_comu_content ComuType, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum);
	ismpu_comus_t* LastAttachComu;	// 最後に通信要求した通信
	bool_t fLastAttachComuFin;		// 最後に通信要求した通信が終わったかフラグ
};

class ismpu_comus_t:public I2c_comu_t{
public:
	ismpu_comus_t(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum)	: I2c_comu_t(DestAddress, TxData, TxNum, RxNum){};	// 送受信
	
	static uint8_t DevId;	// デバイスID
	invensense_MPU* ISMPU;
	ismpu_comu_content ComuType;

	void callBack(void){ISMPU->fetchI2cRcvData(this);};
	
};

#endif
