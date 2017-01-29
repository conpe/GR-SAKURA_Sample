/**************************************************
	aplSerialComSpec.cpp
	�V���A���ʐM�R�}���h�Ǘ�
				2016/09/18 22:14:35
**************************************************/

#include "aplSerialComSpec.h"

// �Ăяo���֐��̐錾
#include "aplSerialCom.h"
#include "aplMemCtrl.h"
#include "inrof2016_lib.h"


// �V���A���ʐM�R�}���h�����e�[�u��
const ST_SERIAL_COM_COMMAND_TABLE_T stSerialComCommandTable[] = {
	/*	ComId,	DataLength,	Func	*/
	{	0x1000,		0,	inrof2016::SerCom_resetBallSensors},
	{	0xF001,		2,	MemRcvCom_Read	},	// EEPROM�ǂݍ���
	{	0xF002,		8,	MemRcvCom_Write	},	// EEPROM��������
	{	0xF003,		0,	MemRcvCom_clearAllMem	},	// EEPROM�S������
	{	0x0000,		0,	NULL	},	// 
};


