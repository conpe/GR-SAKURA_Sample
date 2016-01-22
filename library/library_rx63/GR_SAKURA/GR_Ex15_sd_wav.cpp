
#include "GR_Ex15_sd_wav.h"



/****************************
SD�J�[�h�����wav�Đ�
�R���X�g���N�^
�T�v�F
�@SD�J�[�h�ƃt���O������������
****************************/
sd_wav::sd_wav(void){
	//MMC.begin();	//SD�J�[�h������
	fPlay = 0;
}

/*********************
�Đ�
�T�v�F
 SD�J�[�h�ɂ���FileName�̍Đ����n�߂�
�����F
 �t�@�C����
�Ԓl�F
**********************/
int8_t sd_wav::play(const char *FileName){
	int8_t ack;
	
	//Oled.print("play:");
	//Oled.println(FileName);
	
	if(fPlay){	// �Đ����ɍĐ����ꂽ��
		stop(0);	//���Đ����Ă�̂͒�~
	}
	
	// �O�ƈႤ�t�@�C�����Ȃ�ŏ�����Đ�
	if(strcmp(PlayFileName, FileName)){
		PcmReadPos = 0;
		if(PlayFileName){
			delete[] PlayFileName;			// �O�̃t�@�C�����̗̈�J��
		}
		PlayFileName = new char[strlen(FileName)+1];	// �̈�m��
		strcpy(PlayFileName, FileName);		// �t�@�C�����Z�b�g
	}
	
	*PlayFile = MMC.open(FileName, FILE_READ);		// �w��t�@�C���I�[�v��
	
	// wav�t�@�C���w�b�_���擾
	ack = readHeader();
	
	if(!ack){
		
		GrEx.Sp->setupPlay(PcmSamplingRate);	// �Đ��J�n
		fPlay = 1;
		
	}else{
	}
	
	return ack;
}

/*********************
��~
�T�v�F
 �Đ����~�߂�B
 fResume�ɂ���čĐ��ӏ����L�����Ă������I��
�����F
 bool_t fResume : 0 �Đ��ӏ��Y���, 1 �o���Ƃ�
�Ԓl�F
**********************/
void sd_wav::stop(bool_t fResume){
	
	GrEx.Sp->stopPlay();	// �Đ��I���(�T���v�����O�����ł̊��荞�ݏI��)
	if(fPlay){				// �Đ�����������
		fPlay = 0;			// �Đ����t���O�N���A
		PlayFile->close();	// �t�@�C������
	}
	
	//fReadEndFile = 0;		// �Ō�܂œǂ񂾃t���O�N���A
	if(!fResume){			// �Đ����ӏ��N���A
		PcmReadPos = 0;			//
	}
}


// wav�t�@�C���֌W�ׂ����֐�
//  �����ł����g��񂩂�static�ɂ����
typedef uint8_t ID[4];

typedef struct {
    ID riff;
    int32_t size;
    ID wave;
} WavHdr;

typedef struct {
    ID id;
    int32_t size;
    int16_t format;
    int16_t channels;
    int32_t samplingRate;
    int32_t dataSpeed;
    int16_t blockSize;
    int16_t samplingBits;
} FormatChunk;

typedef struct {
    uint8_t id[4];
    int32_t size;
    uint8_t* data;
} DataChunk;
	
static WavHdr hdr;
static FormatChunk fmt;
static DataChunk dat;  

// File����n�o�C�g�ǂށB
// �ǂ񂾐���Ԃ��B
// �t�@�C���̍Ō�܂ōs�����炻���܂ŁB
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
  

