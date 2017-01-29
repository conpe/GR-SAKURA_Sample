/********************************************/
/*	STmicro VL6180 (I2C)		    */
/*			for RX63n @ CS+     */
/*			Wrote by conpe_	    */
/*			2016/05/23	    */
/********************************************/



#include "StmicroVl6180.h"


//uint8_t stvl6_comus_t::DevId = 0x03;

/*********************
�����Z���T������
�R���X�g���N�^
�����F	I2C
**********************/
stmicro_VL6810::stmicro_VL6810(I2c_t* I2Cn){
	this->I2Cn = I2Cn;
	this->I2cAddress = STVL6_ADRS_DEFAULT;
	this->I2cAddressPre = STVL6_ADRS_DEFAULT;
	init();
}
stmicro_VL6810::stmicro_VL6810(I2c_t* I2Cn, uint8_t I2cAddress){
	this->I2Cn = I2Cn;
	this->I2cAddress = STVL6_ADRS_DEFAULT;
	this->I2cAddressPre = I2cAddress;
	init();
	// Address�ύX�����v
	
}



/*********************
�f�X�g���N�^
**********************/
stmicro_VL6810::~stmicro_VL6810(void){
	
}


/*********************
�t���O�Ƃ�������
**********************/
void stmicro_VL6810::init(void){
	MeasurementRequest = false;
	//fMeasurement = false;
	fI2cErr = false;
	Distance_mm = STVL6_DISTANCE_DEFAULT;
	cntget = 0;
}


/*********************
����J�n
 I2C������
 ���W�X�^������
**********************/
int8_t stmicro_VL6810::begin(void){
	int8_t ret=0;
	
	I2Cn->begin(400, 15, 64);
	
	if(this->I2cAddressPre != STVL6_ADRS_DEFAULT){	// �f�t�H���g�A�h���X����Ȃ����
		ret |= setI2cAddress(this->I2cAddressPre);	// �ݒ肷��
	}
	
	ret |= setReg((register_e)0x0207, 0x01);
	ret |= setReg((register_e)0x0208, 0x01);
	ret |= setReg((register_e)0x0096, 0x00);
	ret |= setReg((register_e)0x0097, 0xfd);
	ret |= setReg((register_e)0x00e3, 0x00);
	ret |= setReg((register_e)0x00e4, 0x04);
	ret |= setReg((register_e)0x00e5, 0x02);
	ret |= setReg((register_e)0x00e6, 0x01);
	ret |= setReg((register_e)0x00e7, 0x03);
	ret |= setReg((register_e)0x00f5, 0x02);
	ret |= setReg((register_e)0x00d9, 0x05);
	ret |= setReg((register_e)0x00db, 0xce);
	ret |= setReg((register_e)0x00dc, 0x03);
	ret |= setReg((register_e)0x00dd, 0xf8);
	ret |= setReg((register_e)0x009f, 0x00);
	ret |= setReg((register_e)0x00a3, 0x3c);
	ret |= setReg((register_e)0x00b7, 0x00);
	ret |= setReg((register_e)0x00bb, 0x3c);
	ret |= setReg((register_e)0x00b2, 0x09);
	ret |= setReg((register_e)0x00ca, 0x09);  
	ret |= setReg((register_e)0x0198, 0x01);
	ret |= setReg((register_e)0x01b0, 0x17);
	ret |= setReg((register_e)0x01ad, 0x00);
	ret |= setReg((register_e)0x00ff, 0x05);
	ret |= setReg((register_e)0x0100, 0x05);
	ret |= setReg((register_e)0x0199, 0x05);
	ret |= setReg((register_e)0x01a6, 0x1b);
	ret |= setReg((register_e)0x01ac, 0x3e);
	ret |= setReg((register_e)0x01a7, 0x1f);
	ret |= setReg((register_e)0x0030, 0x00);
	
	//ret |= setReg((register_e)0x0011, 0x10);
	ret |= setReg((register_e)0x010a, 0x30);
	//ret |= setReg((register_e)0x003f, 0x46);
	//ret |= setReg((register_e)0x0031, 0xff);
	//ret |= setReg((register_e)0x0040, 0x63);
	//ret |= setReg((register_e)0x002e, 0x01);
	 
	/*
	ret |= setReg((register_e)0x001b, 0x09);
	ret |= setReg((register_e)0x003e, 0x31);
	ret |= setReg((register_e)0x0014, 0x24); 
	*/
	
	
	// �v���J�n
	ret |= setMeasurePeriod(20);		// �v������20msec
	ret |= setReg(SYSRANGE_START, 0x03);	// �A���v��
	
	return ret;
}


