/**************************************************
Inrof2015_lib.h
	�m�\���{�R��2015�p�̃��C�u���������B
	
	
	
	
**************************************************/


#ifndef __INROF2015_H__
#define __INROF2015_H__

//#include <sdmmc.h>

#include "GR_define.h"
#include "inrof2015_common.h"

#include "AppliCtrl.h"

#include "RIIC.h"
#include "SCI.h"

#include "portReg.h"
#include "GR_Ex15.h"
#include "GR_Ex15_OLED.h"
#include "inrof2015_psd.h"

#include "FutabaCmdServo.h"
#include "OledDriverSSD1306.h"
#include "SharpGP2Y0E.h"
#include "HamamatsuColorSensS11059.h"
#include "InvensenseMPU.h"


#define PI 3.1415


typedef struct calc_odmetry{
	float REncCntDiff[2];
	float LEncCntDiff[2];
	float CosTh[2];
	float SinTh[2];
	float dth2[2];
}calc_odmetry;

enum inrof_state{
	INROF_IDLE,
	INROF_TANSAKU,
	INROF_TOGOAL,
	INROF_TOSTART
};
enum inrof_action_state{
	INROF_ACTION_IDLE,		// 0
	INROF_ACTION_LINE,		// 1
	INROF_ACTION_STRAIGHT,	// 2
	INROF_ACTION_TURN,		// 3
	INROF_ACTION_BALLGET,	// 4
	INROF_ACTION_BALLRELEASE	// 5
};
/*
enum inrof_action_state{
}
*/
enum inrof_color{
	INROF_NONE,
	INROF_RED,
	INROF_YELLOW,
	INROF_BLUE
};
// ���s����
enum inrof_linedir{
	INROF_LINEDIR_NONE,
	INROF_LINEDIR_FORWARD,
	INROF_LINEDIR_BACKWARD
};

enum inrof_cross_index{
	INROF_CROSS_NONE,	// 0
	INROF_CROSS_START,	// 1
	INROF_CROSS_RED,	// 2
	INROF_CROSS_YELLOW,	// 3
	INROF_CROSS_BLUE,	// 4
	INROF_CROSS_BALLFIELD_BEGIN,	// 5
	INROF_CROSS_BALLFIELD_END,		// 6
	INROF_CROSS_GOAL_RED,			// 7
	INROF_CROSS_GOAL_YELLOW,		// 8
	INROF_CROSS_GOAL_BLUE			// 9
};

enum inrof_orientation{
	ORIENT_NONE, 
	ORIENT_EAST,
	ORIENT_NORTH,
	ORIENT_WEST,
	ORIENT_SOUTH,
};


/****************************
 �m�\���{�R��2015

�T�v�F
 ���������1ms, 10ms�B
 ���C���֐��Ŏg����悤�A200msec���Ƃɗ��t���O(fCnt200)��p��
 
****************************/
class inrof2015{
public:
	
	// �A�N�`���G�[�^
	// ���[�^
	gr_ex_motor* MotorR;
	gr_ex_motor* MotorL;
	// �T�[�{
	futaba_cmd_servo* ServoF;
	futaba_cmd_servo* ServoR;
	// �Z���T
	// �����Z���T
	inrof2015_psd *PsdLeft;
	inrof2015_psd *PsdLeftSide;
	inrof2015_psd *PsdFront;
	inrof2015_psd *PsdRightSide;
	inrof2015_psd *PsdRight;
	// �J���[�Z���T
	hamamatsu_S11059 *ColorSens;
	// IMU
	invensense_MPU* Mpu;
	// �G���R�[�_
	gr_ex_encoder* EncR;
	gr_ex_encoder* EncL;
	
	
	bool_t fCnt200;
	bool_t fMotorOut;
	// �ڕW���x, �p���x
	float SpdTgt, AngVelTgt;	//private�ɂ�������
	
	
	// ������
	inrof2015(void);
	void begin(void);
	
	// ����
	void step(void);
	
	
	
