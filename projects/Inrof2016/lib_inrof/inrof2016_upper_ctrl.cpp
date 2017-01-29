/**************************************************
 �m�\���{�R��2016 �㕔����
�T�v�F
 �擾�{�[���A�A�[���ʒu�Ǘ�
**************************************************/

#include "inrof2016_upper_ctrl.h"

inrof2016_upper_ctrl InrofUpper;

// �{�[���u����̃G���R�[�_�J�E���g�l
const uint8_t inrof2016_upper_ctrl::BallStockEnc[10] = {0x05,0x15,0x25,0x34,0x46,0x56,0x67,0x78,0x87,0x97};


// M�n��G���R�[�_�J�E���g�l
const uint8_t inrof2016_upper_ctrl::MseqArray[] = {
/* +00h */	0x9B, 0xCD, 0xE6, 0xF3, 0x79, 0x3C, 0x1E, 0x8F, 0xC7, 0x63, 0xB1, 0x58, 0xAC, 0xD6, 0xEB, 0x75, 
/* +01h */	0x3A, 0x9D, 0x4E, 0x27, 0x13, 0x09, 0x84, 0xC2, 0xE1, 0xF0, 0x78, 0xBC, 0xDE, 0xEF, 0x77, 0xBB, 
/* +02h */	0xDD, 0xEE, 0xF7, 0x7B, 0xBD, 0x5E, 0x2F, 0x17, 0x0B, 0x05, 0x82, 0x41, 0x20, 0x10, 0x08, 0x04, 
/* +03h */	0x02, 0x81, 0x40, 0xA0, 0xD0, 0x68, 0xB4, 0xDA, 0xED, 0xF6, 0xFB, 0x7D, 0x3E, 0x9F, 0xCF, 0x67, 
/* +04h */	0xB3, 0xD9, 0xEC, 0x76, 0x3B, 0x1D, 0x8E, 0x47, 0xA3, 0xD1, 0xE8, 0x74, 0xBA, 0x5D, 0x2E, 0x97, 
/* +05h */	0xCB, 0x65, 0x32, 0x99, 0x4C, 0xA6, 0x53, 0xA9, 0x54, 0xAA, 0x55, 0x2A, 0x95, 0x4A, 0x25, 0x92, 
/* +06h */	0x49, 0x24, 0x12, 0x89, 0x44, 0xA2, 0x51, 0x28, 0x14, 0x0A, 0x85, 0x42, 0x21, 0x90, 0xC8, 0x64, 
/* +07h */	0xB2, 0x59, 0x2C, 0x16, 0x8B, 0xC5, 0xE2, 0xF1, 0xF8, 0x7C, 0xBE, 0x5F, 0xAF, 0xD7, 0x6B, 0xB5, 
/* +08h */	0x5A, 0x2D, 0x96, 0x4B, 0xA5, 0x52, 0x29, 0x94, 0xCA, 0xE5, 0xF2, 0xF9, 0xFC, 0x7E, 0x3F, 0x1F, 
/* +09h */	0x0F, 0x07, 0x03, 0x01, 0x80, 0xC0, 0x60, 0xB0, 0xD8, 0x6C, 0xB6, 0x5B, 0xAD, 0x56, 0x2B, 0x15, 
/* +0Ah */	0x8A, 0x45, 0x22, 0x91, 0x48, 0xA4, 0xD2, 0xE9, 0xF4, 0x7A, 0x3D, 0x9E, 0x4F, 0xA7, 0xD3, 0x69, 
/* +0Bh */	0x34, 0x1A, 0x8D, 0x46, 0x23, 0x11, 0x88, 0xC4, 0x62, 0x31, 0x98, 0xCC, 0x66, 0x33, 0x19, 0x8C, 
/* +0Ch */	0xC6, 0xE3, 0x71, 0x38, 0x1C, 0x0E, 0x87, 0xC3, 0x61, 0x30, 0x18, 0x0C, 0x06, 0x83, 0xC1, 0xE0, 
/* +0Dh */	0x70, 0xB8, 0xDC, 0x6E, 0x37, 0x1B, 0x0D, 0x86, 0x43, 0xA1, 0x50, 0xA8, 0xD4, 0x6A, 0x35, 0x9A, 
/* +0Eh */	0x4D, 0x26, 0x93, 0xC9, 0xE4, 0x72, 0x39, 0x9C, 0xCE, 0xE7, 0x73, 0xB9, 0x5C, 0xAE, 0x57, 0xAB, 
/* +0Fh */	0xD5, 0xEA, 0xF5, 0xFA, 0xFD, 0xFE, 0xFF, 0x7F};
	



// 20msec
void inrof2016_upper_ctrl::task0(void){


	// ����
	
	
	/* ���� */
	// ��Ԑ���
	if(isEnableOut()){
		ctrlState();
	}
	
	
	/* �o�� */
	// �A�[���̃��[�^�[�𐧌�
	ctrlArmMotor();

	
	
	/* ����p�ʐM */
	
	// �A�[���G���R�[�_�擾�v��
	reqArmEnc();
	// �T�[�{�f�[�^�X�V
	ServoGrab->updatePresentPosition();	// �p�x
	ServoPitch->updatePresentPosition();	// x10[deg]
	ServoGrab->updatePresentSpeed();	// ���x
	ServoPitch->updatePresentSpeed();	// int16_t [deg/sec]
	ServoGrab->updatePresentCurrent();	// �d��
	ServoPitch->updatePresentCurrent();	// uint16_t[mA]
}

void inrof2016_upper_ctrl::task1(void){	// 200msec
	
	ColorSens->updateBallColor();	// �{�[���F����
	
	
}


void inrof2016_upper_ctrl::begin(void){

	// �A�[�����[�^�[
	ArmMotor = GrEx.Motor3;
	ArmMotor->setPeriodCnt(GREX_MOTOR_PWMCNT_10KHz);
	//pid_ctrlArmMotor = new ctrl_pid(SAMPLETIME_CTRL_S, 50, 4, 2, -255, 255, -511, 511);
	pid_ctrlArmMotor = new ctrl_pid(SAMPLETIME_CTRL_S, 100, 0, 2, -255, 255, -511, 511);
	ratelimiter_ctrlArmMotor = new ctrl_ratelimiter_abs(SAMPLETIME_CTRL_S, 1000, 2000);	// �����͑���
	
	// I2C
	
	// �V���A���T�[�{
	// SCI5
	ServoGrab = new futaba_cmd_servo(1, &Sci5, 115200);
	ServoPitch = new futaba_cmd_servo(2, &Sci5, 115200);
	
	// �J���[�Z���T
	ColorSens = new inrof2016_color_sens_t(&I2C0, ARM_I2C_ADDRESS);	
	
	ArmI2C = &I2C0;		// ����̏������ǂ����ł��Ȃ���ˁB�J���[�Z���T��begin�ł�邩�炢���񂾂��ǁB
	
	// ����p
	// �A�[���G���R�[�_���x�v�Z�p���[�p�X
	CtrlArmEncSpdAverage = new ctrl_move_average(ARM_ENC_SPD_AVERAGE_NUM);
	
	
// ������
	//initialize(); //-> Inrof����Ă�
	
}

