/********************************************/
/*		RIIC���W���[���g�p I2C�ʐM			*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/02/26		*/
/********************************************/

#include "RIIC.h"

// RIIC module 
// Rx63n is master. 

//���begin����񂾂���

//�y�X�V�\��z
// ���M�o�b�t�@�������Ȃ��悤�ɂ�����(�ʐM�I�u�W�F�N�g�Ɏ�������)
// �ʐM����ɂ���ĈႤ�o�b�t�@���g���d�g�݂ق����ȁH


I2c_t I2C0(I2C_RIIC0);	//
//I2c_t I2C1(I2C_RIIC1);	//
I2c_t I2C2(I2C_RIIC2);	//
//I2c_t I2C3(I2C_RIIC3);	//



/****************************
 I2C �R���X�g���N�^
�T�v�F
�����F
 ���W���[��No.
�Ԓl�F
 �Ȃ�
****************************/
I2c_t::I2c_t(i2c_module I2cModule)
{
	_ModuleNum = I2cModule;		
	fBegun = false;				// ��begin()
	
	//���W�X�^
	switch(I2cModule){
	case I2C_RIIC0:
		I2Creg = &RIIC0;
		break;
	case I2C_RIIC1:
		I2Creg = &RIIC1;
		break;
	case I2C_RIIC2:
		I2Creg = &RIIC2;
		break;
	case I2C_RIIC3:
		I2Creg = &RIIC3;
		break;
	default:
		break;
	}
	
	NextAttachIndex = 0;
	CurrentComuIndex = 0;
	
	
}

/****************************
 I2C �f�X�g���N�^
�T�v�F
 �m�ۂ������������J������
�����F
 �Ȃ�
�Ԓl�F
 �Ȃ�
****************************/
I2c_t::~I2c_t(void){
	if(TxBuff) delete TxBuff;
	if(ComusBuff) delete ComusBuff;
}


/****************************
 I2C�ʐM������
�T�v�F
 RIIC���W���[���̏��������s���B
 ����̂݁B
�����F
	uint16_t Baud_kbps	�{�[���[�g[kbps]�P��
	uint16_t TxBuffNum	���M�o�b�t�@�̐�
�Ԓl�F
 ����M�o�b�t�@�A�ʐM���e�o�b�t�@�̗̈���m�ۂł��Ȃ����-1
****************************/
int8_t I2c_t::begin(uint16_t Baud_kbps, uint16_t TxBuffNum, uint16_t ComusNum){
	
	if(!fBegun){	// ����̂�
		fBegun = true;
		
		//if(Status!=I2C_IDLE){
			// �ʐM�~�߂�
			
			// �X�e�[�^�X�X�V
			Status = I2C_IDLE;
		//}
		
		if(Baud_kbps<=100){
			Baud_kbps = 100;
		}else{
			Baud_kbps = 400;
		}
		
		this->Baud_kbps = Baud_kbps;
		
		// RIIC setting
		initRegister(Baud_kbps);
		
		// pin mode setting
		setPinModeI2C();
		
		//Buffer
		TxBuff = new RingBuffer<uint8_t>(TxBuffNum);
		if(TxBuff==NULL){
			return -1;
		}
		/*
		// ��M�o�b�t�@�͒ʐM�I�u�W�F�N�g�Ŏ����߁ARIIC�N���X�Ƃ��Ă͎����Ȃ��B
		if(RxBuff) delete RxBuff;
		RxBuff = new RingBuffer<uint8_t>(BuffNum);
		if(RxBuff==NULL){
			return -1;
		}
		*/
		
		// �ʐM���e�o�b�t�@
		this->ComusNum = ComusNum;
		ComusBuff = new RingBuffer<I2c_comu_t*>(this->ComusNum);
		if(ComusBuff==NULL){
			return -1;
		}
	}else{
		return 1;	// begin��
	}
	
	return 0;
}
int8_t I2c_t::begin(uint16_t Baud_kbps, uint16_t TxBuffNum){
	return begin(Baud_kbps, TxBuffNum, I2C_COMUSNUM_DEFAULT);
}
int8_t I2c_t::begin(uint16_t Baud_kbps){
	return begin(Baud_kbps, I2C_BUFFER_SIZE_DEFAULT, I2C_COMUSNUM_DEFAULT);
}
int8_t I2c_t::begin(void){
	return begin(I2C_BAUDRATE_DEFAULT, I2C_BUFFER_SIZE_DEFAULT, I2C_COMUSNUM_DEFAULT);
}



