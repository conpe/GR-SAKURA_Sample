/**************************************************
inrof2016_lib.h
	�m�\���{�R��2016�p�̃��C�u���������B
	
	
	
	
**************************************************/


#ifndef __INROF2016_H__
#define __INROF2016_H__

//#include <sdmmc.h>

#include "GR_define.h"
#include "inrof2016_common.h"

#include "AppliCtrl.h"

//#include "RIIC.h"
//#include "SCI.h"

#include "inrof2016_upper_ctrl.h"
#include "Inrof2016_aplSci.h"


#include "inrof2016_ball_sensor.h"
#include "inrof2016_sharp_dist.h"
#include "inrof2016_st_vl.h"
#include "inrof2016_color_sens.h"

#include "RoombaOi.h"

//#include "FutabaCmdServo.h"
//#include "OledDriverSSD1306.h"
//#include "SharpGP2Y0E.h"
#include "HamamatsuColorSensS11059.h"
#include "InvensenseMPU.h"


// define
//#define PI 3.1415



typedef struct calc_odmetry{
	float REncCntDiff[2];
	float LEncCntDiff[2];
	float CosTh[2];
	float SinTh[2];
	float dth2[2];
}calc_odmetry;

enum inrof_state{
	INROF_INITIALIZE,	// ��������
	INROF_IDLE,
	INROF_TANSAKU,
	INROF_TOGOAL,
	INROF_TOSTART,
	INROF_LINECALIB		// �L�����u���[�V����
};
// ���s���[�h
enum inrof_drive_mode{
	INROF_DRIVE_AUTO,	// ����
	INROF_DRIVE_PS3		// PS3�R���BSCI0����̃f�[�^�����̂܂ܗ���
};

enum inrof_action_state{
	INROF_ACTION_IDLE,		// 0
	INROF_ACTION_LINE,		// 1
	INROF_ACTION_STRAIGHT,		// 2
	INROF_ACTION_TURN,		// 3
	INROF_ACTION_BALLGET,		// 4
	INROF_ACTION_BALLRELEASE,	// 5
	INROF_ACTION_OVERBALL,		// 6 �{�[�����񂾂̂ŉ������čĊJ
	INROF_ACTION_TOFINISH		// 7 �X�^�[�g�]�[���߂������Ƃ̏���
};
/*
enum inrof_action_state{
}
*/
// ���s����
enum inrof_linedir{
	INROF_LINEDIR_NONE,
	INROF_LINEDIR_FORWARD,
	INROF_LINEDIR_BACKWARD
};

enum inrof_cross_index{
	INROF_CROSS_NONE	= 0,	// 0
	INROF_CROSS_START	= 1,	// 1
	INROF_CROSS_RED		= 2,	// 2
	INROF_CROSS_YELLOW	= 3,	// 3
	INROF_CROSS_BLUE	= 4,	// 4
	INROF_CROSS_BALLFIELD_BEGIN	= 5,	// 5
	INROF_CROSS_BALLFIELD_END	= 6,	// 6
	
	INROF_CROSS_GOAL_RED,			// 7
	INROF_CROSS_GOAL_YELLOW,		// 8
	INROF_CROSS_GOAL_BLUE,			// 9
	
	INROF_CROSS_BLUE_EDGE			// 10 (�S�[���̑O�B���C��������)
	
};

enum inrof_orientation{
	ORIENT_NONE, 
	ORIENT_EAST,
	ORIENT_NORTH,
	ORIENT_WEST,
	ORIENT_SOUTH,
};


/****************************
 �m�\���{�R��2016

�T�v�F
 ���������1ms, 10ms�B
 ���C���֐��Ŏg����悤�A200msec���Ƃɗ��t���O(fCnt200)��p��
 
****************************/
class inrof2016{
public:
	// �����o
	RoombaOi* Rmb;
	// �ʐM
	//inrof2016_aplSci_t* ExtSci;	// SBDBT��Ƃ̒ʐM(PC, PS3�R��)
	
	// �ʐM�n���
	// �����o
	// I2C
	int8_t ComuStatusI2C;
	// �T�[�{
	int8_t ComuStatusSv;
	
	// �t�B�[���h
	// �{�[���̎c�萔
	uint8_t FieldBall;
	
