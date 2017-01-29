/********************************************/
/*		InvenSense MPUxxxx					*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/03/31		*/
/********************************************/

//�y�g�����z
// �͂��߂Ɍv���J�n�̂���setup()���ĂԁB
// �Z���T����v���l���擾���邽�߁A����I��measure()���Ă�(I2C����M�J�n)�B
// getAccel(), getGyro(), getTemp()�Ōv���l��Ԃ��B

// �p�x��deg

// �f�t�H���g�ݒ�
// I2C�A�h���X 0x68 or 0x69 (AD0�s���̐ݒ�ɂ��)
// 

//�y�X�V�����z
// 2015.03.31 �V�K�쐬
// 2015.06.29 I2C�ʐM���e�N���X�Ή�


#ifndef __INVENSENSEISMPU_H__
#define __INVENSENSEISMPU_H__

#include "RIIC.h"
#include "CommonDataType.h"
extern uint32_t getTime_ms(void);	// ���Ԍv��

// �f�t�H���g�ݒ�
#define ISMPU_ADRS_DEFAULT 0x68		// I2C�A�h���X(�����Z���T)
#define ISMPU_AK_ADRS_DEFAULT 0x0C	// I2C�A�h���X(���C�Z���T)
#define ISMPU_ACCEL_DEFAULT 0.0F
#define ISMPU_GYRO_DEFAULT 0.0F
#define ISMPU_TEMP_DEFAULT 0.0F

#define ISMPU_TEMP_SCALE 	333.87F		// LSB/deg
#define ISMPU_TEMP_OFFSET	21.0F		// deg


// ���W�X�^
#define  ISMPU_REG_GYRO_CONFIG	0x1B	
#define  ISMPU_REG_ACCEL_CONFIG	0x1C
#define  ISMPU_REG_PWR_MGMT_1	0x6B
#define  ISMPU_REG_ACCEL_XOUT_H	0x3B
#define  ISMPU_REG_ACCEL_XOUT_L	0x3C
#define  ISMPU_REG_ACCEL_YOUT_H	0x3D
#define  ISMPU_REG_ACCEL_YOUT_L	0x3E
#define  ISMPU_REG_ACCEL_ZOUT_H	0x3F
#define  ISMPU_REG_ACCEL_ZOUT_L	0x40
#define  ISMPU_REG_TEMP_OUT_H	0x41
#define  ISMPU_REG_TEMP_OUT_L	0x42
#define  ISMPU_REG_GYRO_XOUT_H	0x43
#define  ISMPU_REG_GYRO_XOUT_L	0x44
#define  ISMPU_REG_GYRO_YOUT_H	0x45
#define  ISMPU_REG_GYRO_YOUT_L	0x46
#define  ISMPU_REG_GYRO_ZOUT_H	0x47
#define  ISMPU_REG_GYRO_ZOUT_L	0x48

// ���C���f�b�N�X
#define ISMPU_IDX_ACC_X		0
#define ISMPU_IDX_ACC_Y		1
#define ISMPU_IDX_ACC_Z		2
#define ISMPU_IDX_GYRO_X	3
#define ISMPU_IDX_GYRO_Y	4
#define ISMPU_IDX_GYRO_Z	5
#define ISMPU_IDX_MAG_X		6
#define ISMPU_IDX_MAG_Y		7
#define ISMPU_IDX_MAG_Z		8



enum ismpu_comu_content{
	MEASURE,
	SET_CONFIG,		// ���ɃR�[���o�b�N�ŏ������Ȃ��ݒ�n�͂����
};

class ismpu_comus_t;

class invensense_MPU{
public:
	// �R���X�g���N�^
	invensense_MPU(I2c_t* I2Cn, uint8_t I2cAddress);	// �A�h���X�� 0x68 or 0x69 (AD0�[�q�ɂ���Č��܂�)
	~invensense_MPU(void);
	

