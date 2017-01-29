
#include "GR_Ex15_v3.h"



/****************************
�g�����Ex15 ���C��
****************************/
gr_ex::gr_ex(void){
	
	
}

gr_ex::~gr_ex(void){
	if(Sp) delete Sp;
	if(Motor0) delete Motor0;
	if(Motor1) delete Motor1;
	if(Motor2) delete Motor2;
	if(Motor3) delete Motor3;
	if(Enc0) delete Enc0;
	if(Enc1) delete Enc1;
}


void gr_ex::begin(void){
	
	
	// GPIO������
	setPinMode((pins)GREX_IO0, PIN_OUTPUT);
	setPinMode((pins)GREX_IO1, PIN_OUTPUT);
	setPinMode((pins)GREX_IO2, PIN_OUTPUT);
	setPinMode((pins)GREX_IO3, PIN_OUTPUT);
	setPinMode((pins)GREX_IO4, PIN_OUTPUT);
	setPinMode((pins)GREX_IO5, PIN_OUTPUT);
	setPinMode((pins)GREX_IO6, PIN_OUTPUT);
	setPinMode((pins)GREX_IO7, PIN_OUTPUT);
	
	
	// �X�s�[�J������
	Sp = new gr_ex_sp();
	
	// ���[�^������
	Motor0 = new gr_ex_motor(MOTOR0);
	Motor1 = new gr_ex_motor(MOTOR1);
	Motor2 = new gr_ex_motor(MOTOR2);
	Motor3 = new gr_ex_motor(MOTOR3);
	
	// ���[�^���G���R�[�_������
	Enc0 = new gr_ex_encoder(ENC0);
	Enc1 = new gr_ex_encoder(ENC1);
	
	if(NULL==Sp) __heap_chk_fail();
	if(NULL==Motor0) __heap_chk_fail();
	if(NULL==Motor1) __heap_chk_fail();
	if(NULL==Motor2) __heap_chk_fail();
	if(NULL==Motor3) __heap_chk_fail();
	if(NULL==Enc0) __heap_chk_fail();
	if(NULL==Enc1) __heap_chk_fail();
	
	// �V���A���T�[�{������
	
	
	
	// CAN������
	
	// �V���A��������
	
	// I2C������ or MPU9250������
	
	
}


/*********************
 AD�ϊ��|�[�g������
�T�v�F
 AD�ϊ��p�|�[�g(IO8-15)�����ׂ�AD�ϊ��p�ɂ��āA�A���X�L�����J�n����
 ���Z���[�h�ɂ���4�񑫂����l��S12AD.ADDRx�ɓ���B(���l�߃��[�h�B4�񑫂��Ă�̂ŁA��14�r�b�g���g��)
�����F

**********************/
void gr_ex::startAD(void){
	SYSTEM.MSTPCRA.LONG &= ~(1 << 17);

	PORT4.PDR.BYTE = 0; // �S�|�[�g����͂ɂ���

	S12AD.ADCSR.BYTE = 0x4c;	// �T���v�����O�N���b�N��PCLK(48MHz)��
								// �A���X�L�������[�h
	
	// �ϊ��Ώ�
	S12AD.ADANS0.BIT.ANS0 = 0xff;	// CH0-15 : on  // 0�`7�܂ł����Ȃ��悤�ȋC�����邯�ǁH
	S12AD.ADANS1.BIT.ANS1 = 0x00;	// CH16-20 : off
	
	// ���Z����
	S12AD.ADADS0.BIT.ADS0 = 0xff;	//�@CH0-15 : on		// ���Z���[�h�ɂ����ꍇ�A��14�r�b�g�����ʂɂȂ�B
	S12AD.ADADS1.BIT.ADS1 = 0x00;	//  CH16-20 : off
	
	// ���Z��
	S12AD.ADADC.BIT.ADC = 0x03;	// 4����Z
	
	
	// AD�ϊ��J�n
	S12AD.ADCSR.BIT.ADST = 1; // AD�ϊ��J�n
}