	// �Z���T
	// �����Z���T
	//inrof2016_ball_sensor_t *PsdLeft;
	//inrof2016_ball_sensor_t *PsdCenter;
	//inrof2016_ball_sensor_t *PsdRight;
	inrof2016_st_vl_t *PsdCenter;
	inrof2016_st_vl_t *PsdRight;
	inrof2016_st_vl_t *PsdLeft;
	void resetBallSensors(void);
	
	// �J���[
	//hamamatsu_S11059* ColorSens;
	
	// IMU
	invensense_MPU* Mpu;
	// �G���R�[�_
	gr_ex_encoder* EncR;
	gr_ex_encoder* EncL;
	
	
	bool_t fCnt200;
	// �ڕW���x, �p���x
	float SpdTgt, AngVelTgt;	//private�ɂ�������
	float SpdTgtRated, AngVelTgtRated;	//private�ɂ�������
	
	
	// ������
	inrof2016(void);
	void begin(void);	// �I�u�W�F�N�g����
	void initialize(void);	// �t���O�n���Z�b�g
	
	// ���[�h
	void setMode(inrof_drive_mode NewMode);
	inrof_drive_mode getMode(void){return DriveMode;};
	
	// ����
	void step(void);
	
	// ���n
	void enableOutTyre(void){this->fMotorOut = true; InrofUpper.enableAction();};
	void disableOutTyre(void){this->fMotorOut = false; InrofUpper.disableAction();};
	bool_t isEnableOutTyre(void){return this->fMotorOut;};
	
	// ���C���Z���T
	uint16_t getLineSensAd(uint8_t LineSensNum){return LineSensAd[LineSensNum];};
	void getLineSenseGlobalPos(position* Pos);
	//�}�V����Ԏ擾
	float getRTireSpd(void){return RTireSpd_rps;};
	float getLTireSpd(void){return LTireSpd_rps;};
	position* getMachinePos(void){return &MachinePos;};
	//�����o�̃o�b�e���[����H
	bool_t isBattLow(void){return (BATT_LOW_RMBSOC > Rmb->getBatterySoc())&&(0!=Rmb->getBatterySoc());};	//0����Ȃ���LOW_SOC��菬����
	
	void setState(inrof_state State){ this->State = State; ActionState = INROF_ACTION_IDLE;};
	int8_t getState(void){return State;};
	
	
	// ���[�e�B���e�B
	uint32_t getTime_ms(void){return Time_msec;};	// �N������̎���
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
	// ����萧����
	inrof_drive_mode DriveMode;	// �v���O�����œ��������APS3�R���œ�������
	
	bool_t fExeReqTask0;
	bool_t fExeReqTask1;
	bool_t fExeReqTask2;
	
	uint16_t CntTask1;
	uint16_t CntTask2;
	
	uint32_t Time_msec;	// �N������̎���
	
	// ������
	inrof_state State;
	inrof_state StateLast;
	int8_t fFinishManageAction;
	inrof_action_state ActionState;
	inrof_action_state ActionStateLast;
	int8_t fFinishAction;
	bool_t fNextAction;	// ���̏�Ԃֈڂ�t���u
	
	// �������Ă�ꏊ
	inrof_goal Goal;
	
	// ���[�^�[�o�͋���
	bool_t fMotorOut;
	
	// ���C���Z���T
	uint16_t LineSensAd[LINESENS_NUM_SENSE];	// ���C���Z���TAD�l
	
	
	//// �}�V����� ////
	
	// �o�b�e��
	uint8_t BattVoltage;		// �o�b�e���d�� [x10 V]
	bool_t fBattEmpty;		// �o�b�e���Ȃ���t���O
	void getBattVoltage(void);		// �o�b�e���d���擾(BattVoltage�ɃZ�b�g)
	
	//�I�h���g���֌W
	uint16_t REncCnt, LEncCnt;	// �G���R�[�_�l
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
	int16_t RTireDutyTgt, LTireDutyTgt;		// ����duty
	ctrl_pid* PidCtrlRTire;				// �^�C��PID
	ctrl_pid* PidCtrlLTire;				//  ���x���f���[�e�B�[
	ctrl_ratelimiter_abs* RateLimitSpdTgt;		// �ԑ̑��x���[�g���~�b�^ [mm/s/s]
	ctrl_ratelimiter_abs* RateLimitAngVelTgt;	// �ԑ̉�]���x���[�g���~�b�^ [rad/s/s]
	
