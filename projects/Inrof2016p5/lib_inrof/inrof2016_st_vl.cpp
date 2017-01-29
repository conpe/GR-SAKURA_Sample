
#include "inrof2016_st_vl.h"

/****************************
 �m�\���{�R��2016
  �{�[���Z���T ST�}�C�N����ToF�����Z���T
�T�v�F
****************************/
// Enable�s���g�p
inrof2016_st_vl_t::inrof2016_st_vl_t(I2c_t* I2Cn, uint8_t adrs, pins EnablePin){
	Tof = new stmicro_VL53L0X(I2Cn, adrs);
		if(NULL==Tof) __heap_chk_fail();
	
	this->EnablePin = EnablePin;
	if(P_NONE != EnablePin){
		setPinMode(EnablePin, PIN_OUTPUT_OPENDRAIN);	// �d���Ⴄ�̂ŃI�[�v���h���C���ŁB
		outPin(EnablePin, 0);
	}
	Distance_mm = STVL5_DISTANCE_DEFAULT;
}

/*********************
begin
�����F
**********************/
int8_t inrof2016_st_vl_t::begin(void){
	return Tof->begin();
}

/*********************
����v��
�����F
**********************/
// �P���v��(�ʐM�n)
int8_t inrof2016_st_vl_t::measure(void){
	Tof->measure();	// ����l�擾�v��
	return 0;
}


/*********************
����J�n
30mA���炢�H��
�����F
**********************/
int8_t inrof2016_st_vl_t::startMeasurement(void){
	
	return 0;
}
/*********************
����I��
�����F
**********************/
int8_t inrof2016_st_vl_t::stopMeasurement(void){
	
	return 0;
}


/*********************
����l
�����F
**********************/
int16_t inrof2016_st_vl_t::getDistance(void){

	Distance_mm = Tof->getDistance();
	return Tof->getDistance();
}

