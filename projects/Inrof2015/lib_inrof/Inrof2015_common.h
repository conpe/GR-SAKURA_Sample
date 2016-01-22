/**************************************************
Inrof2015_common.h
	�m�\���{�R��2015�p�̃��C�u���������B
	
	
	
	
**************************************************/


#ifndef __INROF2015_COMMON_H__
#define __INROF2015_COMMON_H__

#include <math.h>


// �������
#define SAMPLETIME_CTRL_S 0.001	// ���쐧����� 1ms

// ����萔
//#define PI 3.1415

// �o�b�e����΂��d�� x10V
#define BATT_LOW 65	// 6.5V

// SdWav
#define SDWAV_MAXFILENAME 32


// ��{���x
#define VEL_NORMAL 200

// ���C���Z���T
// ���S�ʒu
#define LINESENS_POS_X	72.0	//��������72
#define LINESENS_POS_Y	0.0
// �ԊOLED�|�[�g
#define LINESENS_PORT_LED	GREX_IO7
// �Z���T��
#define LINESENS_NUM_SENSE	4			// �C���f�b�N�X�͍����珇��0, 1, 2, 3
// �Z���T臒l
#define LINESENS_THRESHOLD_SIDE 25000
#define LINESENS_THRESHOLD_CENTER 50000



// �����Z���T
#define PSDSENS_NUM 5
// ������E�O������Z���T
#define PSDSENS_LEFT_ADRS		0x78
#define PSDSENS_LEFT_POS_X		58.0
#define PSDSENS_LEFT_POS_Y		60.0
#define PSDSENS_LEFT_POS_TH		-30.0*PI/180.0
// ��������Z���T
#define PSDSENS_LEFTSIDE_ADRS	0x70		
#define PSDSENS_LEFTSIDE_POS_X	72.0
#define PSDSENS_LEFTSIDE_POS_Y	40.0
#define PSDSENS_LEFTSIDE_POS_TH	90.0*PI/180.0
// �^�񒆃Z���T
#define PSDSENS_FRONT_ADRS		0x60		
#define PSDSENS_FRONT_POS_X		90.0
//#define PSDSENS_FRONT_POS_X		48.0
#define PSDSENS_FRONT_POS_Y		0.0
#define PSDSENS_FRONT_POS_TH	0.0*PI/180.0
// �E������Z���T
#define PSDSENS_RIGHTSIDE_ADRS		0x58
#define PSDSENS_RIGHTSIDE_POS_X		72.0
#define PSDSENS_RIGHTSIDE_POS_Y		-40.0
#define PSDSENS_RIGHTSIDE_POS_TH	-90.0*PI/180.0
// �E���獶�O������Z���T
#define PSDSENS_RIGHT_ADRS		0x50	
#define PSDSENS_RIGHT_POS_X		58.0
#define PSDSENS_RIGHT_POS_Y		-60.0
#define PSDSENS_RIGHT_POS_TH	30.0*PI/180.0


// �J���[�Z���T
// ��LED�̃|�[�g
#define COLORSENS_PORT_LED	GREX_IO6

// �^�C���G���R�[�_�֌W
#define TIRE_TREAD_MM	176.7		// �g���b�h
#define TIRE_DIA_MM	56.0		// ���a
#define GEAR_RATIO	21.3		// �M�A�w�b�h�̌�����
#define ENC_CPR		1336		// �G���R�[�_���������̃J�E���g(4���{�ς�)
#define RTIRE_DIR_FW	CW
#define RTIRE_DIR_BK	CCW
#define LTIRE_DIR_FW	CW
#define LTIRE_DIR_BK	CCW


// �t�B�[���h�֌W
#define FIELD_INROF_CROSS_START_X 0.0
#define FIELD_INROF_CROSS_START_Y 0.0
#define FIELD_INROF_CROSS_RED_X 0.0
#define FIELD_INROF_CROSS_RED_Y 250.0
#define FIELD_INROF_CROSS_YELLOW_X 0.0
#define FIELD_INROF_CROSS_YELLOW_Y 900.0
#define FIELD_INROF_CROSS_BLUE_X 300.0
#define FIELD_INROF_CROSS_BLUE_Y 1550.0
#define FIELD_INROF_CROSS_BALLFIELD_BEGIN_X 1040.0
#define FIELD_INROF_CROSS_BALLFIELD_BEGIN_Y 1300.0
#define FIELD_INROF_CROSS_BALLFIELD_END_X 1040.0
#define FIELD_INROF_CROSS_BALLFIELD_END_Y 50.0
#define FIELD_INROF_CROSS_GOAL_RED_X -250.0
#define FIELD_INROF_CROSS_GOAL_RED_Y 250.0
#define FIELD_INROF_CROSS_GOAL_YELLOW_X -250.0
#define FIELD_INROF_CROSS_GOAL_YELLOW_Y 900.0
#define FIELD_INROF_CROSS_GOAL_BLUE_X -250.0
#define FIELD_INROF_CROSS_GOAL_BLUE_Y 2550.0

#define FIELD_INROF_CROSS_JUDGE_THRESHOLD	150		// �����_���������Ƃ��ɂ����܂ł̃Y���Ȃ狖�e����
#define FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR	400		// �����_���������Ƃ��ɂ����܂ł̃Y���Ȃ狖�e����(���̐��Ɨ���Ă�Ƃ���Ver)
// �{�[���͈�
#define FIELD_BALLFIELD_NORTHLIMIT	1300
#define FIELD_BALLFIELD_SOUTHLIMIT	100
#define FIELD_BALLFIELD_EASTLIMIT	1400
#define FIELD_BALLFIELD_WESTLIMIT	680

// �{�[���T��
#define BALLFIND_MAXDISTANCE_SIDE	270		// ���̋����ȉ��Ń{�[��������Ɣ��肷��
#define BALLFIND_MAXDISTANCE_SIDECLOSS	200		// ���̋����ȉ��Ń{�[��������Ɣ��肷��
#define BALLFIND_MAXDISTANCE_FRONT 250
//#define BALLFIND_MAXDISTANCE_SIDE	18		// ���̋����ȉ��Ń{�[��������Ɣ��肷��
//#define BALLFIND_MAXDISTANCE_SIDECLOSS	18		// ���̋����ȉ��Ń{�[��������Ɣ��肷��
//#define BALLFIND_MAXDISTANCE_FRONT 18

#define PSD_MIN 20							// PSD�Z���T��L���Ƃ݂Ȃ��ŏ��l

// �{�[���ߊl
#define BALLGET_DISTANCE_FRONT 130			// �t�����g�Z���T���炱�̋�����������ăA�[�����낷

// �T�[�{�ʒu
//#define SERVOPOS_START_F 0
//#define SERVOPOS_START_R -900
#define SERVOPOS_RUN_F 0
#define SERVOPOS_RUN_R -50
#define SERVOPOS_BALLGET_F -940
#define SERVOPOS_BALLGET_R 400
#define SERVOPOS_BALLUP_F -550
#define SERVOPOS_BALLUP_R 300
#define SERVOPOS_BALLTRASH_F -900
#define SERVOPOS_BALLTRASH_R -200	// �T�[�{�I�t�ɂ���̂ŏd�v�ł͂Ȃ�
#define SERVOPOS_COLORCHECK_F 450
#define SERVOPOS_COLORCHECK_R -550	//-600








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