// 8�r�b�g�ŕԂ�
uint16_t gr_ex::getAD8(grex_ad_ch AdCh){
	
	unsigned short *result = (unsigned short *)&S12AD.ADDR0;
	
	return (*(result+(int)AdCh))>>8;	// ���Z���[�h�Ȃ̂ō�8�r�b�g����Ԃ�
}
// 10�r�b�g�ŕԂ�
uint16_t gr_ex::getAD10(grex_ad_ch AdCh){
	
	unsigned short *result = (unsigned short *)&S12AD.ADDR0;
	
	return (*(result+(int)AdCh))>>6;	// ���Z���[�h�Ȃ̂ō�10�r�b�g����Ԃ�
}
// 12�r�b�g�ŕԂ�
uint16_t gr_ex::getAD12(grex_ad_ch AdCh){
	
	unsigned short *result = (unsigned short *)&S12AD.ADDR0;
	
	return (*(result+(int)AdCh))>>4;	// ���Z���[�h�Ȃ̂ō�10�r�b�g����Ԃ�
}
// 12�r�b�g�ŕԂ�
uint16_t gr_ex::getAD(grex_ad_ch AdCh){
	
	unsigned short *result = (unsigned short *)&S12AD.ADDR0;
	
	return (*(result+(int)AdCh))>>4;	// ���Z���[�h�Ȃ̂ō�12�r�b�g����Ԃ�
}

gr_ex GrEx;



/****************************
�X�s�[�J
gr_ex_sp
	gr_ex.Sp

�T�v�F
 PWM�ŉ����Đ�
 �o�b�t�@�ɗp�ӂ����g�`�f�[�^�������o�͂���
	
�g�p���W���[���F
	MTU4	:	PWM���� ����255(187.5kHz)
	TMR3	:	�T���v�����O���g�����荞�ݐ��� 

****************************/
gr_ex_sp::gr_ex_sp(void){
	SpBuff = new RingBuffer<uint8_t>(EXSP_BUFFSIZE);	// �o�b�t�@�T�C�Y�ǂ������؁B�Ƃ肠�����f�t�H�B
	if(NULL==SpBuff) __heap_chk_fail();
	setupRegisterPwm();
}

gr_ex_sp::~gr_ex_sp(void){
	delete SpBuff;
}

// ���W�X�^�̏�����
//  MTU4��PWM���[�h1�ŁBTGRD���o�b�t�@�ɂ���MTIOC4A�[�q(PE2)����o��
void gr_ex_sp::setupRegisterPwm(void){
	
	// PWM�̐ݒ�
	// PWM���[�h1
	
	// �������݋���
	SYSTEM.PRCR.WORD = 0xA503u;
	MTU.TRWER.BIT.RWE = 1;	// MTU�֌W���W�X�^�ύX����
	
	MSTP(MTU4) = 0;	// ���W���[���N��
	
	
	MTU.TSTR.BIT.CST4 = 0;
	
	MTU4.TCR.BIT.TPSC = 0;	// PCLK/1 -> 48MHz
	//MTU4.TCR.BIT.TPSC = 5;	// PCLK/1024 -> 46.875kHz -> ����/255���Ă�̂� 183Hz
	MTU4.TCR.BIT.CCLR = 1;	// TGRA�̃R���y�A�}�b�`��TCNT�N���A
	
	MTU4.TMDR.BIT.BFA = 0;	// TGRA, C�͒ʏ퓮��
	MTU4.TMDR.BIT.BFB = 1;	// TGRB, D�̓o�b�t�@����
	
	MTU4.TMDR.BIT.MD = 2;	// PWM���[�h1

	MTU4.TBTM.BIT.TTSB = 1;	// TCNT�N���A���Ƀo�b�t�@����R�s�[
	
	MTU.TOER.BIT.OE4A = 1;	// �o�͋���
	
	MTU4.TIORH.BIT.IOA = 6;	// High -> High
	MTU4.TIORH.BIT.IOB = 1;	// Low -> Low
	
	setPeriodCnt(255);	//����	->	48MHz/255 = 187.5kHz
	//MTU4.TGRA = 255; 		//����
	MTU4.TGRB = 127; 		//�f���[�e�B�[
	setDutyCnt(127); 		//�f���[�e�B�[
	//MTU4.TGRD = 127; 		//�f���[�e�B�[(�o�b�t�@ �������Ƀf���[�e�B�[�����B
	
	
	MTU.TSTR.BIT.CST4 = 1;	// �J�E���g�X�^�[�g
	
	
	// �|�[�g�o�͐ݒ�
	MPC.PWPR.BYTE = 0x00u;
	MPC.PWPR.BYTE = 0x40u;
	SYSTEM.PRCR.WORD = 0xA503u;
	setPinMode(GREX_SPOUT, PIN_OUTPUT);	// output
	MPC.PE2PFS.BIT.PSEL = 1;			// MTIOC4A
	PORTE.PMR.BIT.B2 = 1;				// not GPIO
	
	
}

