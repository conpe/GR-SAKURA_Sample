/**************************************************
inrof2016_lib.h
	�m�\���{�R��2015�p�̃��C�u���������B
	
	
	
	
**************************************************/


#ifndef __inrof2016_STVL_H__
#define __inrof2016_STVL_H__


#include "inrof2016_ball_sensor.h"
#include "StmicroVl6180.h"

#include "GR_Ex15_v3.h"


/****************************
 �m�\���{�R��2016
  PSD�Z���T
�T�v�F
 �Z���T�ʒu�Ƃ��̏��ǉ������B
****************************/
class inrof2016_st_vl_t : public inrof2016_ball_sensor_t{
public: 
	inrof2016_st_vl_t(I2c_t* I2Cn);
	inrof2016_st_vl_t(I2c_t* I2Cn, uint8_t adrs, pins EnablePin);
	
	int8_t begin(void);
	
	// ���W���[���N��
	int8_t enableModule(void){
		if(EnablePin){
			outPin(EnablePin, 1);
		}
		return 0;
	}
	// ���W���[���Q������
	int8_t disableModule(void){
		if(EnablePin){
			Tof->I2cAddress = STVL6_ADRS_DEFAULT;
			outPin(EnablePin, 0);
		}
		return 0;
	}
	
	// �P���v��(�ʐM�n)
	virtual int8_t measure(void);
	// �v���J�n
	virtual int8_t startMeasurement(void);
	// �v����~
	virtual int8_t stopMeasurement(void);
	
	// �v�����ʎ擾
	virtual int8_t getDistance(int16_t* Dist){*Dist = getDistance(); return 0;};
	virtual int16_t getDistance(void);
	
	// �ʐM�A�C�h����Ԃ�
	bool_t isIdleComu(void){return Tof->isIdleComu();};
	
	
	// �����Ă邩�`�F�b�N�v��
	int8_t checkAlive(void){return Tof->checkAlive();};
	// �����Ă邩�擾
	bool_t isAlive(void){return Tof->isAlive();};
	
	// �l�v�����Ȃ��Ȃ������Ƀ��Z�b�g����
	int8_t resetModule(void){return Tof->reset();};
	
private:
	stmicro_VL6810* Tof;
	pins EnablePin;
	
	
};



#endif
