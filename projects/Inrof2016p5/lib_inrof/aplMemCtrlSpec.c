/**************************************************
	aplMemCtrlSpec.c
	EEPROMメモリ管理
				2016/11/10 0:36:37
**************************************************/

#include "aplMemCtrlSpec.h"

// 初期値変数
const uint8_t mem_last_shutdown_Init = 0;		// 前回シャットダウン状態(初期値)
const float mem_linetrace_pid_kp_Init = 0.00016;		// ライントレースPID係数 Kp(初期値)
const float mem_linetrace_pid_ki_Init = 0;		// ライントレースPID係数 Ki(初期値)
const float mem_linetrace_pid_kd_Init = 0.00001;		// ライントレースPID係数 Kd(初期値)


// 値変数
uint8_t mem_last_shutdown;		// 前回シャットダウン状態
float mem_linetrace_pid_kp;		// ライントレースPID係数 Kp
float mem_linetrace_pid_ki;		// ライントレースPID係数 Ki
float mem_linetrace_pid_kd;		// ライントレースPID係数 Kd


// EEPROM管理テーブル
const ST_MEM_TABLE_T stMemTable[MEM_USED_NUM] = {
	/*	DataId,		Adrs,		DataType,	Val		InitVal		MinVal		MaxVal		AccessPermission		DataLength		Resolution		Offset	*/
	{	MEM_LAST_SHUTDOWN,		0x0000,		enUB,		&mem_last_shutdown,		&mem_last_shutdown_Init,		NULL,		NULL,		READ_ONLY,		1,		1,		0	},		// 前回シャットダウン状態
	{	MEM_LINETRACE_PID_KP,		0x0001,		enFL,		&mem_linetrace_pid_kp,		&mem_linetrace_pid_kp_Init,		NULL,		NULL,		FULL,		2,		0.00001,		0	},		// ライントレースPID係数 Kp
	{	MEM_LINETRACE_PID_KI,		0x0005,		enFL,		&mem_linetrace_pid_ki,		&mem_linetrace_pid_ki_Init,		NULL,		NULL,		FULL,		2,		0.00001,		0	},		// ライントレースPID係数 Ki
	{	MEM_LINETRACE_PID_KD,		0x0009,		enFL,		&mem_linetrace_pid_kd,		&mem_linetrace_pid_kd_Init,		NULL,		NULL,		FULL,		2,		0.00001,		0	},		// ライントレースPID係数 Kd
};
