/********************************************/
/*			OLED(SSD1306)�\�� Bmp�t�@�C��	*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/04/03		*/
/********************************************/

/*
�y�T�v�z
�����̃r�b�g�}�b�v��OLED�ɕ\������B
48x128�s�N�Z���B�����B
�ʏ��OLED��16�`63�s�ڂɏ����B


�y�g�����z

OLED�ւ̃f�[�^�A�R�}���h���M�؂�ւ����s�����߂ɁASPI���M���ɃR�[���o�b�N����K�v����B
OledDriverSSD1306::callbackTxIsr()���Ăяo���O���[�o���Ȋ֐��ւ̃|�C���^���Abegin()�ɗ^���Ă��������B


�y�X�V�����z
2015.03.03	�R�[���o�b�N�����Bbegin�ɃR�[���o�b�N����֐���^����悤�ɂ��āA
			�����SPI���M�̊��荞�ݎ��ɌĂяo�����悤�ɃZ�b�g�B
2015.03.07	�R�[���o�b�N�֐�����CD�o�͂���悤��
			
*/

#ifndef OLED_DRIVER_SSD_BMP_HPP_
#define OLED_DRIVER_SSD_BMP_HPP_

#include "CommonDataType.h"
#include "RSPI.h"
#include "portReg.h"
#include <stdio.h>
#include <stdarg.h>
#include <sdmmc.h>

#include "OledDriverSSD1306.h"

// for debug
//#include <rxduino.h>
#include "GR_define.h"



class mmcBmpOled
{
public :
	/////////////////////////////////////////////////
	// initialize
	/////////////////////////////////////////////////
	mmcBmpOled(OledDriverSSD1306 *OledDriver, SDMMC *MMC);
	~mmcBmpOled(void);
	
	// bmp�`��
	// SD�J�[�h���甒��Bmp�t�@�C���ǂ��OLED�ɏo�͂���B
	// OLED��s���ǂ� (8*128/8 = 128�o�C�g) �� OLED�ɏo�͂��� ���J��Ԃ��B
	// �c�̗̈�����C���P�ʂŎw�肵�ĕ\������
	int8_t outBmp(const char *FileName, const uint8_t StartLineNum, const uint8_t WriteLineNum);
	// ���C�������ɕ\������
	int8_t outBmpMain(const char *FileName);
	// ��̑ѕ����ɏo�͂���
	int8_t outBmpHeader(const char *FileName);
	
	
private :
	OledDriverSSD1306* OledDriver;
	SDMMC* MMC;
	File* BmpFile;
	uint8_t DisplayLinePix;
	uint8_t DisplayLineNum;
	uint8_t DisplayWidth;
	
	
};

#endif

