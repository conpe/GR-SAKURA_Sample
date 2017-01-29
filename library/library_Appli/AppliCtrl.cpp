/********************************************/
/*		Application/Control					*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/05/09		*/
/********************************************/



#include "AppliCtrl.h"

/*********************
PID �R���X�g���N�^
�T�v�F
 PID����̏�����
�����F

**********************/
ctrl_pid::ctrl_pid(float dt, float Kp, float Ki, float Kd, float OutMin, float OutMax, float SumMin, float SumMax){
	ctrl_pid::dt = dt;
	ctrl_pid::Kp = Kp;
	ctrl_pid::Ki = Ki;
	ctrl_pid::Kd = Kd;
	ctrl_pid::OutMin = OutMin;
	ctrl_pid::OutMax = OutMax;
	ctrl_pid::SumMin = SumMin;
	ctrl_pid::SumMax = SumMax;
	
	resetStatus();
}

/*********************
PID�v�Z
�T�v�F
 PID�̌v�Z�������
�����F

**********************/
float ctrl_pid::calc(float Err){
	float OutP, OutD, OutI;
	float Out;
	
	StSum = StSum + Err;
	if(StSum>SumMax){
		StSum = SumMax;
	}else if(Out<SumMin){
		StSum = SumMin;
	}
	
	OutP = Kp * Err;
	OutI = Ki * StSum * dt;
	OutD = Kd * (Err - StErr_1)/dt;
	
	StErr_1 = Err;
	
	Out = OutP+OutD+OutI;
	if(Out>OutMax){
		Out = OutMax;
	}else if(Out<OutMin){
		Out = OutMin;
	}
	
	return Out;
}

/*********************
PID���Z�b�g
�T�v�F
 PID�̓����p�����[�^�����Z�b�g
�����F

**********************/
void ctrl_pid::resetStatus(void){
	StErr_1 = 0;
	StSum = 0;
}
void ctrl_pid::resetStatus(float Err){

	StErr_1 = Err;
	StSum = 0;
}




/*********************
�ړ�����
�T�v�F
�����F
**********************/
ctrl_move_average::ctrl_move_average(uint16_t AveNum){
	
	this->AveNum = AveNum;
	
	Nums = new float[AveNum];
		if(NULL==Nums) __heap_chk_fail();
	if(NULL==Nums){
		this->AveNum = 0;
	}
	init = true;
	pWrite = 0;
}

ctrl_move_average::~ctrl_move_average(void){
	delete[] Nums;
}

// �����l�Z�b�g
void ctrl_move_average::setInitial(float Dat){
	for(int i=0; i<AveNum; i++){
		Nums[i] = Dat;
	}
	init = false;
}

// ���Ϗ���
float ctrl_move_average::average(float Dat){
	if(init){
		setInitial(Dat);
	}
	
	// �V�����l���Z�b�g
	if(pWrite < (AveNum-1)){
		pWrite++;
	}else{
		pWrite = 0;
	}
	Nums[pWrite] = Dat;
	
	// �S���𑫂�
	float Sum = 0;
	for(uint16_t i=0; i<AveNum; i++){
		
		Sum += Nums[i];
	}
	
	// ���������Ŋ���
	return Sum/AveNum;
}


/*********************
���[�g���~�b�^
�T�v�F
 
�����F
	Rate �㏸��[/s]
	CtrlPeriod ������� [s]
**********************/
// �R���X�g���N�^����
ctrl_ratelimiter::ctrl_ratelimiter(float CtrlPeriod, float Rate){
	this->CtrlPeriod = CtrlPeriod;
	setRate(Rate, Rate);
	this->SigLast = 0.0;
}
ctrl_ratelimiter::ctrl_ratelimiter(float CtrlPeriod, float UpRate, float DownRate){
	this->CtrlPeriod = CtrlPeriod;
	setRate(UpRate, DownRate);
	this->SigLast = 0.0;
}
ctrl_ratelimiter::ctrl_ratelimiter(float CtrlPeriod, float UpRate, float DownRate, float SigInit){
	this->CtrlPeriod = CtrlPeriod;
	setRate(UpRate, DownRate);
	this->SigLast = SigInit;
}
// �R�s�[�R���X�g���N�^
ctrl_ratelimiter::ctrl_ratelimiter(const ctrl_ratelimiter &rother){
	CtrlPeriod = rother.CtrlPeriod;
	UpRate = rother.UpRate;
	DownRate = rother.DownRate;
	SigLast = rother.SigLast;
}

