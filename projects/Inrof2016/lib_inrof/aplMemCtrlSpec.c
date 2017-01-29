/**************************************************
	aplMemCtrlSpec.c
	EEPROM�������Ǘ�
				2016/09/18 22:14:35
**************************************************/

#include "aplMemCtrlSpec.h"

// �����l�ϐ�
const uint8_t mem_last_shutdown_init = 0;
const float mem_linetrace_pid_kp_init = 0.00016;
const float mem_linetrace_pid_ki_init = 0;
const float mem_linetrace_pid_kd_init = 0.00001;


// �l�ϐ�
uint8_t mem_last_shutdown;
float mem_linetrace_pid_kp;
float mem_linetrace_pid_ki;
float mem_linetrace_pid_kd;


const ST_MEM_TABLE_T stMemTable[MEM_USED_NUM] = {
	/*	Adrs,	DataType,	Val,	InitVal	*/
	{	0,	enUB,	&mem_last_shutdown,	&mem_last_shutdown_init	},	// �O��V���b�g�_�E�����
	{	1,	enFL,	&mem_linetrace_pid_kp,	&mem_linetrace_pid_kp_init	},	// ���C���g���[�XPID�W�� Kp
	{	5,	enFL,	&mem_linetrace_pid_ki,	&mem_linetrace_pid_ki_init	},	// ���C���g���[�XPID�W�� Ki
	{	9,	enFL,	&mem_linetrace_pid_kd,	&mem_linetrace_pid_kd_init	},	// ���C���g���[�XPID�W�� Kd
};
