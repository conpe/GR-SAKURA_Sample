/********************************************/
/*		Application/Control					*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/05/09		*/
/********************************************/


//�y�X�V�����z
// 2015.05.09 �V�K�쐬


#ifndef __APPLI_CTRL_H__
#define __APPLI_CTRL_H__

#include "CommonDataType.h"
#include <new>

/*********************
PID����
�T�v�F
�����F
**********************/
class ctrl_pid
{
public:
	ctrl_pid(float dt, float Kp, float Ki, float Kd, float OutMin, float OutMax, float SumMin, float SumMax);
	float calc(float Err);
	
	void setParam(float Kp, float Ki, float Kd){this->Kp = Kp;this->Ki = Ki;this->Kd = Kd;};
	void resetStatus(void);
	void resetStatus(float Err);

private:
	float dt;			//
	float Kp, Kd, Ki;	//
	// �X�e�[�^�X
	float StErr_1;		// �O��덷
	float StSum;		// �ϕ��l (St:state)
	
	float OutMin, OutMax;	// �o�͍ŏ�, �ő�l
	float SumMin, SumMax;	// �ϕ��ő�l
	
};

/*********************
�ړ�����
�T�v�F
�����F
**********************/
class ctrl_move_average{
public:
	ctrl_move_average(uint16_t AveNum);
	virtual ~ctrl_move_average(void);
	
	void setInitial(float Dat);	// �����l�Z�b�g
	float average(float Dat);
private:
	bool_t init;
	float* Nums;	// �l�o���Ă���
	uint16_t AveNum;	// ���ς��鐔
	uint16_t pWrite;	// �����ꏊ
};


/*********************
���[�g���~�b�^
�T�v�F
 [dif/cycle]
 �_�E�����[�g�������Ŏw�肷�� 
�����F

**********************/
class ctrl_ratelimiter
{
public:
	ctrl_ratelimiter(){};	// �f�t�H���g�R���X�g���N�^
	ctrl_ratelimiter(float CtrlPeriod, float Rate);					// �㉺�ꏏ
	ctrl_ratelimiter(float CtrlPeriod, float UpRate, float DownRate);	//
	ctrl_ratelimiter(float CtrlPeriod, float UpRate, float DownRate, float SigInit);	//	
	ctrl_ratelimiter(const ctrl_ratelimiter& rother);	// �R�s�[�R���X�g���N�^	
	virtual ~ctrl_ratelimiter(void){};					// �f�X�g���N�^
	
	void setCtrlPeriod(float CtrlPeriod);
	void setParam(float CtrlPeriod, float Rate);
	void setParam(float CtrlPeriod, float UpRate, float DownRate);
	void setParam(float CtrlPeriod, float UpRate, float DownRate, float SigInit);
	void setRate(float Rate);
	void setRate(float UpRate, float DownRate);
	void setValue(float Value);
	
	// ���[�g���~�b�g����
	virtual float limitRate(float Sig);	// �l�Ԃ�
	virtual void limitRate(float* Sig);	// �Q�ƕԂ�
protected:
	float CtrlPeriod;
	float UpRate;	// 1�X�e�b�v������
	float DownRate;	// �����Ŏw��
	float SigLast;
};


/*********************
���[�g���~�b�^(��Βl)
�T�v�F
 [dif/cycle]
 �_�E�����[�g�������Ŏw�肷��B
 ���̒l�̎��̃_�E�����[�g�́A�������ɑ΂��Č���
�����F

**********************/
class ctrl_ratelimiter_abs : public ctrl_ratelimiter
{
public:
	ctrl_ratelimiter_abs(float CtrlPeriod, float Rate) : ctrl_ratelimiter(CtrlPeriod, Rate){};					// �㉺�ꏏ
	ctrl_ratelimiter_abs(float CtrlPeriod, float UpRate, float DownRate) : ctrl_ratelimiter(CtrlPeriod, UpRate, DownRate, 0.0){};	//
	ctrl_ratelimiter_abs(float CtrlPeriod, float UpRate, float DownRate, float SigInit) : ctrl_ratelimiter(CtrlPeriod, UpRate, DownRate, SigInit){};	//	
	ctrl_ratelimiter_abs(){};	// �f�t�H���g�R���X�g���N�^
	ctrl_ratelimiter_abs(const ctrl_ratelimiter_abs& rother);	// �R�s�[�R���X�g���N�^
	
	float limitRate(float Sig);
	void limitRate(float* Sig);
};


#endif