void ctrl_ratelimiter::setParam(float CtrlPeriod, float Rate){
	setParam(CtrlPeriod, Rate, Rate, 0.0);
}
void ctrl_ratelimiter::setParam(float CtrlPeriod, float UpRate, float DownRate){
	setParam(CtrlPeriod, UpRate, DownRate, 0.0);
}
void ctrl_ratelimiter::setParam(float CtrlPeriod, float UpRate, float DownRate, float SigInit){
	setCtrlPeriod(CtrlPeriod);
	setRate(UpRate, DownRate);
	setValue(SigInit);
}

void ctrl_ratelimiter::setCtrlPeriod(float CtrlPeriod){
	this->CtrlPeriod = CtrlPeriod;
}
void ctrl_ratelimiter::setRate(float Rate){
	setRate(Rate, Rate);
}

void ctrl_ratelimiter::setRate(float UpRate, float DownRate){
	this->UpRate = UpRate * CtrlPeriod;
	this->DownRate = DownRate * CtrlPeriod;
}

void ctrl_ratelimiter::setValue(float Value){
	this->SigLast = Value;
};

void ctrl_ratelimiter::limitRate(float* Sig){
	*Sig = limitRate(*Sig);
}

float ctrl_ratelimiter::limitRate(float Sig){
	
	if(SigLast < Sig){
		if(Sig - SigLast > UpRate){
			Sig = SigLast + UpRate;
		}
	}else{
		if(SigLast - Sig > DownRate){
			Sig = SigLast - DownRate;
		}
	}
	
	SigLast = Sig;
	return Sig;	
}



/*********************
���[�g���~�b�^(��Βl)
�T�v�F
 [dif/cycle]
 �_�E�����[�g�������Ŏw�肷��B
 ���̒l�̎��̃_�E�����[�g�́A�������ɑ΂��Č���
�����F

**********************/
ctrl_ratelimiter_abs::ctrl_ratelimiter_abs(const ctrl_ratelimiter_abs &rother){
	CtrlPeriod = rother.CtrlPeriod;
	UpRate = rother.UpRate;
	DownRate = rother.DownRate;
	SigLast = rother.SigLast;
}

void ctrl_ratelimiter_abs::limitRate(float* Sig){
	*Sig = limitRate(*Sig);
}
float ctrl_ratelimiter_abs::limitRate(float Sig){
	float SigAbs;
	float SigLastAbs;
	float fSigned;
	float fSignedLast;
	if(SigLast<0){
		SigLastAbs = -SigLast;
		fSignedLast = true;
	}else{
		SigLastAbs = SigLast;
		fSignedLast = false;
	}
	if(Sig<0){
		SigAbs = -Sig;
		fSigned = true;
	}else{
		SigAbs = Sig;
		fSigned = false;
	}
	/*
	// �O�񕄍��ƍ��񕄍�������
	if((SigLastAbs < SigAbs)||(fSigned!=fSignedLast)){	// ��Βl���������Ȃ��� �������� ���������]
		if(SigLastAbs - SigAbs > DownRate){
			Sig = SigLast - DownRate;
		}
	}else{
		if(SigAbs - SigLastAbs > UpRate){
			Sig = SigLast + UpRate;
		}
	}
	*/
	if(fSigned==fSignedLast){	//�O��ƕ�������
		if((SigAbs-SigLastAbs)>UpRate){
			Sig = SigLastAbs+UpRate;
			if(fSigned){
				Sig = -Sig;
			}
		}else if((SigAbs-SigLastAbs)<(-DownRate)){
			Sig = SigLastAbs-DownRate;
			if(fSigned){
				Sig = -Sig;
			}
		}else{
			//this->Sig = Sig;
		}
	}else{
		if((Sig-SigLast)>UpRate){
			Sig = SigLast+UpRate;
		}else if((Sig-SigLast)<(-DownRate)){
			Sig = SigLast-DownRate;
		}else{
			//this->Sig = Sig;
		}
	}
	
	
	this->SigLast = Sig;
	return Sig;	
}
