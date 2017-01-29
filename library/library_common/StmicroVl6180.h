/********************************************/
/*	STmicro VL6180 (I2C)		    */
/*			for RX63n @ CS+     */
/*			Wrote by conpe_	    */
/*			2016/05/23	    */
/********************************************/

//【使い方】
// まずbegin();でI2C初期化
// センサから計測値を取得するため、定期的にmeasure()を呼ぶ(I2C送受信開始)。
// getDistance()で計測値を返す。

// デフォルト設定
// I2Cアドレス 0x29

//【更新履歴】
// 2016.05.23 新規作成

//【更新したい】
// attachの戻り値は、アタッチ失敗に加えて、デバイスと通信ができなかったことを返したい
// 現状だとアタッチ失敗のみの判定なので、デバイスと通信できなくてもok判定になる。
// デバイスと通信できない場合、バッファクリアしちゃうので、アタッチ自体は成功する。
// 一気に送りすぎた時だけ失敗する。

#ifndef __STVL6180_H__
#define __STVL6180_H__

#include "RIIC.h"
#include "CommonDataType.h"
// debug
#include "SCI.h"

// デフォルト設定
#define STVL6_ADRS_DEFAULT		0x29
#define STVL6_DISTANCE_DEFAULT	-1
#define STVL6_FAIL_DISTANCE_MM	255

class stvl6_comus_t;

class stmicro_VL6810{
public:
	enum register_e{
		IDENTIFICATION_MODEL_ID = 0x0000,
		SYSTEM__INTERRUPT_CLEAR = 0x0015,
		SYSRANGE_START = 0x0018,
		RESULT_RANGE_STATUS = 0x004D,
		RESULT_RANGE_VAL = 0x0062,
		RESULT_RANGE_RAW = 0x0064,
		RESULT_RANGE_RETURN_SIGNAL_COUNT = 0x006C,
		SYSRANGE_INTERMEASUREMENT_PERIOD = 0x001B,
		SYSRANGE_PART_TO_PART_RANGE_OFFSET = 0x0024,
		I2C_SLAVE__DEVICE_ADDRESS = 0x0212
	};

	// コンストラクタ
	stmicro_VL6810(I2c_t* I2Cn);
	stmicro_VL6810(I2c_t* I2Cn, uint8_t I2cAddress);
	// デストラクタ
	virtual ~stmicro_VL6810(void);
	
	//動作開始
	int8_t begin(void);
	
	// バースト計測開始(ずーっと通信しまくるからおすすめしない
	int8_t startMeasurement(void);
	// バースト計測停止
	int8_t stopMeasurement(void);
	
	
	// 計測結果取得
	int8_t getDistance(int16_t* Dist);
	int16_t getDistance(void){return Distance_mm;};
	
	// 測定結果取得要求
	int8_t measure(void);
	// 測定周期セット
	int8_t setMeasurePeriod(uint16_t msec);
	// 割り込みクリア
	int8_t clearInterrupt(void);
	//I2Cアドレス設定
	int8_t setI2cAddress(uint8_t NewAdrs);
	
	// 各種通信
	int8_t setReg(register_e ComuReg, uint8_t SetData);	// レジスタセット
	int8_t reqDat(register_e ComuReg, uint16_t ReqNum);	// データ受信
	
	
	// 生きてるか判定
	bool_t isAlive(void){return RangeStatus;};
	//bool_t isAlive(void){return RangeReturnSignalCount;};
	int8_t checkAlive(void){ return reqDat(RESULT_RANGE_STATUS, 1); };
	//int8_t checkAlive(void){ return reqDat(RESULT_RANGE_RETURN_SIGNAL_COUNT, 1); };
	
	// 復活の儀式
	int8_t reset(void);
	
	// 通信アイドル状態か
	// I2Cアドレス設定時に順に起動させるために必要
	bool_t isIdleComu(void){return I2Cn->isIdle();};
	
	
	// 受信データ処理用コールバック関数
	// I2C受信データを取得して通信内容ごとに処理するよ
	int8_t fetchI2cRcvData(const stvl6_comus_t* Comu);
	
	
	uint8_t I2cAddress;		// I2Cアドレス
private:
	I2c_t *I2Cn;
	uint8_t I2cAddressPre;		// I2Cアドレス(確定前)
	uint8_t *RcvBuff;
	int16_t Distance_mm;	// 計測した距離[mm]
	bool_t MeasurementRequest;	// 計測要求
	bool_t fI2cErr;			// I2C通信エラー ->送受信やめる
	uint8_t *FuseRcv;	// 
	uint8_t cntget;
	
	uint8_t RangeStatus;
	uint32_t RangeReturnSignalCount;
	
	void init(void);
	
	// I2Cクラスに通信内容をアタッチ
	int8_t attachI2cComu(register_e ComuReg, uint8_t SetData, uint16_t RxNum);
	
};

class stvl6_comus_t:public I2c_comu_t {
public:
	stvl6_comus_t(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum)	: I2c_comu_t(DestAddress, TxData, TxNum, RxNum){DevId = (0x03);};	// 送受信
	
	//uint8_t DevId;	// デバイスID
	stmicro_VL6810* STVL6;
	stmicro_VL6810::register_e ComuReg;

	void callBack(void){
		STVL6->fetchI2cRcvData(this);
	};
};

#endif