	// manageAction�p
	position MachinePosActionStart;		// �}�V���ʒu ����J�n��
	int8_t ManageActionTansakuState_BallFind;	//	�T�����[�h��� �{�[���T���p 
	int8_t ManageActionTansakuState_BallFind_Last;	//	 �O��l
	int8_t ManageActionTansakuState_BallFind_turn;	//	�T�����[�h��� �{�[���T�����̃^�[���p 
	int8_t ManageActionTansakuState_BallFind_turn_Last;	//	�T�����[�h��� �{�[���T���p 
	int8_t ManageActionTansakuState_BallRelease;	//	�T�����[�h��� �{�[��������p 
	int8_t ManageActionTansakuState_BallReleaseLast;	//	�T�����[�h��� �{�[��������p 
	int8_t ManageActionTansakuState_BallOver;	//	�T�����[�h��� �{�[���T���p 
	int8_t ManageActionTansakuState_BallOver_Last;	//	 �O��l
	int8_t ManageActionToStartState_ToFinish;	//	�X�^�[�g�֖߂�
	int8_t ManageActionToStartState_ToFinish_Last;	//	 �O��l
	
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
	
	
	// �{�[���Z���T
	// ������
	void beginBallSensor(void);	// �J�n
	void initBallSensor(void);	// ������(I2C�A�h���X�ݒ�)
	
	
	
	
	
	// �l�^
	// ���|��
	bool_t fCleaning;
	int8_t startClean(void){fCleaning = true; return Rmb->sendCommand(ROI_OPCODE_MOTORS, (uint8_t)0x03);};	// Vacume, SideBrush
	int8_t stopClean(void){fCleaning = false; return Rmb->sendCommand(ROI_OPCODE_MOTORS, (uint8_t)0x00);};
	bool_t isCleaning(void){return fCleaning;};
	
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
	void stepTask0(void);	// 1ms���Ə���
	void stepTask1(void);	// 10ms���Ə���
	void stepTask2(void);	// 200ms���Ə���
	
	void setup1msInterrupt(void);
	
/***********
 ���͏���
************/
	/*********************
	���C���Z���T�X�V
	�T�v�FAD�ϊ��l���Z�b�g
	**********************/
	void updateLineSense(void);
	/*********************
	�G���R�[�_�X�V
	�T�v�F	�G���R�[�_�̑O��l�Ƃ̍������Z�b�g
		������s���͍���0���Z�b�g����
	**********************/
	void updateEncoder(void);
	bool_t fResetEncoder;	// �����Ă�ԍ���0�ɂȂ�
	
	
	/*********************
	�^�C���p�����[�^�X�V
	�T�v�F	�p�x[rev]�A�p���x[rps]
	�����F
	**********************/
	void updateTyre(void);

	/*********************
	�I�h���g���X�V
	�T�v�F
	�����F
	�����o�ϐ��FMachinePos,MachineOrientation,Spd,CalcOdmetry
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
	void ctrlRun(float SpdTgt_mmps, float AngVelTgt_rps, int16_t* RTireDutyTgt, int16_t* LTireDutyTgt);
	
	/*********************
	�����Z���T�l���u���[�o�����W��
	�T�v�F
	�����F
	**********************/
	void updatePsdObjPosGlobal(void);
	
	/*********************
	��_����
	�T�v�F��_�ɂ���
	�����F
	�ߒl�F��_�ɂ��邩
	**********************/
	bool_t onCross(void);
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
	bool_t BallExist;
	
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
	bool_t manageAction_goToGoal(void);
	bool_t manageAction_goToStart(void);
	bool_t manageAction_calibLine(void);
	
	// ���C���g���[�X
	ctrl_pid* PidCtrlLineTrace;
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
	void outTyres(int16_t RTireDutyTgt, int16_t LTireDutyTgt, bool_t fMotorOut);
	
	
/***********
 �}�V�����
************/
	
	
	bool_t checkInitialize(void);
	
	
/***********
 PS3�R���֌W
************/
	inrof2016_aplSci_t::ps3data_t Ps3ConDataLast;
	uint8_t Ps3ConArmMode;
	uint8_t ArmPosTgt;
	int16_t GrabAngleTgt;
	int16_t PitchAngleTgt;
	void drivePs3Con(void);
	
	
};


extern inrof2016 Inrof;
extern inrof2016_aplSci_t ExtSci;


#endif
