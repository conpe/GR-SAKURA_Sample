/********************************************/
/*			OLED(SSD1306)�\�� Bmp�t�@�C��	*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/04/03		*/
/********************************************/

// �y�X�V�\��z


#include "mmcBmpOled.h"

mmcBmpOled::mmcBmpOled(OledDriverSSD1306 *OledDriver, SDMMC *MMC){
	mmcBmpOled::OledDriver = OledDriver;
	mmcBmpOled::MMC = MMC;
	
	DisplayLinePix = OledDriver->getOledHeightPerLine();
	DisplayLineNum = OledDriver->getOledLine();
	DisplayWidth = OledDriver->getOledWidth();
	
	OledDriver->println(".. Launch Bmp Driver.");
}

mmcBmpOled::~mmcBmpOled(void){
	
}


static int readn(void* buffer, File* file, int n)
{
    for (int i = 0; i < n; i++) {
        int d;

        if ((d = file->read()) >= 0) {
            ((uint8_t*)buffer)[i] = d;
        } else {
            return i;
        }
    }
    return n;
}

/*************************************
OLED�Ƀr�b�g�}�b�v�t�@�C���\��
����
 *FileName
 StartLineNum	: �\�����n�� (��ʂ̉����珑���̂ŁA�\���͈͂̉������C��)
 WriteLineNum	: �\�����C����   (�����ʒu����̕\�����C����)
�Ԓl
 -1 : �t�@�C���I�[�v�����s
 -2 : �r�b�g�}�b�v�t�@�C���łȂ�
 -3 : �摜�̉�����OLED�ɂ����ĂȂ�
 -4 : ���m�N���łȂ�
**************************************/
int8_t mmcBmpOled::outBmp(const char *FileName, const uint8_t StartLineNum, const uint8_t WriteLineNum){
	uint8_t Data[4];
	uint8_t *DisplayBuff;
	uint16_t i;
	
	// �f�B�X�v���C�ɓn�����߂̃o�b�t�@�B
	// 1���C�����m��
	DisplayBuff = new uint8_t[DisplayWidth];
	
	
	// �t�@�C���I�[�v��
	*BmpFile = MMC->open(FileName, FILE_READ);	// ����new���Ă邾�낤���璍��
	if(!(*BmpFile)){
		OledDriver->println("..Cant Open %s", FileName);
		return -1;
	}
	// �w�b�_�`�F�b�N
	// �擪��BM
	readn(Data, BmpFile, 2);
	if(memcmp(Data, "BM", 2)){
		OledDriver->println("..Not Bmp %s", FileName);
		return -2;		// no Bitmap file
	}
	
	// �摜�f�[�^�J�n�A�h���X
	BmpFile->seek(0x0A);
	readn(Data, BmpFile, 4);	// ���g���G���f�B�A���ɂȂ邩�m�F�p
	//uint32_t Image = (uint16_t)Data[3]<<24 | (uint16_t)Data[2]<<16 | (uint16_t)Data[1]<<8 | (uint16_t)Data[0];
	uint32_t BmpAdrsImage = *(uint32_t*)Data;
	
	// �摜�T�C�Y�`�F�b�N
	BmpFile->seek(0x12);
	readn(Data, BmpFile, 4);
	uint32_t BmpWidth = *(uint32_t*)Data;
	readn(Data, BmpFile, 4);
	int32_t BmpHeight = *(uint32_t*)Data;		// ����������ォ�牺�Ƀf�[�^������

	// ���m�N���`�F�b�N
	BmpFile->seek(0x1C);
	readn(Data, BmpFile, 2);
	uint16_t BitPix = *(uint16_t*)Data;
	if( BitPix != 1){
		return -4;
	}
	
	// �摜���
	// ��
	uint16_t WidthByte = BmpWidth/BitPix/8;
	// 4�̔{���ɍ��킹��
	WidthByte += WidthByte%4;
	// ����(���C�� (=�c�����̂ЂƂ܂Ƃ܂�))
	uint8_t BmpHeightLine = (BmpHeight+8)/8;	// OLED�̃��C����(�؂�グ)
	
	uint8_t CntRowLine;
	uint8_t CntColByte;
	uint8_t CntRow;
	uint8_t CntColBit;
	
	
	// �摜��OLED�ɏ����o��
	// ����8�Ƃ���4�ō��킹���Ȃ����̏��������I
	
	uint32_t StartAdrs;	
	uint8_t LineNumMax;	// �����C��������
	if(BmpHeightLine<WriteLineNum){	// �������C���͏���������I��
		LineNumMax = BmpHeightLine;
	}else{
		LineNumMax = WriteLineNum;
	}
	for(CntRowLine=0; CntRowLine<LineNumMax; CntRowLine++){		// �c�FOLED�̃��C���ŃJ�E���g
		
		StartAdrs = BmpAdrsImage + WidthByte*DisplayLinePix*CntRowLine;	// �e���C���̍����̃f�[�^�̏ꏊ
		BmpFile->seek(StartAdrs);
		
		// �o�b�t�@�N���A
		for(i=0; i<DisplayWidth; i++){
			DisplayBuff[i] = 0;
		}
		
		for(CntRow=0; CntRow<DisplayLinePix; CntRow++){					// 1���C�����ł�Row
			if(CntRowLine*DisplayLinePix+CntRow < BmpHeight){			// �������f�B�X�v���C�͈͓�
				for(CntColByte=0; CntColByte<WidthByte; CntColByte++){	// ���F�o�C�g���ŃJ�E���g
					if(CntColByte * (8/BitPix) < DisplayWidth ){		// �����f�B�X�v���C�͈͓�
					
						*Data = BmpFile->read();	// 1�o�C�g�ǂ�
						
						for(CntColBit=0;CntColBit<8;CntColBit++){	// �e�񏈗�
							//if(CntColByte * (8/BitPix) + CntColBit < DisplayWidth){	// ���͂ݏo���Ȃ����������� -> �ǂ���0�ɂȂ邾���Ȃ̂�
							DisplayBuff[CntColByte*8 + CntColBit] |= (((*Data)>>(8-CntColBit-1)&0x01)<<(DisplayLinePix-CntRow-1));
							//}
						}
					}
				}
			}
		}
		OledDriver->setCur(StartLineNum-CntRowLine, 0);	// �����珇�ɕ\���ꏊ�I��
		for(i=0;i<DisplayWidth;i++){
			OledDriver->writeData(DisplayBuff[i]);		// OLED�ɏo��
		}
		
	}
	
	delete[] DisplayBuff;
	BmpFile->close();
	
	return 0;
}



int8_t mmcBmpOled::outBmpMain(const char *FileName){
	return outBmp(FileName, DisplayLineNum-1, 6);
}
int8_t mmcBmpOled::outBmpHeader(const char *FileName){
	return outBmp(FileName, 1, 2);
}




