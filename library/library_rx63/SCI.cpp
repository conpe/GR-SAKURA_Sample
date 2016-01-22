
#include "SCI.h"


// Sci module 

//���begin����񂾂���


Sci_t Sci5(SCI_SCI5);
Sci_t Sci2(SCI_SCI2);

// Constructor
Sci_t::Sci_t(sci_module SciModule)
{
	
	//���W�X�^
	switch(SciModule){
	case SCI_SCI0:
		SCIreg = &SCI0;
		break;
	case SCI_SCI1:
		SCIreg = &SCI1;
		break;
	case SCI_SCI2:
		SCIreg = &SCI2;
		break;
	case SCI_SCI3:
		SCIreg = &SCI3;
		break;
	case SCI_SCI4:
		SCIreg = &SCI4;
		break;
	case SCI_SCI5:
		SCIreg = &SCI5;
		break;
	}
	
	SciConfig.SciModule = SciModule;
	
	fTdrEmpty = true;
	fTxEndInterruptEnable = false;
	TxEndCallBackFunc = NULL;
	RxCallBackFunc = NULL;
	
	
	TxBuff = NULL;
	RxBuff = NULL;
	
	fEmptySciTxBuff = true;
}

Sci_t::~Sci_t(void){
	// �o�b�t�@�J��	
	delete TxBuff;
	delete RxBuff;
}

int8_t Sci_t::begin(uint32_t BaudRate){
	
	return begin(BaudRate, true, false);	// default:only tx
	
}

int8_t Sci_t::begin(uint32_t BaudRate, bool_t UseTx, bool_t UseRx){
	return begin(BaudRate, UseTx, UseRx, SCI_TX_BUFFER_SIZE_DEFAULT, SCI_RX_BUFFER_SIZE_DEFAULT);
}


int8_t Sci_t::begin(uint32_t BaudRate, bool_t UseTx, bool_t UseRx, int16_t TxBuffSize, int16_t RxBuffSize){
	SciConfig.BaudRate = BaudRate;
	SciConfig.TxEnable = UseTx;
	SciConfig.RxEnable = UseRx;
	SciConfig.TxBuffSize = TxBuffSize;
	SciConfig.RxBuffSize = RxBuffSize;


	// �o�b�t�@�m��
	if(UseTx){	
		if(NULL!=TxBuff){
			delete TxBuff;
		}
		TxBuff = new RingBuffer<uint8_t>(SciConfig.TxBuffSize); 
		if(NULL==TxBuff) return -1;
	}

	if(UseRx){
		if(NULL!=RxBuff){
			delete RxBuff;
		}
		RxBuff = new RingBuffer<uint8_t>(SciConfig.RxBuffSize);
		if(NULL==RxBuff) return -1;
	}
	
	
	initRegister();
	setPinModeSci();
	
	// �t���O������
	fEmptySciTxBuff = true;
	
	State = SCI_BEGIN;
	return 0;
}


void Sci_t::initRegister(void){
	
	SYSTEM.PRCR.WORD = 0xA503u;
	switch(SciConfig.SciModule){
	case SCI_SCI2:
		MSTP(SCI2) = 0;
		break;	
	case SCI_SCI5:
		MSTP(SCI5) = 0;
		break;	
	}
	
	// ������
	SCIreg->SCR.BYTE = 0x00;
	
	// �N���b�N�Z���N�g
	SCIreg->SMR.BIT.CKS = 0;
	
	// �{�[���[�g
	//�{�[���[�g�v�Z
	uint8_t n = SCIreg->SMR.BIT.CKS;
	
	if( (uint16_t)((float)getPCLK() / (unsigned long)((float)(32) * (float)pow(2,(2*n-1))*(float)SciConfig.BaudRate) - 1) > 255) {
		SCIreg->SEMR.BIT.ABCS = 0;
	}else{
		SCIreg->SEMR.BIT.ABCS = 1;
	}
		
	SCIreg->BRR = (uint8_t)( (float)getPCLK() / ((float)(((uint8_t)SCIreg->SEMR.BIT.ABCS&0x01)?32:64) * (float)pow(2, (2*n-1))*(float)SciConfig.BaudRate) - 1);
	/*
	// 115200bps
	SCIreg->SMR.BIT.CKS = 0;
	SCIreg->SEMR.BIT.ABCS = 0;
	SCIreg->BRR = 12;
	*/
	
	// ����M����
	// ���荞�ݗL��, �D��x�ݒ�
	SCIreg->SCR.BYTE = 0x30;	// ����M����
	SCIreg->SCR.BIT.TEIE = 0;	// ���M�������荞��disable(���X1����������enable����)
	SCIreg->SCR.BIT.TIE = 1;	// ���M���荞��enable
	SCIreg->SCR.BIT.RIE = 1;	// ��M���荞��enable
	
	enableInterrupts();
	
	SCIreg->SSR.BYTE &= 0x80; 	//�G���[�t���O�N���A
	
}

