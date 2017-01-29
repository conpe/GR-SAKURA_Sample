
// E2データフラッシュ読み書き
// データ型：uint16_t
// 容量：32kbyte = 16word
// アドレス：0～16383


//ブロック：32byte * 1024ブロック = 32kbyte
//イレース 32byte単位
//ライト 2byte単位


#include "drvDataFlash.h"
#include <string.h>

// ドライバ
static int8_t initE2flash(void);
static int8_t writeE2flash(uint16_t Address, uint16_t Data);
static int8_t writeE2flash32byte(uint16_t* Address, uint8_t* Data, uint8_t Num);
static int8_t readE2flash(uint16_t Address, uint16_t* RetData);

static int8_t fld_blank_check_2B(uint32_t addr);
static int8_t fld_erase_32B(uint32_t addr);
static int8_t fld_program_word(uint32_t addr, uint16_t Data);
static int8_t fld_enable_read(void);
static int8_t fld_disable_read(void);

static void (*AddedWriteTask)(void) = 0x00000000;

#define E2_START_ADRS 0x00100000
#define E2_DF_WRITE_BUF_NUM 64
#define E2_DF_AREA_DATA_NUM 32		// 1エリアのデータ数

// 書き込みバッファ
static uint8_t drvDataFlashWriteBufPt = 0;
static uint16_t drvDataFlashWriteBufAdrs[E2_DF_WRITE_BUF_NUM];
static uint8_t drvDataFlashWriteBufData[E2_DF_WRITE_BUF_NUM];

static uint16_t WriteBufAdrs[E2_DF_AREA_DATA_NUM];	// 書き込み用一時バッファ
static uint8_t WriteBufData[E2_DF_AREA_DATA_NUM];

/*****************************************
データフラッシュ書き込み
概要：
	データフラッシュに書き込むデータ登録
	書き込みバッファに格納し、書き込みタスクによって書き込まれる
引数：
	なし
返値：
	書き込み結果
	DF_OK：成功
******************************************/
int8_t drvDataFlash_write(uint16_t Adrs, uint8_t Data){
	
	if(drvDataFlashWriteBufPt < E2_DF_WRITE_BUF_NUM){	// バッファ空きあり
		drvDataFlashWriteBufAdrs[drvDataFlashWriteBufPt] = Adrs;
		drvDataFlashWriteBufData[drvDataFlashWriteBufPt] = Data;
		++drvDataFlashWriteBufPt;
		
		return DF_OK;
	}else{
		return DF_BUFF_FULL;
	}
}

