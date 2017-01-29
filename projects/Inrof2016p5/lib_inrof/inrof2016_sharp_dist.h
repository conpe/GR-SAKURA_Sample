/**************************************************
inrof2016_lib.h
	�m�\���{�R��2015�p�̃��C�u���������B
	
	
	
	
**************************************************/



// getDistance();�ő��苗���擾




#ifndef __inrof2016_SHARPDIST_H__
#define __inrof2016_SHARPDIST_H__


#include "inrof2016_ball_sensor.h"

// AD�ϊ�10bit�ɂ�����A����[mm]�ւ̐ؕЂƌX��
// 10cm��2V(620.6ad), 50cm��0.6V(186.2ad)
// Dist = SLOPE * AD10 + INTERCEPT
#define SHPDIST_AD10_DIST_SLOPE		-0.92F
#define SHPDIST_AD10_DIST_INTERCEPT	670.95F

// �����l(�����A�����l���ɂ��̒l�ɂȂ�)
#define SHGP2AD_DISTANCE_DEFAULT	-1
// ����ȏ�͖����l
#define SHGP2AD_FAIL_DISTANCE_MM	500


/****************************
 �m�\���{�R��2016
  Sharp�̑����Z���T
�T�v�F
 �Z���T�ʒu�Ƃ��̏��ǉ������B
****************************/
class inrof2016_sharp_dist_t : public inrof2016_ball_sensor_t{
public: 
	inrof2016_sharp_dist_t(grex_ad_ch AdCh, pins EnablePin);
	~inrof2016_sharp_dist_t(void){disableModule();};
	
	int8_t begin(void);
	
	// �P���v��(�ʐM�n)
	virtual int8_t measure(void);
	// �v���J�n(�d���H����
	virtual int8_t enableModule(void);
	// �v����~
	virtual int8_t disableModule(void);
	
	// �v�����ʎ擾
	virtual int8_t getDistance(int16_t* Dist){*Dist = getDistance(); return 0;};
	virtual int16_t getDistance(void){ measure(); return Distance_mm;};
	
private:
	grex_ad_ch AdCh;
	pins EnablePin;

};



#endif