/****************************
 RIIC���W�X�^�ݒ�
�T�v�F
�����F
 �{�[���[�g[kbps](100or400)
�Ԓl�F
 �Ȃ�
****************************/
void I2c_t::initRegister(uint16_t Baud_kbps){
	uint8_t BaudCnt;
	
	// ���W���[���N����
	SYSTEM.PRCR.WORD = 0xA503u;
	switch(_ModuleNum){
	case I2C_RIIC0:
		MSTP(RIIC0) = 0u;
		break;
	case I2C_RIIC1:
		MSTP(RIIC1) = 0u;
		break;
	case I2C_RIIC2:
		MSTP(RIIC2) = 0u;
		break;
	case I2C_RIIC3:
		MSTP(RIIC3) = 0u;
		break;
	}
		
	
	// I2C���Z�b�g
	I2Creg->ICCR1.BIT.ICE = 0;		
	I2Creg->ICCR1.BIT.IICRST = 1;	
	I2Creg->ICCR1.BIT.ICE = 1;	
	
	// �r�b�g���[�g�ݒ�
	//ICMR1.CKS
	//ICBRL 0�`31
	//ICBRH
	//getPCLK();
	//I2C_CLOCK_DIV128
	switch(Baud_kbps){
	case 100:	// �����オ�藧�������莞�Ԃ����ꂼ��1000ns, 300ns�Ƃ���
		I2Creg->ICMR1.BIT.CKS = I2C_CLOCK_DIV8;
		BaudCnt = (uint8_t)((float)::getPCLK() / 8.0 * (1.0/(float)Baud_kbps/1000.0 - 0.0000013));
		// BaudCnt = 53;
		I2Creg->ICBRH.BIT.BRH = BaudCnt/2;
		I2Creg->ICBRL.BIT.BRL = (BaudCnt+1)/2;
		break;
	case 400:	// �����オ�藧�������莞�Ԃ����ꂼ��300ns�Ƃ���
		I2Creg->ICMR1.BIT.CKS = I2C_CLOCK_DIV4;
		BaudCnt = (uint8_t)((float)::getPCLK() / 4.0 * (1.0/(float)Baud_kbps/1000.0 - 0.0000006));
		// BaudCnt = 23;
		I2Creg->ICBRH.BIT.BRH = BaudCnt/2;
		I2Creg->ICBRL.BIT.BRL = (BaudCnt+1)/2;
		break;
	}
	
	//ICMR2
	//ICMR3
	// �^�C���A�E�g�@�\�g���Ȃ�
	/*
	ICMR2.TMWE
	TMOCNTL=00h;
	TMOCNTU=00h;
	ICFER.TMOE=1;
	*/
	
	// �����Ȍ��o�Ƃ��̋@�\�ݒ�
	//ICFER
	I2Creg->ICFER.BIT.MALE = 1;	// �A�[�r�g���[�V�������X�g���o
	
	
	// ���荞�ݐݒ�
	//ICIER
	//��M�f�[�^�t�����荞�݋���
	I2Creg->ICIER.BIT.RIE = 1;
	//���M�f�[�^�G���v�e�B���荞�݋���
	I2Creg->ICIER.BIT.TIE = 1;	
	//���M�I�����荞�݋���
	I2Creg->ICIER.BIT.TEIE = 1;
	//�X�g�b�v�R���f�B�V�������荞�݋���
	I2Creg->ICIER.BIT.SPIE = 1;	
	//NACK��M���荞�݋���(�X���[�u���Ȃ�)
	I2Creg->ICIER.BIT.NAKIE = 1;
	
	// ACKBT�r�b�g�ւ̏������݋���
	I2Creg->ICMR3.BIT.ACKWP = 1;
	
	// �]�����f����
	I2Creg->ICFER.BIT.NACKE = 1;
	
	// �A�[�r�g���[�V�������X�g���o���荞�݋���(EEI���荞��)
	I2Creg->ICIER.BIT.ALIE = 1;
	
	
	switch(_ModuleNum){
	case I2C_RIIC0:
		IEN(RIIC0,RXI0) = 1;
		IPR(RIIC0,RXI0) = 9;
		IEN(RIIC0,TXI0) = 1;
		IPR(RIIC0,TXI0) = 9;
		IEN(RIIC0,TEI0) = 1;
		IPR(RIIC0,TEI0) = 9;
		IEN(RIIC0,EEI0) = 1;
		IPR(RIIC0,EEI0) = 9;
		break;
	case I2C_RIIC1:
		IEN(RIIC1,RXI1) = 1;
		IPR(RIIC1,RXI1) = 9;
		IEN(RIIC1,TXI1) = 1;
		IPR(RIIC1,TXI1) = 9;
		IEN(RIIC1,TEI1) = 1;
		IPR(RIIC1,TEI1) = 9;
		IEN(RIIC1,EEI1) = 1;
		IPR(RIIC1,EEI1) = 9;
		break;
	case I2C_RIIC2:
		IEN(RIIC2,RXI2) = 1;
		IPR(RIIC2,RXI2) = 9;
		IEN(RIIC2,TXI2) = 1;
		IPR(RIIC2,TXI2) = 9;
		IEN(RIIC2,TEI2) = 1;
		IPR(RIIC2,TEI2) = 9;
		IEN(RIIC2,EEI2) = 1;
		IPR(RIIC2,EEI2) = 9;
		break;
	}
	
	// �������Z�b�g����
	I2Creg->ICCR1.BIT.IICRST = 0;
	
}

