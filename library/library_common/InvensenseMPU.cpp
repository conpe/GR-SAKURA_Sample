/********************************************/
/*		InvenSense MPUxxxx					*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/03/31		*/
/********************************************/



#include "InvensenseMPU.h"


// LSB
static const float IsMpuAccelScaleFactor[4] = {16384, 8192, 4096, 2048};	// LSB/g
static const float IsMpuGyroScaleFactor[4] = {131, 65.5, 32.8, 16.4};		// LSB/(deg/s)



/*********************
�����Z���T(I2C������)
�R���X�g���N�^
�����F	I2C
**********************/
invensense_MPU::invensense_MPU(I2c_t* I2Cn, uint8_t I2cAddress){
	invensense_MPU::I2Cn = I2Cn;
	invensense_MPU::I2cAddress = I2cAddress;
	
	I2Cn->begin(400, 32);
	
	MeasurementRequest = false;
	fMeasurement = false;
	fI2cErr = false;
	Accel[0] = ISMPU_ACCEL_DEFAULT;
	Accel[1] = ISMPU_ACCEL_DEFAULT;
	Accel[2] = ISMPU_ACCEL_DEFAULT;
	Gyro[0] = ISMPU_GYRO_DEFAULT;
	Gyro[1] = ISMPU_GYRO_DEFAULT;
	Gyro[2] = ISMPU_GYRO_DEFAULT;
	Temp = ISMPU_TEMP_DEFAULT;
	
	GyroFs = 0;
	AccelFs = 0;
	
}
/*********************
�����Z���T
�f�X�g���N�^
**********************/
invensense_MPU::~invensense_MPU(void){
}



/*********************
�v�����ʎ擾
�T�v�F
 �e�v�����ʂ�Ԃ�
�����F

**********************/
int8_t invensense_MPU::getAccel(float* AccX, float* AccY, float* AccZ ){
	*AccX = invensense_MPU::Accel[0];
	*AccY = invensense_MPU::Accel[1];
	*AccZ = invensense_MPU::Accel[2];
	
	return 0;
}
int8_t invensense_MPU::getGyro(float* GyroX, float* GyroY, float* GyroZ){
	*GyroX = invensense_MPU::Gyro[0];
	*GyroY = invensense_MPU::Gyro[1];
	*GyroZ = invensense_MPU::Gyro[2];
	
	return 0;
}
int8_t invensense_MPU::getTemp(float* Temp){
	*Temp = invensense_MPU::Temp;
	
	return 0;
}





/*********************
�A�������v���J�n
�T�v�F
 ����I������炷�����̑���v�����o��
�����F
 
**********************/
int8_t invensense_MPU::startMeasurement(void){
	
	MeasurementRequest = 1;
	
	// �v�����ĂȂ��Ȃ�J�n
	if(!fMeasurement){
		measure();
	}
	return 0;
}
/*********************
�����v���I��
�T�v�F
 ���g��o�^
�����F	
**********************/
int8_t invensense_MPU::stopMeasurement(void){
	MeasurementRequest = 0;
	return 0;
}





/*********************
�Z�b�g�A�b�v
�T�v�F
 ���W���[���N����
 ISMPU_REG_PWR_MGMT_1��0x00������
�����F
 
**********************/
int8_t invensense_MPU::setup(void){
	int8_t AttachedIndex;
	uint8_t TxData[2];
	uint16_t TxNum;
	
	// I2C������������
	TxData[0] = (uint8_t)ISMPU_REG_PWR_MGMT_1;
	TxData[1] = 0x00;
	TxNum = 2;
	
	// I2C�����[�����I
	AttachedIndex = attachI2cComu(SET_CONFIG, TxData, TxNum, 0);
	if((0<=AttachedIndex) && (I2Cn->getComusNum()>=AttachedIndex)){
		//fMeasurement = true;
	}else{
		fI2cErr = true;
	}

	return AttachedIndex;
}

/*********************
�W���C���Z���T�̃����W�ݒ�
�T�v�F
 ISMPU_REG_GYRO_CONFIG��3,4�r�b�g�ڂŐݒ�
�����F
 0: +-250
 1: +-500
 2: +-1000
 3: +-2000
**********************/
int8_t invensense_MPU::setGyroRange(uint8_t Fs){
	int8_t AttachedIndex;
	uint8_t TxData[2];
	uint16_t TxNum;
	
	// I2C������������
	TxData[0] = (uint8_t)ISMPU_REG_GYRO_CONFIG;
	TxData[1] = (Fs&0x03)<<3;
	TxNum = 2;
	
	AttachedIndex = attachI2cComu(SET_CONFIG, TxData, TxNum, 0);
	if((0<=AttachedIndex) && (I2Cn->getComusNum()>=AttachedIndex)){
		GyroFs = Fs;
	}else{
		fI2cErr = true;
	}
	
	return AttachedIndex;
}