void Sci_t::enableInterrupts(void){
	
	switch(SciConfig.SciModule){
	case SCI_SCI2:	// �V���A���ʐM�͂̓��A���^�C�����Ȃ��Ă�������
		IEN(SCI2,TXI2) = 1;
		IPR(SCI2,TXI2) = 7;
		IEN(SCI2,TEI2) = 1;
		IPR(SCI2,TEI2) = 2;
		IEN(SCI2,RXI2) = 1;
		IPR(SCI2,RXI2) = 7;
		break;	
	case SCI_SCI5:
		IEN(SCI5,TXI5) = 1;
		IPR(SCI5,TXI5) = 0x07;
		IEN(SCI5,TEI5) = 1;
		IPR(SCI5,TEI5) = 0x07;
		IEN(SCI5,RXI5) = 1;
		IPR(SCI5,RXI5) = 0x0F;
		break;	
	}
}

void Sci_t::setPinModeSci(void){
	
	MPC.PWPR.BYTE = 0x00u;
	MPC.PWPR.BYTE = 0x40u;
	SYSTEM.PRCR.WORD = 0xA503u;
	
	switch(SciConfig.SciModule){
	case SCI_SCI2:
		if(SciConfig.TxEnable){
			setPinMode(P50, PIN_OUTPUT);	// TX output
			MPC.P50PFS.BIT.PSEL = 0x0A;		// TXD2
			PORT5.PMR.BIT.B0 = 1;			// not GPIO	
		}
		if(SciConfig.RxEnable){
			setPinMode(P52, PIN_INPUT);		// RX input
			MPC.P52PFS.BIT.PSEL = 0x0A;		// RXD2 
			PORT5.PMR.BIT.B2 = 1;			// not GPIO	
		}
		break;	
	case SCI_SCI5:
		if(SciConfig.TxEnable){
			setPinMode(PC3, PIN_OUTPUT);	// TX output
			MPC.PC3PFS.BIT.PSEL = 0x0A;		// TXD5
			PORTC.PMR.BIT.B3 = 1;			// not GPIO	
		}
		if(SciConfig.RxEnable){
			setPinMode(PC2, PIN_INPUT);		// RX input
			MPC.PC2PFS.BIT.PSEL = 0x0A;		// RXD5 
			PORTC.PMR.BIT.B2 = 1;			// not GPIO	
		}
		break;	
	}
	
}


int8_t Sci_t::transmit(uint8_t TrData){
	int8_t ack;
	
	//if(this==&Sci5)	PORTD.PODR.BYTE = 0x44;
	ack = TxBuff->add(TrData);
	if(!ack){
	//	SCIreg->SCR.BIT.TIE = 1;	// ���M�o�b�t�@�󂫊��荞��enable <- ���enable
	//	SCIreg->SCR.BIT.TEIE = 0;	// ���M�������荞��disable
	}
	
	//if((fEmptySciTxBuff==true)||isIdle()){	//���M�����O�o�b�t�@���� �������� ���W���[�����A�C�h����ԂȂ瑗��
	if(needTxStartSequence()||isIdle()){	// TDR���W�X�^�J���Ă� �������� ���W���[�����A�C�h����ԂȂ瑗��(�����̔��q�Ƀt���O�X�V����Ȃ������^�f)
		isrTx();	// Transmit one time
	}
	//if(this==&Sci5)	PORTD.PODR.BYTE = 0x4F;
		
	State = SCI_TX_START;
	return ack;
}

int8_t Sci_t::receive(uint8_t *RcData){
	return RxBuff->read(RcData);
}

uint8_t Sci_t::receive(void){
	uint8_t RcData = 0xff;
	RxBuff->read(&RcData);
	return RcData;
}

void Sci_t::clearRxBuff(void){
	RxBuff->clear();
}


int8_t Sci_t::print(char ch){
	return transmit(ch);
}


int8_t Sci_t::print(int num){
	int16_t Cnt;
	int16_t Num;
	
	char pStrCnst[SCI_MAX_STRING];
	char *pStr = pStrCnst;
	//char *pStr = new char[SCI_MAX_STRING];
	if(NULL==pStr){		// �m�ۂł��Ȃ���΂��̂܂܏o�����Ⴄ
		return -1;	
	}
	
	Num = sprintf(pStr, "%d", num);
	
	for(Cnt=0; Cnt<Num; Cnt++){
		transmit(*pStr++);
	}
	
	//delete[] pStr;
	return 0;
}
	
