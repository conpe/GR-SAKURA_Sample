/********************************************/
/*			OLED(SSD1306)表示 Bmpファイル	*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/04/03		*/
/********************************************/

// 【更新予定】


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
OLEDにビットマップファイル表示
引数
 *FileName
 StartLineNum	: 表示し始め (画面の下から書くので、表示範囲の下限ライン)
 WriteLineNum	: 表示ライン数   (下限位置からの表示ライン数)
返値
 -1 : ファイルオープン失敗
 -2 : ビットマップファイルでない
 -3 : 画像の横幅がOLEDにあってない
 -4 : モノクロでない
**************************************/
int8_t mmcBmpOled::outBmp(const char *FileName, const uint8_t StartLineNum, const uint8_t WriteLineNum){
	uint8_t Data[4];
	uint8_t *DisplayBuff;
	uint16_t i;
	
	// ディスプレイに渡すためのバッファ。
	// 1ライン分確保
	DisplayBuff = new uint8_t[DisplayWidth];
	
	
	// ファイルオープン
	*BmpFile = MMC->open(FileName, FILE_READ);	// 中でnewしてるだろうから注意
	if(!(*BmpFile)){
		OledDriver->println("..Cant Open %s", FileName);
		return -1;
	}
	// ヘッダチェック
	// 先頭がBM
	readn(Data, BmpFile, 2);
	if(memcmp(Data, "BM", 2)){
		OledDriver->println("..Not Bmp %s", FileName);
		return -2;		// no Bitmap file
	}
	
	// 画像データ開始アドレス
	BmpFile->seek(0x0A);
	readn(Data, BmpFile, 4);	// リトルエンディアンになるか確認用
	//uint32_t Image = (uint16_t)Data[3]<<24 | (uint16_t)Data[2]<<16 | (uint16_t)Data[1]<<8 | (uint16_t)Data[0];
	uint32_t BmpAdrsImage = *(uint32_t*)Data;
	
	// 画像サイズチェック
	BmpFile->seek(0x12);
	readn(Data, BmpFile, 4);
	uint32_t BmpWidth = *(uint32_t*)Data;
	readn(Data, BmpFile, 4);
	int32_t BmpHeight = *(uint32_t*)Data;		// 負だったら上から下にデータが並ぶ

	// モノクロチェック
	BmpFile->seek(0x1C);
	readn(Data, BmpFile, 2);
	uint16_t BitPix = *(uint16_t*)Data;
	if( BitPix != 1){
		return -4;
	}
	
	// 画像情報
	// 幅
	uint16_t WidthByte = BmpWidth/BitPix/8;
	// 4の倍数に合わせる
	WidthByte += WidthByte%4;
	// 高さ(ライン (=縦方向のひとまとまり))
	uint8_t BmpHeightLine = (BmpHeight+8)/8;	// OLEDのライン数(切り上げ)
	
	uint8_t CntRowLine;
	uint8_t CntColByte;
	uint8_t CntRow;
	uint8_t CntColBit;
	
	
	// 画像をOLEDに書き出し
	// 高さ8とか幅4で合わせられない時の処理いれる！
	
	uint32_t StartAdrs;	
	uint8_t LineNumMax;	// 何ライン書くか
	if(BmpHeightLine<WriteLineNum){	// 書くラインは小さい方を選択
		LineNumMax = BmpHeightLine;
	}else{
		LineNumMax = WriteLineNum;
	}
	for(CntRowLine=0; CntRowLine<LineNumMax; CntRowLine++){		// 縦：OLEDのラインでカウント
		
		StartAdrs = BmpAdrsImage + WidthByte*DisplayLinePix*CntRowLine;	// 各ラインの左下のデータの場所
		BmpFile->seek(StartAdrs);
		
		// バッファクリア
		for(i=0; i<DisplayWidth; i++){
			DisplayBuff[i] = 0;
		}
		
		for(CntRow=0; CntRow<DisplayLinePix; CntRow++){					// 1ライン内でのRow
			if(CntRowLine*DisplayLinePix+CntRow < BmpHeight){			// 高さがディスプレイ範囲内
				for(CntColByte=0; CntColByte<WidthByte; CntColByte++){	// 幅：バイト数でカウント
					if(CntColByte * (8/BitPix) < DisplayWidth ){		// 幅がディスプレイ範囲内
					
						*Data = BmpFile->read();	// 1バイト読む
						
						for(CntColBit=0;CntColBit<8;CntColBit++){	// 各列処理
							//if(CntColByte * (8/BitPix) + CntColBit < DisplayWidth){	// 幅はみ出さない時だけ処理 -> どうせ0になるだけなので
							DisplayBuff[CntColByte*8 + CntColBit] |= (((*Data)>>(8-CntColBit-1)&0x01)<<(DisplayLinePix-CntRow-1));
							//}
						}
					}
				}
			}
		}
		OledDriver->setCur(StartLineNum-CntRowLine, 0);	// 下から順に表示場所選択
		for(i=0;i<DisplayWidth;i++){
			OledDriver->writeData(DisplayBuff[i]);		// OLEDに出力
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




