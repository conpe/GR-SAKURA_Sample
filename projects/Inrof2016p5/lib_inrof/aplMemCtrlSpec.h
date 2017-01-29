/**************************************************
	aplMemCtrlSpec.h
	EEPROM�������Ǘ�
				2016/11/10 0:36:37
**************************************************/

#include "CommonDataType.h"

// EEPROM�e�� [byte]
#define MEM_CAPACITY 32768	// �e��(byte)

// EEPROM�g�p�� [byte]
#define MEM_USED_BYTE 13	// �g�p��(byte)

// EEPROM�g�pID��
#define MEM_USED_NUM 4	// �g�pID��

// �A�N�Z�X��
#define	FULL	0
#define	READ_ONLY	1
#define	WRITE_ONLY	2
#define	NONE	3



// �f�[�^�^
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



// �Ǘ��e�[�u��
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



#define	MEM_LAST_SHUTDOWN	0x0000	// �O��V���b�g�_�E�����
#define	MEM_LINETRACE_PID_KP	0x0001	// ���C���g���[�XPID�W�� Kp
#define	MEM_LINETRACE_PID_KI	0x0002	// ���C���g���[�XPID�W�� Ki
#define	MEM_LINETRACE_PID_KD	0x0003	// ���C���g���[�XPID�W�� Kd
