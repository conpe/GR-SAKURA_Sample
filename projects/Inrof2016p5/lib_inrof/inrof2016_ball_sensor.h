/****************************
 �m�\���{�R��2016
  �{�[���Z���T
�T�v�F
****************************/


#ifndef __inrof2016_BALLSENSOR_H__
#define __inrof2016_BALLSENSOR_H__


#include "mathf.h"
//#include "Inrof2016_lib.h"
#include "Inrof2016_common.h"
#include "CommonDataType.h"
#include "portReg.h"




/****************************
 �m�\���{�R��2016
  PSD�Z���T
�T�v�F
 �Z���T�ʒu�Ƃ��̏��ǉ������B
****************************/
class inrof2016_ball_sensor_t{
public: 
	virtual ~inrof2016_ball_sensor_t(void){/*  */};
	virtual int8_t begin(void) = 0;
	
	
	void setSensPos(float PosX, float PosY, float PosTh);
	void setSensPos(position* Pos);
	
	
	// �P���v��(�ʐM�n)
	virtual int8_t measure(void) = 0;
	// ���W���[������J�n
	virtual int8_t enableModule(void) = 0;
	// ���W���[�������~
	virtual int8_t disableModule(void) = 0;
	
	// �v�����ʎ擾
	virtual int8_t getDistance(int16_t* Dist){*Dist = getDistance(); return 0;};
	virtual int16_t getDistance(void) = 0;
	
	int8_t getGlobalObjPos(position* MachinePos, float* PosXw, float* PosYw);	// �Ώە��̈ʒu
	int8_t getGlobalObjPos(position* MachinePos, position* ObjPos);	
	//void setMachinePos(position* MachinePos);
	void getGlobalPos(position* MachinePos, float* PosXw, float* PosYw);	// �Z���T�ʒu
	void getGlobalPos(position* MachinePos, position* SensPos);		// �Z���T�ʒu
	int16_t getObjDistFromMachine(position* MachinePos); 			//�}�V����������̋���
	
	bool_t isObjectBallArea(position* MachinePos);			// �Ώە��̓{�[���G���A�H
	
	
	// �ʐM�A�C�h����Ԃ�
	virtual bool_t isIdleComu(void) = 0;
	
protected:
	int16_t Distance_mm;	// �v����������[mm]
	position Pos;
	position MachinePos;
	
};



#endif
