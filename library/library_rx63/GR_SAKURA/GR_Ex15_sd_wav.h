/**************************************************
GR_Ex15_sd_wav.h
	GR_Ex15��p��WAV�t�@�C���Đ��N���X
	
**************************************************/


#ifndef __GREX15_SD_WAV_H__
#define __GREX15_SD_WAV_H__

#include <sdmmc.h>

#include "GR_define.h"

#include "GR_Ex15.h"
#include "GR_Ex15_OLED.h"





// SdWav
#define SDWAV_MAXFILENAME 32

// ���C���Z���T �ԊOLED
#define LINESENS_PORT_LED	GREX_IO7
// �Z���T��
#define LINESENS_NUM_SENSE	4


/****************************
 SD�J�[�h�����wav�Đ�
�T�v�F
 SD�J�[�h����wav�t�@�C�����Đ�����B
 ��~���Ƀ��W���[���w�肷�邱�ƂŁA
 ���̓����t�@�C�����Đ����Ɏ~�߂��Ƃ��납��Đ��ĊJ�ł���B
 
�֘A�N���X�F
 GR_EX15�{�[�h�p�́Agr_ex_sp�N���X�g�p�B
 gr_ex_sp::Sp->setupPlay()
 gr_ex_sp::Sp->stopPlay()
****************************/
enum wav_status{
	WAV_PLAYING,	// �Đ���
	WAV_STOP,		// ��~
	WAV_END,		// �I������Ƃ�
	WAV_FORCESTOP	// ������~�����Ƃ�
};

class sd_wav{
public:
	sd_wav(void);
	
	int8_t play(const char* FileName);	// �Đ�
	void stop(bool_t fResume);			// ��~
	//int8_t addPlayList(const char* FileName);
	//void getPlayList(uint16_t FileNum, char* FileName);
	//uint16_t getFileNum(void){return FileNum;};
	
	// �X�e�[�^�X
	bool_t isPlay(void){return fPlay;};
	
	// �Đ�����
	wav_status playHandler(void);		// ����I�ɌĂяo���ĂˁB
	
private:
	
	// wav�t�@�C���Đ�
	char *PlayFileName;
	File *PlayFile;
	bool_t fPlay;		// �Đ����t���O
	uint32_t PcmReadPos;		// �f�[�^�ǂݍ��݈ʒu
	uint32_t PcmDataSize;		// �f�[�^��
	uint16_t PcmSamplingRate;	// �T���v�����O���g��
	uint8_t PcmSamplingBit;		// 8 or 16
	//uint16_t FileNum;		// ���X�g�ɂ��܂��Ă���t�@�C����
	
	int8_t readHeader(void);	//wav�̃w�b�_���ǂ�
};

extern SDMMC MMC;


#endif