int8_t Sci_t::print(const char *control, ...){
	int16_t Cnt;
	int16_t Num;
	
	//char *pStr = new char[SCI_MAX_STRING];	// ���I�Ɋm�ۂ��Ă��瑼�̂Ƃ��N�H���Ă��B�Ȃ�ł���B
	char pStrCnst[SCI_MAX_STRING];
	char *pStr = pStrCnst;
	
	if(NULL==pStr){		// �m�ۂł��Ȃ���΂��̂܂܏o�����Ⴄ
		pStr = (char *)control;	
		Num = strlen(control);
	}else{
		va_list arg;
		va_start(arg, control);
		Num = vsprintf(pStr, control, arg);
		va_end(arg);
	}
	for(Cnt=0; Cnt<Num; Cnt++){
		transmit(*pStr++);
	}
	
	//delete[] pStr;
	
	return 0;
}

int8_t Sci_t::println(const char *control, ...){
	int16_t Cnt;
	int16_t Num;
	
	//char *pStr = new char[SCI_MAX_STRING];
	char pStrCnst[SCI_MAX_STRING];
	char *pStr = pStrCnst;
	
	if(NULL==pStr){		// �m�ۂł��Ȃ���΂��̂܂܏o�����Ⴄ
		pStr = (char *)control;	
		Num = strlen(control);
	}else{
		va_list arg;
		va_start(arg, control);
		Num = vsprintf(pStr, control, arg);
		va_end(arg);
	}
	
	for(Cnt=0; Cnt<Num; Cnt++){
		transmit(*pStr++);
	}
	transmit('\n');
	
	//delete[] pStr;
	return 0;
}





/*********************
 ���M���荞�ݏ���
  TDR���W�X�^�󂢂ĂȂ��Ă��������Ⴄ�̂Œ���
**********************/
void Sci_t::isrTx(void){
	uint8_t Data;
	if(!fIsr){	// �������Ɋ��荞�݂Ƃ����Ă�����̂ŁI
		fIsr = true;
	//	if(this==&Sci5) PORTD.PODR.BYTE = 0x4a;
		
		if(!TxBuff->read(&Data)){	//�o�b�t�@�ǂށB�����Ɠǂ߂���set����
		
	//		if(this==&Sci5)	PORTD.PODR.BYTE = 0x40;
		
			
			if(fTxEndInterruptEnable && (TxBuff->isEmpty())){	// ���M�������荞�ݗL��ŁA���X1����̂ő��M���荞��off
				SCIreg->SCR.BIT.TIE = 0;
			}else{
				SCIreg->SCR.BIT.TIE = 1;
			}
				
			// �f�[�^���M
			fTdrEmpty = false;		// ���M���W�X�^�󂢂Ă�t���O�N���A
			
			setSciTxData(Data);		// Transmit data
	//		if(this==&Sci5) PORTD.PODR.BYTE = Data;
			// ���荞�ݐݒ�
			// �Ō�̑��M��������A���M�������荞�݂��g��
			if(fTxEndInterruptEnable && (TxBuff->isEmpty())){	// ���M�������荞��, ���X1�������Ƃ�
				SCIreg->SCR.BIT.TIE = 0;	// ���M�o�b�t�@�G���v�e�B���荞�ݒ�~
				//SCIreg->SSR.BIT.TEND = 0;	// ����0�ɂȂ��Ă�̂ŁATEIE��1�ɂ����u�ԂɊ��荞�ݓ������Ⴄ�̂�j�~����B
				//							// �n�[�h�E�F�A�}�j���A��"35.11 ���荞�ݗv��"�Q��
											// -> read��p�������̂ŁA���荞�ݐ��SSR.TEND���m�F����悤�ɁB
											// -> �܂��A�f�[�^���M����(������TEND��0�ɂȂ�͂�)���犄�荞�ݐݒ�����邱�Ƃł����
											
				SCIreg->SCR.BIT.TEIE = 1;	// ���M�������荞��enable
	//		if(this==&Sci5) PORTD.PODR.BYTE = 0xE0;
			}else{
				SCIreg->SCR.BIT.TIE = 1;
				SCIreg->SCR.BIT.TEIE = 0;
	//		if(this==&Sci5) PORTD.PODR.BYTE = 0xE1;
			}
			
			
		}else{	// �����o�b�t�@����
	//		if(this==&Sci5) PORTD.PODR.BYTE = 0x41;
			
			fTdrEmpty = true;	// ���M���W�X�^�󂢂Ă�t���O�Z�b�g
		}
		
		
		fIsr = false;
	}else{
	//	if(this==&Sci5) PORTD.PODR.BYTE = 0x4b;
	}
	
	//if(this==&Sci5) PORTD.PODR.BYTE = 0xFF;
	
	State = SCI_TX_TXI;
}