void inrof2016_upper_ctrl::initialize(void){
	
	// �V�[�P���X�������Z�b�g
	SeqNum = 0;
	pCtrlSeqFunc = NULL;
	State = UPPER_STATE_IDLE;
	
	// �ێ��{�[��������
	for(uint8_t i;i<NUM_BALL_STOCK;i++){
		BallStock[i] = INROF_NONE;
	}
	// ���R�{�[���Z�b�g
	BallStock[2] = INROF_YELLOW;
	//BallStock[3] = INROF_BLUE;
	
	
	
	// �A�[�����[�^�[
	ArmMotor->setDir(CCW);
	ArmMotor->setDuty(0);
	// �T�[�{
	ServoGrab->begin();
	ServoPitch->begin();
	ServoGrab->setLimit(SERVOPOS_MAX_G, SERVOPOS_MIN_G);
	ServoPitch->setLimit(SERVOPOS_MAX_P, SERVOPOS_MIN_P);
	
	// �T�[�{on
	ServoGrab->enableTorque(CMDSV_TORQUE_ON);
	ServoPitch->enableTorque(CMDSV_TORQUE_ON);
	ServoGrab->setMaxTorque(100);
	ServoPitch->setMaxTorque(100);
	ServoGrab->setGoalTime(SERVO_GOALTIME_NORMAL);
	ServoPitch->setGoalTime(SERVO_GOALTIME_NORMAL);
	//ServoGrab->setGoalPosition(SERVOPOS_RUN_G);
	//ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
	ServoGrab->setGoalPosition(SERVOPOS_BALLUP2_G);
	ServoPitch->setGoalPosition(SERVOPOS_BALLUP2_P);
	
	//�J���[�Z���T
	ColorSens->begin();
	ColorSens->wakeup();	// ���W���[���N����
	ColorSens->startMeasurement();	// �v���J�n
	// debug.
	//ColorSens->enableMeasure();	// �v���J�n
	
	
	// �A�[���G���R�[�_
	//ArmEncModeMseqIndex();	// M�n��G���R�[�h���[�h��
	ArmEncModeRaw();	// �������ŃG���R�[�h���[�h��
	ArmEnc = 0xFE;	// �����������
	ArmEncCorrect = ArmEnc;
	
	DutyTgt = 0;
	BallPosTgt = 4;
	EncCntTgt = 0x4B;
	
	CtrlMode = MODE_DUTY;
	
	// ���싖��
	enableAction();
}


// ��Ԑ���
int8_t inrof2016_upper_ctrl::ctrlState(void){
	int8_t ret;
	
	if(NULL != pCtrlSeqFunc){
		
		ret = (this->*pCtrlSeqFunc)();
		
	}
	
	return ret;
}



// �V�[�P���X�����֐��Z�b�g
int8_t inrof2016_upper_ctrl::attachCtrlSeqFunc(int8_t (inrof2016_upper_ctrl::*pCtrlSeqFunc)(void)){
	
	SeqNum = 0;
	SeqNumLast = 0xFF;
	this->pCtrlSeqFunc = pCtrlSeqFunc;
	
	return 0;
};



/*********************
�{�[���̂Ă鏀���v��
�T�v�F
	�w�肵���F�̃{�[����͂�ŁA�w�肵���p�x�܂Ŏ����Ă���
	
	�ړ����Ԃ����_�ɂ��Ȃ����߁A�ڕW�p�x���牓�����ɂ����Ă���{�[����I������B
������ϐ��F
	SeqArmEncObj
	SeqBallStockObj
	CtrlSeqFunc
	State
**********************/
int8_t inrof2016_upper_ctrl::reqBallTrashPre(inrof_color Col, float AngleRad){
	
	if((UPPER_STATE_IDLE == State) || (UPPER_STATE_ARM_TO_CENTER==State)){
		
		// �������p�x�ݒ�(�{�[����u���G���R�[�h�l)
		this->SeqArmEncObj = Rad2Enc(AngleRad);		// �ڕW�G���R�[�_�ʒu
		
		// ���Ԗڂ̃{�[������邩������
		// �ڕW�n�_���牓������珇�ɂƂ��
		
		// 0�ɋ߂����T��
		int8_t i;
		uint8_t NearCw = 0xFF;
		uint8_t NearCcw = 0xFF;
		for(i=0;i<NUM_BALL_STOCK;i++){	// ���񂳂����񂳂���
			if(Col == BallStock[i]){	// �~�����F�̃{�[����������
				NearCw = (uint8_t)i;
			}
		}
		for(i=NUM_BALL_STOCK-1;i>=0;i--){	// ���񂳂����񂳂���
			if(Col == BallStock[i]){	// �~�����F�̃{�[����������
				NearCcw = i;
			}
		}
		if((0xFF==NearCw) || (0xFF==NearCcw)){	// �{�[���Ȃ�������
			State = UPPER_STATE_IDLE;
			return -1;			// �߂�
		}
		// �������ꏊ���牓������ڕW�{�[���u����ɐݒ�
		if( abs((int16_t)this->SeqArmEncObj - (int16_t)BallStockEnc[NearCw]) > abs((int16_t)this->SeqArmEncObj - (int16_t)BallStockEnc[NearCcw]) ){
			// CW���̂ق��������̂ł�������܂��B
			this->SeqBallStockObj = NearCw;		// �ڕW�{�[���u����
		}else{
			this->SeqBallStockObj = NearCcw;	// �ڕW�{�[���u����
		}
		
		
		
		// �֐��o�^
		attachCtrlSeqFunc(ctrlSeqBallTrashPre);
		// ��ԍX�V
		State = UPPER_STATE_BALL_TRASH_PRE;
#ifdef DEBUGSCI0_UPPER
Sci0.print("reqBallTrashPre. Ang%f, BallPos%d, Col%d\r\n", AngleRad, this->SeqBallStockObj, Col);
#endif
		return 0;
	}else{
#ifdef DEBUGSCI0_UPPER
Sci0.print("reqBallTrashPre. n\r\n");
#endif
		return -1;
	}
}


