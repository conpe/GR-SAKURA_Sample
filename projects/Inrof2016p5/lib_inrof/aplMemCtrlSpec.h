/**************************************************
	aplMemCtrlSpec.h
	EEPROMメモリ管理
				2016/11/10 0:36:37
**************************************************/

#include "CommonDataType.h"

// EEPROM容量 [byte]
#define MEM_CAPACITY 32768	// 容量(byte)

// EEPROM使用量 [byte]
#define MEM_USED_BYTE 13	// 使用量(byte)

// EEPROM使用ID数
#define MEM_USED_NUM 4	// 使用ID数

// アクセス権
#define	FULL	0
#define	READ_ONLY	1
#define	WRITE_ONLY	2
#define	NONE	3



// データ型
typedef enum EN_DATA_TYPE_T{
	enUB,
	enB,
	enUH,
	enH,
	enUW,
	enW,
	enFL,
	enDB
} EN_DATA_TYPE_T;



// 管理テーブル
typedef struct ST_MEM_TABLE_T{
	uint16_t DataId;
	uint16_t Adrs;
	EN_DATA_TYPE_T DataType;
	void* Val;
	const void* InitVal;
	const void* MinVal;
	const void* MaxVal;
	uint8_t AccessPermission;
	uint8_t DataLength;
	float Resolution;
	float Offset;
} ST_MEM_TABLE_T;

extern const ST_MEM_TABLE_T stMemTable[MEM_USED_NUM];



#define	MEM_LAST_SHUTDOWN	0x0000	// 前回シャットダウン状態
#define	MEM_LINETRACE_PID_KP	0x0001	// ライントレースPID係数 Kp
#define	MEM_LINETRACE_PID_KI	0x0002	// ライントレースPID係数 Ki
#define	MEM_LINETRACE_PID_KD	0x0003	// ライントレースPID係数 Kd
