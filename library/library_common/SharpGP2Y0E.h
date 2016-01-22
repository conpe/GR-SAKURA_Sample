/********************************************/
/*		SHARP 測距センサ __GP2Y0E*(I2C)		*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/03/11		*/
/********************************************/

//【使い方】
// まずbegin();でI2C初期化
// センサから計測値を取得するため、定期的にmeasure()を呼ぶ(I2C送受信開始)。
// getDistance()で計測値を返す。

// デフォルト設定
// I2Cアドレス 0x40

//【更新履歴】
// 2015.03.11 新規作成
// 2015.06.26 I2C通信内容クラス対応

//【更新したい】
// センサによって
// AD変換Ver. の追加


#ifndef __SHARPGP2Y0E_H__
#define __SHARPGP2Y0E_H__

#include "RIIC.h"
#include "CommonDataType.h"
#include "portReg.h"


// デフォルト設定
#define SHGP2_ADRS_DEFAULT		0x40
#define SHGP2_DISTANCE_DEFAULT	-1
#define SHGP2_FAIL_DISTANCE_MM	639

// E-fuse用電源管理
#define SHGP2_VPP_ON 0		// ポートに0出力でon
#define SHGP2_VPP_OFF 1		// ポートに1出力でoff

// レジスタ
#define  SHGP2_REG_MEDIANFILTER	0x3F
#define  SHGP2_REG_DISTANCE		0x5E



enum shgp2_comu_content{
	GET_DISTANCE,
	SET_MEDIANFILTER,
	SET_FUSE_I2CADDRESS,
};

class shgp2_comus_t;


class sharp_GP2Y0E{
private:
	I2c_t *I2Cn;
	uint8_t I2cAddress;		// I2Cアドレス
	uint8_t *RcvBuff;
	int16_t Distance_mm;	// 計測した距離[mm]
	bool_t MeasurementRequest;	// 計測要求
	bool_t fMeasurement;	// 計測中フラグ
	bool_t fI2cErr;			// I2C通信エラー ->送受信やめる
	uint8_t *FuseRcv;	// 
	
	// I2Cクラスに通信内容をアタッチ
	int8_t attachI2cComu(shgp2_comu_content ComuType, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum);
	shgp2_comus_t* LastAttachComu;	// 最後に通信要求した通信
	bool_t fLastAttachComuFin;		// 最後に通信要求した通信が終わったかフラグ
public:
	// コンストラクタ
	sharp_GP2Y0E(I2c_t* I2Cn, uint8_t I2cAddress);
	// デストラクタ
	~sharp_GP2Y0E(void);
	
	//動作開始
	int8_t begin(void);
	
	// バースト計測開始(ずーっと通信しまくるからおすすめしない
	int8_t startMeasurement(void);
	// バースト計測停止
	int8_t stopMeasurement(void);
	
	
	// 計測結果取得
	int8_t getDistance(int16_t* Dist);
	int16_t getDistance(void){return Distance_mm;};
	
	// 各種通信
	// 1, 測定 (定期的に呼ぶ)
	int8_t measure(void);
	// 2, メディアンフィルタの設定
	int8_t setMedianFilter(uint8_t MedianCalcNum);
	// 3, スリープモード
	int8_t setSleepMode(bool_t toSleep);
	
	// Fuse bit 書き込み
	// 1, I2Cアドレス変更
	int8_t setI2cAddress(uint8_t NewAddress, pins Vpp);
	
	// debug
	i2c_status getStatus(void){return I2Cn->getStatus();};
	i2c_statusin getStatusIn(void){return I2Cn->getStatusIn();};
	
	
	// 受信データ処理用コールバック関数
	// I2C受信データを取得して通信内容ごとに処理するよ
	int8_t fetchI2cRcvData(const shgp2_comus_t* Comu);
};

class shgp2_comus_t:public I2c_comu_t {
public:
	shgp2_comus_t(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum)	: I2c_comu_t(DestAddress, TxData, TxNum, RxNum){};	// 送受信
	
	sharp_GP2Y0E* SHGP2;
	shgp2_comu_content ComuType;

	void callBack(void){SHGP2->fetchI2cRcvData(this);};
};

#endif
