/********************************************/
/*		�l���t�H�g�j�N�X					*/
/*			�f�W�^���J���[�Z���TS11059(I2C)	*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/03/12		*/
/********************************************/

// wakeup()���āAstartMeasurement()���āA����I��updateSens()����B
// getLuminance()�ő���l�擾�B

// I2C�A�h���X 0x2A(�Œ�)


//�y�X�V�����z
// 2015.03.12 �V�K�쐬
// 2015.06.29 I2C�ʐM���e�N���X�Ή�


#ifndef __HAMAS11059_H__
#define __HAMAS11059_H__

#include "RIIC.h"
#include "CommonDataType.h"

#define HMS11059_I2CADRS 0x2A


// ���W�X�^
#define  HMS11059_REG_CONTROL		0x00
#define  HMS11059_REG_MANUALTIMING	0x01
#define  HMS11059_REG_SENSRED		0x03
#define  HMS11059_REG_SENSGREEN		0x05
#define  HMS11059_REG_SENSBLUE		0x07
#define  HMS11059_REG_SENSIR		0x09



enum mhs11059_comu_content{
	SET_CTRL,
	SET_MANUALTIMING,
	GET_SENS,
};

class mhs11059_comus_t;

class hamamatsu_S11059{
private:
	I2c_t *I2Cn;
	uint8_t I2cAddress;		// I2C�A�h���X
	int16_t SensRed, SensGreen, SensBlue, SensIr;	// �v�������Z���T�l
	bool_t fMeasurement;	// �v�����t���O
	bool_t fRepeatMeasurement;	// �v���J��Ԃ��t���O
	bool_t fI2cErr;			// I2C�ʐM�G���[ ->����M��߂�
	
	// I2C�N���X�ɒʐM���e���A�^�b�`
	int8_t attachI2cComu(mhs11059_comu_content ComuType, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum);
	mhs11059_comus_t* LastAttachComu;	// �Ō�ɒʐM�v�������ʐM
	bool_t fLastAttachComuFin;		// �Ō�ɒʐM�v�������ʐM���I��������t���O
public:
	// �R���X�g���N�^
	hamamatsu_S11059(I2c_t* I2Cn);
	// �f�X�g���N�^
	~hamamatsu_S11059(void);
	// ����J�n
	void begin(void);
	
	// �v�����ʎ擾
	int8_t getLuminance(int16_t* LuminanceRed, int16_t* LuminanceGreen, int16_t* LuminanceBlue);
	int8_t getLuminance(int16_t* LuminanceRed, int16_t* LuminanceGreen, int16_t* LuminanceBlue, int16_t* LuminanceIr);
	int8_t getLuminanceRed(int16_t* LuminanceRed);
	int16_t getLuminanceRed(void){return SensRed;};
	int8_t getLuminanceGreen(int16_t* LuminanceGreen);
	int16_t getLuminanceGreen(void){return SensGreen;};
	int8_t getLuminanceBlue(int16_t* LuminanceBlue);
	int16_t getLuminanceBlue(void){return SensBlue;};
	int8_t getLuminanceIr(int16_t* LuminanceIr);
	int16_t getLuminanceIr(void){return SensIr;};
	
	// �e��ʐM
	// 1, �Z���T�l�ǂݎ��
	int8_t updateSens(void);
	// 2, ���W���[�����N����
	int8_t wakeup(void);
	// 3, �v���J�n
	int8_t startMeasurement(void);
	// 4, �v����~
	int8_t stopMeasurement(void);
	
	
	// ���荞�ݗp
	// I2C��M�f�[�^���擾���ĒʐM���e���Ƃɏ��������
	int8_t fetchI2cRcvData(const mhs11059_comus_t* Comu);
};

class mhs11059_comus_t:public I2c_comu_t{
public:
	mhs11059_comus_t(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum)	: I2c_comu_t(DestAddress, TxData, TxNum, RxNum){};	// ����M
	
	hamamatsu_S11059* MHS11059;
	mhs11059_comu_content ComuType;

	void callBack(void){MHS11059->fetchI2cRcvData(this);};
};

#endif