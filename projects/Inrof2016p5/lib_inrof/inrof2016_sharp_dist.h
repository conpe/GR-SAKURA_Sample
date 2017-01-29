/**************************************************
inrof2016_lib.h
	知能ロボコン2015用のライブラリたち。
	
	
	
	
**************************************************/



// getDistance();で測定距離取得




#ifndef __inrof2016_SHARPDIST_H__
#define __inrof2016_SHARPDIST_H__


#include "inrof2016_ball_sensor.h"

// AD変換10bitにおける、距離[mm]への切片と傾き
// 10cmで2V(620.6ad), 50cmで0.6V(186.2ad)
// Dist = SLOPE * AD10 + INTERCEPT
#define SHPDIST_AD10_DIST_SLOPE		-0.92F
#define SHPDIST_AD10_DIST_INTERCEPT	670.95F

// 初期値(初期、無効値時にこの値になる)
#define SHGP2AD_DISTANCE_DEFAULT	-1
// これ以上は無効値
#define SHGP2AD_FAIL_DISTANCE_MM	500


/****************************
 知能ロボコン2016
  Sharpの測距センサ
概要：
 センサ位置とかの情報追加した。
****************************/
class inrof2016_sharp_dist_t : public inrof2016_ball_sensor_t{
public: 
	inrof2016_sharp_dist_t(grex_ad_ch AdCh, pins EnablePin);
	~inrof2016_sharp_dist_t(void){disableModule();};
	
	int8_t begin(void);
	
	// 単発計測(通信系)
	virtual int8_t measure(void);
	// 計測開始(電流食うよ
	virtual int8_t enableModule(void);
	// 計測停止
	virtual int8_t disableModule(void);
	
	// 計測結果取得
	virtual int8_t getDistance(int16_t* Dist){*Dist = getDistance(); return 0;};
	virtual int16_t getDistance(void){ measure(); return Distance_mm;};
	
private:
	grex_ad_ch AdCh;
	pins EnablePin;

};



#endif
