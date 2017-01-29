/********************************************/
/*		�t�^�o�̃R�}���h�T�[�{				*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/07/19		*/
/********************************************/

#include "FutabaCmdServo.h"

// �e���W�X�^�̃A�h���X�ƃf�[�^��
//  �z��̈ʒu��ftcmdsv_comu_content�̒l�ɑ���
const uint8_t futaba_cmd_servo::ResisterAdrsLength[24][2] = {
	{0x04, 1},
	{0x05, 1},
	{0x06, 1},
	{0x07, 1},
	{0x08, 2},
	{0x0A, 2},
	{0x0E, 2},
	{0x16, 1},
	{0x17, 1},
	{0x18, 1},
	{0x19, 1},
	{0x1A, 1},
	{0x1B, 1},
	{0x1C, 2},
	{0x1E, 2},	// RAM 
	{0x20, 2},
	{0x23, 1},
	{0x24, 1},
	{0x2A, 2},
	{0x2C, 2},
	{0x2E, 2},
	{0x30, 2},
	{0x32, 2},
	{0x34, 2}
};

bool_t futaba_cmd_servo::fRcv = false;
RingBuffer<futaba_cmd_servo_comu_t*> futaba_cmd_servo::ComusBuff(FTCMDSV_COMUSNUM_DEFAULT);

futaba_cmd_servo_comu_t* futaba_cmd_servo::CurrentComu = NULL;
bool_t futaba_cmd_servo::fAttaching = false;
bool_t futaba_cmd_servo::fManaging = false;

/*********************
�T�[�{(SCI������)
�����F	SCI���W���[���ԍ�
		�{�[���[�g (9600�`230400)
**********************/
futaba_cmd_servo::futaba_cmd_servo(uint8_t ID, Sci_t* Sci, uint32_t BaudRate){
	
	futaba_cmd_servo::ID = ID;
	
	ServoSci = Sci;
	this->BaudRate = BaudRate;
	
	FailCnt = 0;
	
	fRcv = false;
	fAttaching = false;
	fManaging = false;
	
	LimitCW = FTCMDSV_LIMIT_CW;
	LimitCCW = FTCMDSV_LIMIT_CCW;
}


futaba_cmd_servo::~futaba_cmd_servo(void){
	//delete ComusBuff;
}



int8_t futaba_cmd_servo::begin(void){
	
	if(ServoSci->begin(BaudRate, true, true)){	// use tx, rx
		return -2;
	}
	//ServoSci->enableTxEndInterrupt();
	ServoSci->attachTxEndCallBackFunction(intTxEnd);
	
	return 0;
}

void futaba_cmd_servo::setLimit(int16_t cw, int16_t ccw){
	LimitCW = cw;
	LimitCCW = ccw;
}


// ROM�̈�

/*********************
�T�[�{ID�ݒ�
�T�v�F
	�T�[�{��ID��ύX����B
	�V����ID���������݁A�ۑ�����B
�����F
		newID	�F�V�����T�[�{ID
**********************/
void futaba_cmd_servo::setID(uint8_t newID){
	
	
	sendShortPacket(CMDSV_SERVOID, 0x00, CMDSV_ADRS_ServoID, 1, 1, &newID);
	
	ID = newID;
	
	waitEndCommand();
	writeROM();
	waitEndCommand();
	reset();
	waitEndCommand();
}

/*********************
�ʐM���x�ݒ�
�����F
		newBR	�F�V�����ʐM���x(CMDSV_BR_115200,...)
**********************/
void futaba_cmd_servo::setBaudRate(uint8_t newBR){

	sendShortPacket(CMDSV_BAUDRATE, 0x00, CMDSV_ADRS_BaudRate, 1, 1, &newBR);
	waitEndCommand();
	writeROM();
	waitEndCommand();
	reset();
	waitEndCommand();
}



/*********************
ROM�ɏĂ�
�����F	
**********************/
void futaba_cmd_servo::writeROM(void){
	sendShortPacket(CMDSV_WRITEROM, 0x40, 0xFF, 0, 0, &ID);	//&ID�̓_�~�[
	waitEndCommand();
}

/*********************
�T�[�{�����Z�b�g
�����F	
**********************/
void futaba_cmd_servo::reset(void){
	sendShortPacket(CMDSV_RESET, 0x20, 0xFF, 0, 0, &ID);	//&ID�̓_�~�[
	waitEndCommand();
}