/*****************************************
データフラッシュ書き込みタスク
概要：
	バッファにあるデータを順次書き込む
引数：
	なし
返値：
	書き込み結果
	DF_OK：成功
******************************************/
int8_t drvDataFlash_WriteTask(void){
	int8_t ret = DF_OK;
	uint8_t WriteNum;
	int16_t PtTmp;
	uint8_t fNotUniqueAdrs;
	uint8_t i;
	uint32_t AreaHeadAdrs;
	
	uint16_t NotWriteBufAdrs[E2_DF_WRITE_BUF_NUM-1];	// 書かなかったデータ
	uint8_t NotWriteBufData[E2_DF_WRITE_BUF_NUM-1];
	uint16_t NotWriteNum;
	
	// 実行タスク登録されていれば
	if(0 != AddedWriteTask){
		(*AddedWriteTask)();	// 実行する
	}
	
	while((0 < drvDataFlashWriteBufPt)&&(DF_OK==ret)){	// 書き込みデータまだあり かつ 前回書込結果OK
		// 書き込みデータあり
		// 1バイト(アプリ書き込み単位)→2バイト(E2データフラッシュ書き込み単位)
		// 同じ書き込み領域をまとめて送る
		
		
		PtTmp = drvDataFlashWriteBufPt-1;
		WriteNum = 0;
		NotWriteNum = 0;
		AreaHeadAdrs = ((uint32_t)drvDataFlashWriteBufAdrs[PtTmp])&0xFFFFFFE0;
		while((PtTmp>=0)&&(WriteNum<E2_DF_AREA_DATA_NUM)){	// 書き込みバッファにあるデータ分。もしくは、一度に書き込めるデータ分
			
			if(AreaHeadAdrs == ((uint32_t)drvDataFlashWriteBufAdrs[PtTmp]&0xFFFFFFE0)){	// 同じエリア内のデータ
				
			
				// 同じアドレスに書く場合、あとからバッファに追加したもののみを書く
				i = 0;
				fNotUniqueAdrs = false;
				while(i<WriteNum){
					if(WriteBufAdrs[i]==drvDataFlashWriteBufAdrs[PtTmp]){
						fNotUniqueAdrs = true;
					}
					++i;
				}
				if(false == fNotUniqueAdrs){	// 今回初めて書くアドレス
					WriteBufAdrs[WriteNum] = drvDataFlashWriteBufAdrs[PtTmp];
					WriteBufData[WriteNum] = drvDataFlashWriteBufData[PtTmp];
					--PtTmp;
					++WriteNum;
				}else{				// 今回、すでに書いているアドレス
								// 書き込みデータを無視する
					--PtTmp;
				}
			}else{		// 違う書き込み領域
					// 次回書くように覚えておく
				NotWriteBufAdrs[NotWriteNum] = drvDataFlashWriteBufAdrs[PtTmp];
				NotWriteBufData[NotWriteNum] = drvDataFlashWriteBufData[PtTmp];
				++NotWriteNum;
				--PtTmp;
			}
		}
		
		ret = writeE2flash32byte(WriteBufAdrs, WriteBufData, WriteNum);	// 書き込み実施
		
		if(FLD_OK == ret){
			// 書き込み成功なら
			drvDataFlashWriteBufPt -= WriteNum;	// 書いた分ポインタを減らす
			
			// 書かなかったデータを覚え直す
			for(i=0; i<NotWriteNum; i++){
				drvDataFlashWriteBufAdrs[drvDataFlashWriteBufPt-1-i] = NotWriteBufAdrs[i];
				drvDataFlashWriteBufData[drvDataFlashWriteBufPt-1-i] = NotWriteBufData[i];
			}
			
			ret = DF_OK;
		}
	}
	
	return ret;
}

/*****************************************
書き込み時実行タスク登録
概要：
	定期的に呼び出す書き込み処理で実行するタスクを登録
引数：
	実行関数ポインタ
返値：
	なし
******************************************/
void drvDataFlash_attachWriteTask( void (*Tsk)(void)){
	AddedWriteTask = Tsk;
}

/*****************************************
書き込み時実行タスク登録解除
概要：
	定期的に呼び出す書き込み処理で実行するタスクを解除
引数：
	なし
返値：
	なし
******************************************/
void drvDataFlash_dettachWriteTask(void){
	AddedWriteTask = 0x00000000;
}


