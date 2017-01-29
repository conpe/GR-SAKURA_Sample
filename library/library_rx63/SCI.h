/********************************************/
/*		SCI���W���[�����g�p����SCI�ʐM		*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/07/19		*/
/********************************************/

// �Ƃ肠����SCI0, 1, 2, 5�̂ݎ���
// SCI2�͎�M,���M�������荞�݂��ł��Ȃ�(�L�E�ցE�M)��GR-SAKURA

// ���M�v���o����(transmit)��x�����O�o�b�t�@�ɒ~�����A���M�o�b�t�@�G���v�e�B���荞�݂ɂ���ď������M���Ă����B
// ��M�͏�Ɋ��荞�݂Ń����O�o�b�t�@�ɒ~������Breceive�œǂݏo���B
//  �e�o�b�t�@�́A�����ς��ɂȂ��-1��Ԃ��������܂Ȃ��B

//�y�f�t�H���g�ݒ�z
// ���M���荞�� on(�K�{)
// ���M�������荞�� off
// ��M���荞�� on(�K�{)


// �y���Ӂz
// �p�����Ȃ����ƁB�o�b�t�@���]�v�ɍ���邩�疳�ʂ���B���荞�ݎv�����Ƃ���ɓ���Ȃ����B
// 
// �߂�����^�C�~���O�������ƁA1�o�C�g�̂ݒP���ő���Ƒ����Ȃ������I
// (�u�o�b�t�@���ׂĂ̑��M�I���̏u�ԂɐV����1�o�C�g�ǉ��v�Ŕ�������\������)
// ����1�o�C�g����Η��ꂾ���̂ł��܂�C�ɂ��Ȃ��Ă����������B

//�y�X�V�����z
// 2015.07.19 ���荞�ݎ��萮��
// 2015.11.28 print���\�b�h�A���I�Ɋm�ۂ��Ă��瑼�̂Ƃ��N�H���Ă��̂ŐÓI�Ɋm�ۂ���悤�ɕύX�B
// 2016.03.15 ��M�s���͓����v���A�b�v�L���ɂ���悤�ɂ���
// 2016.06.22 TXI,TEI���荞�݂���ɗL���ɂ����B
//		SCI1����

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
	//SCI_SCI3 = 3,
	//SCI_SCI4 = 4,
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

enum Sci_err
{
	SCI_OK		= 0,
	SCI_NODATA	= 1,
	SCI_TX_ERR	= 2,
	SCI_RX_ERR	= 3,
	SCI_MEM_ERR	= 4,	// �������m�ۃG���[
};

class Sci_t
{
public:
	// Constructor
	Sci_t(sci_module SciModule);
	~Sci_t(void);
	
	// Configration
	int8_t begin(uint32_t BaudRate, bool_t UseTx = true, bool_t UseRx = false, int16_t TxBuffSize = SCI_TX_BUFFER_SIZE_DEFAULT, int16_t RxBuffSize = SCI_RX_BUFFER_SIZE_DEFAULT);
	
	// CallBack
	void attachTxEndCallBackFunction(void (*CallBackFunc)(void)){this->TxEndCallBackFunc = CallBackFunc;};
	void detachTxEndCallBackFunction(void){this->TxEndCallBackFunc = NULL;};
	void attachRxCallBackFunction(void (*CallBackFunc)(RingBuffer<uint8_t> * RxBuff)){	this->RxCallBackFunc = CallBackFunc;	};
	void detachRxCallBackFunction(void){this->RxCallBackFunc = NULL;};
	
	// Transmit/Receive
	Sci_err transmit(uint8_t Data);		// ���M
	Sci_err receive(uint8_t *Data);		// ��M�f�[�^�ǂ�(�|�C���^)
	uint8_t receive(void);			// ��M�f�[�^�ǂ�
	uint8_t read(void){return receive();};	// ��M�f�[�^�ǂ�
	Sci_err watch(uint8_t *Data){return (0==RxBuff->watch(Data))?SCI_OK:SCI_RX_ERR;};	// �l���邾��
	uint8_t watch(void){return RxBuff->watch();};		// �l���邾��
	Sci_err watch(uint16_t IndexRel, uint8_t *Data){return (0==RxBuff->watch(IndexRel, Data))?SCI_OK:SCI_RX_ERR;};	// �l���邾��
	uint8_t watch(uint16_t IndexRel){return RxBuff->watch(IndexRel);};		// �l���邾��
	