// RAM�̈�

/*********************
�ڕW�ʒu�ݒ�
�����F
		angle	�F�ڕW�p�x(0.1�x�P�� ��F90.2�x��902 )
**********************/
void futaba_cmd_servo::setGoalPosition(int16_t angle){
	uint8_t ang[2];
	
	// �ő��]�p�x�𐧌�
	if(LimitCW<angle){
		angle = LimitCW;
	}else if(LimitCCW>angle){
		angle = LimitCCW;
	}
	
	
	ang[0] = angle&0x00FF;
	ang[1] = (angle>>8)&0xFF;
	
	writeMemory(CMDSV_GOALPOSITION, ang);
	
	this->GoalPosition = angle;
	
}

/*********************
�ړ����Ԑݒ�
�����F
		TimeMs	�F�ڕW���� [ms]
**********************/
void futaba_cmd_servo::setGoalTime(uint16_t TimeMs){
	uint8_t gt[2];
	uint16_t Time = TimeMs/10;
	
	Time = Time&0x3FFF;
	
	gt[0] = Time&0x00FF;
	gt[1] = (Time>>8)&0xFF;
	
	writeMemory(CMDSV_GOALTIME, gt);
	
}


/*********************
�ő�g���N
�����F	
	Torque : �ő�g���N [%]
**********************/
void futaba_cmd_servo::setMaxTorque(uint8_t MaxTorque){
	
	writeMemory(CMDSV_MAXTORQUE, &MaxTorque);
	
}

/*********************
�g���NEnable
�����F
		enagle	�Ffutaba_cmd_servo::TORQUE_ON, futaba_cmd_servo::TORQUE_OFF, futaba_cmd_servo::TORQUE_BRAKE
**********************/
int8_t futaba_cmd_servo::enableTorque(ftcmdsv_torque_enable Enable){
	
	return writeMemory(CMDSV_TORQUEENABLE, (uint8_t*)&Enable);
	
}





	
/*********************
�V���[�g�p�P�b�g�f�[�^����
 TxPacket�ɑ���p�P�b�g�f�[�^��new���ĕԂ��̂ŁA
 �g���I�������delete���邱�ƁB
�����F	
		**TxPacket: ���M�p�P�b�g(�߂�)
		Flag	�F�t���O����
		Address	�F�������A�h���X
		Length	�F�f�[�^�̒���(���M���͑��M�f�[�^��, ��M���͎�M�f�[�^��, �R�}���h����Ƃ���0, ����������0xFF)
		Cnt		: �T�[�{�̐�(�V���[�g�p�P�b�g�ł͊�{1�B�R�}���h���邾���Ƃ��̎���0)
		*Data	�F�f�[�^
�ߒl�F	
	�p�P�b�g�̒���
**********************/
int8_t futaba_cmd_servo::genShortPacket(uint8_t **TxPacket, uint8_t Flag, uint8_t Address, uint8_t Length, uint8_t Cnt, uint8_t *TrData){
	uint8_t i;
	uint8_t Sum;
	
	
	uint8_t TxNum = 7+Length*Cnt+1;
	
//	PORTD.PODR.BYTE = 0x81;
	
	*TxPacket = new uint8_t[TxNum];	// �w�b�_+�f�[�^+�`�F�b�N�T��
		if(NULL==TxPacket) __heap_chk_fail();
	if( NULL == *TxPacket){
		return -1;
	}
	
	(*TxPacket)[0] = 0xFA;
	(*TxPacket)[1] = 0xAF;
	(*TxPacket)[2] = this->ID;
	(*TxPacket)[3] = Flag;
	(*TxPacket)[4] = Address;
	(*TxPacket)[5] = Length;
	(*TxPacket)[6] = Cnt;
	Sum =  this->ID ^ Flag ^ Address ^ Length ^ Cnt;
	if(0!=Length*Cnt){
		for(i = 0; i<Length*Cnt; i++){
			(*TxPacket)[7+i] = *TrData;
			Sum = Sum ^ *TrData;
			TrData++;
		}
	}
	(*TxPacket)[TxNum-1] = Sum;
	
	
	return TxNum;
}
// �̈�͊O�Ŋm�ۂ����
int8_t futaba_cmd_servo::genShortPacket(uint8_t *TxPacket, uint8_t Flag, uint8_t Address, uint8_t Length, uint8_t Cnt, uint8_t *TrData){
	uint8_t i;
	uint8_t Sum;
	
	uint8_t TxNum = 7+Length*Cnt+1;
	
	(TxPacket)[0] = 0xFA;
	(TxPacket)[1] = 0xAF;
	(TxPacket)[2] = this->ID;
	(TxPacket)[3] = Flag;
	(TxPacket)[4] = Address;
	(TxPacket)[5] = Length;
	(TxPacket)[6] = Cnt;
	Sum =  this->ID ^ Flag ^ Address ^ Length ^ Cnt;
	if(0!=Length*Cnt){
		for(i = 0; i<Length*Cnt; i++){
			(TxPacket)[7+i] = *TrData;
			Sum = Sum ^ *TrData;
			TrData++;
		}
	}
	(TxPacket)[TxNum-1] = Sum;
	
	
	return TxNum;
}


