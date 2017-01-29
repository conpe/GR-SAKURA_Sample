/**************************************************
inrof2016_upper_ctrl.h
	�m�\���{�R��2015�p�̃��C�u���������B
	
	
	10����u����̂����A�{�[����u���̂͋����ӏ��̂݁B
	
	
**************************************************/


#ifndef __inrof2016_UPPERCTRL_H__
#define __inrof2016_UPPERCTRL_H__


#include "inrof2016_common.h"

#include "FutabaCmdServo.h"
#include "inrof2016_color_sens.h"

#include "AppliCtrl.h"
#include "RIIC.h"

#include "RoombaOi.h"

extern RoombaOi Rmb;


#define ENC_CNT_MAX 155	// M�n��M���̒���
#define INROF_UPPER_BALLHOLD_CURRENT_TH_mA	30	// �{�[�������Ă锻�肷��T�[�{�̓d��[mA]
//#define INROF_UPPER_BALLHOLD_CURRENT_TH_mA	0	// �{�[�������Ă锻�肷��T�[�{�̓d��[mA]

#define INROF_UPPER_CNT_FAIL_MAX 40

#define ARM_ENC_SPD_AVERAGE_NUM	5U	// �A�[���G���R�[�_���x�̈ړ����ςƂ鐔

#define NUM_BALL_STOCK 10	// �{�[���u����̐�
//#define NUM_BALL_STOCK_MAX 5	// �㕔�ɂ�����{�[����
#define NUM_BALL_STOCK_MAX 1	// �㕔�ɂ�����{�[����

#define INROF_UPPER_COLSENS_POS	5	// �J���[�Z���T�ݒu�ӏ�

enum upper_state{
	UPPER_STATE_IDLE,
	UPPER_STATE_BALL_TRASH_PRE,	// �{�[���̂Ă鏀����
	UPPER_STATE_BALL_TRASH,		// �{�[���̂ĂĂ邿�イ
	UPPER_STATE_BALL_GET_PRE,	// �{�[����鏀����
	UPPER_STATE_BALL_GET,		// �{�[���Ƃ��Ă�
	UPPER_STATE_BALL_GET_POST,	// �{�[���Ƃ����㏈��
	UPPER_STATE_ARM_TO_CENTER,	// �^�񒆂ֈړ���
};



/****************************
 �m�\���{�R��2016 �㕔����
�T�v�F
 �擾�{�[���A�A�[���ʒu�Ǘ�
****************************/
class inrof2016_upper_ctrl{
public: 
	
	// �A�N�`���G�[�^
	// �A�[�����[�^
	gr_ex_motor* ArmMotor;
	uint8_t ArmPosTgt;
	
	
	// �T�[�{
	futaba_cmd_servo* ServoGrab;
	futaba_cmd_servo* ServoPitch;
	// �J���[�Z���T
	inrof2016_color_sens_t *ColorSens;
	// �A�[���G���R�[�_
	uint8_t ArmEnc;		// �A�[���ʒu(0�`ENC_CNT_MAX-1, 0xFE:��������, 0xFF�����l)
	uint8_t ArmEncRaw;	// �G���R�[�_���l
	uint8_t ArmEncLast;
	uint8_t NoDecideCnt;	// �A�[���ʒu�m�肵�Ȃ��J�E���g
	float ArmEncSpd;	// �A�[�����x(�G���R�[�_���x) [cnt/sec]	���[�p�X�t��
	
	
	// �A�[�����[�^�[
	// �A�[���G���R�[�_
	static inrof2016_upper_ctrl* ReqObject;	// static�Ȏ�M�֐�����ArmEnc����肷�邽��
	
	
	// 
	void begin(void);
	void initialize(void);
	// 
	void task0(void);	// 20msec
	void task1(void);	// 200msec
	
	
	/*********************
	// ���싖��
	**********************/
	bool_t fAction;
	void enableAction(void){fAction = true;};
	void disableAction(void){fAction = false;};
	bool_t isEnableOut(void){return fAction;};
	
	/*********************
	// �����Ԏ擾
	**********************/
	upper_state getState(void){return State;};
	
