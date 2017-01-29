/**************************************************
inrof2016_common.h
	�m�\���{�R��2015�p�̃��C�u���������B
	
	
	
	
**************************************************/


#ifndef __inrof2016_COMMON_H__
#define __inrof2016_COMMON_H__


#define DEBUGSCI0		// �������̃f�o�b�O���[�b�Z�[�W
//#define DEBUGSCI0_UPPER	// �㔼�g�̃f�o�b�O���b�Z�[�W


#include <math.h>

#include "portReg.h"
#include "GR_Ex15_v3.h"
#include "GR_Ex15_OLED.h"

// inrof_lib�ł�inrof_upper�ł���������
#include "aplMemCtrl.h"


// �������
#define SAMPLETIME_CTRL_S 0.020	// ���쐧����� 20ms

// ����萔
#define PI 3.1415

// �o�b�e����΂��d�� x10V
#define BATT_LOW 65	// 6.5V
#define BATT_LOW_RMBSOC 60	// �����o��SOC 60%

// SdWav
#define SDWAV_MAXFILENAME 32


// ��{���x
#define VEL_NORMAL 200

// ���C���Z���T
// ���S�ʒu
#define LINESENS_POS_X	110.0F	// �^�C���ʒu����
#define LINESENS_POS_Y	0.0F
// �ԊOLED�|�[�g
#define LINESENS_PORT_LED	GREX_IO7
// �Z���T��
#define LINESENS_NUM_SENSE	4	// �C���f�b�N�X�͍����珇��0, 1, 2, 3
					// AD�|�[�g��0�`3�ɑΉ�	
// �Z���T臒l
//#define LINESENS_THRESHOLD_SIDE 40000
//#define LINESENS_THRESHOLD_CENTER 40000
#define LINESENS_THRESHOLD_SIDE 550
#define LINESENS_THRESHOLD_CENTER 500



// �����Z���T
//#define PSDSENS_NUM 3
// ������E�O������Z���T
#define PSDSENS_LEFT_ADRS		0x25		// �����ڑ����ɐ���ɐݒ�ł��Ȃ��Ȃ邽�߃f�t�H���g�l(0x29)�Ɠ����ɂ��Ȃ�����
#define PSDSENS_LEFT_POS_X		175.0F
#define PSDSENS_LEFT_POS_Y		60.0F
#define PSDSENS_LEFT_POS_TH		-45.0F*PI/180.0F
#define PSDSENS_LEFT_ENABLEPIN 		GREX_IO4		// �Ԃ���
// ��������Z���T
/*
#define PSDSENS_LEFTSIDE_ADRS	0x70		
#define PSDSENS_LEFTSIDE_POS_X	72.0F
#define PSDSENS_LEFTSIDE_POS_Y	40.0F
#define PSDSENS_LEFTSIDE_POS_TH	90.0F*PI/180.0F
*/
// �^�񒆃Z���T
#define PSDSENS_FRONT_ADRS		0x26		// �����ڑ����ɐ���ɐݒ�ł��Ȃ��Ȃ邽�߃f�t�H���g�l�Ɠ����ɂ��Ȃ�����
#define PSDSENS_FRONT_POS_X		180.0F
//#define PSDSENS_FRONT_POS_X		48.0F
#define PSDSENS_FRONT_POS_Y		0.0F
#define PSDSENS_FRONT_POS_TH		0.0F
#define PSDSENS_FRONT_ENABLEPIN 	GREX_IO5	// ������

// �E������Z���T
/*
#define PSDSENS_RIGHTSIDE_ADRS		0x58
#define PSDSENS_RIGHTSIDE_POS_X		72.0F
#define PSDSENS_RIGHTSIDE_POS_Y		-40.0F
#define PSDSENS_RIGHTSIDE_POS_TH	-90.0F*PI/180.0F
*/
// �E���獶�O������Z���T
#define PSDSENS_RIGHT_ADRS		0x27		// �����ڑ����ɐ���ɐݒ�ł��Ȃ��Ȃ邽�߃f�t�H���g�l�Ɠ����ɂ��Ȃ�����
#define PSDSENS_RIGHT_POS_X		175.0F
#define PSDSENS_RIGHT_POS_Y		-60.0F
#define PSDSENS_RIGHT_POS_TH	45.0F*PI/180.0F
#define PSDSENS_RIGHT_ENABLEPIN 	GREX_IO6	// �΂̐�


// �J���[�Z���T
// ��LED�̃|�[�g
//#define COLORSENS_PORT_LED	GREX_IO6

// �A�[���}�C�R��
#define ARM_I2C_ADDRESS 0x09

// IMU(MPU9250)
#define IMU_I2C_ADDRESS 0x68


// �����o
// �^�C���G���R�[�_�֌W
//#define TIRE_TREAD_MM	235.0F		// �g���b�h
#define TIRE_TREAD_MM	248.0F		// �g���b�h
//#define TIRE_DIA_MM	70.0F		// ���a
#define TIRE_DIA_MM	72.0F		// ���a
#define GEAR_RATIO	1.0F		// �M�A�w�b�h�̌�����
#define ENC_CPR		490.0F		// �G���R�[�_���������̃J�E���g

// �ő�l
#define RMB_ANGVEL_MAX 4.0F
#define RMB_VEL_MAX	400.0F

#define RMB_VEL_NORMAL	200.0F

#define SERVO_GOALTIME_NORMAL 800

#define ARM_MOTOR_CW	CW
#define ARM_MOTOR_CCW	CCW


// �t�B�[���h�֌W
#define FIELD_INROF_CROSS_START_X 0.0F
#define FIELD_INROF_CROSS_START_Y 0.0F
#define FIELD_INROF_CROSS_RED_X 0.0F
#define FIELD_INROF_CROSS_RED_Y 250.0F
#define FIELD_INROF_CROSS_YELLOW_X 0.0F
#define FIELD_INROF_CROSS_YELLOW_Y 900.0F
#define FIELD_INROF_CROSS_BLUE_X 300.0F
#define FIELD_INROF_CROSS_BLUE_Y 1550.0F
#define FIELD_INROF_CROSS_BALLFIELD_BEGIN_X 1040.0F
#define FIELD_INROF_CROSS_BALLFIELD_BEGIN_Y 1300.0F
#define FIELD_INROF_CROSS_BALLFIELD_END_X 1040.0F
#define FIELD_INROF_CROSS_BALLFIELD_END_Y 50.0F
#define FIELD_INROF_CROSS_GOAL_RED_X -250.0F
#define FIELD_INROF_CROSS_GOAL_RED_Y 250.0F
#define FIELD_INROF_CROSS_GOAL_YELLOW_X -250.0F
#define FIELD_INROF_CROSS_GOAL_YELLOW_Y 900.0F
#define FIELD_INROF_CROSS_GOAL_BLUE_X -250.0F
#define FIELD_INROF_CROSS_GOAL_BLUE_Y 1550.0F
#define FIELD_INROF_CROSS_BLUE_EDGE_X 0.0F
#define FIELD_INROF_CROSS_BLUE_EDGE_Y 1550.0F

#define FIELD_INROF_CROSS_JUDGE_THRESHOLD	150		// �����_���������Ƃ��ɂ����܂ł̃Y���Ȃ狖�e����
#define FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR	400		// �����_���������Ƃ��ɂ����܂ł̃Y���Ȃ狖�e����(���̐��Ɨ���Ă�Ƃ���Ver)
// �{�[���͈�
#define FIELD_BALLFIELD_NORTHLIMIT	1300
#define FIELD_BALLFIELD_SOUTHLIMIT	100
#define FIELD_BALLFIELD_EASTLIMIT	1400
#define FIELD_BALLFIELD_WESTLIMIT	680

// �{�[���T��
//#define BALLFIND_MAXDISTANCE_SIDE	270		// ���̋����ȉ��Ń{�[��������Ɣ��肷��
//#define BALLFIND_MAXDISTANCE_SIDECLOSS	300		// ���̋����ȉ��Ń{�[��������Ɣ��肷��
#define BALLFIND_MAXDISTANCE_SIDE	200		// ���̋����ȉ��Ń{�[��������Ɣ��肷��
#define BALLFIND_MAXDISTANCE_SIDECLOSS	200		// ���̋����ȉ��Ń{�[��������Ɣ��肷��
#define BALLFIND_MAXDISTANCE_FRONT 	100
//#define BALLFIND_MAXDISTANCE_SIDE	18		// ���̋����ȉ��Ń{�[��������Ɣ��肷��
//#define BALLFIND_MAXDISTANCE_SIDECLOSS	18		// ���̋����ȉ��Ń{�[��������Ɣ��肷��
//#define BALLFIND_MAXDISTANCE_FRONT 18