/****************************
 I2C�֌W���o�̓|�[�g�ݒ�
�T�v�F
�����F
 �Ȃ�
�Ԓl�F
 �Ȃ�
****************************/
void I2c_t::setPinModeI2C(void){
	
	// �|�[�g�o�͐ݒ�
	MPC.PWPR.BYTE = 0x00u;
	MPC.PWPR.BYTE = 0x40u;
	SYSTEM.PRCR.WORD = 0xA503u;
	switch(_ModuleNum){
	case I2C_RIIC0:
		setPinMode(PORT_SCL0, PIN_INPUT);	// output
		MPC.P12PFS.BIT.PSEL = 0x0F;			// SCL0
		PORT1.PMR.BIT.B2 = 1;				// not GPIO	
		setPinMode(PORT_SDA0, PIN_INPUT);	// output
		MPC.P13PFS.BIT.PSEL = 0x0F;			// SDA0 
		PORT1.PMR.BIT.B3 = 1;				// not GPIO	
		break;
	case I2C_RIIC1:
		break;
	case I2C_RIIC2:
		setPinMode(PORT_SCL2, PIN_INPUT);	// output
		MPC.P16PFS.BIT.PSEL = 0x0F;			// SCL2
		PORT1.PMR.BIT.B6 = 1;				// not GPIO	
		setPinMode(PORT_SDA2, PIN_INPUT);	// output
		MPC.P17PFS.BIT.PSEL = 0x0F;			// SDA2 
		PORT1.PMR.BIT.B7 = 1;				// not GPIO	
		break;
	}
	
}


/****************************
 I2C�ʐM�o�^
�T�v�F
�����F
 ���M����A�h���X
 ���M�f�[�^
 ���M�f�[�^��
 ��M�f�[�^
 ��M�f�[�^��
 �R�[���o�b�N�֐�(�ʐM�I�����ɌĂ΂��)
�Ԓl�F
 AttachIndex, 
 -1 : Attach���������ς�(I2C_COMUS)�Ȃ�-1,
 -2 : Comus�̗̈���m�ۂł��Ȃ����-2
****************************/