// Duty���Z�b�g
//	�^�C�}�J�E���g�l�Ŏw��
void gr_ex_sp::setDutyCnt(uint16_t DutyCnt){
	
	if(MTU4.TGRA==DutyCnt){	// duty�ő�̎��̓[���̂܂܂ɂȂ��Ă��܂����Ƃ�����̂ŁA���high�ɂȂ�悤��+1����B
		MTU4.TGRD = DutyCnt+1;
	}else{
		MTU4.TGRD = DutyCnt;
	}
}

// �������Z�b�g
//	�^�C�}�J�E���g�l�Ŏw��
void gr_ex_sp::setPeriodCnt(uint16_t PeriodCnt){
	_Freq = PCLK/(PeriodCnt);	// Hz
	
	MTU4.TGRA = PeriodCnt;
}


// �Đ��p�̐ݒ�
//  SamplingFreq�ł̊��荞�݂�ݒ�
//  TMR3�g�p(�R���y�A�}�b�`A�Ŋ��荞��)
//   ->���荞�݂��Ƃ�buffer����ǂ��Duty���Z�b�g����B
void gr_ex_sp::setupPlay(uint16_t SamplingFreq){	
	uint8_t CntSetting = 2;		// 0:1/1, 1:1/2, 2:1/8, 3:32, 4:64
	uint8_t cnt = 255;
	
	
	SYSTEM.PRCR.WORD = 0xA503u;		// �������݋���
	MTU.TRWER.BIT.RWE = 1;	// MTU�֌W���W�X�^�ύX����
	MSTP(TMR3) = 0;					// ���W���[���N��
	
	TMR3.TCR.BYTE = 0x00;
	TMR3.TCR.BIT.CCLR = 1;			// �R���y�A�}�b�`A�ŃJ�E���^�N���A
	TMR3.TCR.BIT.CMIEA = 1;			// �R���y�A�}�b�`A���荞�݋���(CMIAn)
	
	
	TMR3.TCSR.BYTE = 0x00;
	// �J�E���^�����I��
	if(256 > PCLK/1/SamplingFreq){
		CntSetting = 0;
		cnt = PCLK/1/SamplingFreq;
	}else if(256 > PCLK/2/SamplingFreq){
		CntSetting = 1;
		cnt = PCLK/2/SamplingFreq;
	}else if(256 > PCLK/8/SamplingFreq){
		CntSetting = 2;
		cnt = PCLK/8/SamplingFreq;
	}else if(256 > PCLK/32/SamplingFreq){
		CntSetting = 3;
		cnt = PCLK/32/SamplingFreq;
	}else if(256 > PCLK/64/SamplingFreq){
		CntSetting = 4;
		cnt = PCLK/64/SamplingFreq;
	}else if(256 > PCLK/1024/SamplingFreq){
		CntSetting = 5;
		cnt = PCLK/1024/SamplingFreq;
	}else{
		CntSetting = 6;
		cnt = PCLK/8192/SamplingFreq;
	}
	
	
	// �J�E���^
	//uint8_t cnt = PCLK/8/SamplingFreq;
	TMR3.TCNT = 0;
	TMR3.TCORA = cnt;		//�R���y�A�}�b�`�p(�g��Ȃ�)
	TMR3.TCORB = cnt+1;		//�R���y�A�}�b�`�p(�g��Ȃ�)
	
	// ���荞�݋���
	IEN(TMR3,CMIA3) = 1;
	IPR(TMR3,CMIA3) = 6;
	
	// �N���b�N�ݒ�
	TMR3.TCCR.BYTE = 0x00;
	TMR3.TCCR.BIT.CKS = CntSetting;	// �N���b�N�I��			PCLK/8
	TMR3.TCCR.BIT.CSS = 0x01;	// �N���b�N�\�[�X�I�� �����N���b�N
	
	// ����~�߂�̂͂��ꂵ���Ȃ��H
	//TMR3.TCCR.BIT.CKS = 0x00;	// �N���b�N�I��			PCLK/8
	//TMR3.TCCR.BIT.CSS = 0x00;
	
	
}

