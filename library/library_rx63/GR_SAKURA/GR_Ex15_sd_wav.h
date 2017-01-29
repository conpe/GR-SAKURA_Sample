/**************************************************
GR_Ex15_sd_wav.h
	GR_Ex15基板用のWAVファイル再生クラス
	
**************************************************/


#ifndef __GREX15_SD_WAV_H__
#define __GREX15_SD_WAV_H__

#include <sdmmc.h>

#include "GR_define.h"

#include "GR_Ex15.h"
#include "GR_Ex15_OLED.h"





// SdWav
#define SDWAV_MAXFILENAME 32

// ラインセンサ 赤外LED
#define LINESENS_PORT_LED	GREX_IO7
// センサ数
#define LINESENS_NUM_SENSE	4


/****************************
 SDカードからのwav再生
概要：
 SDカードからwavファイルを再生する。
 停止時にレジューム指定することで、
 次の同じファイルを再生時に止めたところから再生再開できる。
 
関連クラス：
 GR_EX15ボード用の、gr_ex_spクラス使用。
 gr_ex_sp::Sp->setupPlay()
 gr_ex_sp::Sp->stopPlay()
****************************/
enum wav_status{
	WAV_PLAYING,	// 再生中
	WAV_STOP,		// 停止
	WAV_END,		// 終わったとこ
	WAV_FORCESTOP	// 強制停止したとこ
};

class sd_wav{
public:
	sd_wav(void);
	
	int8_t play(const char* FileName);	// 再生
	void stop(bool_t fResume);			// 停止
	//int8_t addPlayList(const char* FileName);
	//void getPlayList(uint16_t FileNum, char* FileName);
	//uint16_t getFileNum(void){return FileNum;};
	
	// ステータス
	bool_t isPlay(void){return fPlay;};
	
	// 再生処理
	wav_status playHandler(void);		// 定期的に呼び出してね。
	
private:
	
	// wavファイル再生
	char *PlayFileName;
	File *PlayFile;
	bool_t fPlay;		// 再生中フラグ
	uint32_t PcmReadPos;		// データ読み込み位置
	uint32_t PcmDataSize;		// データ数
	uint16_t PcmSamplingRate;	// サンプリング周波数
	uint8_t PcmSamplingBit;		// 8 or 16
	//uint16_t FileNum;		// リストにたまっているファイル数
	
	int8_t readHeader(void);	//wavのヘッダ情報読む
};

extern SDMMC MMC;


#endif