int8_t I2c_t::attach(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum, void (*CallBackFunc)(I2c_comu_t*)){		// ����M
	int8_t AttachIndex;
	I2c_comu_t* Comu;
	
	if(!ComusBuff->isFull()){
		AttachIndex = ComusBuff->getWriteIndex();
		Comu = new I2c_comu_general_t(AttachIndex, DestAddress, TxData, TxNum, RxNum, CallBackFunc);
		if(Comu == NULL){	// new���s
			return -2;
		}
		ComusBuff->add(Comu);	// �ʐM���e�o�^
		
		if(Status == I2C_IDLE){	// ���M�~�܂��Ă��Ԃ�������
			startComu();	// ���M�J�n
		}
	}else{
		
		// �l�܂��Ă�ۂ��̂�I2C���Z�b�g
		resetI2C();
		
		return -1;
	}
	
	return AttachIndex;
}
// ���M�̂�
int8_t I2c_t::attach(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, void (*CallBackFunc)(I2c_comu_t* Comu)){						// ���M�̂�
	return attach(DestAddress, TxData, TxNum, 0, CallBackFunc);
}
// ���M�̂�(�R�[���o�b�N�֐��Ȃ�)
int8_t I2c_t::attach(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum){		// ���M�̂�
	return attach(DestAddress, TxData, TxNum, 0, NULL);
}
// �ʐM�N���X�ŃA�^�b�`
// -1:�o�b�t�@�l�܂��Ă�(�Ăяo������Comu��delete���Ă�)
// -2:�ʐM�J�n���s
int8_t I2c_t::attach(I2c_comu_t* AttachComu){
	//int8_t AttachIndex;
	
	//AttachIndex = ComusBuff->getWriteIndex();
	if(!ComusBuff->add(AttachComu)){	// �ʐM���e�o�^
	// �o�^����
		if(I2C_IDLE == Status){	// ���M�~�܂��Ă��Ԃ�������
			if(startComu()){	// ���M�J�n
				// �J�n���s
				return -2;
			}
		}
	}else{
		
		// �l�܂��Ă�ۂ��̂�I2C���Z�b�g
		resetI2C();
		
		return -1;
	}
	
	return 0;
}

/****************************
 I2C�ʐM�J�n
����
 �Ȃ�
�Ԓl
 0	:����
 -1	:�ʐM��
 -2	:�ʐM������̂��o�^����Ă��Ȃ�
****************************/
int8_t I2c_t::startComu(void){
	int8_t ack = 0;
	I2c_comu_t* CurrentComuTmp;
	
	//if((Status == I2C_IDLE)&&(!isBusyBus())){	// �ʐM���łȂ�
	if((Status == I2C_IDLE)){	// �ʐM���łȂ�
							// isBusyBus���ĒʐM�I����Ă�����Ƃ��Ȃ���0�ɂȂ�Ȃ��̂��S�z
		if(!ComusBuff->watch(&CurrentComuTmp)){		// �o�b�t�@����ʐM���擾&&�o�b�t�@��߂�
			// �ʐM�J�n
			if(CurrentComuTmp->RxNum > 0){	// ��M������
				ack = transmit_receive(CurrentComuTmp->DestAddress, CurrentComuTmp->TxData, CurrentComuTmp->TxNum, CurrentComuTmp->RxData, CurrentComuTmp->RxNum );	// ����M
			}else{
				ack = transmit(CurrentComuTmp->DestAddress, CurrentComuTmp->TxData, CurrentComuTmp->TxNum);	// ���M
			}
			if(!ack){	// �ʐM�J�n�����Ȃ�CurrentComu�X�V
				ComusBuff->read(&CurrentComu);
			}
		}else{
			ack = -2;	// �o�b�t�@����
		}
	}else{
		ack = -1;	// �܂��ʐM���ł�
	}
	
	return ack;
}
	




/****************************
 I2C���M�J�n(�P�����[�h)
����
 ���M����A�h���X
 ���M�f�[�^
 ���M�f�[�^��
�Ԓl
 0	: ����
 -1	: �X�e�[�^�X��I2C_IDLE����Ȃ����A�o�X���r�W�[���
****************************/
int8_t I2c_t::transmit(uint8_t DstAddress, uint8_t* TrData, uint8_t TrDataNum){
	int8_t ack = 0x00;
	//if((Status == I2C_IDLE)&&(!isBusyBus())){
		// �X�e�[�^�X�X�V
		Status = I2C_TRANSMIT;
		// �o�b�t�@�ɓ���Ă���
		ack |= TxBuff->add(DstAddress<<1);
		for(int i=0; i<TrDataNum; i++){
			ack |= TxBuff->add(*TrData++);
		}
		
		// �J�E���g���Z�b�g
		CntTransmit = TrDataNum;
		
		// �X�^�[�g�r�b�g���s�v��
		reqStartCondition();	// ���ۂɔ��s������TDRE�̊��荞�݂�����A��A�̏������n�܂�B
		
	//}else{
	//	return -1;
	//}
	
	return ack;
}