// �A�[���V�[�P���X
// �{�[���̂Ă鏀��
int8_t inrof2016_upper_ctrl::ctrlSeqBallTrashPre(void){
	bool_t SeqInit = false;
	
	if(SeqNum!=SeqNumLast){	// ���񏈗�
		SeqInit = true;
	}
	SeqNumLast = SeqNum;
	
	switch(SeqNum){
	case 0:	// �T�[�{���ړ��ʒu��
		/* ���񏈗� */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			// �T�[�{���ړ��ʒu��
			ServoGrab->setGoalTime(SERVO_GOALTIME_NORMAL);
			ServoPitch->setGoalTime(SERVO_GOALTIME_NORMAL);
			ServoGrab->setGoalPosition(SERVOPOS_BALLTRASH0_G);
			ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+2000<getTime_ms())){	// �T�[�{���슮��
			SeqNum ++;
		}
		break;
	
	case 1:	// �{�[�����Ɍ�����
		/* ���񏈗� */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			// �ڕW�ʒu�ݒ�
			moveArmBallPos(this->SeqBallStockObj);	// �{�[�����ɍs���{�[���u�����ݒ�
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. Move\r\n");
#endif
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for ctrlArmMotor()) */
		
		
		/* �J�ڏ��� */
		if(isFinishCtrlArmMove() || (SeqStartTime_ms+10000<getTime_ms())){	// �{�[���̈ʒu�܂ŗ���
			SeqNum ++;
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. MoveDone\r\n");
#endif
		}
		break;
	case 2:	// �A�[��������
		/* ���񏈗� */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			ServoGrab->setGoalPosition(SERVOPOS_BALLTRASH0_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLTRASH0_P);
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. ArmDown\r\n");
#endif
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+3000<getTime_ms())){	// �A�[����������
#ifdef DEBUGSCI0_UPPER
Sci0.print("SCg%d,p%d\r\n", ServoGrab->getPresentCurrent(), ServoPitch->getPresentCurrent());
#endif
			SeqNum ++;
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. ArmDownDone\r\n");
#endif
		}
		break;
	case 3:	// �{�[���͂�
		/* ���񏈗� */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			ServoGrab->setGoalPosition(SERVOPOS_BALLTRASH1_G);
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. Grab\r\n");
#endif
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+2000<getTime_ms())){	// �A�[���͂�
#ifdef DEBUGSCI0_UPPER
Sci0.print("SCg%d,p%d\r\n", ServoGrab->getPresentCurrent(), ServoPitch->getPresentCurrent());
#endif
			SeqNum ++;
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. GrabDone\r\n");
#endif
		}
		break;
	case 4:	// �{�[�������グ��
		/* ���񏈗� */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. BallUp\r\n");
#endif
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+2000<getTime_ms())){	// �{�[�������オ����
			SeqNum ++;
			BallStock[this->SeqBallStockObj] = INROF_NONE;	// ���������Ƀ{�[���͂Ȃ��́B
#ifdef DEBUGSCI0_UPPER
Sci0.print("SCg%d,p%d\r\n", ServoGrab->getPresentCurrent(), ServoPitch->getPresentCurrent());
Sci0.print("ctrlSeqBallTrashPre. BallUpDone\r\n");
#endif
		}
		break;
	case 5:	// �̂Ă�ʒu�܂ňړ�
		/* ���񏈗� */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			// �ڕW�ʒu�ݒ�
			moveArmEncCnt(this->SeqArmEncObj);	// �{�[���̂Ă�ʒu���w��
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. moveTrash\r\n");
#endif
		}
		
		/* �ʏ펞���� */
		/* wait for ctrlArmMotor() */
		
		
// �����Ń{�[���͂�łȂ������I�I
// �ĂȂ����������x���ɍs��
// 3�񂭂炢����ă_���Ȃ���߂�B

		/* �J�ڏ��� */
		if(isFinishCtrlArmMove() || (SeqStartTime_ms+10000<getTime_ms())){	// �̂Ă�ʒu�܂ŗ���
#ifdef DEBUGSCI0_UPPER
Sci0.print("SCg%d,p%d\r\n", ServoGrab->getPresentCurrent(), ServoPitch->getPresentCurrent());
#endif
if(State != UPPER_STATE_IDLE){Sci0.print("ctrlSeqBallTrashPre. moveTrashDone\r\n");}
			State = UPPER_STATE_IDLE;
			dettachCtrlSeqFunc();	// ��������
		}
		break;
	default:
		/* �Ȃɂ����������񂾂��� */
		return -1;
	}
	
	return 0;
}
	
	

/*********************
�{�[���̂Ă�v��
�T�v�F
	�A�[���|���ă{�[���ۂ���
������ϐ��F
	CtrlSeqFunc
	State
**********************/
int8_t inrof2016_upper_ctrl::reqBallTrash(void){
	
	if((UPPER_STATE_IDLE == State) || (UPPER_STATE_ARM_TO_CENTER==State)){
		
		// �֐��o�^
		attachCtrlSeqFunc(ctrlSeqBallTrash);
		// ��ԍX�V
		State = UPPER_STATE_BALL_TRASH;
	
		return 0;
	}else{
		return -1;
	}
}

