/**************************************************
	aplMemCtrlSpec.c
	EEPROMメモリ管理
				2016/09/18 22:14:35
**************************************************/

#include "aplMemCtrlSpec.h"

// 初期値変数
const uint8_t mem_last_shutdown_init = 0;
const float mem_linetrace_pid_kp_init = 0.00016;
const float mem_linetrace_pid_ki_init = 0;
const float mem_linetrace_pid_kd_init = 0.00001;


// 値変数
uint8_t mem_last_shutdown;
float mem_linetrace_pid_kp;
float mem_linetrace_pid_ki;
float mem_linetrace_pid_kd;


const ST_MEM_TABLE_T stMemTable[MEM_USED_NUM] = {
	/*	Adrs,	DataType,	Val,	InitVal	*/
	{	0,	enUB,	&mem_last_shutdown,	&mem_last_shutdown_init	},	// 前回シャットダウン状態
	{	1,	enFL,	&mem_linetrace_pid_kp,	&mem_linetrace_pid_kp_init	},	// ライントレースPID係数 Kp
	{	5,	enFL,	&mem_linetrace_pid_ki,	&mem_linetrace_pid_ki_init	},	// ライントレースPID係数 Ki
	{	9,	enFL,	&mem_linetrace_pid_kd,	&mem_linetrace_pid_kd_init	},	// ライントレースPID係数 Kd
};