/*********************
�V���[�g�p�P�b�g���M
�@��̃T�[�{�Ƀf�[�^�𑗂�
�@�T�[�{�ʐM�N���X��new����attachSciComu�ɓ�����
�����F	
		Flag	�F�t���O����
		Address	�F�������A�h���X
		Length	�F1�T�[�{������̃f�[�^�̒���(����)
		Cnt		�F�T�[�{�̐�
		*Data	�F�f�[�^ sizeof(Data) = Length*Cnt
**********************/
int8_t futaba_cmd_servo::sendShortPacket(ftcmdsv_comu_content ComuType, uint8_t Flag, uint8_t Address, uint8_t Length, uint8_t Cnt, uint8_t *TrData){
	uint8_t TxData[10];	// �ő��10�o�C�g�܂ł�������Ȃ��̂ŁBCnt=1�̏ꍇ�B
	int8_t TxNum;
	futaba_cmd_servo_comu_t* NewComu;
	uint8_t RxNum;
	int8_t ack;
	
	
	if(fAttaching){
		return -1;
	}
	
	fAttaching = true;
	
	if(Flag == 0x0F){	// ��M
		RxNum = Length;
	}else{
		RxNum = 0;
	}
	
	TxNum = genShortPacket(TxData, Flag, Address, Length, Cnt, TrData);	// ���M�f�[�^����
	
	if(TxNum<0){
		fAttaching = false;
		return -0x10;
	}
	
	NewComu = new futaba_cmd_servo_comu_t(this, ComuType, TxData, (uint8_t)TxNum, RxNum);
		if(NULL==NewComu) __heap_chk_fail();
	if(NULL == NewComu){
		// �ʐM�p�P�b�g�̈�m�ۂł��Ȃ�����
		fAttaching = false;
		return -0x20;
	}
	if(0<TxNum){
		if(NULL == NewComu->TxData){
			// ���M�o�b�t�@�m�ۂł��Ȃ�����
			fAttaching = false;
			delete NewComu;
			return -0x21;
		}
	}
	if(0<RxNum){
		if(NULL == NewComu->RxData){
			//��M����Ȃ̂Ɏ�M�o�b�t�@�m�ۂł��Ȃ�����
			fAttaching = false;
			delete NewComu;
			return -0x22;
		}
	}
	
	ack = attachComu(NewComu);	// ���M�o�b�t�@�ɓo�^
	if(ack){	// �o�^���s������Ȃ��������Ƃ�
		delete NewComu;
	}
	
	fAttaching = false;
	return ack;
}

/*********************
�f�[�^����
�V���[�g�p�P�b�g�������ĒʐM�o�b�t�@�ɓ˂�����
�����F	ID		�F�T�[�{ID
		Flag	�F�t���O���� ��{�[���BROM�������胊�Z�b�g����Ƃ��Ɏg��
		Address	�F�������A�h���X
		Cnt		�F����T�[�{�� ��{1�ŁB
		Length	�F�f�[�^�̒���
		*Data	�F�f�[�^
�ߒl�F
	�G���[���
**********************/
int8_t futaba_cmd_servo::writeMemory(ftcmdsv_comu_content ComuType, uint8_t *TrData){
	uint8_t Flag, Address, Length, Cnt;
	
	Flag = 0;	// ���M�̂�
	Address = ResisterAdrsLength[ComuType][0];
	Length = ResisterAdrsLength[ComuType][1];
	Cnt = 1;
	
	return sendShortPacket(ComuType, Flag, Address, Length, Cnt, TrData);
}