	/*********************
	// �����Ă�{�[���֌W
	**********************/
	// �����Ă�{�[����
	uint8_t numBall(void){return BallHoldNum;};
	// �����Ă�{�[����(�F�w��)
	uint8_t numBallColor(inrof_color Col);
	// ���{�[�������Ă鐔
	uint8_t numBlue(void){return numBallColor(INROF_BLUE);};
	// ���F���{�[�������Ă鐔
	uint8_t numYellow(void){return numBallColor(INROF_YELLOW);};
	// �Ԃ��{�[�������Ă鐔
	uint8_t numRed(void){return numBallColor(INROF_RED);};
	// ���������ς��H
	bool_t isBallFull(void){return (NUM_BALL_STOCK_MAX<=BallHoldNum);};
	
	// ���쏈��
	int8_t ctrlState(void);		 // �����V�[�P���X�����s
	int8_t attachCtrlSeqFunc(int8_t (inrof2016_upper_ctrl::*pCtrlSeqFunc)(void));	// �V���������V�[�P���X���Z�b�g����
	int8_t dettachCtrlSeqFunc(void){pCtrlSeqFunc = NULL; return 0;};
	/*********************
	�{�[���̂Ă鏀��
	�T�v�F
		�w�肵���F�̃{�[����͂�ŁA�w�肵���p�x�܂Ŏ����Ă���
	**********************/
	// �v��
	int8_t reqBallTrashPre(inrof_color Col, float AngleRad);
	// �����V�[�P���X
	int8_t ctrlSeqBallTrashPre(void);
	
	/*********************
	�{�[���̂Ă�v��
	�T�v�F
		�A�[�����Ăă{�[�������ăA�[���グ��
	**********************/
	// �v��
	int8_t reqBallTrash(void);
	// �����V�[�P���X
	int8_t ctrlSeqBallTrash(void);
	
	
	
	/*********************
	�{�[���E������
	�T�v�F
		�A�[�����w��ʒu�ŃK�o�b�ƊJ��
		�I�������IDLE
	**********************/
	// �v��
	int8_t reqBallGetPre(float AngleRad);
	// �����V�[�P���X
	int8_t ctrlSeqBallGetPre(void);
	
	/*********************
	�{�[���E��
	�T�v�F
		�͂�Ŏ����グ��
		�I������玩����GetPost�������s
	**********************/
	// �v��
	int8_t reqBallGet(void);
	// �����V�[�P���X
	int8_t ctrlSeqBallGet(void);
	
	/*********************
	�{�[���E�����㏈��
	�T�v�F
		�F�`�F�b�N���āA
		�{�[�������������̃}�V����ɒu���B
		�����ԑ͓̂����Ă�����B
		�I������玩���ŃZ���^�[�֖߂�
	**********************/
	// �v��
	int8_t reqBallGetPost(void);
	// �����V�[�P���X
	int8_t ctrlSeqBallGetPost(void);
	
	
	/*********************
	�A�[���^�񒆂֗v��
	�T�v�F
		�A�[�������ɂ��Đ^�񒆂ֈړ�
		�I�������UPPER_STATE_IDLE�ɂȂ�
	**********************/
	// �v��
	int8_t reqArmToCenter(void);
	// �����V�[�P���X
	int8_t ctrlSeqArmToCenter(void);
	
	
	/*********************
	�A�[���ړ��v��(�ʒuor���x)
	�T�v�F
	 �A�[�����ړ�����
	 �ʒu�w��n�͂�����������I�ɂ͖ڕW�G���R�[�_�l�ɕϊ�����
	�����F
	 int16_t Duty : +-255
	 int16_t EncCnt : 0-ENC_CNT_MAX
	 uint8_t BallPos : �{�[���ʒu(CW�[��0, CCW�[��9)
	 float Rad : �p�x�Ŏw��
	**********************/
	void moveArmDuty(int16_t Duty);		// �f���[�e�B�[�w��
	void moveArmEncCnt(uint8_t EncCnt);	// �G���R�[�_�l
	void moveArmBallPos(uint8_t BallPos);	// �{�[���X�g�b�N�ʒu
	void moveArmAngle(float Rad);		// �p�x
	// �A�[���ړ������H
	bool_t isFinishCtrlArmMove(void);	// �ړI�n(�G���R�[�_�l)�ɋ߂� ���� ���x��������
	
	
	/*********************
	�{�[�������Ă�H
	 0:�Ȃ�
	 1:�O�Ŏ����Ă�
	 2:���Ŏ����Ă�
	**********************/
	uint8_t checkHoldBall(void);
	
