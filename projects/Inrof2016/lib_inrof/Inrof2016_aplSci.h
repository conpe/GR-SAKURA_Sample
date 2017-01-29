/**************************************************
inrof2016_aplSci.h
	�m�\���{�R��2016�p�̃��C�u���������B
  SCI�֘A�����N���X
	
	
	
	
**************************************************/


#ifndef __INROF2016_APLSCI_H__
#define __INROF2016_APLSCI_H__

#include "SCI.h"


/****************************
 �m�\���{�R��2016�p
  SCI�֘A�����N���X
�T�v�F
	�O���Ƃ̂������Ǘ��B
	PC�ʐM���[�h��PS3�R���ʐM���[�h
****************************/
#define SBDBT_PS3CON_DATALENGTH 57
#define SBDBT_PS3CON_DISABLECNT 25	// task����ŃR���g���[������𖳌����肷�邩

//class inrof2016_aplSci_t:public Sci_t{
class inrof2016_aplSci_t{
	
public :
	enum mode_t{
		PC = 1,
		PC_THROUGH,	// PC����̒ʐM�����̂܂܎g��
		PS3		// PS3�R������̏��Ƃ��Ďg��
	};
	
	union ps3data_t{
		uint8_t Data[SBDBT_PS3CON_DATALENGTH];
		struct{
			uint8_t Rcb3_Head;		// 0�o�C�g��
			uint8_t Rcb3_Square	:1;	// 1�o�C�g��
			uint8_t Rcb3_L1	:1;
			uint8_t Rcb3_L2	:1;
			uint8_t Rcb3_R1	:1;
			uint8_t Rcb3_R2	:1;
			uint8_t Rcb3_Dummy0	:3;
			uint8_t Rcb3_Up	:1;	// 2�o�C�g��
			uint8_t Rcb3_Down	:1;		// �㉺������1�Ȃ�Start
			uint8_t Rcb3_Right	:1;
			uint8_t Rcb3_Left	:1;		// ���E�����Ƃ�1�Ȃ�Select
			uint8_t Rcb3_Triangle	:1;
			uint8_t Rcb3_Cross	:1;
			uint8_t Rcb3_Circle	:1;
			uint8_t Rcb3_Dummy1	:1;
			uint8_t Rcb3_LStickHorizontal;	// 3�o�C�g��	�Z���^�[ 0x40
			uint8_t Rcb3_LStickVertical;	// 4�o�C�g��
			uint8_t Rcb3_RStickHorizontal;	// 5�o�C�g��
			uint8_t Rcb3_RStickVertical;	// 6�o�C�g��
			uint8_t Rcb3_CheckSum;		// 7�o�C�g��
			uint8_t ReportId;		// 8�o�C�g��
			uint8_t Dummy0;			// 9�o�C�g��
			uint8_t Select	:1;		// 10�o�C�g��
			uint8_t L3	:1;
			uint8_t R3	:1;
			uint8_t Start	:1;
			uint8_t Up	:1;
			uint8_t Right	:1;
			uint8_t Down	:1;
			uint8_t Left	:1;
			uint8_t L2	:1;		// 11�o�C�g��
			uint8_t R2	:1;
			uint8_t L1	:1;
			uint8_t R1	:1;
			uint8_t Triangle	:1;
			uint8_t Circle	:1;
			uint8_t Cross	:1;
			uint8_t Square	:1;
			uint8_t Playstation	:1;	// 12�o�C�g��
			uint8_t Dummy12	:7;
			uint8_t Dummy13;		//13�o�C�g��
			uint8_t LStickHrizontal;	//14�o�C�g��	�Z���^�[ 0x80
			uint8_t LStickVertical;		//15�o�C�g��
			uint8_t RStickHrizontal;	//16�o�C�g��
			uint8_t RStickVertical;		//17�o�C�g��
			uint8_t Dummy18;		//18�o�C�g��	// ���炩�̃f�[�^�ł͂�����ۂ�
			uint8_t Dummy19;		//19�o�C�g��
			uint8_t Dummy20;		//20�o�C�g��
			uint8_t Dummy21;		//21�o�C�g��
			uint8_t AnalogUp;		//22�o�C�g��
			uint8_t AnalogRight;		//23�o�C�g��
			uint8_t AnalogDown;		//24�o�C�g��
			uint8_t AnalogLeft;		//25�o�C�g��
			uint8_t AnalogL2;		//26�o�C�g��
			uint8_t AnalogR2;		//27�o�C�g��
			uint8_t AnalogL1;		//28�o�C�g��
			uint8_t AnalogR1;		//29�o�C�g��
			uint8_t AnalogTriangle;		//30�o�C�g��
			uint8_t AnalogCircle;		//31�o�C�g��
			uint8_t AnalogCross;		//32�o�C�g��
			uint8_t AnalogSquare;		//33�o�C�g��
			uint8_t Dummy34;		//34�o�C�g��
			uint8_t Dummy35;		//35�o�C�g��
			uint8_t Dummy36;		//36�o�C�g��
			uint8_t Status;		//37�o�C�g��
			uint8_t Batt0;		//38�o�C�g��
			uint8_t Batt1;		//39�o�C�g��
			uint8_t Dummy40;	//40�o�C�g��
			uint8_t ComLevel;	//41�o�C�g��
			uint8_t Dummy42;	//42�o�C�g��
			uint8_t Dummy43;	//43�o�C�g��
			uint8_t Dummy44;	//44�o�C�g��
			uint8_t Dummy45;	//45�o�C�g��
			uint8_t Dummy46;	//46�o�C�g��
			uint8_t MotorSmall	:1;	//47�o�C�g��
			uint8_t Dummy47		:7;	
			uint8_t Dummy48		:7;	//48�o�C�g��	
			uint8_t MotorBig	:1;
			uint16_t Gyro0;		//49,50�o�C�g��
			uint16_t Gyro1;		//51,52�o�C�g��
			uint16_t Gyro2;		//53,54�o�C�g��
			uint16_t Gyro3;		//55,56�o�C�g��	//�W���C���Ɖ����x�Ǝv����
		}button;
	};
	
public:
	ps3data_t ControllerData;
	
	
	inrof2016_aplSci_t(Sci_t *SciObject);
	// �ʐM�f�[�^���
	// ���[�h�ɉ����Ď�M�o�b�t�@�����߂���
	// ����I�ɌĂяo������
	void task(void);
	
	// ���[�h���Z�b�g
	void setMode(mode_t NewMode);
	// ���݃��[�h���擾
	mode_t getMode(void){return Mode;}
	
	// PS3�R���g����H
	bool_t isPs3ConAvailable(void){return Ps3ConAvailable;};
	
private:
	Sci_t *Sci;
	mode_t Mode;
	bool_t Ps3ConAvailable;
	uint8_t Ps3ConDisableCnt;
	
};



#endif
