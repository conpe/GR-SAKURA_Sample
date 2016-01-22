/**************************************************
Inrof2015_lib.h
	�m�\���{�R��2015�p�̃��C�u���������B
	
	
	
	
**************************************************/


#ifndef __INROF2015_PSD_H__
#define __INROF2015_PSD_H__


#include "SharpGP2Y0E.h"
#include "RIIC.h"

#include "mathf.h"
#include "Inrof2015_common.h"
//#include "Inrof2015_lib.h"




/****************************
 �m�\���{�R��2015
  PSD�Z���T
�T�v�F
 PSD�Z���T���p������, �Z���T�ʒu�Ƃ��̏��ǉ������B
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