/****************************
 I2C��M�J�n(�P�����[�h)
����
 ��M����A�h���X
 ��M�f�[�^
 ��M�f�[�^��
�Ԓl
 0	: ����
 -1	: �X�e�[�^�X��I2C_IDLE����Ȃ����A�o�X���r�W�[���
****************************/
int8_t I2c_t::receive(uint8_t DstAddress, uint8_t* RcData, uint8_t RcDataNum){
	int8_t ack;
	//if((Status == I2C_IDLE)&&(!isBusyBus())){
		// �X�e�[�^�X�X�V
		Status = I2C_RECEIVE;
		// ����M���
		ack = TxBuff->add((DstAddress<<1) | 0x01);	// Read���[�h�ɂ���
		RcvBuffer = RcData;
		
		// �J�E���g���Z�b�g
		CntTransmit = 1;
		CntReceive = RcDataNum;
		I2c_t::RcDataNum = RcDataNum;
		
		// �X�^�[�g�r�b�g���s�v��
		reqStartCondition();	// ���ۂɔ��s������TDRE�̊��荞�݂�����A��A�̏������n�܂�B
	//}else{
	//	return -1;
	//}
	return ack;
}

/****************************
 I2C����M�J�n(�P�����[�h)
����
 �ʐM����A�h���X
 ���M�f�[�^
 ���M�f�[�^��
 ��M�f�[�^
 ��M�f�[�^��
�Ԓl
 0	: ����
 -1	: �X�e�[�^�X��I2C_IDLE����Ȃ����A�o�X���r�W�[���
****************************/
int8_t I2c_t::transmit_receive(uint8_t DstAddress, uint8_t* TrData, uint8_t TrDataNum, uint8_t* RcData, uint8_t RcDataNum){
	int8_t ack = 0x00;
	//if((Status == I2C_IDLE)&&(!isBusyBus())){
		// �X�e�[�^�X�X�V
		Status = I2C_TRANSMIT_RECEIVE;
		// �o�b�t�@�ɓ���Ă���
		ack |= TxBuff->add(DstAddress<<1);				// ����A�h���X
		for(uint16_t i=0; i<TrDataNum; i++){
			ack |= TxBuff->add(*TrData++);				// ���M�f�[�^
		}
		RcvBuffer = RcData;
		I2c_t::DstAddress = DstAddress;
		
		
		// �J�E���g���Z�b�g
		CntTransmit = TrDataNum;
		CntReceive = RcDataNum;
		I2c_t::RcDataNum = RcDataNum;
		
		// �X�^�[�g�r�b�g���s�v��
		reqStartCondition();	// ���ۂɔ��s������TDRE�̊��荞�݂�����A��A�̏������n�܂�B
	//}else{
	//	return -1;
	//}
	
	return ack;
}


/****************************
 I2C���Z�b�g����
****************************/
void I2c_t::resetI2C(void){
	//I2c_comu_t* Comu;
	//bool IcferMaleTmp;
	uint8_t ReClkCnt = 8;	// �ǉ��N���b�N�ő吔
	uint16_t ReClkCntC = 400;	// �ǉ��N���b�N�ő吔
	
	//if(0==I2Creg->ICCR1.BIT.SDAI){	// SDA���C����low�ɂȂ��Ă�
			
		Status = I2C_RESET;
		
		
		// ���Z�b�g(initResister���Ŏ��{)
		//I2Creg->ICCR1.BIT.ICE = 0;		
		//I2Creg->ICCR1.BIT.IICRST = 1; // ������ƃ��W�X�^�ݒ肵�Ȃ����v
		
		// RIIC setting
		initRegister(Baud_kbps);
		
		// pin mode setting
		setPinModeI2C();
		
		
		// �o�b�t�@�J��
		/*
		// �o�^���ŗv���������܂܂��Ǝv�����Ⴄ�̂ŏ������Ⴞ�߂ł���ł���
		while(!ComusBuff->isEmpty()){
			ComusBuff->read(&Comu);
			delete Comu;
		}
		*/
		
		// �A�[�r�g���[�V�������X�g���o�@�\��U������
		//IcferMaleTmp = I2Creg->ICFER.BIT.MALE;
		I2Creg->ICFER.BIT.MALE = 0;
		
		// �ǉ��N���b�N�o��
		while(0==I2Creg->ICCR1.BIT.SDAI){	// �X���[�u��low�Œ�ɂ��Ă��
			if(0==I2Creg->ICCR1.BIT.CLO){
				I2Creg->ICCR1.BIT.CLO = 1;		// �ǉ��N���b�N�𑗂��ĊJ������邩����
				// �w��񐔒��������߂�
				ReClkCnt--;	
				if(!ReClkCnt){
					break;
				}
			}
			// �w��񐔒��������߂�
			ReClkCntC--;	
			if(!ReClkCntC){
				break;
			}
		}
		
		//I2Creg->ICFER.BIT.MALE = IcferMaleTmp;
		
		
		reqStartCondition();
	//}
};