	// ���C���Z���T
	uint16_t getLineSensAd(uint8_t LineSensNum){return LineSensAd[LineSensNum];};
	void getLineSenseGlobalPos(position* Pos);
	//�}�V����Ԏ擾
	float getRTireSpd(void){return RTireSpd_rps;};
	float getLTireSpd(void){return LTireSpd_rps;};
	position* getMachinePos(void){return &MachinePos;};
	
	
	void setState(inrof_state State){this->State = State;};
	int8_t getState(void){return State;};
	
	
	// ���[�e�B���e�B
	uint32_t getTime(void){return Time_msec;};	// �N������̎���
	bool_t isEmptyBatt(void){return fBattEmpty;};	// �o�b�e����?
	
	// TPU6���g�����t���[�J�E���^
	void setupFreeCounter(void);
	uint16_t getFreeCounter(void){return TPU6.TCNT;};
	bool_t isFreeCounterOvf(void){return TPU6.TSR.BIT.TCFV;};
	void clearFreeCounterOvfFlag(void){TPU6.TSR.BIT.TCFV = 0;};
	
	
	// 1ms���荞��
	void isr1ms(void);
	
private:
// private member
	bool_t fExeReq1ms;
	bool_t fExeReq10ms;
	bool_t fExeReq200ms;
	
	uint16_t Cnt10ms;
	uint16_t Cnt200ms;
	
	uint32_t Time_msec;	// �N������̎���
	
	// ������
	inrof_state State;
	int8_t fFinishManageAction;
	inrof_action_state ActionState;
	int8_t fFinishAction;
	
	
	// ���C���Z���T
	uint16_t LineSensAd[LINESENS_NUM_SENSE];	// ���C���Z���TAD�l
	
	
	//// �}�V����� ////
	
	// �o�b�e��
	uint8_t BattVoltage;		// �o�b�e���d�� [x10 V]
	bool_t fBattEmpty;		// �o�b�e���Ȃ���t���O
	void getBattVoltage(void);		// �o�b�e���d���擾(BattVoltage�ɃZ�b�g)
	
	//�I�h���g���֌W
	int16_t REncCntDiff, LEncCntDiff;	// �G���R�[�_�O��Ƃ̍�
	float RTirePos_r, LTirePos_r;		// �^�C���p�x[rev]
	float RTireSpd_rps, LTireSpd_rps;		// �^�C���p���x[rps]
	position MachinePos;				// �}�V���ʒu
	float Spd;							// �}�V�����x
	position LastFindCrossPos;			// �Ō�Ɍ�_�𔭌��������̃}�V���ʒu
	calc_odmetry CalcOdmetry;			// �I�h���g���v�Z�p
	inrof_orientation MachineOrientation;			// �}�V���̌���(4����)
	bool_t fAdjustMachinePos;
	
	// ���s���
	inrof_linedir LineTraceDir;			// ���s����
	/// ���s����
	ctrl_ratelimiter_abs* RateLimitSpdTgt;
	ctrl_ratelimiter_abs* RateLimitAngVelTgt;
	
	// manageAction�p
	position MachinePosActionStart;		// �}�V���ʒu ����J�n��
	int8_t ManageActionTansakuState_BallFind;	//	�T�����[�h��� �{�[���T���p 
	int8_t ManageActionTansakuState_BallFind_turn;	//	�T�����[�h��� �{�[���T�����̃^�[���p 
	int8_t ManageActionTansakuState_BallFind_last;	//	�T�����[�h��� �{�[���T���p 
	int8_t ManageActionTansakuState_BallRelease;	//	�T�����[�h��� �{�[��������p 
	
	bool_t BallFindEnable;					// �{�[���T���L��
	bool_t IsBallFinding;				// �{�[���������u�Ԃ�true
	inrof_linedir ManageActionLineTraceDir;			// ������������̑��s����
	
	//��_����
	bool_t IsCrossing;				// ��_����������
	inrof_cross_index Cross;	// �ǂ��̌����_���߂����Ƃ��납
	
