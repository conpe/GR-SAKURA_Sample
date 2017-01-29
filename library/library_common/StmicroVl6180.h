/********************************************/
/*	STmicro VL6180 (I2C)		    */
/*			for RX63n @ CS+     */
/*			Wrote by conpe_	    */
/*			2016/05/23	    */
/********************************************/

//�y�g�����z
// �܂�begin();��I2C������
// �Z���T����v���l���擾���邽�߁A����I��measure()���Ă�(I2C����M�J�n)�B
// getDistance()�Ōv���l��Ԃ��B

// �f�t�H���g�ݒ�
// I2C�A�h���X 0x29

//�y�X�V�����z
// 2016.05.23 �V�K�쐬

//�y�X�V�������z
// attach�̖߂�l�́A�A�^�b�`���s�ɉ����āA�f�o�C�X�ƒʐM���ł��Ȃ��������Ƃ�Ԃ�����
// ���󂾂ƃA�^�b�`���s�݂̂̔���Ȃ̂ŁA�f�o�C�X�ƒʐM�ł��Ȃ��Ă�ok����ɂȂ�B
// �f�o�C�X�ƒʐM�ł��Ȃ��ꍇ�A�o�b�t�@�N���A�����Ⴄ�̂ŁA�A�^�b�`���̂͐�������B
// ��C�ɑ��肷�������������s����B

#ifndef __STVL6180_H__
#define __STVL6180_H__

#include "RIIC.h"
#include "CommonDataType.h"
// debug
#include "SCI.h"

// �f�t�H���g�ݒ�
#define STVL6_ADRS_DEFAULT		0x29
#define STVL6_DISTANCE_DEFAULT	-1
#define STVL6_FAIL_DISTANCE_MM	255

class stvl6_comus_t;

class stmicro_VL6810{
public:
	enum register_e{
		IDENTIFICATION_MODEL_ID = 0x0000,
		SYSTEM__INTERRUPT_CLEAR = 0x0015,
		SYSRANGE_START = 0x0018,
		RESULT_RANGE_STATUS = 0x004D,
		RESULT_RANGE_VAL = 0x0062,
		RESULT_RANGE_RAW = 0x0064,
		RESULT_RANGE_RETURN_SIGNAL_COUNT = 0x006C,
		SYSRANGE_INTERMEASUREMENT_PERIOD = 0x001B,
		SYSRANGE_PART_TO_PART_RANGE_OFFSET = 0x0024,
		I2C_SLAVE__DEVICE_ADDRESS = 0x0212
	};

	// �R���X�g���N�^
	stmicro_VL6810(I2c_t* I2Cn);
	stmicro_VL6810(I2c_t* I2Cn, uint8_t I2cAddress);
	// �f�X�g���N�^
	virtual ~stmicro_VL6810(void);
	
	//����J�n
	int8_t begin(void);
	
	// �o�[�X�g�v���J�n(���[���ƒʐM���܂��邩�炨�����߂��Ȃ�
	int8_t startMeasurement(void);
	// �o�[�X�g�v����~
	int8_t stopMeasurement(void);
	
	
	// �v�����ʎ擾
	int8_t getDistance(int16_t* Dist);
	int16_t getDistance(void){return Distance_mm;};
	
	// ���茋�ʎ擾�v��
	int8_t measure(void);
	// ��������Z�b�g
	int8_t setMeasurePeriod(uint16_t msec);
	// ���荞�݃N���A
	int8_t clearInterrupt(void);
	//I2C�A�h���X�ݒ�
	int8_t setI2cAddress(uint8_t NewAdrs);
	
	// �e��ʐM
	int8_t setReg(register_e ComuReg, uint8_t SetData);	// ���W�X�^�Z�b�g
	int8_t reqDat(register_e ComuReg, uint16_t ReqNum);	// �f�[�^��M
	
	
	// �����Ă邩����
	bool_t isAlive(void){return RangeStatus;};
	//bool_t isAlive(void){return RangeReturnSignalCount;};
	int8_t checkAlive(void){ return reqDat(RESULT_RANGE_STATUS, 1); };
	//int8_t checkAlive(void){ return reqDat(RESULT_RANGE_RETURN_SIGNAL_COUNT, 1); };
	
	// �����̋V��
	int8_t reset(void);
	
	// �ʐM�A�C�h����Ԃ�
	// I2C�A�h���X�ݒ莞�ɏ��ɋN�������邽�߂ɕK�v
	bool_t isIdleComu(void){return I2Cn->isIdle();};
	
	
	// ��M�f�[�^�����p�R�[���o�b�N�֐�
	// I2C��M�f�[�^���擾���ĒʐM���e���Ƃɏ��������
	int8_t fetchI2cRcvData(const stvl6_comus_t* Comu);
	
	
	uint8_t I2cAddress;		// I2C�A�h���X
private:
	I2c_t *I2Cn;
	uint8_t I2cAddressPre;		// I2C�A�h���X(�m��O)
	uint8_t *RcvBuff;
	int16_t Distance_mm;	// �v����������[mm]
	bool_t MeasurementRequest;	// �v���v��
	bool_t fI2cErr;			// I2C�ʐM�G���[ ->����M��߂�
	uint8_t *FuseRcv;	// 
	uint8_t cntget;
	
	uint8_t RangeStatus;
	uint32_t RangeReturnSignalCount;
	
	void init(void);
	
	// I2C�N���X�ɒʐM���e���A�^�b�`
	int8_t attachI2cComu(register_e ComuReg, uint8_t SetData, uint16_t RxNum);
	
};

class stvl6_comus_t:public I2c_comu_t {
public:
	stvl6_comus_t(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum)	: I2c_comu_t(DestAddress, TxData, TxNum, RxNum){DevId = (0x03);};	// ����M
	
	//uint8_t DevId;	// �f�o�C�XID
	stmicro_VL6810* STVL6;
	stmicro_VL6810::register_e ComuReg;

	void callBack(void){
		STVL6->fetchI2cRcvData(this);
	};
};

#endif
