/************************************************/
/*	浜松フォトニクス			*/
/*		デジタルカラーセンサS11059(I2C)	*/
/*			for RX63n @ CS+		*/
/*			Wrote by conpe_		*/
/*				2015/04/12	*/
/************************************************/

// wakeup()して、startMeasurement()して、定期的にupdateSens()する。
// getLuminance()で測定値取得。

// I2Cアドレス 0x2A(固定)


//【更新履歴】
// 2015.03.12 新規作成
// 2015.06.29 I2C通信内容クラス対応


#ifndef __HAMAS11059_H__
#define __HAMAS11059_H__

#include "RIIC.h"
#include "CommonDataType.h"

#define HMS11059_I2CADRS 0x2A


// レジスタ
#define  HMS11059_REG_CONTROL		0x00
#define  HMS11059_REG_MANUALTIMING	0x01
#define  HMS11059_REG_SENSRED		0x03
#define  HMS11059_REG_SENSGREEN		0x05
#define  HMS11059_REG_SENSBLUE		0x07
#define  HMS11059_REG_SENSIR		0x09



enum mhs11059_comu_content{
	SET_CTRL,
	SET_MANUALTIMING,
	GET_SENS,
};

class mhs11059_comus_t;

class hamamatsu_S11059{
protected:
	
	enum color{
		red,
		green,
		blue
	};
	
	I2c_t *I2Cn;
	uint8_t I2cAddress;		// I2Cアドレス
	uint16_t SensRed, SensGreen, SensBlue, SensIr;	// 計測したセンサ値
	bool_t fMeasurement;	// 計測中フラグ
	bool_t fRepeatMeasurement;	// 計測繰り返しフラグ
	bool_t fI2cErr;			// I2C通信エラー ->送受信やめる
	
	// I2Cクラスに通信内容をアタッチ
	int8_t attachI2cComu(mhs11059_comu_content ComuType, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum);
	mhs11059_comus_t* LastAttachComu;	// 最後に通信要求した通信
	bool_t fLastAttachComuFin;		// 最後に通信要求した通信が終わったかフラグ
public:
	// コンストラクタ
	hamamatsu_S11059(I2c_t* I2Cn);
	// デストラクタ
	virtual ~hamamatsu_S11059(void);
	// 動作開始
	virtual void begin(void);
	
	// 計測結果取得
	int8_t getLuminance(uint16_t* LuminanceRed, uint16_t* LuminanceGreen, uint16_t* LuminanceBlue);
	int8_t getLuminance(uint16_t* LuminanceRed, uint16_t* LuminanceGreen, uint16_t* LuminanceBlue, uint16_t* LuminanceIr);
	int8_t getLuminanceRed(uint16_t* LuminanceRed);
	uint16_t getLuminanceRed(void){return SensRed;};
	int8_t getLuminanceGreen(uint16_t* LuminanceGreen);
	uint16_t getLuminanceGreen(void){return SensGreen;};
	int8_t getLuminanceBlue(uint16_t* LuminanceBlue);
	uint16_t getLuminanceBlue(void){return SensBlue;};
	int8_t getLuminanceIr(uint16_t* LuminanceIr);
	uint16_t getLuminanceIr(void){return SensIr;};
	// HSV
	uint16_t getHsvH(void);	//HSV空間のH(色相)を返す
	
	// 各種通信
	// 1, センサ値読み取り
	virtual int8_t updateSens(void);
	// 2, モジュールを起こす
	virtual int8_t wakeup(void);
	// 3, 計測開始
	virtual int8_t startMeasurement(void);
	// 4, 計測停止
	virtual int8_t stopMeasurement(void);
	
	
	// 割り込み用
	// I2C受信データを取得して通信内容ごとに処理するよ
	int8_t fetchI2cRcvData(const mhs11059_comus_t* Comu);
};

class mhs11059_comus_t:public I2c_comu_t{
public:
	mhs11059_comus_t(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum)	: I2c_comu_t(DestAddress, TxData, TxNum, RxNum){DevId = (0x05);};	// 送受信
	
	//static uint8_t DevId;	// デバイスID
	hamamatsu_S11059* MHS11059;
	mhs11059_comu_content ComuType;

	void callBack(void){MHS11059->fetchI2cRcvData(this);};
};

#endif