/*********************
�f�[�^�ǂ�
�����F	ID		�F�T�[�{ID
		Address	�F�������A�h���X
		RcvNum	�F��M�f�[�^��
		*RcData	�F�f�[�^�Ԃ��Ƃ�
**********************/
void futaba_cmd_servo::readMemory(uint8_t Address, uint8_t RcvNum, uint8_t *RcData){
	
	uint8_t i;
	uint8_t RcDataTmp;
	
	// ���M�I���܂ő҂�
	while(!ServoSci->isIdle());
	
	
	//���M�s�����n�C�C���s�[�_���X��
	//�����܂łɎ����ő��M�������Ƃɂ��o�b�t�@�I�[�o�[�����G���[������
	// -> �Ƃ肠�����Ȃ���
	
	//�����܂ł̎�M�f�[�^�̂Ă�
	ServoSci->clearRxBuff();

	while(RcDataTmp!=0xFD){
		ServoSci->receive(&RcDataTmp);
	}
	while(RcDataTmp!=0xDF){
		ServoSci->receive(&RcDataTmp);
	}
		
	while(ServoSci->receive(&RcDataTmp));	//ID
	while(ServoSci->receive(&RcDataTmp));	//Flag
	while(ServoSci->receive(&RcDataTmp));	//Address
	while(ServoSci->receive(&RcDataTmp));	//Length
	while(ServoSci->receive(&RcDataTmp));	//Cnt
			
	for(i=0; i<RcvNum; i++){
		while(ServoSci->receive(RcData));
		RcData++;
	}
			

};

// �p�P�b�g�ʐM��
int8_t futaba_cmd_servo::reqReadMemory(ftcmdsv_comu_content ComuType){
	uint8_t Flag, Address, RcvNum, Cnt;
	
	Flag = 0x0F;	// ���M�̂�
	Address = ResisterAdrsLength[ComuType][0];
	RcvNum = ResisterAdrsLength[ComuType][1];
	Cnt = 0;
	
	//��M�v��
	return sendShortPacket(ComuType, Flag, Address, RcvNum, Cnt, NULL);
}


int8_t futaba_cmd_servo::attachComu(futaba_cmd_servo_comu_t* NewComu){
	
	
	
	if(!ComusBuff.add(NewComu)){	// �o�b�t�@�ɒǉ�
		manageComuStart();	//�V�K�ǉ������̂ő��M���Ă݂��
		FailCnt = 0;
	}else{
		// ComusBuff�������ς��B
		// �������������M�҂��Ŏ~�܂��Ă邩��
		// �J�E���^���}�b�N�X�������烊�Z�b�g��������B
		FailCnt++;
		//FailCnt = 0;
		if(FTCMDSV_FAILCNT_MAX<FailCnt){
			
			
			fRcv = false;
			endRcv();
			
			// �o�b�t�@����
			futaba_cmd_servo_comu_t* ComuTmp;
			
			while(BUFFER_READ_OK == ComusBuff.read(&ComuTmp)){
				if(NULL!=ComuTmp){
					delete ComuTmp;
				}
			}
			
			if(NULL!=CurrentComu){
				delete CurrentComu;
				CurrentComu = NULL;
			}
			
			FailCnt = 0;	// ���������Ƃ��悤�B
			
			if(BUFFER_ADD_OK == ComusBuff.add(NewComu)){
				manageComuStart();	// ���M���Ă݂�
				return 0;
			}
			
			
		}
		return -1;	// �ǉ����s
	}
	
	return 0;
}



/*********************
�R�}���h���M��ɑ҂�
�����F	
**********************/
void futaba_cmd_servo::waitEndCommand(void){
	
	volatile uint16_t i;
	
	for(i=0;i<60000;i++);
	
}




