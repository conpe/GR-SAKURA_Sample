/********************************************/
/*	RX63n��E2�f�[�^�t���b�V������ 	    */
/*			for RX63n @ CS+	    */
/*			Wrote by conpe_	    */
/*				2016/06/10  */
/********************************************/
// �f�[�^�^�Fuint16_t (word)
// �e�ʁF32kbyte = 16kword
// �A�h���X�F0�`16383 (�����ł�x2)

//�u���b�N�F32byte * 1024�u���b�N = 32kbyte
//�C���[�X 32byte�P��
//���C�g 2byte�P��

// 1byte���Ɠǂݏ�������B
// RX63n�̃f�[�^�t���b�V����2byte�ň����̂ŁA
// 1byte�����Ă���ꍇ�A����1byte���u�����N�`�F�b�N���Ă��u�����N�Ƃ͂Ȃ�Ȃ�

// �������݃^�X�N�����I�Ɏ��s���邱��(10msec���ƒ��x)

#ifndef __DATAFLASH_RX63_H__
#define __DATAFLASH_RX63_H__

#include "iodefine.h"
#include "CommonDataType.h"

#define DF_OK 0
#define DF_ERROR -1
#define DF_OUT_OF_ADDRESS -4
#define DF_BUFF_FULL -3
#define DF_BLANK 1

#define FLD_OK 0
#define FLD_ERROR -1
#define FLD_TMOUT -2
#define FLD_BUSY -3
#define FLD_OUT_OF_ADDRESS -4
#define FLD_BLANK 1
#define FLD_NOBLANK 2
#define FLD_NODATA 3

#define E2_DF_SIZE_WORD 32768
#define E2_DF_SIZE_BYTE 16384


// EEPROM������
int8_t drvDataFlash_init(void);
// �������݃^�X�N
int8_t drvDataFlash_WriteTask(void);
// �������ݎ��^�X�N�o�^
void drvDataFlash_attachWriteTask( void (*Tsk)(void));
// �o�b�t�@�t����������
int8_t drvDataFlash_write(uint16_t Address, uint8_t Data);
// �ǂݍ���
int8_t drvDataFlash_read(uint16_t Address, uint8_t* Data);


// ������������
int8_t drvDataFlash_writeImmWord(uint16_t Address, uint16_t Data);

#endif