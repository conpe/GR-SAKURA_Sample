
#include "GR_Ex15_sd_wav.h"



/****************************
SDカードからのwav再生
コンストラクタ
概要：
　SDカードとフラグを初期化する
****************************/
sd_wav::sd_wav(void){
	//MMC.begin();	//SDカード初期化
	fPlay = 0;
}

/*********************
再生
概要：
 SDカードにあるFileNameの再生を始める
引数：
 ファイル名
返値：
**********************/
int8_t sd_wav::play(const char *FileName){
	int8_t ack;
	
	//Oled.print("play:");
	//Oled.println(FileName);
	
	if(fPlay){	// 再生中に再生されたら
		stop(0);	//今再生してるのは停止
	}
	
	// 前と違うファイル名なら最初から再生
	if(strcmp(PlayFileName, FileName)){
		PcmReadPos = 0;
		if(PlayFileName){
			delete[] PlayFileName;			// 前のファイル名の領域開放
		}
		PlayFileName = new char[strlen(FileName)+1];	// 領域確保
		strcpy(PlayFileName, FileName);		// ファイル名セット
	}
	
	*PlayFile = MMC.open(FileName, FILE_READ);		// 指定ファイルオープン
	
	// wavファイルヘッダ情報取得
	ack = readHeader();
	
	if(!ack){
		
		GrEx.Sp->setupPlay(PcmSamplingRate);	// 再生開始
		fPlay = 1;
		
	}else{
	}
	
	return ack;
}

/*********************
停止
概要：
 再生を止める。
 fResumeによって再生箇所を記憶しておくか選ぶ
引数：
 bool_t fResume : 0 再生箇所忘れる, 1 覚えとく
返値：
**********************/
void sd_wav::stop(bool_t fResume){
	
	GrEx.Sp->stopPlay();	// 再生終わり(サンプリング周期での割り込み終了)
	if(fPlay){				// 再生中だったら
		fPlay = 0;			// 再生中フラグクリア
		PlayFile->close();	// ファイル閉じる
	}
	
	//fReadEndFile = 0;		// 最後まで読んだフラグクリア
	if(!fResume){			// 再生中箇所クリア
		PcmReadPos = 0;			//
	}
}


// wavファイル関係細かい関数
//  ここでしか使わんからstaticにするよ
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

// Fileからnバイト読む。
// 読んだ数を返す。
// ファイルの最後まで行ったらそこまで。
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
  

// Wavファイルのヘッダ情報読む
//  サンプリング周期	PcmSamplingRate
//	サンプリング分解能	PcmSamplingBit
//	データサイズ		PcmDataSize
int8_t sd_wav::readHeader(void){
	
	if (!(*PlayFile)) {
        Oled.print("file open error: '%s'\n", PlayFileName);
        return -1;
    }

	// ヘッダ読む
    if (readn((void*)&hdr, PlayFile, sizeof(hdr)) != sizeof(hdr)) {
        Oled.print("read error 1: '%s'\n", PlayFileName);
        return -2;
    }
	// wavファイルかチェック
    if (memcmp(hdr.riff, "RIFF", 4) != 0 || memcmp(hdr.wave, "WAVE", 4) != 0) {
        Oled.print("format error 1: '%s'\n", PlayFileName);
        return -3;
    }
	// フォーマット読む
    if (readn((void*)&fmt, PlayFile, sizeof(fmt)) != sizeof(fmt)) {
        Oled.print("read error 2: '%s'\n", PlayFileName);
        return -4;
    }
	// フォーマット情報かチェック
    if (memcmp(fmt.id, "fmt ", 4) != 0) {
        Oled.print("format error 2: '%s'\n", PlayFileName);
        return -5;
    }
	// フォーマットチェック
	// 16bit超えてたらやらない。
    if (fmt.format != 0x01 || fmt.samplingBits > 16) {
        Oled.print("unsuported WAV format: '%s'\n", PlayFileName);
        return -6;
    }
	// フォーマット情報からサンプリングレートget
    PcmSamplingRate = fmt.samplingRate;
	
	// データ読み出し位置まで移動
    PlayFile->seek(sizeof(hdr) + sizeof(fmt.id) + sizeof(fmt.size) + fmt.size);
	// データ分先頭読む
    if (readn((void*)&dat, PlayFile, sizeof(dat)) != sizeof(dat)) {
        Oled.print("read error 3: '%s'\n", PlayFileName);
        return -7;
	}
	// データかチェック
    if (memcmp(dat.id, "data", 4) != 0) {
        Oled.print("format error 3: '%s'\n", PlayFileName);
        return -8;
    }

	// データ部長さ
    PcmDataSize = dat.size / fmt.blockSize;
	
	return 0;
}



// バッファいっぱいになるまでファイル読む。
//  再生中のファイルの最後まで行ったら止める
wav_status sd_wav::playHandler(void){
	static bool_t fReadEndFile = false;
	bool_t fForceEnd = false;	// ファイル読めなかったので強制終了
	uint8_t PcmData[4];
	bool_t fBuffFull;
	wav_status ret = WAV_PLAYING;
	uint16_t LoopCnt = 2048;		// 最大でこのデータ数読んだらループ抜ける
	
	if(fPlay){
		//PlayFileから順次読むよ
		// 読むとこへ移動 ←これ毎回やるの？
	    PlayFile->seek(sizeof(hdr) + sizeof(fmt.id) + sizeof(fmt.size) + fmt.size + sizeof(dat.id) + sizeof(dat.size) + PcmReadPos*fmt.blockSize );
		
		// フラグ初期化
		fBuffFull = GrEx.Sp->isBuffFull();	//バッファ空き有り
		//fEndFile = 0;
		
		#ifdef OUT_SERIAL
		// debug
		uint8_t cntprint=0;
		#endif
		
		while((!fBuffFull)&&(!fReadEndFile)&&(!fForceEnd)){	// バッファがいっぱいかファイル読めないかファイルの最後まで読むまで繰り返し
			// ブロックサイズ分読む
	        if (readn(PcmData, PlayFile, fmt.blockSize) == fmt.blockSize) {
				uint8_t Data = (fmt.samplingBits == 8) ? PcmData[0] : PcmData[1] ^ 0x80;		// 左チャンネルのみ, 0～255化
				
				// 再生バッファに足す
				fBuffFull = (bool_t)GrEx.Sp->addBuff(Data);		// 正常に追加されれば0が返る
				
				// 最後まで読んだかチェック
				if(PcmReadPos<(PcmDataSize-1)){
					PcmReadPos++;
				}else{
					PcmReadPos = 0;
					fReadEndFile = 1;
				}
			}else{
				
				//ブロックサイズ分読めなかったら死
	            Oled.print("read error 4: '%s'\n", PlayFileName);
	            fForceEnd = 1;
	        	fReadEndFile = 1;
				
			}
			
			if(!LoopCnt){
				fBuffFull = 1;	// とりあえず終わる
			}else{
				LoopCnt--;
			}
			
		#ifdef OUT_SERIAL
			cntprint++;
			if(!cntprint){	//0の時
				Serial.println("wait read finish.");
			}
		#endif
		}
		
		
		// ファイル読み終わったしバッファもなくなったよ。もしくは強制停止
		//  再生停止処理
		if((fReadEndFile && GrEx.Sp->isBuffEmpty()) || fForceEnd){
			fReadEndFile = 0;
			stop(0);		// 停止処理(レジュームしない)
			if(fForceEnd){
				GrEx.Sp->clearBuff();	// スピーカバッファクリア
				
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