/*********************
// static�Ȑl����
**********************/
/*********************
�ʐM����
���̃p�P�b�g�𑗐M����
**********************/
int8_t futaba_cmd_servo::manageComuStart(void){
	uint8_t i;
	futaba_cmd_servo_comu_t* CurrentComuTmp;
	
	
	if(!fRcv){	//��M�҂��łȂ����
		
		
		// ��M�҂����łȂ��̂Ńp�P�b�g���M����
		if(0==ComusBuff.watch(&CurrentComuTmp)){		// �ʐM�I�u�W�F�N�g����ǂ�
			
			if(ServoSci->getTxBuffFreeSpace() >= CurrentComuTmp->TxNum){		// ���M���������ȏ��SCI���M�o�b�t�@���J���Ă�
			
				if(fManaging){
					return -1;
				}
				fManaging = true;
	
				// ��M����Ȃ�t���O���ĂĂ���
				if(CurrentComuTmp->RxNum > 0){		// ��M�������Ȃ��M����
					fRcv = true;
				}else{
					
				}
				// SCI�Ƀf�[�^����
				if(0<CurrentComuTmp->TxNum){
					// ���M�������荞��disable
					//ServoSci->disableTxEndInterrupt();
					ServoSci->detachTxEndCallBackFunction();
					for(i=0; i<CurrentComuTmp->TxNum; i++){
						if(i>=CurrentComuTmp->TxNum-1){
							//ServoSci->enableTxEndInterrupt();	// �Ō�̈��ŗL���ɂ���
							ServoSci->attachTxEndCallBackFunction(intTxEnd);
						}
						ServoSci->transmit(CurrentComuTmp->TxData[i]);
					}
				}
				//ServoSci->disableTxEndInterrupt();
				
				// CurrentComu�X�V
				if(NULL!=CurrentComu){
					delete CurrentComu;				// �O�܂ŒʐM���Ă��I�u�W�F�N�g������������폜
					CurrentComu = NULL;
				}
				
				ComusBuff.read(&CurrentComu);	// �ʐM�I�u�W�F�N�g������o���ăJ�����g��
				
				fManaging = false;
				
				if(!fRcv){
					return manageComuStart();	//�ʐM�I�u�W�F�N�g�������J��Ԃ��B��M���͎�M�҂��ɂȂ�̂ŌJ��Ԃ��Ȃ��B
				}
			}
		}
	}else{
	}
	
	return 0;
}


// ���M����
void futaba_cmd_servo::intTxEnd(void){
	if(NULL != CurrentComu){
		if(fRcv){
			CurrentComu->FTCMDSV->startRcv();	// ��M�J�n
		}else{
			CurrentComu->FTCMDSV->manageComuStart();
		}
	}else{
		
	}
}

// ��M�J�n
void futaba_cmd_servo::startRcv(void){
	
	// ���M�������荞��disable
	//ServoSci->disableTxEndInterrupt();
	ServoSci->detachTxEndCallBackFunction();
	
	// ��M���荞��on // �f�t�H��on
	// ��M�o�b�t�@����
	ServoSci->clearRxBuff();
	// �R�[���o�b�N�o�^
	ServoSci->attachRxCallBackFunction(intRx);
	
}
// ��M�I���
void futaba_cmd_servo::endRcv(void){
	// ��M���荞��off // �f�t�H��on
	// �R�[���o�b�N������
	ServoSci->detachRxCallBackFunction();
	
}



// ��M
void futaba_cmd_servo::intRx(RingBuffer<uint8_t> * RxBuff){
	if(fRcv){
		if(NULL!=CurrentComu){
			CurrentComu->FTCMDSV->manageComuReceive(RxBuff);	// ��M����
		}
	}
	
}