// �A�[���V�[�P���X
// �{�[���̂Ă�
int8_t inrof2016_upper_ctrl::ctrlSeqBallTrash(void){
	bool_t SeqInit = false;
	
	if(SeqNum!=SeqNumLast){	// ���񏈗�
		SeqInit = true;
	}
	SeqNumLast = SeqNum;
	
	switch(SeqNum){
	case 0:	// �T�[�{���̂Ă�ʒu��
		/* ���񏈗� */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			// �A�[�����̂Ă�ʒu�֗����グ��
			ServoGrab->setGoalPosition(SERVOPOS_BALLTRASH3_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLTRASH3_P);
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if((isFinishMoveServo() && (SeqStartTime_ms+1500<getTime_ms())) || (SeqStartTime_ms+2000<getTime_ms())){	// �T�[�{���슮��
			SeqNum ++;
		}
		break;
	case 1:	// �A�[���߂�
		/* ���񏈗� */
		if(SeqInit){
			// �w����
			ServoGrab->setGoalTime(1000);
			ServoGrab->setGoalPosition(SERVOPOS_RUN_G);
			ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
			// �J�n�����o����
			SeqStartTime_ms = getTime_ms();
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if((isFinishMoveServo() && (SeqStartTime_ms+1500<getTime_ms())) || (SeqStartTime_ms+10000<getTime_ms())){	// �T�[�{���슮�� && ���傢�҂�
			if(State != UPPER_STATE_IDLE){Sci0.print("ctrlSeqBallTrash. TrashDone\r\n");}
			//State = UPPER_STATE_BALL_TRASH_DONE;
			
			
			ServoGrab->setGoalTime(SERVO_GOALTIME_NORMAL);
			ServoGrab->setGoalPosition(SERVOPOS_RUN_G);
			ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
			
			State = UPPER_STATE_IDLE;		// �{�[���̂Ă�����ĂȂɂ����Ȃ�����IDLE�ɂ��悤
			dettachCtrlSeqFunc();	// ��������
		}
		break;
	default:
		/* �Ȃɂ����������񂾂��� */
		return -1;
	}
	
	return 0;
}



/*********************
�{�[���E�������v��
�T�v�F
	�A�[���|���ăK�o�b�ƊJ��
�����F
	�{�[���E���ꏊ(�p�x)
������ϐ��F
	CtrlSeqFunc
	State
**********************/
int8_t inrof2016_upper_ctrl::reqBallGetPre(float AngleRad){
	
	if((UPPER_STATE_IDLE == State) || (UPPER_STATE_ARM_TO_CENTER==State)){
		
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPre: req\r\n");
#endif
	
		// �������p�x�ݒ�(�{�[�����E���G���R�[�h�l)
		this->SeqArmEncObj = Rad2Enc(AngleRad);		// �ڕW�G���R�[�_�ʒu
		
		// �֐��o�^
		attachCtrlSeqFunc(ctrlSeqBallGetPre);
		// ��ԍX�V
		State = UPPER_STATE_BALL_GET_PRE;
		
		return 0;
	}else{
		return -1;
	}
}

// �A�[���V�[�P���X
// �{�[���E������
int8_t inrof2016_upper_ctrl::ctrlSeqBallGetPre(void){
	bool_t SeqInit = false;
	
	if(SeqNum!=SeqNumLast){	// ���񏈗�
		SeqInit = true;
	}
	SeqNumLast = SeqNum;
	
	switch(SeqNum){
	case 0:	// �T�[�{���ړ��ʒu��
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPre: SvMovePos\r\n");
#endif
			SeqStartTime_ms = getTime_ms();
			// �A�[�����ړ��ʒu�֗����グ��
			ServoGrab->setGoalTime(SERVO_GOALTIME_NORMAL);
			ServoPitch->setGoalTime(SERVO_GOALTIME_NORMAL);
			ServoGrab->setGoalPosition(SERVOPOS_RUN_G);
			ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// �T�[�{���슮��
			SeqNum ++;
		}
		break;
	case 1:	// �A�[���ړ�
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPre: ArmMove\r\n");
#endif
			// �{�[���E���ʒu�ֈړ��v��
			moveArmEncCnt(this->SeqArmEncObj);
			// �J�n�����o����
			SeqStartTime_ms = getTime_ms();
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if(isFinishCtrlArmMove() || (SeqStartTime_ms+10000<getTime_ms())){	//�E���ʒu�܂ŗ���
			SeqNum ++;
		}
		break;
	case 2:	// �T�[�{���낵�ăK�o�b��
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPre: SvOpen\r\n");
#endif
			// �T�[�{���E���ʒu�։��낵�āA�A�[�����L����
			ServoGrab->setGoalPosition(SERVOPOS_BALLGET0_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLGET0_P);
			// �J�n�����o����
			SeqStartTime_ms = getTime_ms();
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// �T�[�{���슮��
			SeqNum ++;
		}
		break;
	case 3:	// �A�[������
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPre: SvDown\r\n");
#endif
			// �A�[������
			ServoGrab->setGoalPosition(SERVOPOS_BALLGET1_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLGET1_P);
			// �J�n�����o����
			SeqStartTime_ms = getTime_ms();
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// �T�[�{���슮��
			State = UPPER_STATE_IDLE;		// �{�[���E����������
			dettachCtrlSeqFunc();			// ��������
		}
		break;
	default:
		/* �Ȃɂ����������񂾂��� */
#ifdef DEBUGSCI0_UPPER
		Sci0.print("<Sequence Error>inrof2016_upper_ctrl::ctrlSeqBallGetPre %d\r\n", SeqNum);
#endif
		return -1;
	}
	
	return 0;
}




/*********************
�{�[���E���v��
�T�v�F
	�͂�Ŏ����グ��
	�����Ă�����ԂɂȂ�����V�[�P���X�I��
	�{�[���F������
�����F
	�Ȃ�
������ϐ��F
	CtrlSeqFunc
	State
**********************/
int8_t inrof2016_upper_ctrl::reqBallGet(void){
	
	if(UPPER_STATE_IDLE == State){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGet req\r\n");
#endif
		
		// �֐��o�^
		attachCtrlSeqFunc(ctrlSeqBallGet);
		// ��ԍX�V
		State = UPPER_STATE_BALL_GET;
		
		return 0;
	}else{
		return -1;
	}
}

// �A�[���V�[�P���X
// �{�[���E��
int8_t inrof2016_upper_ctrl::ctrlSeqBallGet(void){
	bool_t SeqInit = false;
	
	if(SeqNum!=SeqNumLast){	// ���񏈗�
		SeqInit = true;
	}
	SeqNumLast = SeqNum;
	
	switch(SeqNum){
	case 0:	// �͂�
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGet: Grab\r\n");
#endif
			SeqStartTime_ms = getTime_ms();
			// �A�[����͂�
			ServoGrab->setGoalTime(200);
			ServoGrab->setGoalPosition(SERVOPOS_BALLGET1_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLGET1_P);
			
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// �T�[�{���슮��
			
			ServoGrab->setGoalTime(SERVO_GOALTIME_NORMAL);
			// �{�[���͂�?
			if(checkHoldBall()){
				// �͂�ł���J���[����J�n
		//		ColorSens->enableMeasure();
			}
			// �͂�ł�
			SeqNum ++;
		}
		break;
	case 1:	// �����グ��
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGet: Up\r\n");
#endif
			// �A�[���������グ��
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP0_G);
			ServoPitch->setGoalTime(2000);
			ServoPitch->setGoalPosition(SERVOPOS_BALLUP0_P);
			// �J�n�����o����
			SeqStartTime_ms = getTime_ms();
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+2000<getTime_ms())){	// �A�[�������グ��
		
			ServoPitch->setGoalTime(1000);
			
			// �{�[���͂�?
			//if(checkHoldBall()){
			if(1){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGet: Grab Ball Got\r\n");
#endif
				// �͂�ł�
				// �������͏���Ƀ{�[���u�����肵�Ă邩��
				// �����g����͈ړ����Ă�����B
				BallHoldNum ++;	// �l���{�[�������X�V
				
				//State = UPPER_STATE_BALL_GET_POST;	// �{�[���ڂ����鏀������
				//attachCtrlSeqFunc(ctrlSeqBallGetPost);	// �}�V����ɍڂ��ɍs��
				reqBallGetPost();
			}else{
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGet: Grab Ball no Got\r\n");
#endif
				// �͂�łȂ�
				// ������ƃV�[�P���X�I���
				ServoGrab->setGoalPosition(SERVOPOS_RUN_G);	// �T�[�{�͈ړ��\�ʒu��
				ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
				ColorSens->disableMeasure();
				// �Z���^�[�ɖ߂낤
				State = UPPER_STATE_IDLE;		// �{�[���E����������
				dettachCtrlSeqFunc();			// ��������
				//reqArmToCenter();
			}
			
			
		}
		break;
	default:
		/* �Ȃɂ����������񂾂��� */