	// �g��Transmit
	Sci_err print(char ch);
	Sci_err print(int num);
	Sci_err print(const char *control, ...);
	Sci_err println(const char *control, ...);
	
	// ���[�e�B���e�B
	void clearRxBuff(void);				// �����܂ł̎�M�f�[�^���̂Ă�
	void enableTxEndInterrupt(void){fTxEndInterruptEnable = true;};	// ���M�������荞��on/off
	void disableTxEndInterrupt(void){fTxEndInterruptEnable = false;};
	void enableInterrupts(void);
	void enableIntTxBuffEmpty(void);
	void disableIntTxBuffEmpty(void);
	void enableIntTxEnd(void);
	void enableIntRx(void);
	bool_t isEnableIntRx(void);
	void enableTR(void){SCIreg->SCR.BYTE |= ((SciConfig.TxEnable<<5) | (SciConfig.RxEnable<<4));};		// ����M�L��(TXI�L���ł���Ί��荞�݂�����)
	void disableTR(void){SCIreg->SCR.BYTE &= ((SciConfig.TxEnable<<5) | (SciConfig.RxEnable<<4))^0xFF;};	
	
	// Status
	Sci_state getState(void){return State;};			// ��Ԏ擾
	bool_t isIdle(void){return SCIreg->SSR.BIT.TEND;};		// ���M���Ă��Ȃ�
	bool_t isAvailable(void){return !RxBuff->isEmpty();};		// ��M�f�[�^����
	uint16_t available(void){return RxBuff->getNumElements();};	// ��M�f�[�^��
	uint16_t getTxBuffFreeSpace(void){return TxBuff->getFreeSpace();}	// ���M�o�b�t�@�̋󂫃o�b�t�@��
	
	// isr
	void isrTx(void);
	void isrTxEnd(void);
	void isrRx(void);
	
private:
	Sci_state State;
	
	sci_config SciConfig;
	RingBuffer<uint8_t> *TxBuff;
	RingBuffer<uint8_t> *RxBuff;
	
	bool_t fEmptySciTxBuff;
	bool_t fTxEndInterruptEnable;
	
	bool_t fTdrEmpty;	// TDR�Ƀf�[�^�L��
	bool_t fIsr;		// ���M���荞�ݒ�����I���̊Ԃɑ��M��isr���\�b�h�Ă΂Ȃ��ł�I
	
	bool_t fRxBuffOvf;	// ��M�o�b�t�@���ӂꂽ
	bool_t fTxBuffOvf;	// ���M�o�b�t�@���ӂꂽ
	
	// ���W�X�^
	volatile __evenaccess struct st_sci0 *SCIreg;
	
	void initRegister(void);
	void setPinModeSci(void);
	
	// ���W�X�^�@��
	void setSciTxData(uint8_t TxData){SCIreg->TDR = TxData;};	// ���M
	uint8_t getSciRxData(void){return SCIreg->RDR;};		// ��M
	bool_t isFramingErr(void){return SCIreg->SSR.BIT.FER;};		// �t���[�~���O�G���[
	void clearFramingErr(void){SCIreg->SSR.BIT.FER=0;};		//   �N���A
	bool_t isOverrun(void){return SCIreg->SSR.BIT.ORER;};		// ��M����O�ɂ܂���M���������
	void clearOverrun(void){SCIreg->SSR.BIT.ORER=0;};		//   �N���A
	
	// ��M�R�[���o�b�N
	void (*TxEndCallBackFunc)(void);	//���M�����R�[���o�b�N�֐��ւ̃|�C���^
	void (*RxCallBackFunc)(RingBuffer<uint8_t> * RxBuff);	//��M�R�[���o�b�N�֐��ւ̃|�C���^
	
	// ���M�J�n����(isrTx�����s)���Ă���[����(=TDR���W�X�^�󂢂Ă�&&isrTx�������łȂ�)
	bool_t needTxStartSequence(void){return fTdrEmpty&&(!fIsr);};
	
	//debug
	//uint8_t getSciRxData(void){return SCIreg->RDR;};
};


extern Sci_t Sci0;
extern Sci_t Sci1;
extern Sci_t Sci2;
extern Sci_t Sci5;


#endif