// ��M�f�[�^�̃p�P�b�g�����
void futaba_cmd_servo::manageComuReceive(RingBuffer<uint8_t> * RxBuff){
	uint8_t RcDataTmp;
			
	while(!RxBuff->isEmpty()){	// �o�b�t�@�Ȃ��Ȃ�܂ŌJ��Ԃ�
		if(CurrentComu->ReceivedNum==0){
			ServoSci->receive(&RcDataTmp);	// 0xFD // ������ł�buff����ǂނ̂Ō��ǈꏏ�ł�
			//RxBuff->read();
			CurrentComu->ReceivedNum++;
		}else if(CurrentComu->ReceivedNum==1){
			ServoSci->receive(&RcDataTmp);	// 0xDF
			//RxBuff->read();
			CurrentComu->ReceivedNum++;
			CurrentComu->Sum = 0;
		}else if(CurrentComu->ReceivedNum==2){
			ServoSci->receive(&RcDataTmp);	//ID
			//RxBuff->read();
			CurrentComu->ReceivedNum++;
			CurrentComu->Sum ^= RcDataTmp;
		}else if(CurrentComu->ReceivedNum==3){
			ServoSci->receive(&RcDataTmp);	//Flag
			//RxBuff->read();
			CurrentComu->ReceivedNum++;
			CurrentComu->Sum ^= RcDataTmp;
		}else if(CurrentComu->ReceivedNum==4){
			ServoSci->receive(&RcDataTmp);	//Address
			//RxBuff->read();
			CurrentComu->ReceivedNum++;
			CurrentComu->Sum ^= RcDataTmp;
		}else if(CurrentComu->ReceivedNum==5){
			ServoSci->receive(&RcDataTmp);	//Length
			//RxBuff->read();
			CurrentComu->ReceivedNum++;
			CurrentComu->Sum ^= RcDataTmp;
		}else if(CurrentComu->ReceivedNum==6){
			ServoSci->receive(&RcDataTmp);	//Cnt
			//RxBuff->read();
			CurrentComu->ReceivedNum++;
			CurrentComu->Sum ^= RcDataTmp;
		}else if(CurrentComu->ReceivedNum < (7+CurrentComu->RxNum)){	// ��M���e
			ServoSci->receive(&RcDataTmp);
			//RxBuff->read(&RcDataTmp);
			CurrentComu->RxData[CurrentComu->ReceivedNum-7] = RcDataTmp;
			
			CurrentComu->ReceivedNum++;
			CurrentComu->Sum ^= RcDataTmp;
			
		}else{	// �`�F�b�N�T��
			ServoSci->receive(&RcDataTmp);	// Sum
			//RxBuff->read();
			// ��M�f�[�^����
			if(CurrentComu->Sum == RcDataTmp){	// ��M�I�b�P�[
				fetchRcvData(CurrentComu);
			}
			// ���̒ʐM�J�n
			endRcv();		// ��M���[�h�I��
			fRcv = false;		// ��M���t���O��艺��
			manageComuStart();	//���M�f�[�^���܂��Ă��瑗��
		}
	}
	
}

void futaba_cmd_servo::fetchRcvData(futaba_cmd_servo_comu_t* Comu){
	uint16_t retdata_uint16;
	
	switch(Comu->ComuType){
	case CMDSV_PRESENTPOSITION:
		
		retdata_uint16 = ( (((int16_t)Comu->RxData[1])<<8) | (int16_t)Comu->RxData[0]);
		
		if( retdata_uint16 > 65535/2){
			ResisterData.PresentPosition = (int16_t)(retdata_uint16 - 65536);
		}else{
			ResisterData.PresentPosition = (int16_t)retdata_uint16;
		}
		break;
	case CMDSV_PRESENTTIME:
		ResisterData.PresentTime = ( (((uint16_t)Comu->RxData[1])<<8) | (uint16_t)Comu->RxData[0]);
		break;
	case CMDSV_PRESENTSPEED:
	
		retdata_uint16 = ( (((int16_t)Comu->RxData[1])<<8) | (int16_t)Comu->RxData[0]);
		
		if( retdata_uint16 > 65535/2){
			ResisterData.PresentSpeed = (int16_t)(retdata_uint16 - 65536);
		}else{
			ResisterData.PresentSpeed = (int16_t)retdata_uint16;
		}
		break;
	case CMDSV_PRESENTCURRENT:
		ResisterData.PresentCurrent = ( (((uint16_t)Comu->RxData[1])<<8) | (uint16_t)Comu->RxData[0]);
		break;
	case CMDSV_PRESENTVOLTS:
		ResisterData.PresentVolts = ( (((uint16_t)Comu->RxData[1])<<8) | (uint16_t)Comu->RxData[0]);
		break;
	case CMDSV_PRESENTTEMPERATURE:
	
		retdata_uint16 = ( (((int16_t)Comu->RxData[1])<<8) | (int16_t)Comu->RxData[0]);
		
		if( retdata_uint16 > 65535/2){
			ResisterData.PresentTemperature = (int16_t)(retdata_uint16 - 65536);
		}else{
			ResisterData.PresentTemperature = (int16_t)retdata_uint16;
		}
		break;
		
		
		
	}
	
}