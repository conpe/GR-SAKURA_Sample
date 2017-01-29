
#include "inrof2016_sharp_dist.h"




/****************************
 知能ロボコン2016
  ボールセンサ sharp測距センサ(AD版)
概要：
****************************/
inrof2016_sharp_dist_t::inrof2016_sharp_dist_t(grex_ad_ch AdCh, pins EnablePin){
	this->AdCh = AdCh;
	this->EnablePin = EnablePin;
	
	Distance_mm = SHGP2AD_DISTANCE_DEFAULT;
}



/*********************
begin
引数：
**********************/
int8_t inrof2016_sharp_dist_t::begin(void){
	GrEx.startAD();
	enableModule();	// 測定開始
	
	return 0;
}

/*********************
測定要求
引数：
**********************/
int8_t inrof2016_sharp_dist_t::measure(void){
	Distance_mm = SHPDIST_AD10_DIST_SLOPE * GrEx.getAD10((grex_ad_ch)AdCh) + SHPDIST_AD10_DIST_INTERCEPT;
			//10bitで取得
	// 最大値制限
	if(SHGP2AD_FAIL_DISTANCE_MM < Distance_mm){
		Distance_mm = SHGP2AD_DISTANCE_DEFAULT;
	}
	
	return 0;
}

/*********************
測定開始
30mAくらい食う
引数：
**********************/
int8_t inrof2016_sharp_dist_t::enableModule(void){
	outPin(EnablePin, 1);
	return 0;
}
/*********************
測定終了
引数：
**********************/
int8_t inrof2016_sharp_dist_t::disableModule(void){
	outPin(EnablePin, 0);
	return 0;
}