void gr_ex_sp::stopPlay(void){	
	//TMR3.TCCR.BIT.CKS = 0x00;	// �N���b�N�Ȃ�
	//TMR3.TCCR.BIT.CSS = 0x00;
	
	// �~�߂悤���Ǝv�������ǁA�v�b�c���Ȃ�̂Ŏ~�߂Ȃ����ƂɁB
	
}

// �o�b�t�@���珇���f���[�e�B�[�ɐݒ肷��
// �o�b�t�@�Ȃ���΂��ʁB
void gr_ex_sp::isrSpPlay(void){
	/*
	// �m�R�M���g�e�X�g
	static uint8_t cnt;
	cnt++;	// 255�ň������	->	173Hz
	setDutyCnt(cnt);
	*/
	uint8_t dat;
	
	if(!SpBuff->read(&dat)){
		setDutyCnt(dat);
	}

}





/****************************
���[�^�[
gr_ex_motor
	gr_ex.Motor0 (PWM��Ή� ������)
	gr_ex.Motor1 (PWM��Ή� ������)
	gr_ex.Motor2 (PWM�Ή�)
	gr_ex.Motor3 (PWM�Ή�)

�g�p���W���[��
	TPU3, MTU3	:	PWM����
		�����F TPU3 TGRC
		�f���[�e�B:
			//motor0:	TIOCB3 TGRB (PWM���[�h2)
			//motor1:	TIOCA3 TGRA (PWM���[�h2)
		��]����, ��~����
			GPIO x�e2
			
		����: MTU3 TGRA
		�f���[�e�B:
			motor2:	MTIOC3A TGRB (PWM���[�h1)
			motor3:	MTIOC3C TGRD (PWM���[�h1)
		��]����, ��~����
			GPIO x�e2
****************************/
gr_ex_motor::gr_ex_motor(gr_ex_motor_num MotorNum){
	_Motor = MotorNum;	// ���[�^�[�Z�b�g
	
	
	setRegister();		// ���W�X�^�ݒ�
	// �J�E���^�����l
	setDir(FREE);
	setPeriodCnt(GREX_MOTOR_PWMCNT_1KHz);	//����	->	48MHz/47999 = 1kHz
	switch(MotorNum){
	case MOTOR0:	// PWM��Ή��Ȃ̂ŁA�f���[�e�B�[�̓}�b�N�X����
	case MOTOR1:
		setDutyCnt(GREX_MOTOR_PWMCNT_1KHz); 		//�f���[�e�B�[
		break;
	case MOTOR2:	// ��~��ԂŊJ�n
	case MOTOR3:
		setDutyCnt(0); 		//�f���[�e�B�[
		break;
	}
}

