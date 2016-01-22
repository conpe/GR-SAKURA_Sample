/********************************************/
/*		�t�^�o�̃R�}���h�T�[�{				*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/07/19		*/
/********************************************/

//
// enableTorque()����setGoalPosition()����Ύ�肠��������


// �f�t�H���g�ݒ�
// �{�[���[�g : 115200
// �T�[�{ID : 1


//�X�V����
// 2015/02/25 SH7125�p����ڐA�BC++���B�@
// 2015/07/19 ��M�҂����Ȃ������߁A���荞�݂Ŏ��s����悤�ɂ����B


//�X�V������
// �ʐM�̊Ǘ���SCI���W���[�����Ƃɍs���B�ł��ĂȂ����獢�����Ⴄ�B��M���荞�݂Ƃ������ʂɑ����Ă���(�L�E�ցE�M)�B�ʐM���e�̂����sci�o���Ă����̂��₾��
// ->SCI�N���X�ɁA�p�P�b�g�ʐM�ł�����B(�ʏ�ʐM�ƃp�P�b�g�ʐM�̍����́c�l���Ȃ��Ă������ȁH

#ifndef __FUTABASERIALSERVO_H__
#define __FUTABASERIALSERVO_H__

#include <machine.h>	// �}�C�R���R���p�C���ŗL�֐�

#include "SCI.h"
#include "CommonDataType.h"

// debug�p
//#include "SCI.h"

//#define FTCMDSV_COMUSNUM_DEFAULT 32		// �o���Ă�����ʐM���e
#define FTCMDSV_COMUSNUM_DEFAULT 12		// �o���Ă�����ʐM���e

#define CMDSV_BAUDRATE_DEFAULT 115200

//�� ���ꂢ��Ȃ�
#define CMDSV_ADRS_ServoID 0x04
#define CMDSV_ADRS_Reverse 0x05
#define CMDSV_ADRS_BaudRate 0x06
#define CMDSV_ADRS_ReturnDelay 0x07
#define CMDSV_ADRS_CWAngleLimit 0x08
#define CMDSV_ADRS_CCWAngleLimit 0x0A
#define CMDSV_ADRS_TemperatureLimit 0x0E
#define CMDSV_ADRS_TorqueInSilence 0x16
#define CMDSV_ADRS_WarmupTime 0x17
#define CMDSV_ADRS_CWComplianceMargin 0x18
#define CMDSV_ADRS_CCWComplianceMargin 0x19
#define CMDSV_ADRS_CWComplianceSlope 0x1A
#define CMDSV_ADRS_CCWComplianceSlope 0x1B
#define CMDSV_ADRS_Punch 0x64
#define CMDSV_ADRS_GoalPosition 0x1E
#define CMDSV_ADRS_GoalTime 0x20
#define CMDSV_ADRS_MaxTorque 0x23
#define CMDSV_ADRS_TorqueEnable 0x24
#define CMDSV_ADRS_PresentPosition 0x2A
#define CMDSV_ADRS_PresentTime 0x2C
#define CMDSV_ADRS_PresentSpeed 0x2E
#define CMDSV_ADRS_PresentCurrent 0x30
#define CMDSV_ADRS_PresentTemperature 0x32
#define CMDSV_ADRS_PresentVolts 0x34



#define CMDSV_BR_9600 0
#define CMDSV_BR_14400 1
#define CMDSV_BR_19200 2
#define CMDSV_BR_28800 3
#define CMDSV_BR_38400 4
#define CMDSV_BR_57600 5
#define CMDSV_BR_76800 6
#define CMDSV_BR_115200 7	// default
#define CMDSV_BR_153600 8
#define CMDSV_BR_230400 9

enum ftcmdsv_torque_enable{
	CMDSV_TORQUE_OFF	= 0,
	CMDSV_TORQUE_ON		= 1,
	CMDSV_TORQUE_BRAKE	= 2,
};

enum ftcmdsv_comu_content{
	CMDSV_SERVOID			= 0x00,
	CMDSV_REVERS			= 0x01,
	CMDSV_BAUDRATE			= 0x02,
	CMDSV_RETURNDELAY		= 0x03,
	CMDSV_CWANGLELIMIT		= 0x04,
	CMDSV_CCWANGLELIMIT		= 0x05,
	CMDSV_TEMPERATURELIMIT	= 0x06,
	CMDSV_TORQUEINSILENCE	= 0x07,
	CMDSV_WARMUPTIME		= 0x08,
	CMDSV_CWCOMPLIANCEMARGIN	= 0x09,
	CMDSV_CCWCOMPLIANCEMARGIN	= 0x0A,
	CMDSV_CWCOMPLIANCESLOPE		= 0x0B,
	CMDSV_CCWCOMPLIANCESLOPE	= 0x0C,
	CMDSV_PUNCH					= 0x0D,
	CMDSV_GOALPOSITION			= 0x0E,
	CMDSV_GOALTIME				= 0x0F,
	CMDSV_MAXTORQUE				= 0x10,
	CMDSV_TORQUEENABLE			= 0x11,
	CMDSV_PRESENTPOSITION		= 0x12,
	CMDSV_PRESENTTIME			= 0x13,
	CMDSV_PRESENTSPEED			= 0x14,
	CMDSV_PRESENTCURRENT		= 0x15,
	CMDSV_PRESENTTEMPERATURE	= 0x16,
	CMDSV_PRESENTVOLTS			= 0x17,
	CMDSV_WRITEROM				=0x80,
	CMDSV_RESET				=0x81,
};