	// �o�[�X�g�v���J�n
	int8_t startMeasurement(void);
	// �o�[�X�g�v����~
	int8_t stopMeasurement(void);
	// ���W���[���Z�b�g�A�b�v
	int8_t begin(void);
	
	
	// �v�����ʎ擾
	// �����x�Z���T
	int8_t getAccel(float* AccX, float* AccY, float* AccZ );	
	float getAccelX(void){return Accel[0];};
	float getAccelY(void){return Accel[1];};
	float getAccelZ(void){return Accel[2];};
	// �W���C���Z���T [deg/s]
	int8_t getGyro(float* GyroX, float* GyroY, float* GyroZ);
	float getGyroX(void){return Gyro[0];};
	float getGyroY(void){return Gyro[1];};
	float getGyroZ(void){return Gyro[2];};
	// ���x�Z���T
	int8_t getTemp(float* Temp);
	float getTemp(void){return Temp;};
	// ���C�Z���T
	// 6byte�{�����1byte�ǂ܂Ȃ��ƘA���ϊ����Ȃ��̂Œ���
	
	
	// �ݒ�
	void setAxis(bool_t AccX=0, bool_t AccY=0, bool_t AccZ=0, bool_t GyroX=0, bool_t GyroY=0, bool_t GyroZ=0, bool_t MagX=0, bool_t MagY=0, bool_t MagZ=0);	// ������ 0:�f�t�H���g, 1: �t
	void setAccelOffset(float Xofs, float Yofs, float Zofs, bool_t fAdd = 0);
	void setGyroOffset(float Xofs, float Yofs, float Zofs, bool_t fAdd = 0);
	
	
	// �p������
	// Attitude Estimation
	float getAeRoll(void){return SumRoll;};
	float getAePitch(void){return SumPitch;};
	float getAeYaw(void){return SumYaw;};
	void resetCalcAttitudeEstimation(float Roll = 0.0F, float Pitch = 0.0F, float Yaw = 0.0F);	// deg
	
	
	// �e��ʐM
	// ����l�v��
	int8_t measure(void);		// �����I�ɌĂ�
	// �W���C���Z���T�̃����W�ݒ�
	int8_t setGyroRange(uint8_t Fs);
	// �����x�Z���T�̃����W�ݒ�
	int8_t setAccelRange(uint8_t Fs);
	
	// �X�e�[�^�X
	bool_t isComuErr(void){return fI2cErr;};
	// debug
	i2c_status getStatus(void){return I2Cn->getStatus();};
	i2c_statusin getStatusIn(void){return I2Cn->getStatusIn();};
	
	
	// ���荞�ݗp
	// I2C��M�f�[�^���擾���ĒʐM���e���Ƃɏ��������
	int8_t fetchI2cRcvData(const ismpu_comus_t* Comu);
	
private:
	I2c_t *I2Cn;
	uint8_t I2cAddress;		// I2C�A�h���X
	uint8_t *RcvBuff;
	float Accel[3];		// �����xx y z  g
	float Gyro[3];		// �p���xx y z  deg/s
	float AccelOffset[3];		// �����xx y z �I�t�Z�b�g  g
	float GyroOffset[3];		// �p���xx y z �I�t�Z�b�g deg/s
	float Temp;		// ���x  deg
	uint8_t GyroFs;		// �W���C���Z���T�t���X�P�[���ݒ�l
	uint8_t AccelFs;	// �����x�Z���T�t���X�P�[���ݒ�l
	
	bool_t invAxis[9];	// �������t�ɂ���t���O
	
	bool_t MeasurementRequest;	// �v���v��
	bool_t fMeasurement;		// �v�����t���O
	bool_t fI2cErr;			// I2C�ʐM�G���[ ->����M��߂�
	
	// �p������
	uint32_t TimeLastUpdate_ms;	// �O��X�V����
	float SumRoll;
	float SumPitch;
	float SumYaw;
	void updateCalcAttitudeEstimation(void);
	
	
	// I2C�N���X�ɒʐM���e���A�^�b�`
	int8_t attachI2cComu(ismpu_comu_content ComuType, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum);
	ismpu_comus_t* LastAttachComu;	// �Ō�ɒʐM�v�������ʐM
	bool_t fLastAttachComuFin;		// �Ō�ɒʐM�v�������ʐM���I��������t���O
};

class ismpu_comus_t:public I2c_comu_t{
public:
	ismpu_comus_t(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum)	: I2c_comu_t(DestAddress, TxData, TxNum, RxNum){};	// ����M
	
	static uint8_t DevId;	// �f�o�C�XID
	invensense_MPU* ISMPU;
	ismpu_comu_content ComuType;

	void callBack(void){ISMPU->fetchI2cRcvData(this);};
	
};

#endif