/*********************
 ���M�������荞�ݏ���
**********************/
void Sci_t::isrTxEnd(void){
	
	//if(this==&Sci5) PORTD.PODR.BYTE = 0x42;
	
	if(SCIreg->SSR.BIT.TEND){
		if(TxBuff->isEmpty()){	// ���̏����܂łɃo�b�t�@�����Ă��Ă邩����(�n�߂�1�߂̑��M��͂��������Ă��Ă�\������)
			// �o�b�t�@�����ĂȂ��ł��B
			fTdrEmpty = true;			// ���M���W�X�^�󂢂Ă�t���O�N���A
			SCIreg->SCR.BIT.TEIE = 0;	// ���M�������荞�ݖ���
			
			if(NULL!=TxEndCallBackFunc){		// �R�[���o�b�N�֐���attach����Ă�����
				TxEndCallBackFunc();	// �R�[���o�b�N�֐������s
			}
		}else{
			// �����Ƀo�b�t�@���܂��Ă��I
			SCIreg->SCR.BIT.TEIE = 0;	// ���M�������荞�ݖ���
			isrTx();				
		}
	}
	
	//if(this==&Sci5) PORTD.PODR.BYTE = 0xFF;
	State = SCI_TX_TEI;
}


/*********************
 ��M�������荞�ݏ���
**********************/
void Sci_t::isrRx(void){
	//if(this==&Sci5) PORTD.PODR.BYTE = 0x43;
	
	RxBuff->add(getSciRxData());	// �o�b�t�@�ɒ��߂���
		
	if(NULL!=RxCallBackFunc){		// �R�[���o�b�N�֐���attach����Ă�����
		RxCallBackFunc(RxBuff);	// �R�[���o�b�N�֐������s
	}
	
	//if(this==&Sci5) PORTD.PODR.BYTE = 0xFF;
	State = SCI_RX_RXI;
}








// interrupt
#pragma section IntPRG
// tx
#ifndef TARGET_BOARD	// GR-SAKURA�p�v���W�F�N�g�̎��͎g���Ȃ�
#pragma interrupt (Excep_SCI2_TXI2(vect=VECT(SCI2,TXI2), enable))
void Excep_SCI2_TXI2(void){
	setpsw_i();	//���d���荞�݋���
	Sci2.isrTx();
}
#endif

#pragma interrupt (Excep_SCI5_TXI5(vect=VECT(SCI5,TXI5), enable))
void Excep_SCI5_TXI5(void)
{
	setpsw_i();	//���d���荞�݋���
	// PORTD.PODR.BYTE = 0x4c;
	Sci5.isrTx();
	// PORTD.PODR.BYTE = 0x4d;
}

// ���M�������荞��
#ifndef TARGET_BOARD	// GR-SAKURA�p�v���W�F�N�g�̎��͎g���Ȃ�
#pragma interrupt (Excep_SCI2_TEI2(vect=VECT(SCI2,TEI2), enable))
void Excep_SCI2_TEI2(void)
{
	setpsw_i();	//���d���荞�݋���
	Sci2.isrTxEnd();
}
#endif

#pragma interrupt (Excep_SCI5_TEI5(vect=VECT(SCI5,TEI5), enable))
void Excep_SCI5_TEI5(void)
{
	setpsw_i();	//���d���荞�݋���
	Sci5.isrTxEnd();
}

//rx
#ifndef TARGET_BOARD	//
#pragma interrupt (Excep_SCI2_RXI2(vect=VECT(SCI2,RXI2), enable))
void Excep_SCI2_RXI2(void){
	setpsw_i();	//���d���荞�݋���
	Sci2.isrRx();
}
#endif

#pragma interrupt (Excep_SCI5_RXI5(vect=VECT(SCI5,RXI5), enable))
void Excep_SCI5_RXI5(void){
	setpsw_i();	//���d���荞�݋���
	// PORTD.PODR.BYTE = 0x4e;
	Sci5.isrRx();
	// PORTD.PODR.BYTE = 0x4f;
}

// TXI0, RXI0, RXI2�͂ǂ����Ŏg����(���荞�݃x�N�^�[�̐ݒ肪����)���ۂ��̂ŗv�m�F
#pragma section 



