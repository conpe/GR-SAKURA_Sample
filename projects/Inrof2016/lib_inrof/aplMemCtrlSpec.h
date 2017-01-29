/**************************************************
	aplMemCtrlSpec.h
	EEPROM�������Ǘ�
				2016/09/18 22:14:35
**************************************************/

#include "CommonDataType.h"

#define MEM_CAPACITY 32768	// �e��(byte)

#define MEM_USED_BYTE 13	// �g�p��(byte)

#define MEM_USED_NUM 4	// �g�pID��

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

typedef struct ST_MEM_TABLE_T{
	uint16_t Adrs;
	EN_DATA_TYPE_T DataType;
	void* Val;
	const void* InitVal;
} ST_MEM_TABLE_T;

extern const ST_MEM_TABLE_T stMemTable[MEM_USED_NUM];

#define MEM_LAST_SHUTDOWN 0
#define MEM_LINETRACE_PID_KP 1
#define MEM_LINETRACE_PID_KI 2
#define MEM_LINETRACE_PID_KD 3
