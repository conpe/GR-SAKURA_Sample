
#ifndef __COMMON_DATA_TYPE_H__
#define __COMMON_DATA_TYPE_H__

// datatype

#include <stdint.h>

void __heap_chk_fail(void);

// �f�[�^�^
/*
// stdint�Œ�`����Ă���̂ō폜
typedef signed char     int8_t;
typedef unsigned char   uint8_t;
typedef signed short    int16_t;
typedef unsigned short  uint16_t;
typedef signed long     int32_t;
typedef unsigned long   uint32_t;
*/
//typedef bool	bool_t;		// <- OLED�����Ȃ��Ȃ�(�L�E�ցE�M)
typedef uint8_t	bool_t;	// �o�b�t�@�����������Ɠǂ�ŁA�o�b�t�@�̂�uint8_t�ɂ��Ē���B

// �l
#ifndef NULL
#define NULL 0x00000000
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


#endif