// Wav�t�@�C���̃w�b�_���ǂ�
//  �T���v�����O����	PcmSamplingRate
//	�T���v�����O����\	PcmSamplingBit
//	�f�[�^�T�C�Y		PcmDataSize
int8_t sd_wav::readHeader(void){
	
	if (!(*PlayFile)) {
        Oled.print("file open error: '%s'\n", PlayFileName);
        return -1;
    }

	// �w�b�_�ǂ�
    if (readn((void*)&hdr, PlayFile, sizeof(hdr)) != sizeof(hdr)) {
        Oled.print("read error 1: '%s'\n", PlayFileName);
        return -2;
    }
	// wav�t�@�C�����`�F�b�N
    if (memcmp(hdr.riff, "RIFF", 4) != 0 || memcmp(hdr.wave, "WAVE", 4) != 0) {
        Oled.print("format error 1: '%s'\n", PlayFileName);
        return -3;
    }
	// �t�H�[�}�b�g�ǂ�
    if (readn((void*)&fmt, PlayFile, sizeof(fmt)) != sizeof(fmt)) {
        Oled.print("read error 2: '%s'\n", PlayFileName);
        return -4;
    }
	// �t�H�[�}�b�g��񂩃`�F�b�N
    if (memcmp(fmt.id, "fmt ", 4) != 0) {
        Oled.print("format error 2: '%s'\n", PlayFileName);
        return -5;
    }
	// �t�H�[�}�b�g�`�F�b�N
	// 16bit�����Ă�����Ȃ��B
    if (fmt.format != 0x01 || fmt.samplingBits > 16) {
        Oled.print("unsuported WAV format: '%s'\n", PlayFileName);
        return -6;
    }
	// �t�H�[�}�b�g��񂩂�T���v�����O���[�gget
    PcmSamplingRate = fmt.samplingRate;
	
	// �f�[�^�ǂݏo���ʒu�܂ňړ�
    PlayFile->seek(sizeof(hdr) + sizeof(fmt.id) + sizeof(fmt.size) + fmt.size);
	// �f�[�^���擪�ǂ�
    if (readn((void*)&dat, PlayFile, sizeof(dat)) != sizeof(dat)) {
        Oled.print("read error 3: '%s'\n", PlayFileName);
        return -7;
	}
	// �f�[�^���`�F�b�N
    if (memcmp(dat.id, "data", 4) != 0) {
        Oled.print("format error 3: '%s'\n", PlayFileName);
        return -8;
    }

	// �f�[�^������
    PcmDataSize = dat.size / fmt.blockSize;
	
	return 0;
}



// �o�b�t�@�����ς��ɂȂ�܂Ńt�@�C���ǂށB
//  �Đ����̃t�@�C���̍Ō�܂ōs������~�߂�
wav_status sd_wav::playHandler(void){
	static bool_t fReadEndFile = false;
	bool_t fForceEnd = false;	// �t�@�C���ǂ߂Ȃ������̂ŋ����I��
	uint8_t PcmData[4];
	bool_t fBuffFull;
	wav_status ret = WAV_PLAYING;
	uint16_t LoopCnt = 2048;		// �ő�ł��̃f�[�^���ǂ񂾂烋�[�v������
	
	if(fPlay){
		//PlayFile���珇���ǂނ�
		// �ǂނƂ��ֈړ� �����ꖈ����́H
	    PlayFile->seek(sizeof(hdr) + sizeof(fmt.id) + sizeof(fmt.size) + fmt.size + sizeof(dat.id) + sizeof(dat.size) + PcmReadPos*fmt.blockSize );
		
		// �t���O������
		fBuffFull = GrEx.Sp->isBuffFull();	//�o�b�t�@�󂫗L��
		//fEndFile = 0;
		
		#ifdef OUT_SERIAL
		// debug
		uint8_t cntprint=0;
		#endif
		
		while((!fBuffFull)&&(!fReadEndFile)&&(!fForceEnd)){	// �o�b�t�@�������ς����t�@�C���ǂ߂Ȃ����t�@�C���̍Ō�܂œǂނ܂ŌJ��Ԃ�
			// �u���b�N�T�C�Y���ǂ�
	        if (readn(PcmData, PlayFile, fmt.blockSize) == fmt.blockSize) {
				uint8_t Data = (fmt.samplingBits == 8) ? PcmData[0] : PcmData[1] ^ 0x80;		// ���`�����l���̂�, 0�`255��
				
				// �Đ��o�b�t�@�ɑ���
				fBuffFull = (bool_t)GrEx.Sp->addBuff(Data);		// ����ɒǉ�������0���Ԃ�
				
				// �Ō�܂œǂ񂾂��`�F�b�N
				if(PcmReadPos<(PcmDataSize-1)){
					PcmReadPos++;
				}else{
					PcmReadPos = 0;
					fReadEndFile = 1;
				}
			}else{
				
				//�u���b�N�T�C�Y���ǂ߂Ȃ������玀
	            Oled.print("read error 4: '%s'\n", PlayFileName);
	            fForceEnd = 1;
	        	fReadEndFile = 1;
				
			}
			
			if(!LoopCnt){
				fBuffFull = 1;	// �Ƃ肠�����I���
			}else{
				LoopCnt--;
			}
			
		#ifdef OUT_SERIAL
			cntprint++;
			if(!cntprint){	//0�̎�
				Serial.println("wait read finish.");
			}
		#endif
		}
		
		
		// �t�@�C���ǂݏI��������o�b�t�@���Ȃ��Ȃ�����B�������͋�����~
		//  �Đ���~����
		if((fReadEndFile && GrEx.Sp->isBuffEmpty()) || fForceEnd){
			fReadEndFile = 0;
			stop(0);		// ��~����(���W���[�����Ȃ�)
			if(fForceEnd){
				GrEx.Sp->clearBuff();	// �X�s�[�J�o�b�t�@�N���A
				
				ret = WAV_FORCESTOP; 
			}else{
				ret = WAV_END;
			}
		}
	}else{
		ret = WAV_STOP;
	}
	
	return ret;
	
}