#ifdef DEBUGSCI0_UPPER
		Sci0.print("<Sequence Error>inrof2016_upper_ctrl::ctrlSeqBallGet %d\r\n", SeqNum);
#endif
		return -1;
	}
	
	return 0;
}




/*********************
�{�[�����ڂ�����v��
�T�v�F
	�{�[���E��������}�V����ɒu��
�����F
	�Ȃ�
������ϐ��F
	CtrlSeqFunc
	State
**********************/
int8_t inrof2016_upper_ctrl::reqBallGetPost(void){
	
	if(UPPER_STATE_BALL_GET == State){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost req\r\n");
#endif
		
		// �֐��o�^
		attachCtrlSeqFunc(ctrlSeqBallGetPost);
		// ��ԍX�V
		State = UPPER_STATE_BALL_GET_POST;
		
		return 0;
	}else{
		return -1;
	}
}
int8_t inrof2016_upper_ctrl::ctrlSeqBallGetPost(void){
	bool_t SeqInit = false;
	
	if(SeqNum!=SeqNumLast){	// ���񏈗�
		SeqInit = true;
	}
	SeqNumLast = SeqNum;
	
	switch(SeqNum){
	case 0:	// �T�[�{���ړ��ʒu��
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: Sv MovePos\r\n");
#endif
			SeqStartTime_ms = getTime_ms();
			// �A�[�����ړ��ʒu�֗����グ��
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP0_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLUP0_P);
			
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// �T�[�{���슮��
			SeqNum ++;
		}
		break;
		
	case 1:	// �A�[�����J���[�Z���T�ݒu�ʒu�ֈړ�
		if(SeqInit){
			// �{�[���E���ʒu�ֈړ��v��
			moveArmBallPos(INROF_UPPER_COLSENS_POS);
		}
		
		/* do nothing */
		// �A�[���ړ��҂�
		
		/* �J�ڏ��� */
		if(isFinishCtrlArmMove() || (SeqStartTime_ms+10000<getTime_ms())){	// �u���ʒu�܂ŗ���
			SeqNum ++;
		}
		
	case 2:	// �J���[�`�F�b�N�̂��߃{�[������U�u��
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: color check: Sv Put ball pos\r\n");
#endif
			// �T�[�{��u���p�x��
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP1_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLUP1_P);
			// �J�n�����o����
			SeqStartTime_ms = getTime_ms();
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// �T�[�{���슮��
			SeqNum ++;
		}
		break;
		
	case 3:	// �{�[���u��(�J���[�`�F�b�N)
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: color check: Sv put Ball \r\n");
#endif
			// �{�[������� (�����ւ��邽��)
			ServoGrab->setGoalTime(200);
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP2_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLUP2_P);
			// �J�n�����o����
			SeqStartTime_ms = getTime_ms();
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if((isFinishMoveServo() && (SeqStartTime_ms+3000<getTime_ms())) || (SeqStartTime_ms+5000<getTime_ms())){
			// �T�[�{���슮��&&�{�[��������̑҂�
			// �J���[�`�F�b�N�J�n
			ColorSens->enableMeasure();
			
			ServoGrab->setGoalTime(SERVO_GOALTIME_NORMAL);
			SeqNum ++;
		}
		break;
		
	case 4:	// �{�[������+�J���[�`�F�b�N�҂�
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: color check: Sv get Ball \r\n");
#endif
			// �J���[�Z���T��̃{�[����͂�
			ServoGrab->setGoalPosition(SERVOPOS_BALLTRASH1_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLTRASH1_P);
			// �J�n�����o����
			SeqStartTime_ms = getTime_ms();
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if((isFinishMoveServo() && (SeqStartTime_ms+3000<getTime_ms())) || (SeqStartTime_ms+5000<getTime_ms())){
			// �T�[�{���슮��&&�{�[��������̑҂�
			// �F�m��
			HoldingBallColor = ColorSens->getBallColor();
			
			// �����o�ɕ\��
			switch(HoldingBallColor){
			case INROF_RED:
				Rmb.outLedDispAscii("RED");
				break;
			case INROF_YELLOW:
				Rmb.outLedDispAscii("YLOW");
				break;
			case INROF_BLUE:
				Rmb.outLedDispAscii("BLUE");
				break;
			default:
				Rmb.outLedDispAscii("MISS");
				
				break;
			}
			// �J���[�`�F�b�N�I���
			ColorSens->disableMeasure();
			
			// ����ł��Ȃ�����
			if(INROF_NONE == HoldingBallColor){
				//HoldingBallColor = INROF_BLUE;	//�Ƃ肠�����̂Ƃ��ɓ����
				
				// �͂�łȂ�
				// ������ƃV�[�P���X�I���
				ServoGrab->setGoalPosition(SERVOPOS_RUN_G);	// �T�[�{�͈ړ��\�ʒu��
				ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
				ColorSens->disableMeasure();
				// �Z���^�[�ɖ߂낤
				State = UPPER_STATE_IDLE;		// �{�[���E����������
				dettachCtrlSeqFunc();			// ��������
				//reqArmToCenter();
				
			}
			
			
			
			SeqNum ++;
		}
		break;
		
	case 5:	// �{�[�������グ��
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: color check done (%d): Sv up Ball \r\n", HoldingBallColor);
#endif
			// �{�[�����ړ��\��ԂɎ����グ��
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP0_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLUP0_P);
			// �J�n�����o����
			SeqStartTime_ms = getTime_ms();
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+5000<getTime_ms())){
			// �T�[�{���슮��
			SeqNum ++;
		}
		break;
		
	case 6:	// �{�[����u���ʒu�փA�[���ړ�
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: ArmMove\r\n");
#endif
			
			// �}�V����̂��������̈ʒu�����߂�
			// ���̃A�[���ʒu���牓���ʒu(�Ȃ�ׂ��{�[�����u���Ă���Ƃ�����A�[�����ړ����Ȃ��悤��)
			int8_t Sel;
			uint8_t NearCw = 0xFF;
			uint8_t NearCcw = 0xFF;
			// CW�����ŉ����ʒu
			for(Sel=0;Sel<NUM_BALL_STOCK;Sel+=2){	// �����ӏ��ɂ����u���Ȃ�
				if(BallStock[Sel] == INROF_NONE){	// �J���Ă�
					NearCw = Sel;
					break;
				}
			}
			for(Sel=8;Sel>=0;Sel-=2){	// �����ӏ��ɂ����u���Ȃ�
				if(BallStock[Sel] == INROF_NONE){	// �J���Ă�
					NearCcw = Sel;
					break;
				}
			}
			
			// �u���ꏊ�����񂾂���
			if((NearCw==0xFF) && (NearCcw==0xFF)){
				// ���Ƃōl����
			}
			
			// CWCCW�̉�������ڕW�{�[���u����ɐݒ�
			if( fabs(((float)NUM_BALL_STOCK-1.0F)/2.0F - (float)NearCw) > fabs(((float)NUM_BALL_STOCK-1.0F)/2.0F - (float)NearCcw) ){
				// CW���̂ق��������̂ł����ɒu���܂��B
				this->SeqBallStockObj = NearCw;		// �ڕW�{�[���u����
			}else{
				this->SeqBallStockObj = NearCcw;	// �ڕW�{�[���u����
			}
			
			// �{�[���E���ʒu�ֈړ��v��
			moveArmBallPos(this->SeqBallStockObj);
			
			
			
			// �J�n�����o����
			SeqStartTime_ms = getTime_ms();
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if(isFinishCtrlArmMove() || (SeqStartTime_ms+10000<getTime_ms())){	// �u���ʒu�܂ŗ���
			SeqNum ++;
		}
		break;
	case 7:	// �{�[���u���p�x��
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: Sv Put ball pos\r\n");
#endif
			// �T�[�{��u���p�x��
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP2_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLUP2_P);
			// �J�n�����o����
			SeqStartTime_ms = getTime_ms();
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+2000<getTime_ms())){	// �T�[�{���슮��
			SeqNum ++;
		}
		break;
	case 8:	// �{�[���u��
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: Sv put Ball \r\n");
#endif
			// �{�[�������
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP3_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLUP3_P);
			// �J�n�����o����
			SeqStartTime_ms = getTime_ms();
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if((isFinishMoveServo() && (SeqStartTime_ms+1000<getTime_ms())) || (SeqStartTime_ms+5000<getTime_ms())){
			// �T�[�{���슮��&&�{�[��������̑҂�
			SeqNum ++;
		}
		break;
	case 9:	// �T�[�{���ړ��\�ʒu��
		/* ���񏈗� */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: Sv free pos\r\n");
