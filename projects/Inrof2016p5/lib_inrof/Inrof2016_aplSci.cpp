
#include "Inrof2016_aplSci.h"



inrof2016_aplSci_t::inrof2016_aplSci_t(Sci_t *SciObject){
	Sci = SciObject;
	Mode = PC_THROUGH;
	Ps3ConAvailable = false;
	Ps3ConDisableCnt = 0;
};


/*********************
通信データ解析
概要：
	モードに応じて受信バッファを解釈する
	定期的に呼び出すこと
**********************/
void inrof2016_aplSci_t::task(void){
	uint8_t cnt;
	uint8_t tmp[8];
	uint8_t Sum = 0;
	
	switch(Mode){
	case PS3:
		if(Ps3ConDisableCnt>SBDBT_PS3CON_DISABLECNT){
			Ps3ConAvailable = false;
		}else{
			Ps3ConDisableCnt ++;
		}
		// 受信データをコントローラデータにぶっこむ
		while(SBDBT_PS3CON_DATALENGTH <= Sci->available()){	// 受信数がコントローラ全データ数以上だったら
			// データ先頭候補を見つける
			tmp[0] = 0xFF;
			while(0x80 != tmp[0]){
				if(Sci->receive(&tmp[0])){
					return;		// 先頭バイト見つかることなく受信バッファ無くなった
							// 正常な限りありえない
				}
			}
			
			// RCB3フォーマット分読む
			Sum = 0;
			for(cnt=1;cnt<7;cnt++){		// 1~6byte
				Sci->receive(&tmp[cnt]);
				Sum += tmp[cnt];
			}
			Sci->receive(&tmp[cnt]);	// 7byte
			Sum = Sum&0x7F;	// チェックサムは下位7bit
			
			
			// チェックサム確認
			if(tmp[7] == Sum){
				// サムok
				Ps3ConAvailable = true;
				Ps3ConDisableCnt = 0;		// コントローラ無効カウントリセット
				// 残りのデータを突っ込む
				for(cnt=8;cnt<SBDBT_PS3CON_DATALENGTH;cnt++){
					Sci->receive(&ControllerData.Data[cnt]);
				}
			}else{
				// サムng	
				// 次のデータ先頭候補まで削って終わる
				while(1){
					if(Sci->watch(&tmp[0])){
						return;		// バッファなし
					}
					if(0x80 == tmp[0]){	// 先頭バイト発見
						break;		// 
					}else{			// 先頭バイトじゃない
						Sci->receive(&tmp[0]);	// 消す
					}
				}
			}
		}
		
		break;
	case PC:
		
		break;
	case PC_THROUGH:
		// nothing to do
		break;
	default:
		// nothing to do
		break;
	}
	
}


// モードをセット
void inrof2016_aplSci_t::setMode(mode_t NewMode){
	
	if(Mode!=NewMode){	// モード変わったらバッファクリア
		Sci->clearRxBuff();
	}
	
	Mode = NewMode;
}