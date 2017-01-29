/************************************************/
/*	浜松フォトニクス			*/
/*		デジタルカラーセンサS11059(I2C)	*/
/*			for RX63n @ CS+		*/
/*			Wrote by conpe_		*/
/*				2016/05/26	*/
/************************************************/

// wakeup()して、startMeasurement()して、定期的にupdateSens()する。
// getLuminance()で測定値取得。

// I2Cアドレス 0x2A(固定)


//【更新履歴】
// 2016.05.26 新規作成


#ifndef __INROF2016COLOR_H__
#define __INROF2016COLOR_H__

#include <math.h>
#include "CommonDataType.h"
#include "HamamatsuColorSensS11059.h"
#include "inrof2016_common.h"

//#define ENC_MODE_DEFAULT 0x02

#define COLSENS_LED_PIN 	GREX_IO0	// LEDのピン。黄色の線
#define COLSENS_BALL_EXIST_LUM	80
#define COLSENS_BALL_JUDGE_CNT	2		// n回連続で有効値なら確定

class inrof2016_color_sens_t : public hamamatsu_S11059{
public:
	// コンストラクタ
	inrof2016_color_sens_t(I2c_t* I2Cn, uint8_t ArmAdrs);
	// デストラクタ
	~inrof2016_color_sens_t(void);
	// 動作開始
	void begin(void);
	
	// 測定開始
	void enableMeasure(void){fJudgeColor = true;};
	// 測定終了
	void disableMeasure(void){fJudgeColor = false; offBallColorLed();};
	
	// ボール色判定処理 
	// 定期的に呼び出すこと
	int8_t updateBallColor(void);
	
	// ボール色取得
	inrof_color getBallColor(void){return BallColor;};
	
private:
	I2c_t* ArmI2C;
	uint8_t ArmAdrs;
	
	bool_t BallColorLed;		// LED光らせてるか
	
	//ボール判定
	bool_t fJudgeColor;
	inrof_color BallColor;		// 判定ボール色
	inrof_color BallColorTmp;	// 保存値
	inrof_color BallColorTmpLast;	// 保存値
	uint8_t JudgeCnt;		// 色確定カウント
	//inrof_color BallColorFixed;	// [確定]判定ボール色
	uint16_t BallColorVal[3][2];	// センサ値記憶
	
	
	// カラーセンサ用LED onoff
	int8_t onBallColorLed(void);
	int8_t offBallColorLed(void);
	bool_t isLedOn(void){return BallColorLed;};
};


#endif
