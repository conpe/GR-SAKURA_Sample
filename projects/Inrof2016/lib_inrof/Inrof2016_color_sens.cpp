/************************************************/
/*	�l���t�H�g�j�N�X			*/
/*		�f�W�^���J���[�Z���TS11059(I2C)	*/
/*			for RX63n @ CS+		*/
/*			Wrote by conpe_		*/
/*				2016/05/26	*/
/************************************************/

#include "Inrof2016_color_sens.h"




/*********************
�J���[�Z���T������
�R���X�g���N�^
I2C������
�����F	I2C
**********************/
inrof2016_color_sens_t::inrof2016_color_sens_t(I2c_t* I2Cn, uint8_t ArmAdrs):hamamatsu_S11059(I2Cn){
	this->ArmAdrs = ArmAdrs;
	this->ArmI2C = I2Cn;
}
/*********************
�J���[�Z���T������
�f�X�g���N�^
**********************/
inrof2016_color_sens_t::~inrof2016_color_sens_t(void){
}


/*********************
�J���[�Z���T����J�n
**********************/
void inrof2016_color_sens_t::begin(void){
	hamamatsu_S11059::begin();
	I2Cn->begin(400, 32);
	
	offBallColorLed();
	fJudgeColor = false;
}



/*********************
�{�[���̐F�X�V
�T�v�F
 �F�Z���T�̊e�F�̋����F�Ƃ���B
 �����ȂƂ���INROF_UNKNOWN�Ƃ���B
 �Z���T�X�V�Ɠ������ČĂ�łˁB(LED���点��^�C�~���O�̓s��)
**********************/
// debug
#include "SCI.h"
int8_t inrof2016_color_sens_t::updateBallColor(void){
	int16_t BallColorValDiff[3];
	int8_t ret;
	
	if(fJudgeColor){
		
		if(BallColorLed){
			getLuminance(&BallColorVal[0][1], &BallColorVal[1][1], &BallColorVal[2][1]);
		}else{
			getLuminance(&BallColorVal[0][0], &BallColorVal[1][0], &BallColorVal[2][0]);
		}
		// ���邢���ƈÂ����̍����Ƃ�
		BallColorValDiff[0] = abs(BallColorVal[0][1] - BallColorVal[0][0]);
		BallColorValDiff[1] = abs(BallColorVal[1][1] - BallColorVal[1][0]);
		BallColorValDiff[2] = abs(BallColorVal[2][1] - BallColorVal[2][0]);
		
		// �F����
		if((BallColorValDiff[0]>COLSENS_BALL_EXIST_LUM)||(BallColorValDiff[1]>COLSENS_BALL_EXIST_LUM)||(BallColorValDiff[2]>COLSENS_BALL_EXIST_LUM)){	// ���ˌ�����
			
			/*
			if(BallColorValDiff[0] > BallColorValDiff[1]){
				if(BallColorValDiff[0] > BallColorValDiff[2]){
					BallColor = INROF_RED;	// Red
				}else{
					BallColor = INROF_BLUE;	// Blue
				}
			}else if(BallColorValDiff[1] > BallColorValDiff[2]){
				if(BallColorValDiff[1] > BallColorValDiff[0]){
					BallColor = INROF_YELLOW;	// Green
				}else{
					BallColor = INROF_RED;	// Red
				}
			}else{
				if(BallColorValDiff[2] > BallColorValDiff[1]){
					BallColor = INROF_BLUE;	// Blue
				}else{
					BallColor = INROF_YELLOW;	// Green
				}
			}
			*/
			float SensRed;
			float SensGreen;
			float SensBlue;
			uint16_t RgbMax;
			uint16_t RgbMin;
			color MaxCol;
			float H;
			
			SensRed = (float)BallColorValDiff[0];
			SensGreen = (float)BallColorValDiff[1];
			SensBlue = (float)BallColorValDiff[2];
			
			if(SensRed > SensGreen){
				RgbMax = (uint16_t)SensRed;
				MaxCol = red;
			}else{
				RgbMax = (uint16_t)SensGreen;
				MaxCol = green;
			}
			if(RgbMax < SensBlue){
				RgbMax = (uint16_t)SensBlue;
				MaxCol = blue;
			}
			
			if(SensRed < SensGreen){
				RgbMin = (uint16_t)SensRed;
			}else{
				RgbMin = (uint16_t)SensGreen;
			}
			if(RgbMin > SensBlue){
				RgbMin = (uint16_t)SensBlue;
			}
			
			if(RgbMax != RgbMin){
				switch(MaxCol){
				case red:
					H = (60.0f * (SensBlue-SensGreen))/(float)(RgbMax-RgbMin);
					break;
				case green:
					H = 120.0f + (60.0f*(SensRed-SensBlue))/(float)(RgbMax-RgbMin);
					break;
				case blue:
					H = 240.0f + (60.0f*(SensGreen-SensRed))/(float)(RgbMax-RgbMin);
					break;
				default:
					H = 0;
					break;
				}
				
				while(H<0){
					H = H+360.0F;
				}
				
				//if(H<60 || H>300){
				if(H<80.0F || H>340.0F){
					BallColor = INROF_RED;
				//}else if(H>=60 && H<180){
				}else if(H<180.0F){
					BallColor = INROF_YELLOW;
				}else{
					BallColor = INROF_BLUE;
				}
				
			}else{
				H = 0;
				BallColor = INROF_NONE;
			}
			
			
		}else{
			BallColor = INROF_NONE;
		}
		
		if(BallColorLed){
			ret = offBallColorLed();
		}else{
			ret = onBallColorLed();
		}
		
		
		ret = updateSens();	// �J���[�Z���T�l�擾�v��
		
	}else{
		
		BallColor = INROF_NONE;
		ret = offBallColorLed();
	}
	
	return ret;
}

/*********************
�J���[����LED on
�T�v�F
�@I2C�ŃA�[����ɓn��
**********************/
int8_t inrof2016_color_sens_t::onBallColorLed(void){
	//uint8_t TxData;
	
	//TxData = 0x01;
	BallColorLed = true;
	
	//return ArmI2C->attach(ArmAdrs, &TxData, 1);
	
	outPin(COLSENS_LED_PIN, 1);
	
	return 0;
}

/*********************
�J���[����LED off
�T�v�F
�@I2C�œn��
**********************/
int8_t inrof2016_color_sens_t::offBallColorLed(void){
	//uint8_t TxData;
	
	//TxData = 0x00;
	BallColorLed = false;
	
	//return ArmI2C->attach(ArmAdrs, &TxData, 1);
	outPin(COLSENS_LED_PIN, 0);
	
	return 0;
}