	// �{�[���擾�����p
	bool_t BallDir;				// �{�[���̂������ 0:�E, 1:��
	position MachinePosActionManage;		// �}�V���ʒu ����J�n��
	position MachinePosActionManageSub;		// �}�V���ʒu ����J�n��
	float BallFindTh;		// �{�[���������p�x
	uint32_t StartTimeActionManage;		// ����J�n����
	int16_t BallFindDistance;		// �������{�[���̋���
	
	//�{�[������
	bool_t fJudgeColor;
	inrof_color BallColor;		// ����{�[���F
	inrof_color BallColorFixed;		// [�m��]����{�[���F
	bool_t BallColorLed;		// LED���点�Ă邩
	int16_t BallColorVal[3][2];	// �Z���T�l�L��
	
	
	// �t�B�[���h(static�ł����񂾂��ǁc)
	position PosCrossStart;
	position PosCrossRed;
	position PosCrossYellow;
	position PosCrossBlue;
	position PosCrossBallFieldBegin;
	position PosCrossBallFieldEnd;
	position PosCrossGoalRed;
	position PosCrossGoalYellow;
	position PosCrossGoalBlue;
	
// private function
	void step1ms(void);	// 1ms���Ə���
	void step10ms(void);	// 10ms���Ə���
	void step200ms(void);	// 200ms���Ə���
	
	void setup1msInterrupt(void);
	
/***********
 ���͏���
************/
	void updateLineSense(void);
	void updateEncoder(void);
	void updateBallColor(void);
	/*********************
	�^�C���p�����[�^�X�V
	�T�v�F
	�����F
	**********************/
	void updateTyre(void);

	/*********************
	�I�h���g���X�V
	�T�v�F
	�����F
	**********************/
	void updateOdmetry(void);
	
/***********
 ����
************/
	
	
	/*********************
	���s����
	�T�v�F
	 �ڕW���x�A�p���x�ɒǏ]����
	�����F
	 �ڕW���x[mmps]�A�p���x[rps]
	**********************/
	void ctrlRun(float SpdTgt_mmps, float AngVelTgt_rps, int32_t* RTireDutyTgt, int32_t* LTireDutyTgt);
	
	/*********************
	�����Z���T�l���u���[�o�����W��
	�T�v�F
	�����F
	**********************/
	void updatePsdObjPosGlobal(void);
	
	/*********************
	��_����
	�T�v�F
	�����F
	�ߒl�F�V���������_�ɗ���
	**********************/
	bool_t judgeCross(void);
	
	/*********************
	�{�[������������
	�T�v�F	�{�[��������������1�ɂȂ�B
			�����F�{�[���T���t�B�[���h��A���E�Z���T���ǂ��߂��l�A���E�΂߃Z���T���}�V���Ԃ��邭�炢�߂��l
	�����F
	�ߒl�F�V�����{�[����������
	**********************/
	bool_t judgeBallFind(void);
	
	
	/*********************
	���s��������
	�T�v�F	
	�����F�Ȃ�
	�ߒl�F�Ȃ�
	**********************/
	void judgeRunOrient(void);
	
	/*********************
	���W�␳
	�T�v�F���C���g���[�X���͎��@���W�␳����
	�����F�Ȃ�
	�ߒl�F�Ȃ�
	**********************/
	void adjustMachinePos(void);
	
	// ��ԊǗ�
	int8_t manageState(void);
	
	// ����Ǘ�
	// ����
	//  �ړI�n�Ƃ��T�����[�h�Ƃ�
	bool_t manageAction(inrof_state State);
	bool_t manageAction_tansaku(void);
	bool_t manageAction_goToGoal(inrof_color Color);
	bool_t manageAction_goToStart(void);
	
	
	//
	void setTgtVel_LineTrace(void);
	void setTgtVel_LineTrace(float SpdTgt);
	void setTgtVel_Turn(float SpdTgt, float Radius);
	
	
/***********
 �o��
************/
	/*********************
	���[�^�[�o��
	�T�v�F
	 PWM�l���o�͂���
	�����F
	 Duty
	**********************/
	void outTyres(int32_t RTireDutyTgt, int32_t LTireDutyTgt);
	
	
	
	
	
	
};


extern inrof2015 Inrof;
//extern SDMMC MMC;


#endif
