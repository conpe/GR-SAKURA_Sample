/********************************************/
/*		RIIC���W���[���g�p I2C�ʐM			*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/02/26		*/
/********************************************/
// RIIC���W���[�����g�p����I2C�ʐM

// I2C�A�h���X�͉E�Â�7bit�Ŏw��B 0b*xxx,xxxx
// (1bit���V�t�g���đ��M����)

// I2c_comu_t(���p������)�ʐM�N���X��ʂ��đ���M����B
// 



// �߂�����^�C�~���O�������ƁA1�o�C�g�̂ݒP���ő���Ƒ����Ȃ������I
// (�u�o�b�t�@���ׂĂ̑��M�I���̏u�ԂɐV����1�o�C�g�ǉ��v�Ŕ�������\������)
// ����1�o�C�g����Η��ꂾ���̂ł��܂�C�ɂ��Ȃ��Ă����������B


// �g���Ă�O�̊֐�
// getPCLK(); @ GR_define.h

//�y�X�V�����z
// 2015.02.26 �V�K�쐬
// 2015.06.26 I2C�ʐM���e�N���X(I2c_comu_t)�ŒʐM���Ǘ�����悤�ɕύX
// 2015.12.26 �A�[�r�g���[�V�������X�g���o�@�\�ǉ�(�X���[�u�f�o�C�X��SDA��low�Œ�ɂ��Ă��鎞��SCL�N���b�N�����ĊJ��������)

//�y�X�V�\��z
// ���M�o�b�t�@�������Ȃ��悤�ɂ�����(�ʐM�I�u�W�F�N�g�Ɏ�������)
// �ʐM����ɂ���ĈႤ�o�b�t�@���g���d�g�݂ق����ȁH
// ����������p�ɂȂ��Ă�̂��C��������(�|�[�g�ԍ��Ƃ�)

#ifndef __RIIC_H__
#define __RIIC_H__

#include "iodefine.h"
#include "CommonDataType.h"
#include "RingBuffer.h"
#include "portReg.h"


// �|�[�g
#define PORT_SCL0 P12
#define PORT_SDA0 P13
#define PORT_SCL2 P16
#define PORT_SDA2 P17

// �{�[���[�g
#define I2C_BAUDRATE_DEFAULT 400
// �����O�o�b�t�@�̃T�C�Y
#define I2C_BUFFER_SIZE_DEFAULT 64
// �o���Ă�����ʐM������
#define I2C_COMUSNUM_DEFAULT 32


// �ǂ����Ɏ��̂�����֐�
extern uint32_t getPCLK(void);



enum i2c_module
{
	I2C_RIIC0 = 0,
	I2C_RIIC1 = 1,
	I2C_RIIC2 = 2,
	I2C_RIIC3 = 3
};


enum i2c_clock_div
{
	I2C_CLOCK_DIV1 = 0x00,  
	I2C_CLOCK_DIV2 = 0x01,  
	I2C_CLOCK_DIV4 = 0x02,  
	I2C_CLOCK_DIV8 = 0x03, 
	I2C_CLOCK_DIV16 = 0x04, 
	I2C_CLOCK_DIV32 = 0x05, 
	I2C_CLOCK_DIV64 = 0x06, 
	I2C_CLOCK_DIV128 = 0x07
};

enum i2c_status
{
	I2C_IDLE,
	I2C_TRANSMIT,
	I2C_RECEIVE,
	I2C_TRANSMIT_RECEIVE,
	I2C_RESET
};
enum i2c_statusin
{
	I2C_TRANSMIT_DSTADDRESS_T,
	I2C_RESTART,
	I2C_TRANSMIT_DSTADDRESS_R,
	I2C_TRANSMIT_DATA,
	I2C_READ_DATA,
	I2C_READ_END,
	I2C_NACK,
};



class I2c_comu_t;
class I2c_comu_general_t;

// I2C����(RIIC���W���[��)
class I2c_t{
private:
	
	volatile __evenaccess struct st_riic *I2Creg;	// ���W�X�^
	i2c_module _ModuleNum;	// RIIC���W���[���I��
	bool_t fBegun;			// I2C�������ς݃t���O
	uint16_t ComusNum;		// �ʐM���e�ێ���
	uint16_t Baud_kbps;	// �ʐM���x 100or400
	
	RingBuffer<uint8_t> *TxBuff;		// ���M�o�b�t�@ //Comus�̒��Ŏ����Ă�z�g�����ق����悢�̂ł�
	//RingBuffer<uint8_t> *RxBuff;		// ��M�o�b�t�@
	uint8_t *RcvBuffer;		// ��M�o�b�t�@�|�C���^
	i2c_status Status;		// I2C�X�e�[�^�X (���M���Ƃ�)
	i2c_statusin StatusIn;	// I2C�X�e�[�^�X�ڍ� (����M�̂ǂ̒i�K��)
	uint8_t DstAddress;		// �ʐM����
	int16_t CntTransmit;	// �̂��著�M��
	int16_t CntReceive;		// �̂����M��
	int16_t RcDataNum;		// ��M����f�[�^��		
	
	int8_t NextAttachIndex;	// ���ɃA�^�b�`����ꏊ
	int8_t NextComuIndex;	// ���ɏ�������ʐM
	int8_t CurrentComuIndex;	// ���������Ă�ʐM
	