/*****************************************
E2データフラッシュ書き込み(wordサイズ)
概要：
	指定アドレスに値を書く
	データは16bit。
引数：
	address: アドレス 0～16383 (1刻み)
返値：
	書込結果
	アドレス範囲外の場合は値を書かずにFLD_ERRORを返す
******************************************/
int8_t drvDataFlash_writeImmWord(uint16_t Address, uint16_t Data){
	switch(writeE2flash(Address, Data)){
	case FLD_OK:
		return DF_OK;
		break;
	case FLD_OUT_OF_ADDRESS:
		return DF_OUT_OF_ADDRESS;
		break;
	default:
		return DF_ERROR;
	}
}
static int8_t writeE2flash(uint16_t Address, uint16_t Data){
	
	uint16_t DataTmp[16];
	bool_t DataExist[16] = {0};
	uint32_t CheckingAdrs;
	uint32_t i;
	int8_t ret;
	uint32_t AreaHeadAdrs = ((uint32_t)Address*2)&0xFFFFFFE0;
	
	// イレースがブロック(32byte)単位なので、
	// 書きたいアドレスがブランクじゃない場合、
	// 32byteのち、ブランクじゃない場所のデータを読みだして、
	// イレース後、また書いておく。
	
	if(Address>=E2_DF_SIZE_BYTE){	// アドレス範囲外
		return FLD_OUT_OF_ADDRESS;
	}
	
	if(1==FLASH.FSTATR0.BIT.FRDY){	// フラッシュ動作中じゃないフラグ
		// ブランクチェック(ブランクじゃないと書けない)
	
		switch(fld_blank_check_2B(E2_START_ADRS + (uint32_t)Address*2)){
		case FLD_BLANK:	
			// ブランクなので
			// そのアドレスのみ書き込み
			ret = fld_program_word(E2_START_ADRS + (uint32_t)Address*2, Data);
			break;
		case FLD_NOBLANK:
			// ブランクじゃないので
			// 書き込み済みの値を読み込む
			for(i=0; i<16; i++){
				CheckingAdrs = E2_START_ADRS + AreaHeadAdrs + i*2;
				
				if(CheckingAdrs == (E2_START_ADRS + ((uint32_t)Address*2))){
					// 書き込み対象アドレスは指定データをセット
					DataTmp[i] = Data;
					DataExist[i] = true;
				}else{	// それ以外は書いてあるデータをセット
					if(readE2flash(((uint32_t)Address&0xFFFFFFF0)+i, &DataTmp[i]) == FLD_OK){
						DataExist[i] = true;
					}else{
						DataExist[i] = false;
					}
				}
			}
			
			// イレースする
			if(fld_erase_32B(E2_START_ADRS + AreaHeadAdrs) != FLD_OK){
				return FLD_ERROR;
			}
			
			// 書き込み済みの値と新規のデータをセット
			for(i=0; i<16; i++){
				if(DataExist[i]){
					CheckingAdrs = E2_START_ADRS + AreaHeadAdrs + i*2;
					ret = fld_program_word(CheckingAdrs, DataTmp[i]);
					if(ret != FLD_OK){
						return FLD_ERROR;
					}
				}
			}
			break;
		//default:
			ret = FLD_ERROR | 0x10;
		}
	}else{
		ret = FLD_BUSY;
	}
	
	return ret;
}

