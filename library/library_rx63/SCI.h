/********************************************/
/*		SCI���W���[�����g�p����SCI�ʐM		*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/07/19		*/
/********************************************/

// �Ƃ肠����SCI2, 5�̂ݎ���
// SCI2�͎�M,���M�������荞�݂��ł��Ȃ�(�L�E�ցE�M)��GR-SAKURA

// ���M�v���o����(transmit)��x�����O�o�b�t�@�ɒ~�����A���M�o�b�t�@�G���v�e�B���荞�݂ɂ���ď������M���Ă����B
// ��M�͏�Ɋ��荞�݂Ń����O�o�b�t�@�ɒ~������Breceive�œǂݏo���B
//  �e�o�b�t�@�́A�����ς��ɂȂ��-1��Ԃ��������܂Ȃ��B

//�y�f�t�H���g�ݒ�z
// ���M���荞�� on(�K�{)
// ���M�������荞�� off
// ��M���荞�� on(�K�{)




// �߂�����^�C�~���O�������ƁA1�o�C�g�̂ݒP���ő���Ƒ����Ȃ������I
// (�u�o�b�t�@���ׂĂ̑��M�I���̏u�ԂɐV����1�o�C�g�ǉ��v�Ŕ�������\������)
// ����1�o�C�g����Η��ꂾ���̂ł��܂�C�ɂ��Ȃ��Ă����������B

//�y�X�V�����z
// 2015.07.19 ���荞�ݎ��萮��
// 2015.11.28 print���\�b�h�A���I�Ɋm�ۂ��Ă��瑼�̂Ƃ��N�H���Ă��̂ŐÓI�Ɋm�ۂ���悤�ɕύX�B

//�y�X�V�������z
// ���W�X�^�ݒ�֘A��ʃt�@�C����
// �p�P�b�g�ʐM�������ꂽ��
// SCI2�g���Ȃ��̒���

#ifndef __SCI_H__
#define __SCI_H__

#include <machine.h>	// �}�C�R���R���p�C���ŗL�֐�

#include <stdio.h>
#include <stdarg.h>

#include <math.h>
#include "iodefine.h"
#include "CommonDataType.h"
#include "RingBuffer.h"
#include "portReg.h"

// �����O�o�b�t�@�̃T�C�Y
#define SCI_TX_BUFFER_SIZE_DEFAULT 255
#define SCI_RX_BUFFER_SIZE_DEFAULT 127

#define SCI_MAX_STRING 127

extern uint32_t getPCLK(void);


enum sci_module
{
	SCI_SCI0 = 0,
	SCI_SCI1 = 1,
	SCI_SCI2 = 2,
	SCI_SCI3 = 3,
	SCI_SCI4 = 4,
	SCI_SCI5 = 5,
};
/*
enum Sci_pin
{
};
*/

// debug�p
class Sci_t;
extern Sci_t Sci2;


struct sci_config
{
	sci_module		SciModule;
	bool_t			TxEnable;
	bool_t			RxEnable;
	uint32_t		BaudRate;
	int16_t			TxBuffSize;
	int16_t			RxBuffSize;
};


enum Sci_state
{
	SCI_BEGIN = 0,
	SCI_TX_START = 1,
	SCI_TX_TXI = 2,
	SCI_TX_TEI = 3,
	SCI_RX_RXI = 4
};

class Sci_t
{
private:
	Sci_state State;

	sci_config SciConfig;
	RingBuffer<uint8_t> *TxBuff;
	RingBuffer<uint8_t> *RxBuff;
	
	bool_t fEmptySciTxBuff;
	bool_t fTxEndInterruptEnable;
	
	bool_t fTdrEmpty;	// TDR�Ƀf�[�^�L��
	bool_t fIsr;		// ���M���荞�ݒ�����I���̊Ԃɑ��M��isr���\�b�h�Ă΂Ȃ��ł�I
	
	// ���W�X�^
	volatile __evenaccess struct st_sci0 *SCIreg;
	
	void initRegister(void);
	void setPinModeSci(void);
	
	// ���W�X�^�@��
	void setSciTxData(uint8_t TxData){SCIreg->TDR = TxData;};
	//uint8_t getSciRxData(void){return SCIreg->RDR;};
	
	// ��M�R�[���o�b�N
	void (*TxEndCallBackFunc)(void);	//���M�����R�[���o�b�N�֐��ւ̃|�C���^
	void (*RxCallBackFunc)(RingBuffer<uint8_t> * RxBuff);	//��M�R�[���o�b�N�֐��ւ̃|�C���^
	
	// ���M�J�n����(isrTx�����s)���Ă���[����(=TDR���W�X�^�󂢂Ă�&&isrTx�������łȂ�)
	bool_t needTxStartSequence(void){return fTdrEmpty&&(!fIsr);};
	
public:
	// Constructor
	Sci_t(sci_module SciModule);
	~Sci_t(void);
	
	// Configration
	int8_t begin(uint32_t BaudRate);
	int8_t begin(uint32_t BaudRate, bool_t UseTx, bool_t UseRx);
	int8_t begin(uint32_t BaudRate, bool_t UseTx, bool_t UseRx, int16_t TxBuffSize, int16_t RxBuffSize);
	
	// CallBack
	void attachTxEndCallBackFunction(void (*CallBackFunc)(void)){this->TxEndCallBackFunc = CallBackFunc;};
	void detachTxEndCallBackFunction(void){this->TxEndCallBackFunc = NULL;};
	void attachRxCallBackFunction(void (*CallBackFunc)(RingBuffer<uint8_t> * RxBuff)){
		this->RxCallBackFunc = CallBackFunc;
		//Sci2.print("Sci_t::attachRxCallBackFunction RxCallBackFunc(%08p)\r\n", RxCallBackFunc);	
	};
	void detachRxCallBackFunction(void){this->RxCallBackFunc = NULL;};
	
	// Transmit/Receive
	int8_t transmit(uint8_t Data);		// ���M
	int8_t receive(uint8_t *Data);		// ��M�f�[�^�ǂ�(�|�C���^)
	uint8_t receive(void);			// ��M�f�[�^�ǂ�
	uint8_t read(void){return receive();};	// ��M�f�[�^�ǂ�
	
	// �g��Transmit
	int8_t print(char ch);
	int8_t print(int num);
	int8_t print(const char *control, ...);
	int8_t println(const char *control, ...);
	
	// ���[�e�B���e�B
	void clearRxBuff(void);				// �����܂ł̎�M�f�[�^���̂Ă�
	void enableTxEndInterrupt(void){fTxEndInterruptEnable = true;};	// ���M�������荞��on/off
	void disableTxEndInterrupt(void){fTxEndInterruptEnable = false;};
	void enableInterrupts(void);
	
	// Status
	bool_t isIdle(void){return SCIreg->SSR.BIT.TEND;};		// ���M���Ă��Ȃ�
	bool_t isAvailable(void){return !RxBuff->isEmpty();};	// ��M�f�[�^����
	bool_t available(void){return !RxBuff->isEmpty();};	// ��M�f�[�^����
	int16_t getTxBuffFreeSpace(void){return TxBuff->getFreeSpace();}	// ���M�o�b�t�@�̋󂫃o�b�t�@��
	
	// isr
	void isrTx(void);
	void isrTxEnd(void);
	void isrRx(void);
	
	//debug
	uint8_t getSciRxData(void){return SCIreg->RDR;};
};


extern Sci_t Sci2;
extern Sci_t Sci5;


#endif