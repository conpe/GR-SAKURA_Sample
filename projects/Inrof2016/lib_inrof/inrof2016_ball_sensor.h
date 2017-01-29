/****************************
 知能ロボコン2016
  ボールセンサ
概要：
****************************/


#ifndef __inrof2016_BALLSENSOR_H__
#define __inrof2016_BALLSENSOR_H__


#include "mathf.h"
//#include "Inrof2016_lib.h"
#include "Inrof2016_common.h"
#include "CommonDataType.h"
#include "portReg.h"




/****************************
 知能ロボコン2016
  PSDセンサ
概要：
 センサ位置とかの情報追加した。
****************************/
class inrof2016_ball_sensor_t{
public: 
	virtual ~inrof2016_ball_sensor_t(void){/*  */};
	virtual int8_t begin(void) = 0;
	
	
	void setSensPos(float PosX, float PosY, float PosTh);
	void setSensPos(position* Pos);
	
	
	// 単発計測(通信系)
	virtual int8_t measure(void) = 0;
	// モジュール動作開始
	virtual int8_t enableModule(void) = 0;
	// モジュール動作停止
	virtual int8_t disableModule(void) = 0;
	
	// 計測結果取得
	virtual int8_t getDistance(int16_t* Dist){*Dist = getDistance(); return 0;};
	virtual int16_t getDistance(void) = 0;
	
	int8_t getGlobalObjPos(position* MachinePos, float* PosXw, float* PosYw);
	//void setMachinePos(position* MachinePos);
	void getGlobalPos(position* MachinePos, float* PosXw, float* PosYw);
	bool_t isObjectBallArea(position* MachinePos);
	
	
	// 通信アイドル状態か
	virtual bool_t isIdleComu(void) = 0;
	
protected:
	int16_t Distance_mm;	// 計測した距離[mm]
	position Pos;
	position MachinePos;
	
};



#endif