/*****************************************
E2データフラッシュ書き込み(byteサイズ 32byteまで)
概要：
	指定アドレスに値を書く
	データは8bit。
	アドレスはすべて同じ領域にある必要がある
引数：
	address: アドレス 0～32768
返値：
	書込結果
	アドレス範囲外の場合は値を書かずにFLD_ERRORを返す
******************************************/
static int8_t writeE2flash32byte(uint16_t* Address, uint8_t* Data, uint8_t Num){
	uint16_t DataTmp[16];
	bool_t DataExist[16] = {0};
	uint8_t CheckingIdx;
	uint8_t WritingIdx;
	uint32_t i;
	int8_t ret = FLD_OK;
	uint32_t AreaHeadAdrs = ((uint32_t)Address[0])&0xFFFFFFE0;
	int8_t Blank;
	uint32_t fWriteDone = 0;
	
	// イレースがブロック(32byte)単位なので、
	// 書きたいアドレスがブランクじゃない場合、
	// 32byteのち、ブランクじゃない場所のデータを読みだして、
	// イレース後、また書いておく。
	
	if(0<Num){
		for(i=0;i<Num;++i){
			if((Address[i]>=E2_DF_SIZE_BYTE)||(AreaHeadAdrs!=((uint32_t)Address[i])&0xFFFFFFE0)){
				// アドレス範囲外 もしくは 領域がまたがっている
				return FLD_OUT_OF_ADDRESS;
			}
		}
		if(1==FLASH.FSTATR0.BIT.FRDY){	// フラッシュ動作中じゃないフラグ
			
			Blank = FLD_BLANK;
			// 書き込み済みの値を読み込む
			for(i=0; i<16; i++){
				DataTmp[i] = 0x0000;
				switch(readE2flash(AreaHeadAdrs/2 + i, &DataTmp[i])){	// 値読み込み
				case FLD_OK:
					DataExist[i] = true;					// 値書き込み済みだった
					Blank = FLD_NOBLANK;
					break;
				case FLD_BLANK:
					DataExist[i] = false;					// ブランクだった
					break;
				default:
					return FLD_ERROR - 20;					// 読み込み失敗
					break;
				}
			}
			
			// 書き込みデータ設定
			fWriteDone = 0;
			for(WritingIdx = 0; WritingIdx<Num; ++WritingIdx ){
				if(!(fWriteDone&(0x00000001<<WritingIdx))){	// まだ見てない
					fWriteDone |= (0x00000001<<WritingIdx);	// 書いたフラグ立てる
					//AdrsTmp = Address[WritingIdx]&0xFFFFFFFE;	// 書き込むE2データフラッシュアドレス設定(2byte刻み)
					CheckingIdx = Address[WritingIdx]>>1;		// 書き込むアドレスのインデックス(AreaHeadAdrsを0としたWORDサイズのインデックス)
					DataExist[CheckingIdx] = true;			// 書き込み対象としてチェック
					
					if(Address[WritingIdx]&0x00000001){	// Wordの下半分
						DataTmp[CheckingIdx] = (DataTmp[CheckingIdx]&0xFF00) | (uint16_t)Data[WritingIdx];	// 下半分に書く
					}else{					// Wordの上半分
						DataTmp[CheckingIdx] = (DataTmp[CheckingIdx]&0x00FF) | (((uint16_t)Data[WritingIdx])<<8);	// 上半分に書く
					}
					
					// 2バイトまとめて書き込みデータ作る
					for(i=0; i<Num;++i){	// 同じE2フラッシュアドレスのを探す
						if(!(fWriteDone&(0x00000001<<i))){	// まだ見てない
							if((Address[i]>>1) == CheckingIdx){	// 下位1bitを削って同じなら同じ
								// データを構築
								// 同じアドレスなら後ろにあるのを優先する(上書きする)
								if(Address[i]&0x00000001){	// アドレス+1
									DataTmp[CheckingIdx] = (DataTmp[CheckingIdx]&0xFF00) | (uint16_t)Data[i];	// 下半分に書く
								}else{
									DataTmp[CheckingIdx] = (DataTmp[CheckingIdx]&0x00FF) | (((uint16_t)Data[i])<<8);	// 上半分に書く
								}
								
								fWriteDone |= (0x00000001<<i);	// 書いたフラグ立てる
							}
						}
					}
				}
			}
			
			// 書き込み済みのところがある場合は書けないのでイレースする
			if(FLD_NOBLANK == Blank){
				if(FLD_OK != fld_erase_32B(E2_START_ADRS + AreaHeadAdrs)){
					return FLD_ERROR - 30;
				}
			}
			
			// 16wordの書き込み実施
			for(i=0; i<16; i++){
				if(true == DataExist[i]){	// 書き込み対象
					ret = fld_program_word(E2_START_ADRS + AreaHeadAdrs + i*2, DataTmp[i]);
					if(FLD_OK != ret){
						return FLD_ERROR - 40 - (int8_t)i;
					}
				}
			}
			
		}else{
			ret = FLD_BUSY;
		}
	}
	return ret;
}

/*****************************************
E2データフラッシュ読み込み(byteサイズ)
概要：
	指定アドレスにかかれている値を返す
引数：
	address: アドレス 0～32768 (1刻み)
返値：
	指定アドレスに書かれている値
	アドレス範囲外の場合は0xFFFFを返す
******************************************/
int8_t drvDataFlash_read(uint16_t Address, uint8_t* RetData){
	uint16_t RetDataTmp; 
	
	switch(readE2flash(Address/2, &RetDataTmp)){
	case FLD_OK:
		if(Address&0x00000001){	// 上位アドレス (データの下半分)
			*RetData = (uint8_t)(RetDataTmp&0x00FF);
		}else{			// 下位アドレス (データの上半分)
			*RetData = (uint8_t)((RetDataTmp>>8)&0x00FF);
		}
		
		return DF_OK;
	case FLD_OUT_OF_ADDRESS:
		return DF_OUT_OF_ADDRESS;
	case FLD_BLANK:
		return DF_BLANK;
	default:
		return DF_ERROR;
	}
}