/****************************
 I2C���M���荞��
�w��񐔕��f�[�^�𑗂�B

�Ă΂������F
 TDRE��������(ICDRT ���W�X�^����ɂȂ���)
 TRS�r�b�g��������(�X�^�[�g�R���f�B�V���������s���ꂽ)

****************************/
void I2c_t::isrTx(void){
	uint8_t Data;
	
	switch(Status){
	case I2C_TRANSMIT:
		if(I2Creg->ICSR2.BIT.NACKF == 0){
			if(!TxBuff->read(&Data)){	// �f�[�^���邾������
				I2Creg->ICDRT = Data;
				CntTransmit--;
			}
		}else{
			I2Creg->ICSR2.BIT.NACKF = 0;
			isrTxEnd();	// ���M�����I��
		}
		break;
		
	case I2C_RECEIVE:
		// ����A�h���X����
		if(!TxBuff->read(&Data)){
			I2Creg->ICDRT = Data;
		}
		
		break;
	case I2C_TRANSMIT_RECEIVE:
		// [���M]�f�[�^����
		if(CntTransmit>=0){
			if(I2Creg->ICSR2.BIT.NACKF == 0){
				if(!TxBuff->read(&Data)){	// �f�[�^���邾������
					I2Creg->ICDRT = Data;
				}
			}else{
				isrTxEnd();	// ���M�����I��
			}
			CntTransmit--;
		}else{
			
			if(StatusIn == I2C_RESTART){	// ���X�^�[�g�R���f�B�V������������ɗ����I
				// [��M]����A�h���X����
				I2Creg->ICDRT = (DstAddress<<1) | 0x01;
				StatusIn = I2C_TRANSMIT_DSTADDRESS_R;
				// ��M���荞�ݑ҂�
			}
		}
		
		break;
	case I2C_RESET:
		reqStopCondition();
		break;
	}
}


/****************************
 I2C���M�������荞��
���ׂẴf�[�^�̑��M���I��������ɌĂ΂��B
�X�g�b�v�R���f�B�V�����������̓��X�^�[�g�R���f�B�V�����𔭍s����B
****************************/
void I2c_t::isrTxEnd(void){
	I2Creg->ICSR2.BIT.TEND = 0;
	
	switch(Status){
	case I2C_TRANSMIT:
			I2Creg->ICSR2.BIT.STOP = 0;
			reqStopCondition();
		break;
	case I2C_TRANSMIT_RECEIVE:
		if(!I2Creg->ICSR2.BIT.NACKF){
			
			if(CurrentComu->Err == false){
				reqRestartCondition();
				StatusIn = I2C_RESTART;
			}else{
				I2Creg->ICSR2.BIT.STOP = 0;
				reqStopCondition();
			}
		}
		break;
	}
}
/****************************
 I2C�X�g�b�v�R���f�B�V�����������荞��
****************************/
void I2c_t::isrStop(void){
	
	I2Creg->ICMR3.BIT.RDRFS = 0;
	I2Creg->ICMR3.BIT.ACKBT = 0;
	I2Creg->ICSR2.BIT.NACKF = 0;
	I2Creg->ICSR2.BIT.STOP = 0;
	I2Creg->ICFER.BIT.MALE = 1;	// I2C_RESET�̂Ƃ�MALE�I�t�ɂ��Ă�̂ł����ŃI��
	//if(I2C_RESET != Status){
	//void (I2c_comu_t::*pFunc)() = &I2c_comu_t::callBack;
	
	//if(NULL != (CurrentComu->*pFunc())){
		
	if(NULL != CurrentComu){
		// �R�[���o�b�N�֐����s
		CurrentComu->callBack();
		// �ʐM���e����
		delete CurrentComu;
		CurrentComu = NULL;
	}
	TxBuff->clear();
	Status = I2C_IDLE;
	// ���̒ʐM�J�n
	startComu();
	
}