extern uint8_t motorset;
void gr_ex_motor::setRegister(void){
	
	
	switch(_Motor){
	case MOTOR0:
			
		// PWM�̐ݒ�
		
		// �������݋���
		
		/*
		SYSTEM.PRCR.WORD = 0xA503u;
		
		MSTP(TPU3) = 0;	// ���W���[���N��
		TPUA.TSTR.BIT.CST3 = 0;
		TPU3.TCR.BIT.TPSC = 0;	// PCLK/1 -> 48MHz
		
		TPU3.TCR.BIT.CCLR = 5;	// TGRC���W�X�^�̃R���y�A�}�b�`��TCNT�J�E���^�N���A
		TPU3.TMDR.BIT.MD = 3;	// PWM���[�h2

		TPU3.TIORH.BIT.IOB = 5;	// High -> Low
		
		TPUA.TSTR.BIT.CST3 = 1;	// �J�E���g�X�^�[�g
		
		// �|�[�g�o�͐ݒ�
		MPC.PWPR.BYTE = 0x00u;
		MPC.PWPR.BYTE = 0x40u;
		SYSTEM.PRCR.WORD = 0xA503u;
		setPinMode(GREX_PIN_MOTOR0P, PIN_OUTPUT);
		MPC.P20PFS.BIT.PSEL = 3;			// TIOCB3
		PORT2.PMR.BIT.B0 = 1;				// not GPIO
		*/
		setPinMode(GREX_PIN_MOTOR0A, PIN_OUTPUT);
		PORT0.PMR.BIT.B5 = 0;				// GPIO
		setPinMode(GREX_PIN_MOTOR0B, PIN_OUTPUT);
		PORT0.PMR.BIT.B5 = 0;				// GPIO
		
		break;
	case MOTOR1:
			
		// PWM�̐ݒ�
		
		/*
		// �������݋���
		SYSTEM.PRCR.WORD = 0xA503u;
		MSTP(TPU3) = 0;	// ���W���[���N��
		
		TPUA.TSTR.BIT.CST3 = 0;
		TPU3.TCR.BIT.TPSC = 0;	// PCLK/1 -> 48MHz
		
		TPU3.TCR.BIT.CCLR = 5;	// TGRC���W�X�^�̃R���y�A�}�b�`��TCNT�J�E���^�N���A
		TPU3.TMDR.BIT.MD = 3;	// PWM���[�h2

		TPU3.TIORH.BIT.IOA = 5;	// High -> Low
		
		TPUA.TSTR.BIT.CST3 = 1;	// �J�E���g�X�^�[�g
		
		// �|�[�g�o�͐ݒ�
		MPC.PWPR.BYTE = 0x00u;
		MPC.PWPR.BYTE = 0x40u;
		SYSTEM.PRCR.WORD = 0xA503u;
		setPinMode(GREX_PIN_MOTOR1P, PIN_OUTPUT);
		MPC.P21PFS.BIT.PSEL = 3;			// TIOCA3
		PORT2.PMR.BIT.B1 = 1;				// not GPIO
		*/
		setPinMode(GREX_PIN_MOTOR1A, PIN_OUTPUT);
		PORT1.PMR.BIT.B7 = 0;				// GPIO
		setPinMode(GREX_PIN_MOTOR1B, PIN_OUTPUT);
		PORT5.PMR.BIT.B1 = 0;				// GPIO
		
		break;
	case MOTOR2:
		
		// �������݋���
		SYSTEM.PRCR.WORD = 0xA503u;
		MSTP(MTU3) = 0;	// ���W���[���N��
		MTU.TRWER.BIT.RWE = 1;	// MTU�֌W���W�X�^�ύX����
		
		MTU.TSTR.BIT.CST3 = 0;
		MTU3.TCR.BIT.TPSC = 0;	// PCLK/1 -> 48MHz
		
		MTU3.TCR.BIT.CCLR = 1;	// TGRA�̃R���y�A�}�b�`�ŃN���A
		MTU3.TMDR.BIT.MD = 2;	// PWM���[�h1
		
		MTU3.TMDR.BIT.BFA = 0;	// ��o�b�t�@���[�h

		MTU3.TIORH.BIT.IOA = 6;	// High -> High
		MTU3.TIORH.BIT.IOB = 5;	// High -> Low
		
		MTU.TSTR.BIT.CST3 = 1;	// �J�E���g�X�^�[�g
		
		// �|�[�g�o�͐ݒ�
		MPC.PWPR.BYTE = 0x00u;
		MPC.PWPR.BYTE = 0x40u;
		SYSTEM.PRCR.WORD = 0xA503u;
		setPinMode(GREX_PIN_MOTOR2P, PIN_OUTPUT);
		MPC.PC1PFS.BIT.PSEL = 1;			// MTIOC3A
		PORTC.PMR.BIT.B1 = 1;				// not GPIO
		setPinMode(GREX_PIN_MOTOR2A, PIN_OUTPUT);
		PORT5.PMR.BIT.B4 = 0;				// GPIO
		setPinMode(GREX_PIN_MOTOR2B, PIN_OUTPUT);
		PORT5.PMR.BIT.B5 = 0;				// GPIO
		
		break;
	case MOTOR3:
		// �������݋���
		SYSTEM.PRCR.WORD = 0xA503u;
		MSTP(MTU3) = 0;	// ���W���[���N��
		MTU.TRWER.BIT.RWE = 1;	// MTU�֌W���W�X�^�ύX����
		
		MTU.TSTR.BIT.CST3 = 0;
		MTU3.TCR.BIT.TPSC = 0;	// PCLK/1 -> 48MHz
		
		MTU3.TCR.BIT.CCLR = 1;	// TGRA�̃R���y�A�}�b�`�ŃN���A
		MTU3.TMDR.BIT.MD = 2;	// PWM���[�h1
		
		MTU3.TMDR.BIT.BFB = 0;	// ��o�b�t�@���[�h
	
		MTU3.TIORL.BIT.IOC = 6;	// High -> High
		MTU3.TIORL.BIT.IOD = 5;	// High -> Low
		
		MTU.TSTR.BIT.CST3 = 1;	// �J�E���g�X�^�[�g
		
		// �|�[�g�o�͐ݒ�
		MPC.PWPR.BYTE = 0x00u;
		MPC.PWPR.BYTE = 0x40u;
		SYSTEM.PRCR.WORD = 0xA503u;
		setPinMode(GREX_PIN_MOTOR3P, PIN_OUTPUT);
		//PORTJ.ODR0.BIT.B6 = 0;	// �I�[�v�h���C��off
		MPC.PJ3PFS.BIT.PSEL = 1;			// MTIOC3C
		PORTJ.PMR.BIT.B3 = 1;				// not GPIO
		setPinMode(GREX_PIN_MOTOR3A, PIN_OUTPUT);
		PORTC.PMR.BIT.B4 = 0;				// GPIO
		setPinMode(GREX_PIN_MOTOR3B, PIN_OUTPUT);
		PORTE.PMR.BIT.B0 = 0;				// GPIO
		
		break;
	
	}
	
}