	/*********************
	�T�[�{�ړ������H
	**********************/
	bool_t isFinishMoveServo(void);
	
	
private:
	enum ctrl_mode_t{
		MODE_DUTY,
		MODE_ENCCNT,
		MODE_BALLPOS,
		MODE_RAD
	};
	
	enum arm_enc_mode{
		ARM_ENC_RAW,
		ARM_ENC_MSEQINDEX
	};
	
	// �㕔�̏��
	upper_state State;
	
	// �A�[���ړ�����(�ʒuor���x)
	ctrl_mode_t CtrlMode;	// �A�[�����䃂�[�h(�ڕW�l�w����@)
	int16_t DutyTgt;	// �f���[�e�B�w��l
	uint8_t EncCntTgt;	// �G���R�[�_�ʒu�w��l
	uint8_t BallPosTgt;	// �{�[���̈ʒu�w��l
	uint8_t RadTgt;		// �p�x�w��l [rad]
	
	// �{�[���F
	inrof_color BallColor;		// ����{�[���F
	inrof_color HoldingBallColor;	// ��Ŏ����Ă�{�[���F
	
	// �G���R�[�_�l
	static const uint8_t BallStockEnc[NUM_BALL_STOCK];	// �{�[���u����̃G���R�[�_�l
	static const uint8_t MseqArray[];	// �G���R�[�_�l�ƃC���f�b�N�X�̑Ή�
	
	// �����Ă�{�[��
	inrof_color BallStock[NUM_BALL_STOCK];
	int8_t BallHoldNum;	// �m�ۂ��Ă�{�[����
	void updateBallHoldNum(void);	// �{�[�����X�V	// ctrlSeqBallGet()�ŉ��ɑ��₵���肷��̂ŁA���ۂɏ���������ɉ��߂Đ�������
	
	/* �V�[�P���X�����p */
	int8_t (inrof2016_upper_ctrl::*pCtrlSeqFunc)(void);	//��������V�[�P���X�֐�
	// �e�V�[�P���X���̏����ԍ�
	uint8_t SeqNum;
	uint8_t SeqNumLast;
	// �ڕW�{�[���u����
	uint8_t SeqBallStockObj;	// �ڕW�{�[���u����
	uint8_t SeqArmEncObj;		// �ڕW�G���R�[�_�ʒu
	// �e�V�[�P���X�J�n����
	uint32_t SeqStartTime_ms;	
	
	/*********************
	�A�[���G���R�[�_
	�T�v�F	
	**********************/
	I2c_t* ArmI2C;
	bool_t ArmEncMode;
	int8_t ArmEncModeRaw(void);	// ���l��M���[�h��
	int8_t ArmEncModeMseqIndex(void);	// M�n��G���R�[�h�ςݎ�M���[�h��
	int8_t reqArmEnc(void);	// �ʐM�v��
	static void isrArmEnc(I2c_comu_t *Comu);	// ��M����
	int8_t updateArmEnc(uint8_t);	// m�n��M������C���f�b�N�X(ArmEnc)�X�V
	ctrl_move_average* CtrlArmEncSpdAverage;	// �A�[���G���R�[�_���x�ړ����Ϗ���
	
	
	/*********************
	�A�[���ړ�����
	�T�v�F	
	**********************/
	int8_t ctrlArmMotor(void);
	
	ctrl_pid* pid_ctrlArmMotor;
	ctrl_ratelimiter_abs* ratelimiter_ctrlArmMotor;
	ctrl_mode_t CtrlModeLast;
	uint8_t CntFail;
	uint8_t ArmEncCorrect;
	
	
	/*********************
	���܂���
	**********************/
	// rad -> �G���R�[�_�l
	uint8_t Rad2Enc(float Rad);
	uint8_t Deg2Enc(float Deg);
	
};



extern inrof2016_upper_ctrl InrofUpper;

#endif