/****************************
 I2C��M���荞��
****************************/
void I2c_t::isrRx(void){
	
	if(CurrentComu->Err == false){
		
		if(!I2Creg->ICSR2.BIT.NACKF){	// ���̃`�F�b�N�͏��񂾂������I
			if((CntReceive<=2) && !I2Creg->ICMR3.BIT.WAIT){
				I2Creg->ICMR3.BIT.WAIT = 1;
			}
			if(CntReceive == 1){
				I2Creg->ICMR3.BIT.RDRFS = 1;	//RDRF(��M�f�[�^�t��)�t���O��SCL�N���b�N��8�N���b�N�ڂ̗����オ�莞�Ɂg1�h�ɂ���
			}
			
			if(CntReceive == 0){
				I2Creg->ICSR2.BIT.STOP = 0;	// �X�g�b�v�R���f�B�V�����o�Ă�t���O�N���A
				reqStopCondition();
				RcvBuffer[RcDataNum-1] = I2Creg->ICDRR;	// �ŏI�f�[�^��M

				I2Creg->ICMR3.BIT.ACKBT = 1;	// �iNACK���M�j�j
				I2Creg->ICMR3.BIT.WAIT = 0;
				// -> �X�g�b�v�r�b�g���s�҂�
				StatusIn = I2C_READ_END;
			}else{
				if(RcDataNum==CntReceive){		// ������T
					0 == I2Creg->ICDRR;	// �_�~�[���[�h(�����]�����Ȃ��Ɠǂ񂾂��ƂɂȂ�Ȃ��݂����B�œK���ŏ����Ȃ����v����)
				}else{	// �f�[�^��M
					RcvBuffer[RcDataNum-CntReceive-1] = I2Creg->ICDRR;

					StatusIn = I2C_READ_DATA;
				}
			}
			
			CntReceive--;
			
		}else{
			// �X���[�u���Ȃ��̂ŃX�g�b�v�R���f�B�V�����o���ďI��
			I2Creg->ICSR2.BIT.STOP = 0;
			reqStopCondition();
			I2Creg->ICDRR;
			
			CurrentComu->Err = true;
		}
	}
	
}



/****************************
 I2C �X���[�u���Ȃ�
****************************/
void I2c_t::isrNack(void){
	StatusIn = I2C_NACK;
	
	CurrentComu->Err = true;
	
	//I2Creg->ICFER.BIT.NACKE = 0;
	I2Creg->ICSR2.BIT.NACKF = 0;
	I2Creg->ICSR2.BIT.STOP = 0;
	reqStopCondition();

}

/****************************
 I2C �A�[�r�g���[�V�������X�g���o
 SDA���C�����X���[�u�ɗ}����ꂿ����Ă�
****************************/
void I2c_t::isrArbitrationLost(void){
	
	CurrentComu->Err = true;
	resetI2C();
	
}


// interrupt
#pragma section IntPRG

