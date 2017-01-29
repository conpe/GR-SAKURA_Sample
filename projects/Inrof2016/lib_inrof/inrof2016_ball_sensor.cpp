
#include "inrof2016_ball_sensor.h"




/****************************
 �m�\���{�R��2016
  �{�[���Z���T
�T�v�F
****************************/

/*********************
�Z���T�ʒu�o�^
�����F
**********************/
void inrof2016_ball_sensor_t::setSensPos(float PosX, float PosY, float PosTh)
{
	Pos.X = PosX;
	Pos.Y = PosY;
	Pos.Th = PosTh;
}
void inrof2016_ball_sensor_t::setSensPos(position* Pos)
{
	this->Pos = *Pos;
}



/*********************
(���̃��\�b�h�v��Ȃ��Ȃ�)
�}�V���ʒu���Z�b�g
�����F
	�}�V���̈ʒu
**********************/
/*
void inrof2016_ball_sensor_t::setMachinePos(position* MachinePos){
	this->MachinePos = *MachinePos;
}
*/
/*********************
�Z���T�l�������_���O���[�o�����W�n�Ŏ擾
�����F
	�}�V���̈ʒu
	(�Ԃ�)*X
	(�Ԃ�)*Y
**********************/
int8_t inrof2016_ball_sensor_t::getGlobalObjPos(position* MachinePos, float* PosXw, float* PosYw){
	int16_t Dist = getDistance();
	
	this->MachinePos = *MachinePos;	// �Ƃ肠����
	
	float cosMTh = cosf(MachinePos->Th);
	float sinMTh = sinf(MachinePos->Th);
	
	if(Dist>=0){
		*PosXw = MachinePos->X + (Pos.X*cosMTh - Pos.Y*sinMTh) + Dist * cosf(Pos.Th + MachinePos->Th);
		*PosYw = MachinePos->Y + (Pos.X*sinMTh + Pos.Y*cosMTh) + Dist * sinf(Pos.Th + MachinePos->Th);
	}else{	// ���͖����l
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
void inrof2016_ball_sensor_t::getGlobalPos(position* MachinePos, float* PosXw, float* PosYw){
	float cosMTh = cosf(MachinePos->Th);
	float sinMTh = sinf(MachinePos->Th);
	
	this->MachinePos = *MachinePos;	// �Ƃ肠����
	
	*PosXw = MachinePos->X + (Pos.X*cosMTh - Pos.Y*sinMTh);
	*PosYw = MachinePos->Y + (Pos.X*sinMTh + Pos.Y*cosMTh);
	
}

/*********************
���Ă镨���{�[���G���A�������肷��
�����F
	�Ȃ�
�ߒl�F
	�G���A���Ȃ�true
**********************/
bool_t inrof2016_ball_sensor_t::isObjectBallArea(position* MachinePos){
	float x, y;
	if(!getGlobalObjPos(MachinePos, &x, &y)){	// �l���Ă�		
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




