/**************************************************
aplSerialCom.h
	�V���A���ʐM����
**************************************************/


#ifndef __APL_SERIALCOM_H__
#define __APL_SERIALCOM_H__


#include "SCI.h"
#define Serial_t Sci_t

#include "aplSerialComSpec.h"

// �w�b�_�ǂ�łȂ��ꍇ
#ifndef __APL_SERIALCOM_SPEC_H__
// �V���A���ʐM�R�}���h�����e�[�u��
typedef struct ST_SERIAL_COM_COMMAND_TABLE_T{
	uint16_t ComId;			// �R�}���hID
	uint8_t DataLength;		// �f�[�^��
	int8_t (*Func)(uint8_t*);	// �����֐�(���s or ������Ԃ�)
} ST_SERIAL_COM_COMMAND_TABLE_T;

extern const ST_SERIAL_COM_COMMAND_TABLE_T stSerialComCommandTable[]
#endif



#define FUNC_RET_OK 0		// OK��Ԃ�
#define FUNC_RET_NG -1		// NG��Ԃ�
#define FUNC_RET_NO_RET 1	// �����Ԃ��Ȃ�




enum SerialComMode_t{
	SELCOM_MODE_COMMAND,	// �R�}���h���[�h(�R�}���h�ł����)
	SELCOM_MODE_THROUGH,	// �p�X�X���[���[�h(����M����������R)
};

class SerialCom_t{
public:
	// ������
	static int8_t init(Serial_t *SerialObj, SerialComMode_t InitialMode);
	
	// �����^�X�N
	static void task(void);
	
	// ���[�h�Z�b�g
	static void setMode(SerialComMode_t NewMode);
	// ���[�h�Z�b�g(�V���A���ʐMIF)
	static int8_t setModeSerialCom(uint8_t *RcvData);
	
	// ��M���^�X�N�o�^
	static void attachRcvTask(void (*Tsk)(void));
	// ��M���^�X�N����
	static void dettachRcvTask(void);
	
	
	
	
	
	
private:
	static Serial_t *SerialObj;		// �V���A���ʐM�h���C�o�I�u�W�F�N�g
	static SerialComMode_t Mode;		// �������[�h
	static void (*AddedRcvTask)(void);	// ��M�������^�X�N
	static const ST_SERIAL_COM_COMMAND_TABLE_T stSerialComDefauldCommandTable[];	// �V���A���ʐM�f�t�H���g�R�}���h
	void SerialCom(void);	// �C���X�^���X�������Ȃ����߂�private
	// �`�F�b�N�T���֌W
	static uint8_t calcCheckSum(uint8_t* RcvData);		// �`�F�b�N�T���v�Z
	static uint8_t isValidCheckSum(uint8_t* RcvData);	// �`�F�b�N�T�������Ă邩�m�F
	static void setCheckSum(uint8_t* RcvData);		// �`�F�b�N�T���Z�b�g
	
	
};
#endif