void gr_ex_motor::setPeriodCnt(uint16_t PeriodCnt){
	
	PeriodCnt_ = PeriodCnt;
	
	switch(_Motor){
	case MOTOR0:
	case MOTOR1:
	//	TPU3.TGRC = PeriodCnt;
		break;
	case MOTOR2:
	case MOTOR3:
		MTU3.TGRA = PeriodCnt;
		MTU3.TGRC = PeriodCnt;
		break;
	}
}

void gr_ex_motor::setDutyCnt(uint16_t DutyCnt){
	
	switch(_Motor){
	case MOTOR0:
	case MOTOR1:
	//	if(TPU3.TGRC==DutyCnt){	// duty�ő�̎��̓[���̂܂܂ɂȂ��Ă��܂����Ƃ�����̂ŁA���high�ɂȂ�悤��+1����B
	//		DutyCnt = TPU3.TGRC+1;
	//	}
		if(DutyCnt>0){
			DutyCnt = GREX_MOTOR_PWMCNT_1KHz;
			setDir(_Dir);
		}else{
			DutyCnt = 0;
			setDir(FREE);
		}
		break;
	case MOTOR2:
	case MOTOR3:
		if(MTU3.TGRA==DutyCnt){	// duty�ő�̎��̓[���̂܂܂ɂȂ��Ă��܂����Ƃ�����̂ŁA���high�ɂȂ�悤��+1����B
			DutyCnt = MTU3.TGRA+1;
		}
		break;
	}
	
	switch(_Motor){
	case MOTOR0:
	//	TPU3.TGRB = DutyCnt;
		break;
	case MOTOR1:
	//	TPU3.TGRA = DutyCnt;
		break;
	case MOTOR2:
		MTU3.TGRB = DutyCnt;
		break;
	case MOTOR3:
		MTU3.TGRD = DutyCnt;
		break;
	}
}

void gr_ex_motor::setDir(gr_ex_motor_dir Dir){
	_Dir = Dir;
	bool_t OutA, OutB;
	
	switch(Dir){
	case CW:
		OutA = 1;
		OutB = 0;
		break;
	case CCW:
		OutA = 0;
		OutB = 1;
		break;
	case BRAKE:
		OutA = 1;
		OutB = 1;
		break;
	case FREE:
		OutA = 0;
		OutB = 0;
		break;
	}
	
	
	switch(_Motor){
	case MOTOR0:
		outPin(GREX_PIN_MOTOR0A, OutA);
		outPin(GREX_PIN_MOTOR0B, OutB);
		break;
	case MOTOR1:
		outPin(GREX_PIN_MOTOR1A, OutA);
		outPin(GREX_PIN_MOTOR1B, OutB);
		break;
	case MOTOR2:
		outPin(GREX_PIN_MOTOR2A, OutA);
		outPin(GREX_PIN_MOTOR2B, OutB);
		break;
	case MOTOR3:
		outPin(GREX_PIN_MOTOR3A, OutA);
		outPin(GREX_PIN_MOTOR3B, OutB);
		break;
	}
	
}

// +-255�Ńf���[�e�B�ݒ�
void gr_ex_motor::setDuty(int16_t Duty){

	if(Duty<0){
		setDir(CCW);
		Duty = -Duty;
	}else{
		setDir(CW);
	}
	
	if(Duty>255){
		Duty = 255;
	}
	
	
	setDutyCnt(Duty * PeriodCnt_ / 255);
	
}