typedef struct ftcmdsv_data{
	uint8_t ServoID;	// ROM
	uint8_t Reverse;
	uint16_t BaudRate;
	uint8_t ReturnDelay;
	uint16_t CWAngleLimit;
	uint16_t CCWAngleLimit;
	uint16_t TemperatureLimit;
	uint8_t TorqueInSilence;
	uint8_t WarmupTime;
	uint8_t CWComplianceMargin;
	uint8_t CCWComplianceMargin;
	uint8_t CWComplianceSlope;
	uint8_t CCWComplianceSlope;
	uint16_t Punch;
	uint16_t GoalPosition;		// RAM
	uint16_t GoalTime;
	uint8_t MaxTorque;
	ftcmdsv_torque_enable TorqueEnable;
	int16_t PresentPosition;	// *10 deg
	uint16_t PresentTime;
	int16_t PresentSpeed;
	uint16_t PresentCurrent;
	int16_t PresentTemperature;
	uint16_t PresentVolts;
} ftcmdsv_data;


class futaba_cmd_servo_comu_t;

class futaba_cmd_servo{
	
public:
		
	/*********************
	�R���X�g���N�^
	�����F
			SCI����C���X�^���X
			�{�[���[�g 4800�`115200(default)
	**********************/
	futaba_cmd_servo(uint8_t ID, Sci_t* Sci, uint32_t BaudRate);
	futaba_cmd_servo(uint8_t ID, Sci_t* Sci);
	~futaba_cmd_servo(void);

	int8_t begin(void);

	/*********************
	�T�[�{ID�ݒ�
	ROM�ۑ��܂ōs��
	�����F
			newID	�F�V�����T�[�{ID
	**********************/
	void setID(uint8_t newID);

	/*********************
	�ʐM���x�ݒ�
	�����F	
		newBR	�F�V�����ʐM���x(CMDSV_BR_9600,...)
	**********************/
	void setBaudRate(uint8_t newBR);

	/*********************
	ROM�̈��ۑ�����
	�����F	
	**********************/
	void writeROM(void);

	/*********************
	�T�[�{�����Z�b�g
	�����F	
	**********************/
	void reset(void);
	
	
	/*********************
	���W�X�^�ǂݍ��ݗv��
	�����F	
		ftcmdsv_comu_content : �ǂރ��W�X�^
	**********************/
	int8_t update(ftcmdsv_comu_content Reg){return reqReadMemory(Reg);};
	
	
	/*********************
	�ڕW�ʒu
	�����F
			angle	�F�ڕW�p�x(0.1�x�P�� 90.2�x��902 )
	**********************/
	void setGoalPosition(int16_t angle);
	int8_t updateGoalPosition(void){return reqReadMemory(CMDSV_GOALPOSITION);};		// ���W�X�^�ǂޒʐM
	int16_t getGoalPosition(void){return ResisterData.GoalPosition;};				// ���ʎ擾

	/*********************
	�ړ�����
	�����F
			TimeMs	�F�ڕW���� [ms]
	**********************/
	void setGoalTime(uint16_t TimeMs);
	int8_t updateGoalTime(void){return reqReadMemory(CMDSV_GOALTIME);};		// ���W�X�^�ǂޒʐM
	int16_t getGoalTime(void){return ResisterData.GoalTime;};				// ���ʎ擾

	/*********************
	�ő�g���N
	�����F	
		Torque : �ő�g���N [%]
	**********************/
	void setMaxTorque(uint8_t MaxTorque);


	/*********************
	�g���NEnable
	�����F	ID		�F�T�[�{ID
			enable	�FCMDSV_TORQUE_ON, CMDSV_TORQUE_OFF, CMDSV_TORQUE_BRAKE
	**********************/
	int8_t enableTorque(ftcmdsv_torque_enable Enable);






