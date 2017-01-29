/********************************************/
/*		Sharp �����Z���T __GP2Y0E*(I2C)		*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/03/11		*/
/********************************************/



#include "SharpGP2Y0E.h"


uint8_t shgp2_comus_t::DevId = 0x02;

/*********************
�����Z���T������
�R���X�g���N�^
�����F	I2C
**********************/
sharp_GP2Y0E::sharp_GP2Y0E(I2c_t* I2Cn, uint8_t I2cAddress){
	sharp_GP2Y0E::I2Cn = I2Cn;
	sharp_GP2Y0E::I2cAddress = I2cAddress;
	
	
	MeasurementRequest = false;
	fMeasurement = false;
	fI2cErr = false;
	Distance_mm = SHGP2_DISTANCE_DEFAULT;
}
/*********************
�f�X�g���N�^
**********************/
sharp_GP2Y0E::~sharp_GP2Y0E(void){
	
}


/*********************
����J�n
I2C������
**********************/
int8_t sharp_GP2Y0E::begin(void){
	return I2Cn->begin(400, 32);
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
int8_t sharp_GP2Y0E::getDistance(int16_t* Dist){
	*Dist = Distance_mm;
	
	if(Distance_mm==SHGP2_DISTANCE_DEFAULT){
		return -1;
	}
	
	return 0;
}

/*********************
�A�������v���J�n
�T�v�F
 ����I������炷�����̑���v�����o��
�����F
 
**********************/
int8_t sharp_GP2Y0E::startMeasurement(void){
	
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
int8_t sharp_GP2Y0E::stopMeasurement(void){
	MeasurementRequest = 0;
	return 0;
}




/*********************
�v��
�T�v�F
 �����̎擾
�����F
 �Ȃ�
�Ԓl�F
 AttachedIndex
 -5 ���łɌv����
 -6 �ʐM��������
 -1 Attach���������ς�(I2C_COMUS)(RIIC�N���X�̎d�l),
 -2 Comus�̗̈���m�ێ��s(RIIC�N���X�̎d�l)
**********************/
int8_t sharp_GP2Y0E::measure(void){
	int8_t AttachedIndex;
	uint8_t TxData[1];
	uint16_t TxNum;
	uint16_t RxNum;
	
	//if(!fMeasurement || I2Cn->isIdle()){
	//if(!fI2cErr){
	//	if(!fMeasurement){	// �O�̌v���I����ĂȂ�
	
			// I2C������������
			TxData[0] = (uint8_t)SHGP2_REG_DISTANCE;
			TxNum = 1;
			RxNum = 2;
			
			// I2C�����[�����I
			AttachedIndex = attachI2cComu(GET_DISTANCE, TxData, TxNum, RxNum);
			if(0<=AttachedIndex){
				// ����
				fMeasurement = true;
				fI2cErr = false;
			}else{
				// ���s
				fMeasurement = false;
				fI2cErr = true;
			}
			
	//	}else{
	//		return -5;
	//	}
	//}else{
	//	return -6;
	//}
	
	return AttachedIndex;
}



/*********************
���f�B�A���t�B���^�̐ݒ�
�T�v�F
 
�����F
 ���f�B�A���t�B���^�̌v�Z�Ɏg����
  1, 5, 7, 9�̂����ꂩ
**********************/
int8_t sharp_GP2Y0E::setMedianFilter(uint8_t MedianCalcNum){
	int8_t AttachedIndex;
	uint8_t TxData[2];
	uint16_t TxNum;
	
	// I2C������������
	TxData[0] = (uint8_t)SHGP2_REG_MEDIANFILTER;
	switch(MedianCalcNum){
	case 7:
		TxData[1] = 0x00;
		break;
	case 5:
		TxData[1] = 0x10;
		break;
	case 9:
		TxData[1] = 0x20;
		break;
	case 1:
		TxData[1] = 0x30;
		break;
	default:
		TxData[1] = 0x30;
		break;
	}
	TxNum = 2;
	
	// I2C�����[�����I
	AttachedIndex = attachI2cComu(SET_MEDIANFILTER, TxData, TxNum, 0);
	
	return AttachedIndex;
}


/*********************
�yE-FUSE�zI2C�A�h���X�̐ݒ�
�T�v�F
 E-fuse�ȃv���O���~���O�Ńf�o�C�X��I2C�A�h���X������������B
 �d��onoff������̂ŁAFET���Ȃɂ���Vpp�𐧌䂷��K�v����B
 �Q�l:�A�v���P�[�V�����m�[�ghttp://www.sharp.co.jp/products/device/doc/opto/gp2y0e02_03_appl_e.pdf
�����F
 uint8_t NewAddress �V����I2C�A�h���X(0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78)
 					�f�[�^�V�[�g�ł�1bit���V�t�g�����A�h���X�ŕ\�L����Ă���̂Œ���
 pins Vpp �d��(�^�C�~���O�ɍ��킹��on/off����)

�Ԓl�F
 0	������������
 -5	�������l���Ⴄ
 ���̑�	I2C�A�^�b�`���s
**********************/
int8_t sharp_GP2Y0E::setI2cAddress(uint8_t NewAddress, pins Vpp){
	uint8_t TxData[2];
	uint8_t SetI2cAddress;
	int8_t AttachedIndex;
	
	
	
	SetI2cAddress = (NewAddress>>3)&0x0F;	// E[0]�`E[3]�ɍ��킹��
	
	outPin(Vpp, SHGP2_VPP_OFF);
	
	// Stage1
	TxData[0] = 0xEC;	// Register Address
	TxData[1] = 0xFF;	// data
	
	
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	// wait for finish transmit. 
	while(true == fLastAttachComuFin);
	outPin(Vpp, SHGP2_VPP_ON);		// turn on
	
	// Stage2
	TxData[0] = 0xC8;
	TxData[1] = 0x00;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	
	// Stage3
	TxData[0] = 0xC9;
	TxData[1] = 0x45;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	
	// Stage4
	TxData[0] = 0xCD;
	TxData[1] = SetI2cAddress;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	
	// Stage5
	TxData[0] = 0xCA;
	TxData[1] = 0x01;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	volatile uint16_t i;
	for(i=0;i<30000; i++){
		// �������ݑ҂�
		// 500usec
	}
	
	//��������͐V�����A�h���X�ŃA�N�Z�X���Ȃ���Ȃ�Ȃ��H
	
	// Stage6
	TxData[0] = 0xCA;
	TxData[1] = 0x00;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	
	// wait for finish transmit. 
	while(true == fLastAttachComuFin);
	outPin(Vpp, SHGP2_VPP_OFF);		// turn off
	
	// Stage7
	// Stage7.0
	TxData[0] = 0xEF;
	TxData[1] = 0x00;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	// Stage7.1
	TxData[0] = 0xC8;
	TxData[1] = 0x40;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	// Stage7.2
	TxData[0] = 0xC8;
	TxData[1] = 0x00;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	
	// Stage8
	TxData[0] = 0xEE;
	TxData[1] = 0x06;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	
	
	// Stage9 Check
	// Stage9.0
	TxData[0] = 0xEF;
	TxData[1] = 0x00;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	// Stage9.1
	TxData[0] = 0xEC;
	TxData[1] = 0xFF;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	// Stage9.2
	TxData[0] = 0xEF;
	TxData[1] = 0x03;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	// Stage9.3 read
	TxData[0] = 0x27;
	FuseRcv = new uint8_t[1];
		if(NULL==FuseRcv) __heap_chk_fail();
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 1, 1);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	//read
	// wait for finish receive. 
	while(true == fLastAttachComuFin);
	int8_t ack = (int8_t)FuseRcv[0];
	
	delete[] FuseRcv;
	
	// Stage9.4
	TxData[0] = 0xEF;
	TxData[1] = 0x00;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	// Stage9.5
	TxData[0] = 0xEC;
	TxData[1] = 0x7F;
	AttachedIndex = attachI2cComu(SET_FUSE_I2CADDRESS, TxData, 2, 0);
	if(AttachedIndex<0){	// �G���[���c
		return AttachedIndex;
	}
	
	
	return ack;
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
int8_t sharp_GP2Y0E::attachI2cComu(shgp2_comu_content ComuType, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum){
	int8_t AttachedIndex = 0;
	
	shgp2_comus_t* NewComu;
	
	NewComu = new shgp2_comus_t(I2cAddress, TxData, TxNum, RxNum);
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
	
	NewComu->SHGP2 = this;
	NewComu->ComuType = ComuType;
	
	AttachedIndex = I2Cn->attach(NewComu);		// ����M�o�^
	if(-1 == AttachedIndex){
		// �o�^���s������Ȃ��������Ƃ�
		delete NewComu;
	}else{
		fLastAttachComuFin = false;
		LastAttachComu = NewComu;
	}
	
	return AttachedIndex;
}




/*********************
I2C�ǂݏI������̂ŕߊl
�T�v�F
�����F	
**********************/
int8_t sharp_GP2Y0E::fetchI2cRcvData(const shgp2_comus_t* Comu){
	uint8_t* RcvData = Comu->RxData;
	
	if(NULL!=Comu){
		
		// �ʐM�����Əo���Ă�H
		fI2cErr = Comu->Err;
		
		// �ʐM�����t���O���Ă�
		if(LastAttachComu == Comu){
			fLastAttachComuFin = true;
		}
		
		if(!fI2cErr){
			
			// �ʐM���e�ɂ���Ď�M�f�[�^����������
			switch(Comu->ComuType){
			case GET_DISTANCE:	// �����擾
				Distance_mm = (int16_t)((((uint16_t)RcvData[0])<<4)|(((uint16_t)RcvData[1])&0x000F))*10/16/4;
				
				if(Distance_mm == SHGP2_FAIL_DISTANCE_MM){
					Distance_mm = SHGP2_DISTANCE_DEFAULT;
				}
			
				fMeasurement = false;
					
				//�X�Ɍv������
				if(MeasurementRequest){
					measure();
				}
				
				break;
			case SET_MEDIANFILTER:
				// none
				break;
				
			case SET_FUSE_I2CADDRESS:
				if(Comu->RxNum>0){
					FuseRcv[0] = RcvData[0];
				}
				break;
			}
		}else{
			Distance_mm = SHGP2_DISTANCE_DEFAULT;
			fMeasurement = false;
		}
		
		return 0;
	}else{
		return -1;
	}
}