/****************************
�G���R�[�_
gr_ex_encoder
	gr_ex.Enc0
	gr_ex.Enc1

�g�p���W���[��
	MTU1, MTU2
		Enc0 : MTU1 / MTCLKA, MTCLKB
		Enc1 : MTU2 / MTCLKC, MTCLKD
****************************/
gr_ex_encoder::gr_ex_encoder(gr_ex_encoder_num EncNum){
	Enc_ = EncNum;	// �G���R�[�_�Z�b�g
	
	setRegister();		// ���W�X�^�ݒ�
	
	setCnt(65535/2);	// �J�E���^�����l
	CntLast_ = 65535/2;	
	
	stopCnt();
}

void gr_ex_encoder::setRegister(void){
	
	switch(Enc_){
	case ENC0:
	
		// �������݋���
		SYSTEM.PRCR.WORD = 0xA503u;
		MSTP(MTU1) = 0;	// ���W���[���N��
		MTU.TSTR.BIT.CST1 = 0;
		
		
		MTU1.TMDR.BIT.MD = 4;		// �ʑ��v�����[�h1(4���{)

		//MTU.TSTR.BIT.CST1 = 1;	// �J�E���g�X�^�[�g
		
		// �|�[�g�o�͐ݒ�
		MPC.PWPR.BYTE = 0x00u;
		MPC.PWPR.BYTE = 0x40u;
		SYSTEM.PRCR.WORD = 0xA503u;
		setPinMode(GREX_PIN_ENC0A, PIN_INPUT_PULLUP);
		MPC.P24PFS.BIT.PSEL = 2;			// MTCLKA
		PORT2.PMR.BIT.B4 = 1;				// not GPIO
		setPinMode(GREX_PIN_ENC0B, PIN_INPUT_PULLUP);
		MPC.P25PFS.BIT.PSEL = 2;			// MTCLKB
		PORT2.PMR.BIT.B5 = 1;				// not GPIO
		
		// ���荞�݋���
		MTU1.TIER.BIT.TCIEV = 1;	// �I�[�o�[�t���[
		MTU1.TIER.BIT.TCIEU = 1;	// �A���_�[�t���[
		EN(MTU1,TCIV1) = 1;			// �I�[�o�[�t���[(ICU)
		EN(MTU1,TCIU1) = 1;			// �A���_�[�t���[(ICU)
		IEN(ICU,GROUP1) = 1;
		IPR(ICU,GROUP1) = 1;
	
		break;
	case ENC1:
	
		// �������݋���
		SYSTEM.PRCR.WORD = 0xA503u;
		MSTP(MTU2) = 0;	// ���W���[���N��
		MTU.TSTR.BIT.CST2 = 0;
		
		MTU2.TMDR.BIT.MD = 4;		// �ʑ��v�����[�h1(4���{)

		//MTU.TSTR.BIT.CST2 = 1;	// �J�E���g�X�^�[�g
		
		// �|�[�g�o�͐ݒ�
		MPC.PWPR.BYTE = 0x00u;
		MPC.PWPR.BYTE = 0x40u;
		SYSTEM.PRCR.WORD = 0xA503u;
		setPinMode(GREX_PIN_ENC1A, PIN_INPUT_PULLUP);
		MPC.P22PFS.BIT.PSEL = 2;			// MTCLKC
		PORT2.PMR.BIT.B2 = 1;				// not GPIO
		setPinMode(GREX_PIN_ENC1B, PIN_INPUT_PULLUP);
		MPC.P23PFS.BIT.PSEL = 2;			// MTCLKD
		PORT2.PMR.BIT.B3 = 1;				// not GPIO
		
		// ���荞�݋���
		MTU2.TIER.BIT.TCIEV = 1;	// �I�[�o�[�t���[
		MTU2.TIER.BIT.TCIEU = 1;	// �A���_�[�t���[
		EN(MTU2,TCIV2) = 1;			// �I�[�o�[�t���[(ICU)
		EN(MTU2,TCIU2) = 1;			// �A���_�[�t���[(ICU)
		IEN(ICU,GROUP2) = 1;
		IPR(ICU,GROUP2) = 1;
		
		break;
	}
	
	
}