/*********************
�v�����ʎ擾
�T�v�F
 
�����F
 Dist �����ɕԂ���
�Ԓl�F
 0 ����
 -1 ���v��
**********************/
int8_t stmicro_VL6810::getDistance(int16_t* Dist){
	*Dist = Distance_mm;
	
	if(cntget<10){
		cntget++;
	}else if(cntget>10){
		
	}else{
	//	begin();	// �N���サ�΂炭���Ă����邽�߂����ŁI
		cntget++;
	}
	
	return 0;
}

/*********************
�A�������v���J�n
�T�v�F
 ����I������炷�����̌v�����ʎ擾�v�����o��
�����F
 
**********************/
int8_t stmicro_VL6810::startMeasurement(void){
	
	MeasurementRequest = 1;
	
	//setReg(VL6180X_REG_SYSRANGE_INTERMEASUREMENT_PERIOD, 4);	// �v������50msec
	//setReg(VL6180X_REG_SYSRANGE_START, 0x03);	// �A���v��
	
	return 0;
}
/*********************
�����v���I��
�T�v�F
 ���g��o�^
�����F	
**********************/
int8_t stmicro_VL6810::stopMeasurement(void){
	MeasurementRequest = 0;
	
	//setReg(VL6180X_REG_SYSRANGE_START, 0x03);	// �A���v��
	
	return 0;
}

/*********************
�v�������ݒ�
�T�v�F
�����F	uint16_t msec 10�`2550
**********************/
int8_t stmicro_VL6810::setMeasurePeriod(uint16_t msec){
	// �͈�
	if(msec<10){
		msec = 10;
	}
	if(msec>(2540+10)){
		msec = 2540+10;
	}
	// ����l�֕ϊ�
	msec = (msec-10)/10;
	
	return setReg(SYSRANGE_INTERMEASUREMENT_PERIOD, (uint8_t)(msec&0x00FF));	// �v������50msec
}


/*********************
���荞�݃N���A
�T�v�F
�����F	
**********************/
int8_t stmicro_VL6810::clearInterrupt(void){
	return setReg(SYSTEM__INTERRUPT_CLEAR, 0x07);
}

/*********************
I2C�A�h���X�ݒ�
�T�v�F
�����F	
**********************/
int8_t stmicro_VL6810::setI2cAddress(uint8_t NewAdrs){
	int8_t ret;	
	
	ret = setReg(I2C_SLAVE__DEVICE_ADDRESS, NewAdrs);
	if(ret >= 0){	// ����������Ă����
		this->I2cAddress = NewAdrs;
	}
	return ret;
}


/*********************
�v��
�T�v�F�����̎擾�v��
�����F�Ȃ�
�Ԓl�F
**********************/
int8_t stmicro_VL6810::measure(void){
	return reqDat(RESULT_RANGE_RAW, 1);
}


/*********************
�����̋V��
�T�v�F�v�����~�܂�����������ɍĐݒ�Ƃ����Ă݂�
	�ЂƂ܂�����Ōv���ĊJ����B
	�������A2�x����Ȃ��ƃ_��
�����F�Ȃ�
�Ԓl�F
**********************/
int8_t stmicro_VL6810::reset(void){
	int8_t ret=0;
	
	//ret |= setMeasurePeriod(20);		// �v������20msec
	//ret |= setReg(SYSRANGE_START, 0x03);	// �A���v��
	begin();
	
	return ret;
}




/*********************
���W�X�^�փZ�b�g
�T�v�FI2C���M�J�n
�����F�Ȃ�
�Ԓl�F
**********************/
int8_t stmicro_VL6810::setReg(register_e ComuReg, uint8_t SetData){
	int8_t AttachedIndex;
	
	AttachedIndex = attachI2cComu(ComuReg, SetData, 0);
	if(0 <= AttachedIndex){
		// ����
		//fMeasurement = true;
		fI2cErr = false;
	}else{
		// ���s
		//fMeasurement = false;
		fI2cErr = true;
	}
	return AttachedIndex;
}