#define PSD_MIN 10							// PSD�Z���T��L���Ƃ݂Ȃ��ŏ��l


// �{�[��
#define FIELD_BALL_NUM_INIT 3	// �t�B�[���h��̃{�[��

// �{�[���ߊl
#define BALLGET_MAXDISTANCE_FRONT 	200.0F		// �{�[���m�ے��̍ő勗��
#define BALLGET_DISTANCE_FRONT_ARMDOWN	170.0F		// �t�����g�Z���T���炱�̋�����������ăA�[�����낷
#define BALLGET_DISTANCE_FRONT_GRAB	90.0F		// �t�����g�Z���T���炱�̋�����������Ē͂�

// �T�[�{�ʒu
// �ő�p�x
#define SERVOPOS_MAX_G 500
#define SERVOPOS_MIN_G -150
#define SERVOPOS_MAX_P 280
#define SERVOPOS_MIN_P -900
// ��{�ʒu
#define SERVOPOS_START_G 80		// �����ʒu
#define SERVOPOS_START_P -200
#define SERVOPOS_RUN_G 80		// �ړ����ʒu
#define SERVOPOS_RUN_P -200		// (�����Ă�{�[���A�t�B�[���h�̃{�[���ɓ�����Ȃ�)
// �{�[���ߊl
#define SERVOPOS_BALLGET0_G 400		// �{�[���ߊl �J��	// ����̃{�[���ɓ�����Ȃ��悤�ɋ��߂����ǁA�ʒu���܂������܂�Ȃ��悤�Ȃ�����ƊJ��
#define SERVOPOS_BALLGET0_P -750	// 850���Ɖ��C��������ē����Ȃ��Ȃ�
#define SERVOPOS_BALLGET1_G 80		// �{�[���ߊl �͂�
#define SERVOPOS_BALLGET1_P -750	// 850���Ɖ��C��������ē����Ȃ��Ȃ�
// �{�[���ߊl��
#define SERVOPOS_BALLUP0_G 80		// �{�[���ߊl �����グ��(�ړ���)
#define SERVOPOS_BALLUP0_P -200
#define SERVOPOS_BALLUP1_G 80		// �{�[���ߊl �u���ʒu
#define SERVOPOS_BALLUP1_P -200
#define SERVOPOS_BALLUP2_G 80		// �{�[���ߊl �R�����b
#define SERVOPOS_BALLUP2_P 250
#define SERVOPOS_BALLUP3_G -150		// �{�[������
#define SERVOPOS_BALLUP3_P 200
// �{�[�����̂Ăɍs��
#define SERVOPOS_BALLTRASH0_G -150	// �}�V����̂����ވʒu
#define SERVOPOS_BALLTRASH0_P 280	
#define SERVOPOS_BALLTRASH1_G 80		// �͂�
#define SERVOPOS_BALLTRASH1_P 280
#define SERVOPOS_BALLTRASH2_G 80		// �����グ���
#define SERVOPOS_BALLTRASH2_P -200
#define SERVOPOS_BALLTRASH3_G 0		// �̂Ă�
#define SERVOPOS_BALLTRASH3_P -900


enum inrof_color{
	INROF_NONE,
	INROF_RED,
	INROF_YELLOW,
	INROF_BLUE
};

enum inrof_goal{
	INROF_GOAL_NONE,
	INROF_GOAL_RED,
	INROF_GOAL_YELLOW,
	INROF_GOAL_BLUE
};





// �ʒu�����̃N���X��
// �ʒu�Ƒ��x�̃N���X��
// ��]�n�����킹���N���X��肽��

class position{
public:
	float X, Y;
	float dX, dY;
	float Th;			// [rad]
	float dTh;
	
	// �����Z�� ������Ԃ�
	float operator - (position Pos){
		return sqrtf(powf((this->Y - Pos.Y), 2) + powf((this->X - Pos.X), 2));
	}
	
	// +-PI[rad]�ɐ��K��
	float getNormalizeTh(void){	return normalize(Th); };
private:
	float normalize(float Th){
		/*
		if(Th > 3.1415){
			return normalize(Th - 2*3.1415);
		}else if(Th< -3.1415){
			return normalize(Th + 2*3.1415);
		}
		*/
		while(Th > 3.1415){
			Th -= (2*3.1415);
		}
		while(Th < -3.1415){
			Th += (2*3.1415);
		}


		return Th;
	}
};




#endif