#endif
			// �T�[�{���ړ��p�x��
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP3_G);		// Grab�͊J�����܂܁B
			ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
			// �J�n�����o����
			SeqStartTime_ms = getTime_ms();
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// �T�[�{���슮��
			// �u�����{�[����o�^
			BallStock[this->SeqBallStockObj] = HoldingBallColor;
			// �{�[�����X�V(�O�̂���)
			updateBallHoldNum();
			
			State = UPPER_STATE_IDLE;		// �{�[���u����
			dettachCtrlSeqFunc();			// ��������
			//reqArmToCenter();			// �u������߂�A�^�񒆂�
		}
		break;
	default:
		/* �Ȃɂ����������񂾂��� */
#ifdef DEBUGSCI0_UPPER
		Sci0.print("<Sequence Error>inrof2016_upper_ctrl::ctrlSeqBallGetPost %d\r\n", SeqNum);
#endif
		return -1;
	}
	
	return 0;
}




/*********************
�A�[���^�񒆂ֈړ��v��
�T�v�F
	�A�[�������ɂ��Đ^�񒆂ֈړ�
	�I�������UPPER_STATE_IDLE�ɂȂ�
**********************/
// �v��
int8_t inrof2016_upper_ctrl::reqArmToCenter(void){
	
	if(State == UPPER_STATE_IDLE){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqArmToCenter req\r\n");
#endif
		// �֐��o�^
		attachCtrlSeqFunc(ctrlSeqArmToCenter);
		// ��ԍX�V
		State = UPPER_STATE_ARM_TO_CENTER;
		
		return 0;
	}else{
		return -1;
	}
}

// �����V�[�P���X
int8_t inrof2016_upper_ctrl::ctrlSeqArmToCenter(void){
	bool_t SeqInit = false;
	
	if(SeqNum!=SeqNumLast){	// ���񏈗�
		SeqInit = true;
	}
	SeqNumLast = SeqNum;
	
	switch(SeqNum){
	case 0:	// �T�[�{���ړ��ʒu��
		/* ���񏈗� */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			// �T�[�{���ړ��ʒu��
			ServoGrab->setGoalTime(500);
			ServoPitch->setGoalTime(500);
			ServoGrab->setGoalPosition(SERVOPOS_RUN_G);
			ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for servo move) */
		
		
		/* �J�ڏ��� */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// �T�[�{���슮��
			ServoGrab->setGoalTime(SERVO_GOALTIME_NORMAL);
			ServoPitch->setGoalTime(SERVO_GOALTIME_NORMAL);
			SeqNum ++;
		}
		break;
	
	case 1:	// �����֌�����
		/* ���񏈗� */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			// �ڕW�ʒu�ݒ�
			moveArmAngle(PI/2);	// �{�[�����ɍs���{�[���u�����ݒ�
		}
		
		/* �ʏ펞���� */
		/* do nothing (wait for ctrlArmMotor()) */
		
		
		/* �J�ڏ��� */
		if(isFinishCtrlArmMove()  || (SeqStartTime_ms+10000<getTime_ms())){	// �����ʒu�܂ŗ���
			State = UPPER_STATE_IDLE;
			dettachCtrlSeqFunc();	// ��������
		}
		break;
	default:
		/* �Ȃɂ����������񂾂��� */
		return -1;
	}
	
	return 0;
}




/*********************
�{�[�������Ă�H
�߂�l�F
	0:�Ȃ�
	1:�O�Ŏ����Ă�
	2:���Ŏ����Ă�
**********************/
uint8_t inrof2016_upper_ctrl::checkHoldBall(void){
	return 1;
	if(INROF_UPPER_BALLHOLD_CURRENT_TH_mA <= ServoGrab->getPresentCurrent()){
		if( 0 < ServoGrab->updatePresentPosition()){	// �O
			return 1;
		}else{	//���
			return 2;
		}
	}else{	// �����ĂȂ�
		return 0;
	}
	
	return 0;
}


/*********************
�T�[�{�ړ������H
 ���x 10deg/sec�ȓ�
 �ʒu 2deg�ȓ�
**********************/
bool_t inrof2016_upper_ctrl::isFinishMoveServo(void){
	
	if(abs(ServoPitch->getPresentSpeed()) < 10){
		if(abs(ServoPitch->GoalPosition - ServoPitch->getPresentPosition()) < 20){	// 20*0.1�x
			if(abs(ServoGrab->getPresentSpeed()) < 10){
				if(abs(ServoGrab->GoalPosition - ServoGrab->getPresentPosition()) < 20){	// 20*0.1�x
					return 1;
				}
			}
		}
	}
	return 0;
}



