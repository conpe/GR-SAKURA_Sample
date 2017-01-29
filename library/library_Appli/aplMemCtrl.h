/**************************************************
aplDataFlash.h
	�f�[�^�t���b�V���ǂݏ���
**************************************************/


#ifndef __APL_MEMCTRL_H__
#define __APL_MEMCTRL_H__


#include "drvDataFlash.h"
#include "aplMemCtrlSpec.h"


typedef union UN_FL_DATA{
	float Val;
	uint8_t Bit[4];
} UN_FL_DATA;

typedef union UN_DB_DATA{
	float Val;
	uint8_t Bit[8];
} UN_DB_DATA;



// E2�f�[�^�t���b�V��������
int8_t initMem(void);

// E2�f�[�^�t���b�V������ǂ�
int8_t readMem(uint16_t Id, void* ReadData);

// E2�f�[�^�t���b�V���֏���
int8_t writeMem(uint16_t Id, void* Data);
int8_t writeMemUB(uint16_t Id, uint8_t Data);
int8_t writeMemUH(uint16_t Id, uint16_t Data);
int8_t writeMemUW(uint16_t Id, uint32_t Data);
int8_t writeMemFL(uint16_t Id, float Data);
int8_t writeMemDB(uint16_t Id, double Data);

// �萔������
int8_t clearAllMem(void);

// �V���A���ʐM�C���^�[�t�F�[�X
int8_t MemRcvCom_Write(uint8_t* RcvData);	// E2�f�[�^�t���b�V����������
int8_t MemRcvCom_Read(uint8_t* Data);		// E2�f�[�^�t���b�V���ǂݍ���
int8_t MemRcvCom_clearAllMem(uint8_t* Data);	// E2�f�[�^�t���b�V��������

#endif
