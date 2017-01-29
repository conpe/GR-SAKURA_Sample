/**************************************************
	aplMemCtrlSpec.c
	EEPROM�������Ǘ�
				2016/11/10 0:36:37
**************************************************/

#include "aplMemCtrlSpec.h"

// �����l�ϐ�
const uint8_t mem_last_shutdown_Init = 0;		// �O��V���b�g�_�E�����(�����l)
const float mem_linetrace_pid_kp_Init = 0.00016;		// ���C���g���[�XPID�W�� Kp(�����l)
const float mem_linetrace_pid_ki_Init = 0;		// ���C���g���[�XPID�W�� Ki(�����l)
const float mem_linetrace_pid_kd_Init = 0.00001;		// ���C���g���[�XPID�W�� Kd(�����l)


// �l�ϐ�
uint8_t mem_last_shutdown;		// �O��V���b�g�_�E�����
float mem_linetrace_pid_kp;		// ���C���g���[�XPID�W�� Kp
float mem_linetrace_pid_ki;		// ���C���g���[�XPID�W�� Ki
float mem_linetrace_pid_kd;		// ���C���g���[�XPID�W�� Kd


// EEPROM�Ǘ��e�[�u��
const ST_MEM_TABLE_T stMemTable[MEM_USED_NUM] = {
	/*	DataId,		Adrs,		DataType,	Val		InitVal		MinVal		MaxVal		AccessPermission		DataLength		Resolution		Offset	*/
	{	MEM_LAST_SHUTDOWN,		0x0000,		enUB,		&mem_last_shutdown,		&mem_last_shutdown_Init,		NULL,		NULL,		READ_ONLY,		1,		1,		0	},		// �O��V���b�g�_�E�����
	{	MEM_LINETRACE_PID_KP,		0x0001,		enFL,		&mem_linetrace_pid_kp,		&mem_linetrace_pid_kp_Init,		NULL,		NULL,		FULL,		2,		0.00001,		0	},		// ���C���g���[�XPID�W�� Kp
	{	MEM_LINETRACE_PID_KI,		0x0005,		enFL,		&mem_linetrace_pid_ki,		&mem_linetrace_pid_ki_Init,		NULL,		NULL,		FULL,		2,		0.00001,		0	},		// ���C���g���[�XPID�W�� Ki
	{	MEM_LINETRACE_PID_KD,		0x0009,		enFL,		&mem_linetrace_pid_kd,		&mem_linetrace_pid_kd_Init,		NULL,		NULL,		FULL,		2,		0.00001,		0	},		// ���C���g���[�XPID�W�� Kd
};