#pragma interrupt (Excep_RIIC0_TXI0(enable, vect=VECT(RIIC0,TXI0)))
void Excep_RIIC0_TXI0(void)
{
	I2C0.isrTx();
}
#pragma interrupt (Excep_RIIC0_RXI0(enable, vect=VECT(RIIC0,RXI0)))
void Excep_RIIC0_RXI0(void){
	I2C0.isrRx();
}
#pragma interrupt (Excep_RIIC0_TEI0(enable, vect=VECT(RIIC0,TEI0)))
void Excep_RIIC0_TEI0(void){
	I2C0.isrTxEnd();
}
#pragma interrupt (Excep_RIIC0_EEI0(enable, vect=VECT(RIIC0,EEI0)))
void Excep_RIIC0_EEI0(void){
	//IEN(RIIC0,EEI0) = 0;	// ���g���d�˂ē���Ȃ��悤�ɂ���
	
	if(RIIC0.ICSR2.BIT.STOP){	// �X�g�b�v�R���f�B�V����
		I2C0.isrStop();
	}
	if(RIIC0.ICSR2.BIT.NACKF){	// NACK��M(�X���[�u���Ȃ�)
		RIIC0.ICSR2.BIT.NACKF = 0;
		I2C0.isrNack();
	}
	if(RIIC0.ICSR2.BIT.AL){
		RIIC0.ICSR2.BIT.AL = 0;
		I2C0.isrArbitrationLost();	// �A�[�r�g���[�V�������X�g�Ȃ̂Ń��Z�b�g����
	}
	
	//IEN(RIIC0,EEI0) = 1;
}
#pragma interrupt (Excep_RIIC2_TXI2(enable, vect=VECT(RIIC2,TXI2)))
void Excep_RIIC2_TXI2(void)
{
	I2C2.isrTx();
}
#pragma interrupt (Excep_RIIC2_RXI2(enable, vect=VECT(RIIC2,RXI2)))
void Excep_RIIC2_RXI2(void){
	I2C2.isrRx();
}
#pragma interrupt (Excep_RIIC2_TEI2(enable, vect=VECT(RIIC2,TEI2)))
void Excep_RIIC2_TEI2(void){
	I2C2.isrTxEnd();
}
#pragma interrupt (Excep_RIIC2_EEI2(enable, vect=VECT(RIIC2,EEI2)))
void Excep_RIIC2_EEI2(void){
	if(RIIC2.ICSR2.BIT.STOP){
		I2C2.isrStop();
	}
	if(RIIC2.ICSR2.BIT.NACKF){	// NACK��M(�X���[�u���Ȃ�)
		RIIC2.ICSR2.BIT.NACKF = 0;
		I2C2.isrNack();
	}
	if(RIIC2.ICSR2.BIT.AL){
		RIIC2.ICSR2.BIT.AL = 0;
		I2C2.isrArbitrationLost();	// �A�[�r�g���[�V�������X�g�Ȃ̂Ń��Z�b�g����
	}
}


#pragma section 



/****************************
 �ʐM�f�[�^�N���X �R���X�g���N�^
 ����M
****************************/
I2c_comu_t::I2c_comu_t(void){
	
}

I2c_comu_t::I2c_comu_t(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum){
	setI2c(AttachIndex, DestAddress, TxData, TxNum, RxNum);
}
I2c_comu_t::I2c_comu_t(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum){
	setI2c(0, DestAddress, TxData, TxNum, RxNum);
}


/****************************
 �ʐM�f�[�^�N���X �R���X�g���N�^
 ���M�̂�
  ��M�o�b�t�@��NULL, ��M����0�Ƃ���B
****************************/
I2c_comu_t::I2c_comu_t(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum){
	setI2c(AttachIndex, DestAddress, TxData, TxNum, 0);
}

/****************************
 �ʐM�f�[�^�N���X �f�X�g���N�^
****************************/
I2c_comu_t::~I2c_comu_t(void){
	
	if(TxData){
		delete[] TxData;
	}
	if(RxData){
		delete[] RxData;
	}
}


void I2c_comu_t::setI2c(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum){

	this->AttachIndex = AttachIndex;
	this->DestAddress = DestAddress;
	this->TxNum = TxNum;
	this->RxNum = RxNum;
	if(TxNum>0){
		if(TxNum<0x10){		// ��{�I�ɓ����������������m��(�f�Љ��΍�)
			TxNum = 0x10;
		}
		this->TxData = new uint8_t[TxNum];	// new�ł��Ȃ��������̏������Ăяo�����ŏ�������
	}else{
		this->TxData = NULL;
	}
	
	if(RxNum>0){
		if(RxNum<0x10){
			RxNum = 0x10;
		}
		this->RxData = new uint8_t[RxNum];
	}else{
		this->RxData = NULL;
	}
	
	// ���M�f�[�^���R�s�[����B
	if(NULL!=this->TxData){	// �o�b�t�@�m�ۂł���������
		memcpy(this->TxData, TxData, this->TxNum);
	}
	
	this->Err = false;
}