/*********************
���W�X�^����ǂݍ���
�T�v�FI2C����M�J�n
�����F�Ȃ�
�Ԓl�F
**********************/
int8_t stmicro_VL6810::reqDat(register_e ComuReg, uint16_t ReqNum){
	int8_t AttachedIndex;
	
	AttachedIndex = attachI2cComu(ComuReg, 0, ReqNum);
	if(0<=AttachedIndex){
		// ����
		//fMeasurement = true;
		fI2cErr = false;
	}else{
		// ���s
		//fMeasurement = false;
		fI2cErr = true;
	}
	return AttachedIndex;
}



/*********************
I2C�N���X�ɒʐM���A�^�b�`����
�T�v�F
 �ʐM���J�n����B
 �I����fetchI2cRcvData���Ă΂��B
 [��O : I2C�o�b�t�@�����ς�]
  new�����ʐM�N���X��delete���āA�Ȃ��������Ƃɂ���B-1��Ԃ��B
 
�����F

�Ԓl�F
 AttachedIndex
**********************/
int8_t stmicro_VL6810::attachI2cComu(register_e ComuReg, uint8_t SetData, uint16_t RxNum){
	int8_t AttachedIndex = 0;
	uint8_t TxNum;
	stvl6_comus_t* NewComu;
	uint8_t TxData[3];
	
	TxData[0] = (uint8_t)(((uint16_t)ComuReg)>>8)&0x00FF;
	TxData[1] = (uint8_t)((uint16_t)ComuReg)&0x00FF;
	TxData[2] = SetData;
	
	if(RxNum>0){
		TxNum = 2;	// �A�h���X�̂ݑ���
	}else{
		TxNum = 3;
	}
	
	
	NewComu = new stvl6_comus_t(I2cAddress, TxData, TxNum, RxNum);
		if(NULL==NewComu) __heap_chk_fail();
	
	if(NULL == NewComu){
		return -1;	// �q�[�v����Ȃ��H
	}
	if(TxNum>0){
		if(NULL == NewComu->TxData){
			delete NewComu;
			return -1;
		}
	}
	if(RxNum>0){
		if(NULL == NewComu->RxData){
			delete NewComu;
			return -1;
		}
	}
	
	NewComu->STVL6 = this;
	NewComu->ComuReg = ComuReg;
	
	
	// ����M�o�^
	AttachedIndex = I2Cn->attach(NewComu);
	if(-1 == AttachedIndex){
		// �o�^���s������Ȃ��������Ƃ�
		delete NewComu;
	}else{
	}
	
	return AttachedIndex;
}




/*********************
I2C�ǂݏI������̂ŕߊl
�T�v�F
�����F	
**********************/
int8_t stmicro_VL6810::fetchI2cRcvData(const stvl6_comus_t* Comu){
	if(NULL!=Comu){
		// �ʐM�����Əo���Ă�H
		fI2cErr = Comu->Err;
		
		if(!fI2cErr){
			
			// �ʐM���e�ɂ���Ď�M�f�[�^����������
			switch(Comu->ComuReg){
			case RESULT_RANGE_RAW:	// �����擾
				Distance_mm = *Comu->RxData;
				if(STVL6_FAIL_DISTANCE_MM == Distance_mm){
					Distance_mm = STVL6_DISTANCE_DEFAULT;
				}
				
			//	clearInterrupt();
				
			
				//�X�Ɍv������
				if(MeasurementRequest){
					measure();
				}
				
				break;
			case SYSRANGE_START:
				break;
			case RESULT_RANGE_STATUS:
				RangeStatus = (*Comu->RxData)>>4;
				break;
			case RESULT_RANGE_RETURN_SIGNAL_COUNT:
				RangeReturnSignalCount = *(uint32_t*)Comu->RxData;
				break;
			default:
				/* do nothing */
				break;
			}
		}else{
			Distance_mm = STVL6_DISTANCE_DEFAULT;
		}
	
		return 0;
	}else{
		return -1;
	}
}