/*********************
�����Ă�{�[�����X�V
�T�v�F
	�z��𐔂���
**********************/
void inrof2016_upper_ctrl::updateBallHoldNum(void){
	uint8_t i;
	uint8_t cnt=0;
	for(i=0;i<NUM_BALL_STOCK;i++){
		if(BallStock[i]!=INROF_NONE){
			cnt++;
		}
	}
	BallHoldNum = cnt;
}
/*********************
�{�[���������Ă�H
�����F
	�F
�߂�l�F
	��
**********************/
uint8_t inrof2016_upper_ctrl::numBallColor(inrof_color Col){
	uint8_t cnt = 0;
	uint8_t i;
	
	for(i = 0; i<NUM_BALL_STOCK; i++){
		if(Col == BallStock[i]){
			cnt++;
		}
	}
	
	return cnt;
}



/*********************
�A�[���G���R�[�_ �G���R�[�h���[�h
�@�t�H�g�C���^���v�^ �֑J��
�T�v�F
**********************/
int8_t inrof2016_upper_ctrl::ArmEncModeRaw(void){
	uint8_t TxData;
	
	ArmEncMode = ARM_ENC_RAW;
	TxData = 0x02;
	return ArmI2C->attach(ARM_I2C_ADDRESS, &TxData, 1);
}

/*********************
�A�[���G���R�[�_ �G���R�[�h���[�h
�@�G���R�[�h�ς� �֑J��
�T�v�F
**********************/
int8_t inrof2016_upper_ctrl::ArmEncModeMseqIndex(void){
	uint8_t TxData;
	
	ArmEncMode = ARM_ENC_MSEQINDEX;
	TxData = 0x03;
	return ArmI2C->attach(ARM_I2C_ADDRESS, &TxData, 1);
}

/*********************
�A�[���G���R�[�_����v��
�T�v�F
 I2C�Ŏ󂯂�B
 1�o�C�g��M
**********************/
int8_t inrof2016_upper_ctrl::reqArmEnc(void){
	uint8_t dummy;
	
	ReqObject = this;
	//return ArmI2C->attach(ARM_I2C_ADDRESS, &dummy, 0, 1, isrArmEnc);
	return ArmI2C->attach(ARM_I2C_ADDRESS, &dummy, 0, 2, isrArmEnc);
}

/*********************
I2C��M����
static
�T�v�F
 I2C�Ŏ󂯂�B
 1�o�C�g��M
**********************/
inrof2016_upper_ctrl* inrof2016_upper_ctrl::ReqObject;
void inrof2016_upper_ctrl::isrArmEnc(I2c_comu_t *Comu){
	uint8_t tmp[2];
	uint8_t ArmEncLast = ReqObject->ArmEnc;
	
	if(!Comu->Err){
		 tmp[0] = Comu->RxData[0];	// �����ς�
		 tmp[1] = Comu->RxData[1];	// raw
	}else{
		 tmp[0] = 0xFE;	// �ʐM���s
		 tmp[1] = 0xFE;	// �ʐM���s
	}
	
	if(ARM_ENC_RAW == ReqObject->ArmEncMode){			// ���l����M
		
		ReqObject->ArmEncRaw = tmp[1];
		
		if(tmp[0] == 0xFF){		// �t�H�g�C���^���v�^�����ĂȂ�or�S��������(����Ȃ炠�肦�Ȃ�)
			ReqObject->ArmEnc = 0xFF;	// ��xFF�ɂȂ����Ⴄ�ƕ��A�Ɏ�Ԏ��̂łȂ������B
			//return -1;
		}else if(Comu->Err){		// �ʐM���s
			Sci0.print("Mseq Comu Error. %d\r\n", I2C0.isError());
			ReqObject->ArmEnc = 0xFE;
			//return -1;
		}else{
			ReqObject->updateArmEnc(ReqObject->ArmEncRaw);	// ���O�ŃG���R�[�h
		}
	}else if(ARM_ENC_MSEQINDEX == ReqObject->ArmEncMode){	// �G���R�[�h�ς݂���M
		
		ReqObject->ArmEnc = tmp[0];
		ReqObject->ArmEncRaw = tmp[1];
		
	}
	
	// ���x�X�V
	if(((ReqObject->ArmEnc)<=ENC_CNT_MAX) && (ArmEncLast<=ENC_CNT_MAX)){
		ReqObject->ArmEncSpd = ReqObject->CtrlArmEncSpdAverage->average( (ReqObject->ArmEnc - ArmEncLast)/SAMPLETIME_CTRL_S );
	}
}

/*********************
m�n��M������C���f�b�N�X���X�V
�T�v�F���ԍl���B�ςȒl�ɂȂ�����X�V���Ȃ��B
 �X�V����Ȃ�������-1��Ԃ�
**********************/
int8_t inrof2016_upper_ctrl::updateArmEnc(uint8_t Mseq){
	uint8_t i;
	uint8_t Kouho = 0xFF;
	
	uint8_t Window = 5;	// �O��l���炱�ꂾ���̃Y���͋���
	const uint8_t NoDecideCntMax = 5;	// �O��l���炱�ꂾ���̃Y���͋���
	
	/*
	// 2015.05.19
	// �傫���l�ɔ�񂶂�������ɕ��A�ł��Ȃ��Ȃ�̂ō폜�B
	if((ArmEnc<5)||(ArmEnc==0xFE)){	// �����������������l
		i = 0;			// ��������A0����T���X�^�[�g
	}else{
		i = ArmEnc-5;
	}
	*/
	/*
	// debug. 
	if(Mseq == 0xFF){	// �t�H�g�C���^���v�^�����ĂȂ�or�S��������(����Ȃ炠�肦�Ȃ�)
		//ArmEnc = 0xFF;	// ��xFF�ɂȂ����Ⴄ�ƕ��A�Ɏ�Ԏ��̂łȂ������B
		return -1;
	}else if(Mseq == 0xFE){	// �ʐM���s
	Sci0.print("Mseq Comu Error.\r\n");
		ArmEnc = 0xFE;
		return -1;
	}
	*/
	
	// �C���f�b�N�X�ɕϊ�(�Y���Ȃ��Ȃ�0xFF�̂܂�)
	for(i=0;i<ENC_CNT_MAX;i++){
		if(Mseq == MseqArray[i]){
			Kouho = i;
			break;
		}
	}
	
	//if(Kouho!=0xFF){	// �l�L��
		// ���������������́A(�O��l�Ɣ�r���ăv���X�}�C�i�X1�͈̔͂ɂ���)�Ȃ�X�V
		// �O��0xFF�Ƃ�FE�������瑦�����H���̉񕜃��W�b�N�ŉ񕜂���Ȃ�ǂ����B
		if((((ArmEnc==0xFE) || (ArmEnc==0xFF))&&(Kouho<ENC_CNT_MAX)) || ((Kouho<=(ArmEnc+Window))&&((Kouho+Window)>=ArmEnc))){
			ArmEnc = Kouho;
			NoDecideCnt = 0;
			
			return 0;
		}
		
		// �������Ɠ����ĂȂ�������B
		
		// �����Ă�̂ɘA�����Ă���������Β����ׂ��B
		// (�����������ɓK�p�����l����̂Ƃ��������Ƃ��ŁA
		// �ςȒl���m�肳�ꂿ�Ⴄ���Ƃ�����̂ŁB)
		if(ArmEncLast != Kouho){	// �����Ă���
			if(NoDecideCnt > NoDecideCntMax){	// n�}�X�����Ă��߂Ȃ�A
				ArmEnc = Kouho;	// ����̂��m�肳�����Ⴄ
						// �O��lArmEncLast�Ƃ̍��݂͂Ȃ��Ă����H
						// ����Ŏ��񂿂��Ƃ����l�ɂȂ��Cnt�̓��Z�b�g�����
				Sci0.print("ArmEncAdj = 0x%02X\r\n", ArmEnc);
			}else{
				NoDecideCnt++;
				Sci0.print("ArmEncCnt = 0x%02d\r\n", NoDecideCnt);
			}
		}
		
		ArmEncLast = Kouho;
	//}
	return -1;
}




