
#include "inrof2016_sharp_dist.h"




/****************************
 �m�\���{�R��2016
  �{�[���Z���T sharp�����Z���T(AD��)
�T�v�F
****************************/
inrof2016_sharp_dist_t::inrof2016_sharp_dist_t(grex_ad_ch AdCh, pins EnablePin){
	this->AdCh = AdCh;
	this->EnablePin = EnablePin;
	
	Distance_mm = SHGP2AD_DISTANCE_DEFAULT;
}



/*********************
begin
�����F
**********************/
int8_t inrof2016_sharp_dist_t::begin(void){
	GrEx.startAD();
	enableModule();	// ����J�n
	
	return 0;
}

/*********************
����v��
�����F
**********************/
int8_t inrof2016_sharp_dist_t::measure(void){
	Distance_mm = SHPDIST_AD10_DIST_SLOPE * GrEx.getAD10((grex_ad_ch)AdCh) + SHPDIST_AD10_DIST_INTERCEPT;
			//10bit�Ŏ擾
	// �ő�l����
	if(SHGP2AD_FAIL_DISTANCE_MM < Distance_mm){
		Distance_mm = SHGP2AD_DISTANCE_DEFAULT;
	}
	
	return 0;
}

/*********************
����J�n
30mA���炢�H��
�����F
**********************/
int8_t inrof2016_sharp_dist_t::enableModule(void){
	outPin(EnablePin, 1);
	return 0;
}
/*********************
����I��
�����F
**********************/
int8_t inrof2016_sharp_dist_t::disableModule(void){
	outPin(EnablePin, 0);
	return 0;
}





