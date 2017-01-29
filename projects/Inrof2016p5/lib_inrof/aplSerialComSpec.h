/**************************************************
	aplSerialComSpec.h
	�V���A���ʐM�R�}���h�Ǘ�
				2016/09/18 22:14:35
**************************************************/

#ifndef __APL_SERIALCOM_SPEC_H__
#define __APL_SERIALCOM_SPEC_H__


#include "CommonDataType.h"

// �V���A���ʐM�R�}���h�����e�[�u��
typedef struct ST_SERIAL_COM_COMMAND_TABLE_T{
	uint16_t ComId;			// �R�}���hID
	uint8_t DataLength;		// �f�[�^��
	int8_t (*Func)(uint8_t*);	// �����֐�(���s or ������Ԃ�)
} ST_SERIAL_COM_COMMAND_TABLE_T;

extern const ST_SERIAL_COM_COMMAND_TABLE_T stSerialComCommandTable[];

#endif