/*****************************************
E2データフラッシュ読み込み(wordサイズ)
概要：
	指定アドレスにかかれている値を返す
引数：
	address: アドレス 0～16383 (1刻み)
返値：
	指定アドレスに書かれている値
	アドレス範囲外の場合は0xFFFFを返す
******************************************/
int8_t readE2flash(uint16_t address, uint16_t* RetData){
	int8_t ret;
	
	if(address>=E2_DF_SIZE_BYTE){	// アドレス範囲外
		return FLD_OUT_OF_ADDRESS;
	}
	
	ret = fld_blank_check_2B(E2_START_ADRS + (uint32_t)address*2);	// ブランクチェック
	if(FLD_NOBLANK == ret){
		// ブランクじゃない
		fld_enable_read();
		*RetData = *(uint16_t *)(E2_START_ADRS + (uint32_t)address*2);	// 指定箇所返す
		fld_disable_read();
		return FLD_OK;
	}else{
		// ブランク か エラーか
		return ret;
	}
	
	return ret;
}


// 初期化
int8_t drvDataFlash_init(void){
	return initE2flash();
}

static int8_t initE2flash(void){
	volatile uint32_t i;
	static const int fcu_ram_size = 8*1024;	// 8k byte
	void *fcu_ram = (uint32_t *)0x007F8000;
	void *fcu_fw = (uint32_t *)0xFEFFE000;
	
	volatile uint8_t *addr_b = (uint8_t *)E2_START_ADRS;
	volatile uint16_t *addr_w = (uint16_t *)E2_START_ADRS;
	
	
	/* ROMに格納されているFCUのファームウェアをRAMへコピー */
	// リードモードへ
	if((FLASH.FENTRYR.WORD & 0x00FF)!=0x0000){
		FLASH.FENTRYR.WORD = 0xAA00;
	}
	// FCU RAMエリアアクセス許可
	FLASH.FCURAME.WORD = 0xC401;
	
	// RAMにFCUファームをコピー
	memcpy(fcu_ram, fcu_fw, fcu_ram_size);
	
	/* 周辺クロックを設定 */
	// P/Eモードへ
	if((FLASH.FENTRYR.WORD & 0x00FF)!=0x0080){
		FLASH.FENTRYR.WORD = 0xAA80;
	}
	// 周辺クロック値をセット
	FLASH.PCKAR.BIT.PCKA = 48;	//48MHz (ひとまず固定値;;;)
	
	// 周辺クロック設定を実行
	*addr_b = 0xE9;
	*addr_b = 0x03;
	*addr_w = 0x0f0f;
	*addr_w = 0x0f0f;
	*addr_w = 0x0f0f;
	*addr_b = 0xD0;
	
	// tPCKA時間待つ
	//if( wait_FRDY(120) == FLD_TMOUT){	
	//}
	for(i=0;i<65535;++i){
	}
	
	
	if(FLASH.FSTATR0.BIT.ILGLERR == 1){
		return FLD_ERROR;
	}
	
	return FLD_OK;
}


// 細かい関数 //

// ブランクチェック 2B
// addr : 絶対アドレス
static int8_t fld_blank_check_2B(uint32_t addr){
	
	volatile uint8_t *adrr_b = (uint8_t *)(addr&0xFFFFFC00);	// ブロック先頭アドレス
									// 2kB単位
	volatile uint16_t adrr_ofs = (uint16_t)(addr&0x000003FF);	// ブロック先頭アドレスからのオフセット
	
	
	// P/Eモードへ
	if((FLASH.FENTRYR.WORD & 0x0080) != 0x0080){
		FLASH.FENTRYR.WORD = 0xAA80;
	}
	
	FLASH.FWEPROR.BIT.FLWE = 1; // Unptorect
	
	// use the blank checking command
	FLASH.FMODR.BIT.FRDMD = 1;
	
	// set the blank check size (2B)
	FLASH.DFLBCCNT.BIT.BCSIZE = 0;
	FLASH.DFLBCCNT.BIT.BCADR = adrr_ofs;		// 2kB単位にした時の余り
	
	// execute the blank checking command
	*adrr_b = 0x71;
	*adrr_b = 0xD0;
	
	while(FLASH.FSTATR0.BIT.FRDY == 0);
	//wait for tDBC2K time (timeout is 770us)
	// if(wait_FRDY(700 * 1.1) == FLD_TMOUT)
	// {
	// reset_fcu();
	// }
	// error check
	
	FLASH.FWEPROR.BIT.FLWE = 2; // protect
	
	if(FLASH.FSTATR0.BIT.ILGLERR){
		return FLD_ERROR;
	}
	
	// get result of blank checking command
	if(0 == FLASH.DFLBCSTAT.BIT.BCST){
		return FLD_BLANK;
	}
	
	return FLD_NOBLANK;
}