	void initRegister(uint16_t Baud_kbps);
	void setPinModeI2C(void);
	
	// �������M���[�h�ł̑��M�J�n
	int8_t startComu(void);
	
	// 
	void reqStartCondition(void){I2Creg->ICCR2.BIT.ST=1;};		// �X�^�[�g�R���f�B�V�������s�v��
	void reqRestartCondition(void){I2Creg->ICCR2.BIT.RS=1;};	// ���X�^�[�g�R���f�B�V�������s�v��
	void reqStopCondition(void){I2Creg->ICCR2.BIT.SP=1;};		// �X�g�b�v�R���f�B�V�������s�v��
	bool_t isBusyBus(void){return I2Creg->ICCR2.BIT.BBSY;};		// �o�X���r�W�[���`�F�b�N(�X�^�[�g�R���f�B�V������1, �X�g�b�v�R���f�B�V������0)
	
	
public:
	// Constructor
	I2c_t(i2c_module I2cModule);
	~I2c_t(void);
	
	int8_t begin(void);
	int8_t begin(uint16_t Baud_kbps);
	int8_t begin(uint16_t Baud_kbps, uint16_t TxBuffNum);
	int8_t begin(uint16_t Baud_kbps, uint16_t TxBuffNum, uint16_t ComusNum);
	
	RingBuffer<I2c_comu_t*> *ComusBuff;	// �ʐM�����L���o�b�t�@
	I2c_comu_t* CurrentComu;				// ���������̒ʐM
	
	// ���荞�ݑ���M
	int8_t attach(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum, void (*CallBackFunc)(I2c_comu_t* Comu));		// ����M
	int8_t attach(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, void (*CallBackFunc)(I2c_comu_t* Comu));						// ���M�̂�
	int8_t attach(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum);																// ���M�̂�(�R�[���o�b�N����)
	int8_t attach(I2c_comu_t* AttachComu);	
	
	// �ʑ���M(���̂���private�ɂ��邩��)
	int8_t transmit(uint8_t DstAddress, uint8_t* TrData, uint8_t TrDataNum);
	int8_t receive(uint8_t DstAddress, uint8_t* RcData, uint8_t RcDataNum);
	int8_t transmit_receive(uint8_t DstAddress, uint8_t* TrData, uint8_t TrDataNum, uint8_t* RcData, uint8_t RcDataNum);
	
	// �X�e�[�^�X
	uint16_t getComusNum(void){return ComusNum;};
	bool_t isIdle(void){return Status==I2C_IDLE;};
	i2c_status getStatus(void){return Status;};
	i2c_statusin getStatusIn(void){return StatusIn;};
	
	// isr
	void isrTx(void);
	void isrTxEnd(void);
	void isrRx(void);
	void isrIdle(void);
	void isrStop(void);
	void isrNack(void);
	void isrArbitrationLost(void);
	
	// I2C�X���[�u���񂽂������Z�b�g
	void resetI2C(void);
};


// I2C�ʐM���e�N���X
class I2c_comu_t{
public:
	I2c_comu_t(void);
	I2c_comu_t(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum);	// ����M
	I2c_comu_t(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum);					// ���M�̂�
	I2c_comu_t(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum);	// ����M
	virtual ~I2c_comu_t(void);	// ���z�֐��ɂ���ƃT�u�N���X�̃f�X�g���N�^���Ă΂�Ă���X�[�p�[�N���X�̃f�X�g���N�^���Ă΂��
	
	int8_t AttachIndex;
	uint8_t DestAddress;
	uint8_t* TxData;
	uint16_t TxNum;
	uint8_t* RxData;
	uint16_t RxNum;
	bool_t Err;		//�������H
	
	// �R�[���o�b�N�֐�
	virtual void callBack(void){};
	
protected:
	// �ʐM���e�ݒ�(�R���X�g���N�^����Ă΂��)
	void setI2c(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum);	// 
};


// I2C�Ƃ肠�����g���������p�N���X
// �R�[���o�b�N�֐���o�^����`��(���Ȃ��Ă���������I)
class I2c_comu_general_t : public I2c_comu_t{
public:
	I2c_comu_general_t(void): I2c_comu_t(){};
	I2c_comu_general_t(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum, void (*CallBackFunc)(I2c_comu_t*))	: I2c_comu_t(AttachIndex, DestAddress, TxData, TxNum, RxNum){this->CallBackFunc = CallBackFunc;};	// ����M
	I2c_comu_general_t(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, void (*CallBackFunc)(I2c_comu_t*)) 				: I2c_comu_t(AttachIndex, DestAddress, TxData, TxNum){this->CallBackFunc = CallBackFunc;};			// ���M�̂�
	I2c_comu_general_t(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum)													: I2c_comu_t(AttachIndex, DestAddress, TxData, TxNum){this->CallBackFunc = NULL;};					// ���M�̂�(�R�[���o�b�N�Ȃ�)
	
	void callBack(void){ if(NULL!=CallBackFunc){(*CallBackFunc)(this);} };	//�R�[���o�b�N�o�^����Ă��炻����s
	
private:
	void (*CallBackFunc)(I2c_comu_t* Comu);	//�R�[���o�b�N�֐��ւ̃|�C���^
};



extern I2c_t I2C0;
extern I2c_t I2C1;
extern I2c_t I2C2;
extern I2c_t I2C3;


#endif