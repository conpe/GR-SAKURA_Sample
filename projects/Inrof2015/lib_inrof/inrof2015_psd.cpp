
#include "inrof2015_psd.h"




/****************************
 �m�\���{�R��2015
  PSD�Z���T
�T�v�F
 PSD�Z���T���p������, �Z���T�ʒu�Ƃ��̏��ǉ������B
****************************/

/*********************
�R���X�g���N�^
�����F
	I2C�C���X�^���X
	I2C�A�h���X
**********************/
inrof2015_psd::inrof2015_psd(I2c_t* I2Cn, uint8_t I2cAddress, float PosX, float PosY, float PosTh) : sharp_GP2Y0E(I2Cn, I2cAddress)
{
	PosX_ = PosX;
	PosY_ = PosY;
	PosTh_ = PosTh;
}
/*********************
�}�V���ʒu���Z�b�g
�����F
	�}�V���̈ʒu
**********************/
void inrof2015_psd::setMachinePos(position* MachinePos){
	MachinePosX_ = MachinePos->X;
	MachinePosY_ = MachinePos->Y;
	MachinePosTh_ = MachinePos->Th;
}

/*********************
�Z���T�l�������_���O���[�o�����W�n�Ŏ擾
�����F
	�}�V���̈ʒu
	(�Ԃ�)*X
	(�Ԃ�)*Y
**********************/
int8_t inrof2015_psd::getGlobalObjPos(float* PosXw, float* PosYw){
	int16_t Dist = getDistance();
	
	float cosMTh = cosf(MachinePosTh_);
	float sinMTh = sinf(MachinePosTh_);
	
	if(Dist>=0){	// ���͖����l
		*PosXw = MachinePosX_ + (PosX_*cosMTh - PosY_*sinMTh) + Dist * cosf(PosTh_ + MachinePosTh_);
		*PosYw = MachinePosY_ + (PosX_*sinMTh + PosY_*cosMTh) + Dist * sinf(PosTh_ + MachinePosTh_);
	}else{
		*PosXw = 0;
		*PosYw = 0;
		
		return -1;
	}
	return 0;
}

/*********************
�Z���T�ʒu���O���[�o�����W�n�Ŏ擾
�����F
	�}�V���̈ʒu
	(�Ԃ�)*X
	(�Ԃ�)*Y
**********************/
void inrof2015_psd::getGlobalPos(float* PosXw, float* PosYw){
	float cosMTh = cosf(MachinePosTh_);
	float sinMTh = sinf(MachinePosTh_);
	
	*PosXw = MachinePosX_ + (PosX_*cosMTh - PosY_*sinMTh);
	*PosYw = MachinePosY_ + (PosX_*sinMTh + PosY_*cosMTh);
	
}

/*********************
���Ă镨���{�[���G���A�������肷��
�����F
	�Ȃ�
�ߒl�F
	�G���A���Ȃ�true
**********************/
bool_t inrof2015_psd::isObjectBallArea(void){
	float x, y;
	if(!getGlobalObjPos(&x, &y)){	// �l���Ă�		
		if( (x<=FIELD_BALLFIELD_EASTLIMIT)		// ���̒[��荶
			&& (x>=FIELD_BALLFIELD_WESTLIMIT)	// ���̒[���E
			&& (y<=FIELD_BALLFIELD_NORTHLIMIT)	// �k�̒[��艺
			&& (y>=FIELD_BALLFIELD_SOUTHLIMIT)	// ��̒[����
		){
			return true;
		}
		
	}
	
	return false;
}




