/**************************************************
inrof2016_lib.h
	知能ロボコン2015用のライブラリたち。
	
	
	
	
**************************************************/


#ifndef __inrof2016_STVL_H__
#define __inrof2016_STVL_H__


#include "inrof2016_ball_sensor.h"
#include "StmicroVl6180.h"

#include "GR_Ex15_v3.h"


/****************************
 知能ロボコン2016
  PSDセンサ
概要：
 センサ位置とかの情報追加した。
****************************/
class inrof2016_st_vl_t : public inrof2016_ball_sensor_t{
public: 
	inrof2016_st_vl_t(I2c_t* I2Cn);
	inrof2016_st_vl_t(I2c_t* I2Cn, uint8_t adrs, pins EnablePin);
	
	int8_t begin(void);
	
	// モジュール起動
	int8_t enableModule(void){
		if(EnablePin){
			outPin(EnablePin, 1);
		}
		return 0;
	}
	// モジュール寝かせる
	int8_t disableModule(void){
		if(EnablePin){
			Tof->I2cAddress = STVL6_ADRS_DEFAULT;
			outPin(EnablePin, 0);
		}
		return 0;
	}
	
	// 単発計測(通信系)
	virtual int8_t measure(void);
	// 計測開始
	virtual int8_t startMeasurement(void);
	// 計測停止
	virtual int8_t stopMeasurement(void);
	
	// 計測結果取得
	virtual int8_t getDistance(int16_t* Dist){*Dist = getDistance(); return 0;};
	virtual int16_t getDistance(void);
	
	// 通信アイドル状態か
	bool_t isIdleComu(void){return Tof->isIdleComu();};
	
	
	// 生きてるかチェック要求
	int8_t checkAlive(void){return Tof->checkAlive();};
	// 生きてるか取得
	bool_t isAlive(void){return Tof->isAlive();};
	
	// 値計測しなくなった時にリセットする
	int8_t resetModule(void){return Tof->reset();};
	
private:
	stmicro_VL6810* Tof;
	pins EnablePin;
	
	
};



#endif
