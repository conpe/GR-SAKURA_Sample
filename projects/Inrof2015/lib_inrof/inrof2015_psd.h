/**************************************************
Inrof2015_lib.h
	知能ロボコン2015用のライブラリたち。
	
	
	
	
**************************************************/


#ifndef __INROF2015_PSD_H__
#define __INROF2015_PSD_H__


#include "SharpGP2Y0E.h"
#include "RIIC.h"

#include "mathf.h"
#include "Inrof2015_common.h"
//#include "Inrof2015_lib.h"




/****************************
 知能ロボコン2015
  PSDセンサ
概要：
 PSDセンサを継承して, センサ位置とかの情報追加した。
****************************/
class inrof2015_psd : public sharp_GP2Y0E{
public: 
	inrof2015_psd(I2c_t* I2Cn, uint8_t I2cAddress, float PosX, float PosY, float PosTh);
	int8_t getGlobalObjPos(float* PosXw, float* PosYw);
	void setMachinePos(position* MachinePos);
	void getGlobalPos(float* PosXw, float* PosYw);
	bool_t isObjectBallArea(void);
private:
	float PosX_;
	float PosY_;
	float PosTh_;
	float MachinePosX_;
	float MachinePosY_;
	float MachinePosTh_;
};



#endif