// 消去 32B
// addr : 絶対アドレス
static int8_t fld_erase_32B(uint32_t addr){
	volatile uint8_t *addr_b = (uint8_t *)(addr&0xFFFFFFE0);	// 32byte単位
	int32_t ret = FLD_OK;
	
	// P/Eモードへ
	if((FLASH.FENTRYR.WORD & 0x0080) != 0x0080){
		FLASH.FENTRYR.WORD = 0xAA80;
	}
	
	FLASH.FWEPROR.BIT.FLWE = 1; // Unptorect
	FLASH.DFLWE0.WORD = 0x1EFF; //1Eh is Key
	FLASH.DFLWE1.WORD = 0xE1FF; //E1h is Key
	// execute the block erase command
	*addr_b = 0x20;
	*addr_b = 0xD0;
	while(FLASH.FSTATR0.BIT.FRDY == 0);	// 11ms位かかるかも
	if((FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.ERSERR == 1)){ // error check
		ret = FLD_ERROR;
	}else{
		/* nothing to do */
	}
	FLASH.FWEPROR.BIT.FLWE = 2; // protect
	FLASH.DFLWE0.WORD = 0x1E00; // disable
	FLASH.DFLWE1.WORD = 0xE100; // disable
	return ret;
}

// 2byte書き込み
// addr : 絶対アドレス
static int8_t fld_program_word(uint32_t addr, uint16_t Data){
	volatile uint8_t *addr_b = (uint8_t *)addr;
	 volatile uint16_t *addr_w = (uint16_t *)addr;
	int32_t ret = FLD_OK;
	
	// P/Eモードへ
	if ((FLASH.FENTRYR.WORD & 0x0080) != 0x0080) {
		FLASH.FENTRYR.WORD = 0xAA80;
	}
	
	FLASH.FWEPROR.BIT.FLWE = 1; // Unprotect
	FLASH.DFLWE0.WORD = 0x1EFF; // 1Eh is Key
	FLASH.DFLWE1.WORD = 0xE1FF; // 1Eh is Key
	
	// execute the 2-byte programming command
	*addr_b = 0xE8;
	*addr_b = 0x01;
	*addr_w = Data;
	*addr_b = 0xD0;
	
	while(FLASH.FSTATR0.BIT.FRDY == 0);	//11msくらいかかるかも
	
	if((FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.ERSERR == 1)){ //error check
		ret = FLD_ERROR;
	}else{
		/* nothing to do */
	}
	
	FLASH.FWEPROR.BIT.FLWE = 0x02;// protect
	FLASH.DFLWE0.WORD = 0x1E00;// disable
	FLASH.DFLWE1.WORD = 0xE100;// disable
	
	return ret;
}

// 読み出し許可
static int8_t fld_enable_read(void){
	// リードモードへ
	if((FLASH.FENTRYR.WORD & 0x00FF) != 0x0000){
		FLASH.FENTRYR.WORD = 0xAA00;
	}
	
	FLASH.DFLRE0.WORD = 0x2DFF;
	FLASH.DFLRE1.WORD = 0xD2FF;
	
	return FLD_OK;
}
static int8_t fld_disable_read(void){
	// リードモードへ
	if((FLASH.FENTRYR.WORD & 0x00FF) != 0x0000){
		FLASH.FENTRYR.WORD = 0xAA00;
	}
	
	FLASH.DFLRE0.WORD = 0x2D00;
	FLASH.DFLRE1.WORD = 0xD200;
	
	return FLD_OK;
}