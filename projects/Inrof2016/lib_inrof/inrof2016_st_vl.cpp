
#include "inrof2016_st_vl.h"

/****************************
 知能ロボコン2016
  ボールセンサ STマイクロのToF測距センサ
概要：
****************************/
inrof2016_st_vl_t::inrof2016_st_vl_t(I2c_t* I2Cn){
	Tof = new stmicro_VL6810(I2Cn);
	
	this->EnablePin = (pins)0;
	
	Distance_mm = STVL6_DISTANCE_DEFAULT;
}
inrof2016_st_vl_t::inrof2016_st_vl_t(I2c_t* I2Cn, uint8_t adrs, pins EnablePin){
	Tof = new stmicro_VL6810(I2Cn, adrs);
	
	this->EnablePin = EnablePin;
	
	Distance_mm = STVL6_DISTANCE_DEFAULT;
}

/*********************
begin
引数：
**********************/
int8_t inrof2016_st_vl_t::begin(void){
	return Tof->begin();
}

/*********************
測定要求
引数：
**********************/
// 単発計測(通信系)
int8_t inrof2016_st_vl_t::measure(void){
	Tof->measure();	// 測定値取得要求
	return 0;
}


/*********************
測定開始
30mAくらい食う
引数：
**********************/
int8_t inrof2016_st_vl_t::startMeasurement(void){
	
	return 0;
}
/*********************
測定終了
引数：
**********************/
int8_t inrof2016_st_vl_t::stopMeasurement(void){
	
	return 0;
}


/*********************
測定値
引数：
**********************/
int16_t inrof2016_st_vl_t::getDistance(void){

	Distance_mm = Tof->getDistance();
	return Distance_mm;
}