void gr_ex_encoder::startCnt(void){
	switch(Enc_){
	case ENC0:
		MTU.TSTR.BIT.CST1 = 1;	// �J�E���g�X�^�[�g
		break;
	case ENC1:
		MTU.TSTR.BIT.CST2 = 1;	// �J�E���g�X�^�[�g
		break;
	}
}

void gr_ex_encoder::stopCnt(void){
	switch(Enc_){
	case ENC0:
		MTU.TSTR.BIT.CST1 = 0;	// �J�E���g�X�^�[�g
		break;
	case ENC1:
		MTU.TSTR.BIT.CST2 = 0;	// �J�E���g�X�^�[�g
		break;
	}
}


void gr_ex_encoder::setCnt(uint16_t Cnt){
	switch(Enc_){
	case ENC0:
		MTU1.TCNT = Cnt;
		break;
	case ENC1:
		MTU2.TCNT = Cnt;
		break;
	}
}

uint16_t gr_ex_encoder::getCnt(void){
	switch(Enc_){
	case ENC0:
		return MTU1.TCNT;
		break;
	case ENC1:
		return MTU2.TCNT;
		break;
	}
	
	return 0;
}

int16_t gr_ex_encoder::getCntDiff(void){
	uint16_t Cnt;
	int16_t Diff;	
	bool_t fUpCnt;	// �J�E���g����(�A�b�v�J�E���g��1)
	
	switch(Enc_){
	case ENC0:
		Cnt = MTU1.TCNT;
		fUpCnt = MTU1.TSR.BIT.TCFD;
		break;
	case ENC1:
		Cnt = MTU2.TCNT;
		fUpCnt = MTU2.TSR.BIT.TCFD;
		break;
	}
	
	if(fUpCnt){
		if(fOverflow_){
			Diff = Cnt + (0xFFFF - CntLast_) + 1;
		}else{
			Diff = Cnt - CntLast_;
		}
	}else{
		if(fUnderflow_){
			Diff = (Cnt - 0xFFFF) + CntLast_ - 1;
		}else{
			Diff = Cnt - CntLast_;
		}
	}
	
	CntLast_ = Cnt;
	
	fOverflow_ = 0;
	fUnderflow_ = 0;
	
	return Diff;
}
















// ���荞�݊֐��̐ݒ�
// �X�s�[�J
#pragma interrupt (Excep_TMR3_CMIA3(vect=VECT(TMR3,CMIA3), enable))
void Excep_TMR3_CMIA3(void){
	
	GrEx.Sp->isrSpPlay();
	
}


// MTU0 �I�[�o�[�t���[
// MTU1 �I�[�o�[�t���[
//		�A���_�[�t���[
#pragma interrupt (Excep_ICU_GROUP1(vect=VECT(ICU,GROUP1), enable))
void Excep_ICU_GROUP1(void){
	// �I�[�o�[�t���[
	if(IS(MTU1,TCIV1)){			// ���荞�݃X�e�[�^�X�t���O�`�F�b�N
		CLR(MTU1,TCIV1) = 1;	// ���荞�ݗv���N���A
		GrEx.Enc0->fOverflow_ = 1;	// 
	}
	// �A���_�[�t���[
	if(IS(MTU1,TCIU1)){			// ���荞�݃X�e�[�^�X�t���O�`�F�b�N
		CLR(MTU1,TCIU1) = 1;	// ���荞�ݗv���N���A
		GrEx.Enc0->fUnderflow_ = 1;	// 
	}
}

// MTU2 �I�[�o�[�t���[
//		�A���_�[�t���[
// MTU3 �I�[�o�[�t���[
#pragma interrupt (Excep_ICU_GROUP2(vect=VECT(ICU,GROUP2), enable))
void Excep_ICU_GROUP2(void){
	// �I�[�o�[�t���[
	if(IS(MTU2,TCIV2)){			// ���荞�݃X�e�[�^�X�t���O�`�F�b�N
		CLR(MTU2,TCIV2) = 1;	// ���荞�ݗv���N���A
		GrEx.Enc1->fOverflow_ = 1;	// 
	}
	// �A���_�[�t���[
	if(IS(MTU2,TCIU2)){			// ���荞�݃X�e�[�^�X�t���O�`�F�b�N
		CLR(MTU2,TCIU2) = 1;	// ���荞�ݗv���N���A
		GrEx.Enc1->fUnderflow_ = 1;	// 
	}
}