	/*********************
	�T�[�{�p�x
	�����F	
	**********************/
	int8_t updatePresentPosition(void){return reqReadMemory(CMDSV_PRESENTPOSITION);};
	int16_t getPresentPosition(void){return ResisterData.PresentPosition;};


	/*********************
	���ݎ��� 10ms�P��
	**********************/
	int8_t updatePresentTime(void){return reqReadMemory(CMDSV_PRESENTTIME);};
	int16_t getPresentTime(void){return ResisterData.PresentTime;};
		
		
	/*********************
	���ݑ��x
	�����F
	�Ԓl�F
		signed int �F deg/sec
	**********************/
	int8_t updatePresentSpeed(void){return reqReadMemory(CMDSV_PRESENTSPEED);};
	int16_t getPresentSpeed(void){return ResisterData.PresentSpeed;};
	
	/*********************
	���דd��
	�����F
	�Ԓl�F
		uint16_t �F �d�� [mV]
	**********************/
	int8_t updatePresentCurrent(void){return reqReadMemory(CMDSV_PRESENTCURRENT);};
	uint16_t getPresentCurrent(void){return ResisterData.PresentCurrent;};
	
	/*********************
	�d���d��
	�����F
	�Ԓl�F
		signed int �F mV
	**********************/
	int8_t updatePresentVolts(void){return reqReadMemory(CMDSV_PRESENTVOLTS);};
	uint16_t getPresentVolts(void){return ResisterData.PresentVolts;};
	
	/*********************
	����x
	�����F
	�Ԓl�F
		signed int �F ���x [�x]
	**********************/
	int8_t updatePresentTemperature(void){return reqReadMemory(CMDSV_PRESENTTEMPERATURE);};
	int16_t getPresentTemperature(void){return ResisterData.PresentTemperature;};
	
	
	
	
	//debuf
	static bool_t fAttaching;
private:
	Sci_t *ServoSci;
	uint32_t BaudRate;
	uint8_t ID;
	ftcmdsv_data ResisterData;
	static const uint8_t ResisterAdrsLength[24][2];	// �e���W�X�^�̃A�h���X�ƃf�[�^��
	
	//static bool_t fAttaching;
	static bool_t fManaging;
	
	/*********************
	�V���[�g�p�P�b�g����
	�����F	
			Flag	�F�t���O����
			Address	�F�������A�h���X
			Length	�F1�T�[�{������̃f�[�^�̒���
			Cnt		�F�T�[�{�̐�
			*Data	�F�f�[�^ sizeof(Data) = Length*Cnt
	**********************/
	int8_t genShortPacket(uint8_t** TxData, uint8_t Flag, uint8_t Address, uint8_t Length, uint8_t Cnt, uint8_t *TrData);
	int8_t genShortPacket(uint8_t* TxData, uint8_t Flag, uint8_t Address, uint8_t Length, uint8_t Cnt, uint8_t *TrData);
	
	/*********************
	�V���[�g�p�P�b�g���M
	�@��̃T�[�{�Ƀf�[�^�𑗂�
	�@�T�[�{�ʐM�N���X��new����attachSciComu�ɓ�����
	�����F	
			Flag	�F�t���O����
			Address	�F�������A�h���X
			Length	�F1�T�[�{������̃f�[�^�̒���
			Cnt		�F�T�[�{�̐�
			*Data	�F�f�[�^ sizeof(Data) = Length*Cnt
	**********************/
	int8_t sendShortPacket(ftcmdsv_comu_content ComuType, uint8_t Flag, uint8_t Address, uint8_t Length, uint8_t Cnt, uint8_t *TrData);
	
	
	/*********************
	�f�[�^����
	�@��̃T�[�{�Ƀf�[�^�𑗂�
	�@�T�[�{�ʐM�N���X��new����attachSciComu�ɓ�����
	�����F	
			Flag	�F�t���O����
			Address	�F�������A�h���X
			Length	�F1�T�[�{������̃f�[�^�̒���
			Cnt		�F�T�[�{�̐�
			*Data	�F�f�[�^ sizeof(Data) = Length*Cnt
	**********************/
	int8_t writeMemory(ftcmdsv_comu_content ComuType, uint8_t *TrData);
	
	/*********************
	�f�[�^�ǂޗv��
	�����F	ID		�F�T�[�{ID
			Address	�F�������A�h���X
			Length	�F1�T�[�{������̃f�[�^�̒���
			*Data	�F�f�[�^ 
	**********************/
	void readMemory(uint8_t Address, uint8_t Length, uint8_t *RcData);	// ��M����܂ő҂�
	int8_t reqReadMemory(ftcmdsv_comu_content ComuType);				// �p�P�b�g�ʐM��


	/*********************
	�T�[�{�ʐM�N���X���o�b�t�@�ɓo�^
	�����F	
		NewComu	�F�o�^����T�[�{�ʐM�N���X
	�ߒl�F
		attach�����Ȃ�0
	**********************/
	int8_t attachComu(futaba_cmd_servo_comu_t* NewComu);
	
	
	/*********************
	�R�}���h���M��ɑ҂�
	�����F	
	**********************/
	void waitEndCommand(void);
	
	
/********* �ʐM���� **********/
	// ��M�҂����t���O (���̕Ӄz���g��SCI���W���[�����Ƃɗp�ӂ�����)
	static bool_t fRcv;
	// ���ݒʐM���̒ʐM
	static futaba_cmd_servo_comu_t* CurrentComu;
	// �ʐM���e�L���o�b�t�@
	static RingBuffer<futaba_cmd_servo_comu_t*> *ComusBuff;
	
	/*********************
	���M�}�l�W�����g�֐�
	 �ʐM���J�n����
	 �ʐM�o�^���ASCI���M�������A�f�[�^��M�������ɌĂ΂��
	�����F
		�Ȃ�
	�Ԓl�F
		int8_t �G���[���
	**********************/
	int8_t manageComuStart(void);
	
	/*********************
	��M�}�l�W�����g�֐�
	�����F
		�Ȃ�
	�Ԓl�F
		�Ȃ�
	**********************/
	void manageComuReceive(RingBuffer<uint8_t> * RxBuff);
	
	
	/*********************
	���M�������荞��
	 ��M�҂��Ȃ��M�J�n�֐����Ă�
	 �łȂ����manageComuStart�Ŏ��̑��M������
	�����F
		�Ȃ�
	�Ԓl�F
		�Ȃ�
	**********************/
	static void intTxEnd(void);
	
	/*********************
	��M�J�n
	 ��M�R�[���o�b�N�֐���o�^����
	�����F
		�Ȃ�
	�Ԓl�F
		�Ȃ�
	**********************/
	void startRcv(void);
	
	/*********************
	��M�I���
	 ��M�R�[���o�b�N�֐����O��
	�����F
		�Ȃ�
	�Ԓl�F
		�Ȃ�
	**********************/
	void endRcv(void);
	
	/*********************
	��M����
	 ��M�������荞�݂ŌĂ΂��
	�����F
		�Ȃ�
	�Ԓl�F
		�Ȃ�
	**********************/
	static void intRx(RingBuffer<uint8_t> * RxBuff);
	
	/*********************
	��M����
	 ��M���荞�݂ŌĂ΂��
	 �e�ʐM���e�ɉ�����ResisterData���X�V
	�����F
		RingBuffer<uint8_t> * RxBuff
	�Ԓl�F
		�Ȃ�
	**********************/
	void fetchRcvData(futaba_cmd_servo_comu_t* Comu);
	
	
	
	
};


/********
�T�[�{�ʐM�N���X
********/
class futaba_cmd_servo_comu_t{
public:
	futaba_cmd_servo* FTCMDSV;		// �T�[�{�I�u�W�F�N�g
	ftcmdsv_comu_content ComuType;	// �ʐM���e
	uint8_t* TxData;		// ���M�f�[�^
	uint16_t TxNum;			// ���M���鐔
	uint8_t* RxData;		// ���M�f�[�^
	uint16_t RxNum;			// ��M���鐔
	uint16_t ReceivedNum;	// ��M������
	uint8_t Sum;			// �`�F�b�N�T����M���m�F�p
	
	// �R���X�g���N�^
	futaba_cmd_servo_comu_t(futaba_cmd_servo* FTCMDSV, ftcmdsv_comu_content ComuType, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum){
		this->FTCMDSV = FTCMDSV;
		this->ComuType = ComuType;
		this->TxNum = TxNum;
		this->RxNum = RxNum;
		this->ReceivedNum = 0;
		if(TxNum>0){
			// ���M�f�[�^�R�s�[
			if(TxNum<0x10){	// �Œ�m�ۃo�C�g�������m�� (�m�ۂ��郁�����𑵂��邽��)
				TxNum = 0x10;
			}
			this->TxData = new uint8_t[TxNum];
			if(NULL!=this->TxData){
				memcpy(this->TxData, TxData, this->TxNum);
			}
		}else{
			this->TxData = NULL;
		}
		// ��M�f�[�^�̈�
		if(RxNum>0){
			if(RxNum<0x10){
				RxNum = 0x10;
			}
			this->RxData = new uint8_t[RxNum];
		}else{
			this->RxData = NULL;
		}
	};
	// �f�X�g���N�^
	~futaba_cmd_servo_comu_t(void){
		if(NULL!=TxData){
			delete[] TxData;
		}
		if(NULL!=RxData){
			delete[] RxData;
		}
	};
	
};

#endif