/*********************
�����x�Z���T�Z���T�̃����W�ݒ�
�T�v�F
 ISMPU_REG_ACCEL_CONFIG��3,4�r�b�g�ڂŐݒ�
�����F
 0: +-2g
 1: +-4g
 2: +-8g
 3: +-16g
**********************/
int8_t invensense_MPU::setAccelRange(uint8_t Fs){
	int8_t AttachedIndex;
	uint8_t TxData[2];
	uint16_t TxNum;
	
	// I2C������������
	TxData[0] = (uint8_t)ISMPU_REG_ACCEL_CONFIG;
	TxData[1] = (Fs&0x03)<<3;
	TxNum = 2;
	
	AttachedIndex = attachI2cComu(SET_CONFIG, TxData, TxNum, 0);
	if((0<=AttachedIndex) && (I2Cn->getComusNum()>=AttachedIndex)){	//I2Cn->getComusNum()���傫�������ĂȂ�Ȃ񂾁H
		AccelFs = Fs;
	}else{
		fI2cErr = true;
	}
	
	return AttachedIndex;
}

/*********************
�v��
�T�v�F
 �����x�A�p���x�A���x�̌v��
 ISMPU_REG_ACCEL_XOUT_H����14�o�C�g��ǂ�
�����F
 �Ȃ�
�Ԓl�F
 AttachedIndex
 -5 ���łɌv����
 -6 �ʐM��������
 -1 Attach���������ς�(I2C_COMUS)(RIIC�N���X�̎d�l),
 -2 Comus�̗̈���m�ێ��s(RIIC�N���X�̎d�l)
**********************/
int8_t invensense_MPU::measure(void){
	int8_t AttachedIndex;
	uint8_t TxData[1];
	uint16_t TxNum;
	uint16_t RxNum;
	
	//if(!fMeasurement || I2Cn->isIdle()){
	if(!fI2cErr){
		if(!fMeasurement){
			
			// I2C������������
			TxData[0] = (uint8_t)ISMPU_REG_ACCEL_XOUT_H;
			TxNum = 1;
			RxNum = 14;
			
			// I2C�����[�����I
			AttachedIndex = attachI2cComu(MEASURE, TxData, TxNum, RxNum);
			if((0<=AttachedIndex) && (I2Cn->getComusNum()>=AttachedIndex)){
				fMeasurement = true;
			}else{
				fI2cErr = true;
			}
			
		}else{
			return -5;
		}
	}else{
		return -6;
	}
	
	return AttachedIndex;
}





/*********************
I2C�N���X�ɒʐM���A�^�b�`����
�T�v�F
 �ʐM���J�n����B
 �I����fetchI2cRcvData���Ă΂��B
�����F

�Ԓl�F
 AttachedIndex
**********************/
int8_t invensense_MPU::attachI2cComu(ismpu_comu_content ComuType, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum){
	int8_t AttachedIndex = 0;
	
	ismpu_comus_t* NewComu;
	
	NewComu = new ismpu_comus_t(I2cAddress, TxData, TxNum, RxNum);
	if(NULL == NewComu){
		return -1;	// �q�[�v����Ȃ��H
	}
	NewComu->ISMPU = this;
	NewComu->ComuType = ComuType;
	
	AttachedIndex = I2Cn->attach(NewComu);		// ����M�o�^
	if(AttachedIndex<0){	// �o�^���s������Ȃ��������Ƃ�
		delete NewComu;
	}
	LastAttachComu = NewComu;
	fLastAttachComuFin = false;
	
	return AttachedIndex;
}


/*********************
I2C�ǂݏI������̂ŕߊl
�T�v�F
�����F	
**********************/
int8_t invensense_MPU::fetchI2cRcvData(const ismpu_comus_t* Comu){
	uint8_t* RcvData = Comu->RxData;
	
	// �ʐM�����Əo���Ă�H
	fI2cErr = Comu->Err;
	
	// �ʐM�����t���O���Ă�
	if(LastAttachComu == Comu){
		fLastAttachComuFin = true;
	}
	
	if(!fI2cErr){
		
		// �ʐM���e�ɂ���Ď�M�f�[�^����������
		switch(Comu->ComuType){
		case SET_CONFIG:
			break;
		case MEASURE:	// �����擾
			Accel[0] = (int16_t)((uint16_t)RcvData[0]<<8 | ((uint16_t)RcvData[1]&0x00FF)) / IsMpuAccelScaleFactor[AccelFs];
			Accel[1] = (int16_t)((uint16_t)RcvData[2]<<8 | ((uint16_t)RcvData[3]&0x00FF)) / IsMpuAccelScaleFactor[AccelFs];
			Accel[2] = (int16_t)((uint16_t)RcvData[4]<<8 | ((uint16_t)RcvData[5]&0x00FF)) / IsMpuAccelScaleFactor[AccelFs];
			Temp = (int16_t)((uint16_t)RcvData[6]<<8 | ((uint16_t)RcvData[7]&0x00FF)) / ISMPU_TEMP_SCALE + ISMPU_TEMP_OFFSET;
			Gyro[0] = (int16_t)((uint16_t)RcvData[8]<<8 | ((uint16_t)RcvData[9]&0x00FF)) / IsMpuGyroScaleFactor[GyroFs];
			Gyro[1] = (int16_t)((uint16_t)RcvData[10]<<8 | ((uint16_t)RcvData[11]&0x00FF)) / IsMpuGyroScaleFactor[GyroFs];
			Gyro[2] = (int16_t)((uint16_t)RcvData[12]<<8 | ((uint16_t)RcvData[13]&0x00FF)) / IsMpuGyroScaleFactor[GyroFs];

			fMeasurement = false;
				
			//�X�Ɍv������
			if(MeasurementRequest){
				measure();
			}
			
			break;
		}
	}else{
		
		fMeasurement = false;
	}
	
	return 0;
}