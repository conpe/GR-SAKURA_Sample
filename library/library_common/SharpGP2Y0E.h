/********************************************/
/*		SHARP �����Z���T __GP2Y0E*(I2C)		*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/03/11		*/
/********************************************/

//�y�g�����z
// �܂�begin();��I2C������
// �Z���T����v���l���擾���邽�߁A����I��measure()���Ă�(I2C����M�J�n)�B
// getDistance()�Ōv���l��Ԃ��B

// �f�t�H���g�ݒ�
// I2C�A�h���X 0x40

//�y�X�V�����z
// 2015.03.11 �V�K�쐬
// 2015.06.26 I2C�ʐM���e�N���X�Ή�

//�y�X�V�������z
// �Z���T�ɂ����
// AD�ϊ�Ver. �̒ǉ�


#ifndef __SHARPGP2Y0E_H__
#define __SHARPGP2Y0E_H__

#include "RIIC.h"
#include "CommonDataType.h"
#include "portReg.h"


// �f�t�H���g�ݒ�
#define SHGP2_ADRS_DEFAULT		0x40
#define SHGP2_DISTANCE_DEFAULT	-1
#define SHGP2_FAIL_DISTANCE_MM	639

// E-fuse�p�d���Ǘ�
#define SHGP2_VPP_ON 0		// �|�[�g��0�o�͂�on
#define SHGP2_VPP_OFF 1		// �|�[�g��1�o�͂�off

// ���W�X�^
#define  SHGP2_REG_MEDIANFILTER	0x3F
#define  SHGP2_REG_DISTANCE		0x5E



enum shgp2_comu_content{
	GET_DISTANCE,
	SET_MEDIANFILTER,
	SET_FUSE_I2CADDRESS,
};

class shgp2_comus_t;


class sharp_GP2Y0E{
private:
	I2c_t *I2Cn;
	uint8_t I2cAddress;		// I2C�A�h���X
	uint8_t *RcvBuff;
	int16_t Distance_mm;	// �v����������[mm]
	bool_t MeasurementRequest;	// �v���v��
	bool_t fMeasurement;	// �v�����t���O
	bool_t fI2cErr;			// I2C�ʐM�G���[ ->����M��߂�
	uint8_t *FuseRcv;	// 
	
	// I2C�N���X�ɒʐM���e���A�^�b�`
	int8_t attachI2cComu(shgp2_comu_content ComuType, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum);
	shgp2_comus_t* LastAttachComu;	// �Ō�ɒʐM�v�������ʐM
	bool_t fLastAttachComuFin;		// �Ō�ɒʐM�v�������ʐM���I��������t���O
public:
	// �R���X�g���N�^
	sharp_GP2Y0E(I2c_t* I2Cn, uint8_t I2cAddress);
	// �f�X�g���N�^
	~sharp_GP2Y0E(void);
	
	//����J�n
	int8_t begin(void);
	
	// �o�[�X�g�v���J�n(���[���ƒʐM���܂��邩�炨�����߂��Ȃ�
	int8_t startMeasurement(void);
	// �o�[�X�g�v����~
	int8_t stopMeasurement(void);
	
	
	// �v�����ʎ擾
	int8_t getDistance(int16_t* Dist);
	int16_t getDistance(void){return Distance_mm;};
	
	// �e��ʐM
	// 1, ���� (����I�ɌĂ�)
	int8_t measure(void);
	// 2, ���f�B�A���t�B���^�̐ݒ�
	int8_t setMedianFilter(uint8_t MedianCalcNum);
	// 3, �X���[�v���[�h
	int8_t setSleepMode(bool_t toSleep);
	
	// Fuse bit ��������
	// 1, I2C�A�h���X�ύX
	int8_t setI2cAddress(uint8_t NewAddress, pins Vpp);
	
	// debug
	i2c_status getStatus(void){return I2Cn->getStatus();};
	i2c_statusin getStatusIn(void){return I2Cn->getStatusIn();};
	
	
	// ��M�f�[�^�����p�R�[���o�b�N�֐�
	// I2C��M�f�[�^���擾���ĒʐM���e���Ƃɏ��������
	int8_t fetchI2cRcvData(const shgp2_comus_t* Comu);
};

class shgp2_comus_t:public I2c_comu_t {
public:
	shgp2_comus_t(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum)	: I2c_comu_t(DestAddress, TxData, TxNum, RxNum){};	// ����M
	
	sharp_GP2Y0E* SHGP2;
	shgp2_comu_content ComuType;

	void callBack(void){SHGP2->fetchI2cRcvData(this);};
};

#endif