void inrof2016_upper_ctrl::moveArmDuty(int16_t Duty){
	DutyTgt = Duty;
	CtrlMode = MODE_DUTY;
}

void inrof2016_upper_ctrl::moveArmEncCnt(uint8_t EncCnt){
	EncCntTgt = EncCnt;
	CtrlMode = MODE_ENCCNT;
}
void inrof2016_upper_ctrl::moveArmBallPos(uint8_t BallPos){
	
	// BollPos����G���R�[�_�J�E���g�l�֕ϊ�
	EncCntTgt = BallStockEnc[BallPos];
	
	BallPosTgt = BallPos;
	CtrlMode = MODE_BALLPOS;
}


void inrof2016_upper_ctrl::moveArmAngle(float Rad){
	
	// 0x1B Rad=0
	// 0x80 Rad=pi
	
	// Radian����G���R�[�_�J�E���g�l�֕ϊ�
	//EncCntTgt = (0x80 - 0x1B)/3.1415F * Rad + 0x1B;
	EncCntTgt = Rad2Enc(Rad);
	
	RadTgt = Rad;
	CtrlMode = MODE_RAD;
}


/*********************
�ړ������H(�V�[�P���X����)
 �E�ڕW�G���R�[�_�l1digit�ȓ�
 �E0.5count/sec�ȓ��̑��x�ɂȂ���
�����F
	�ڕW�G���R�[�_�l
�߂�l�F
	������� 1
**********************/
bool_t inrof2016_upper_ctrl::isFinishCtrlArmMove(void){
	
	// ���x������
	if(fabs(ArmEncSpd) < 0.5F){
		if( (MODE_ENCCNT==CtrlMode) || (MODE_BALLPOS==CtrlMode) || (MODE_RAD==CtrlMode) ){	// �ʒu����n
			// �ʒu����̏ꍇ�͍X�Ɉʒu�̏���
			if(abs((int16_t)ArmEnc - (int16_t)EncCntTgt) < 2U){
				return 1;
			}else{
				return 0;
			}
		}else{	// ���x����Ƃ�
			return 1;
		}
	}
	return 0;
}



/*********************
�A�[�����[�^����
�T�v�F
 �w��G���R�[�_�ʒu�ֈړ�����
 �G���R�[�_�l�����΂炭�ς��Ȃ��悤�Ȃ�
**********************/
int8_t inrof2016_upper_ctrl::ctrlArmMotor(void){
	
	if(CtrlModeLast != CtrlMode){
		pid_ctrlArmMotor->resetStatus();
	}
	
	
	// �G���R�[�_�J�E���g�l����f���[�e�B�[�֕ϊ�
	if(MODE_DUTY != CtrlMode){	// �ʒu����n
		if(ArmEnc<=ENC_CNT_MAX){	
			// �G���R�[�_�L���l
			// PID�ŖڕW�G���R�[�_�ʒu��
			DutyTgt = pid_ctrlArmMotor->calc(EncCntTgt - ArmEnc);
			DutyTgt = ratelimiter_ctrlArmMotor->limitRate(DutyTgt);
			CntFail = 0;
			ArmEncCorrect = ArmEnc;	// �������ʐM�ł��Ă������̒l���o���Ă���
			
			
		}else{ // �ʐM�r�₦�Ă�
			// 
			if(INROF_UPPER_CNT_FAIL_MAX<CntFail){
				
				if(ArmEnc!=0xFE){
					if(fabs(ArmEncSpd)<0.5){	// �~�܂�������Ă�
								// �[�Ԃ���������Ă鎞�͂ǂ�������
						if(EncCntTgt > ArmEncCorrect){	// �ڕW�Ƃ��ꂽ�Ƃ���Ŏ~�܂�������Ă��炿����Ɠ��������Ⴆ
							DutyTgt = -100;	// �����ċt�����ɉ�
						}else{
							DutyTgt = 100;
						}
					}
				}else{
					DutyTgt = 0;	// �ʐM�r��͎~�߂�
				}
			}else{
				// �J�E���g�}�b�N�X�ɂȂ�܂ł͑O�̂��o��
				CntFail++;
			}
		}
	}
	
	// �[��������
	// ����ȏ�s���Ȃ��悤�ɂ���
	
	if(ArmEnc<=ENC_CNT_MAX){	// �L���l�̎��̂�
		if(ArmEnc > ENC_CNT_MAX-3){
			if(DutyTgt>0){
				DutyTgt = 0;
			}
		}
		if(ArmEnc < 3){
			if(DutyTgt<0){
				DutyTgt = 0;
			}
		}
	}
	
	
	
	if(fAction){	// ���싖��
		ArmMotor->setDuty( (int16_t)DutyTgt );
	}else{
		ArmMotor->setDuty( 0 );
		ArmMotor->setDir(FREE);
	}
	
	CtrlModeLast = CtrlMode;
	return 0;
}



/*********************
rad -> �G���R�[�_�l
�T�v�F
**********************/
uint8_t inrof2016_upper_ctrl::Rad2Enc(float Rad){
	
	
	if(Rad < -1.0F/3.0F*3.14159F){
		Rad = -1.0F/3.0F*3.14159F;
	}
	if(Rad > 4.0F/3.0F*3.14159F){
		Rad = 4.0F/3.0F*3.14159F;
	}
	
	// 270deg��146count 0�x�ŃJ�E���g�l��30
	// 146/270 * rad/pi*180 + 30;
	float tmp = Rad * 30.98F + 30.0F;
	
	if(tmp<0){
		tmp = 0;
	}else if(tmp>ENC_CNT_MAX){
		tmp = ENC_CNT_MAX;
	}
	
	return (uint8_t)tmp;
}



