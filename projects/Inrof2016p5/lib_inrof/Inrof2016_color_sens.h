/************************************************/
/*	�l���t�H�g�j�N�X			*/
/*		�f�W�^���J���[�Z���TS11059(I2C)	*/
/*			for RX63n @ CS+		*/
/*			Wrote by conpe_		*/
/*				2016/05/26	*/
/************************************************/

// wakeup()���āAstartMeasurement()���āA����I��updateSens()����B
// getLuminance()�ő���l�擾�B

// I2C�A�h���X 0x2A(�Œ�)


//�y�X�V�����z
// 2016.05.26 �V�K�쐬


#ifndef __INROF2016COLOR_H__
#define __INROF2016COLOR_H__

#include <math.h>
#include "CommonDataType.h"
#include "HamamatsuColorSensS11059.h"
#include "inrof2016_common.h"

//#define ENC_MODE_DEFAULT 0x02

#define COLSENS_LED_PIN 	GREX_IO0	// LED�̃s���B���F�̐�
#define COLSENS_BALL_EXIST_LUM	80
#define COLSENS_BALL_JUDGE_CNT	2		// n��A���ŗL���l�Ȃ�m��

class inrof2016_color_sens_t : public hamamatsu_S11059{
public:
	// �R���X�g���N�^
	inrof2016_color_sens_t(I2c_t* I2Cn, uint8_t ArmAdrs);
	// �f�X�g���N�^
	~inrof2016_color_sens_t(void);
	// ����J�n
	void begin(void);
	
	// ����J�n
	void enableMeasure(void){fJudgeColor = true;};
	// ����I��
	void disableMeasure(void){fJudgeColor = false; offBallColorLed();};
	
	// �{�[���F���菈�� 
	// ����I�ɌĂяo������
	int8_t updateBallColor(void);
	
	// �{�[���F�擾
	inrof_color getBallColor(void){return BallColor;};
	
private:
	I2c_t* ArmI2C;
	uint8_t ArmAdrs;
	
	bool_t BallColorLed;		// LED���点�Ă邩
	
	//�{�[������
	bool_t fJudgeColor;
	inrof_color BallColor;		// ����{�[���F
	inrof_color BallColorTmp;	// �ۑ��l
	inrof_color BallColorTmpLast;	// �ۑ��l
	uint8_t JudgeCnt;		// �F�m��J�E���g
	//inrof_color BallColorFixed;	// [�m��]����{�[���F
	uint16_t BallColorVal[3][2];	// �Z���T�l�L��
	
	
	// �J���[�Z���T�pLED onoff
	int8_t onBallColorLed(void);
	int8_t offBallColorLed(void);
	bool_t isLedOn(void){return BallColorLed;};
};


#endif
