/********************************************/
/*		SCIモジュールを使用したSCI通信		*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/07/19		*/
/********************************************/

// とりあえずSCI0, 1, 2, 5のみ実装
// SCI2は受信,送信完了割り込みができない(´・ω・｀)＠GR-SAKURA

// 送信要求出すと(transmit)一度リングバッファに蓄えられ、送信バッファエンプティ割り込みによって順次送信していく。
// 受信は常に割り込みでリングバッファに蓄えられる。receiveで読み出す。
//  各バッファは、いっぱいになると-1を返し書き込まない。

//【デフォルト設定】
// 送信割り込み on(必須)
// 送信完了割り込み off
// 受信割り込み on(必須)


// 【注意】
// 継承しないこと。バッファが余計に作られるから無駄だよ。割り込み思ったところに入らないし。
// 
// めっちゃタイミングが悪いと、1バイトのみ単発で送ると送られないかも！
// (「バッファすべての送信終了の瞬間に新しく1バイト追加」で発生する可能性あり)
// もう1バイト送れば流れだすのであまり気にしなくてもいいかも。

//【更新履歴】
// 2015.07.19 割り込み周り整理
// 2015.11.28 printメソッド、動的に確保してたら他のとこ侵食してたので静的に確保するように変更。
// 2016.03.15 受信ピンは内臓プルアップ有効にするようにした
// 2016.06.22 TXI,TEI割り込みを常に有効にした。
//		SCI1実装

//【更新したい】
// レジスタ設定関連を別ファイルに
// パケット通信処理入れたい
// SCI2使えないの直す

#ifndef __SCI_H__
#define __SCI_H__

#include <machine.h>	// マイコンコンパイラ固有関数

#include <stdio.h>
#include <stdarg.h>

#include <math.h>
#include "iodefine.h"
#include "CommonDataType.h"
#include "RingBuffer.h"
#include "portReg.h"

// リングバッファのサイズ
#define SCI_TX_BUFFER_SIZE_DEFAULT 255
#define SCI_RX_BUFFER_SIZE_DEFAULT 127

#define SCI_MAX_STRING 127

extern uint32_t getPCLK(void);


enum sci_module
{
	SCI_SCI0 = 0,
	SCI_SCI1 = 1,
	SCI_SCI2 = 2,
	//SCI_SCI3 = 3,
	//SCI_SCI4 = 4,
	SCI_SCI5 = 5,
};
/*
enum Sci_pin
{
};
*/

// debug用
class Sci_t;
extern Sci_t Sci2;


struct sci_config
{
	sci_module		SciModule;
	bool_t			TxEnable;
	bool_t			RxEnable;
	uint32_t		BaudRate;
	int16_t			TxBuffSize;
	int16_t			RxBuffSize;
};


enum Sci_state
{
	SCI_BEGIN = 0,
	SCI_TX_START = 1,
	SCI_TX_TXI = 2,
	SCI_TX_TEI = 3,
	SCI_RX_RXI = 4
};

enum Sci_err
{
	SCI_OK		= 0,
	SCI_NODATA	= 1,
	SCI_TX_ERR	= 2,
	SCI_RX_ERR	= 3,
	SCI_MEM_ERR	= 4,	// メモリ確保エラー
};

class Sci_t
{
public:
	// Constructor
	Sci_t(sci_module SciModule);
	~Sci_t(void);
	
	// Configration
	int8_t begin(uint32_t BaudRate, bool_t UseTx = true, bool_t UseRx = false, int16_t TxBuffSize = SCI_TX_BUFFER_SIZE_DEFAULT, int16_t RxBuffSize = SCI_RX_BUFFER_SIZE_DEFAULT);
	
	// CallBack
	void attachTxEndCallBackFunction(void (*CallBackFunc)(void)){this->TxEndCallBackFunc = CallBackFunc;};
	void detachTxEndCallBackFunction(void){this->TxEndCallBackFunc = NULL;};
	void attachRxCallBackFunction(void (*CallBackFunc)(RingBuffer<uint8_t> * RxBuff)){	this->RxCallBackFunc = CallBackFunc;	};
	void detachRxCallBackFunction(void){this->RxCallBackFunc = NULL;};
	
	// Transmit/Receive
	Sci_err transmit(uint8_t Data);		// 送信
	Sci_err receive(uint8_t *Data);		// 受信データ読む(ポインタ)
	uint8_t receive(void);			// 受信データ読む
	uint8_t read(void){return receive();};	// 受信データ読む
	Sci_err watch(uint8_t *Data){return (0==RxBuff->watch(Data))?SCI_OK:SCI_RX_ERR;};	// 値見るだけ
	uint8_t watch(void){return RxBuff->watch();};		// 値見るだけ
	Sci_err watch(uint16_t IndexRel, uint8_t *Data){return (0==RxBuff->watch(IndexRel, Data))?SCI_OK:SCI_RX_ERR;};	// 値見るだけ
	uint8_t watch(uint16_t IndexRel){return RxBuff->watch(IndexRel);};		// 値見るだけ
	
	// 拡張Transmit
	Sci_err print(char ch);
	Sci_err print(int num);
	Sci_err print(const char *control, ...);
	Sci_err println(const char *control, ...);
	
	// ユーティリティ
	void clearRxBuff(void);				// ここまでの受信データを捨てる
	void enableTxEndInterrupt(void){fTxEndInterruptEnable = true;};	// 送信完了割り込みon/off
	void disableTxEndInterrupt(void){fTxEndInterruptEnable = false;};
	void enableInterrupts(void);
	void enableIntTxBuffEmpty(void);
	void disableIntTxBuffEmpty(void);
	void enableIntTxEnd(void);
	void enableIntRx(void);
	bool_t isEnableIntRx(void);
	void enableTR(void){SCIreg->SCR.BYTE |= ((SciConfig.TxEnable<<5) | (SciConfig.RxEnable<<4));};		// 送受信有効(TXI有効であれば割り込みも発生)
	void disableTR(void){SCIreg->SCR.BYTE &= ((SciConfig.TxEnable<<5) | (SciConfig.RxEnable<<4))^0xFF;};	
	
	// Status
	Sci_state getState(void){return State;};			// 状態取得
	bool_t isIdle(void){return SCIreg->SSR.BIT.TEND;};		// 送信していない
	bool_t isAvailable(void){return !RxBuff->isEmpty();};		// 受信データあり
	uint16_t available(void){return RxBuff->getNumElements();};	// 受信データ数
	uint16_t getTxBuffFreeSpace(void){return TxBuff->getFreeSpace();}	// 送信バッファの空きバッファ数
	
	// isr
	void isrTx(void);
	void isrTxEnd(void);
	void isrRx(void);
	
private:
	Sci_state State;
	
	sci_config SciConfig;
	RingBuffer<uint8_t> *TxBuff;
	RingBuffer<uint8_t> *RxBuff;
	
	bool_t fEmptySciTxBuff;
	bool_t fTxEndInterruptEnable;
	
	bool_t fTdrEmpty;	// TDRにデータ有り
	bool_t fIsr;		// 送信割り込み中だよ！この間に送信のisrメソッド呼ばないでよ！
	
	bool_t fRxBuffOvf;	// 受信バッファあふれた
	bool_t fTxBuffOvf;	// 送信バッファあふれた
	
	// レジスタ
	volatile __evenaccess struct st_sci0 *SCIreg;
	
	void initRegister(void);
	void setPinModeSci(void);
	
	// レジスタ叩く
	void setSciTxData(uint8_t TxData){SCIreg->TDR = TxData;};	// 送信
	uint8_t getSciRxData(void){return SCIreg->RDR;};		// 受信
	bool_t isFramingErr(void){return SCIreg->SSR.BIT.FER;};		// フレーミングエラー
	void clearFramingErr(void){SCIreg->SSR.BIT.FER=0;};		//   クリア
	bool_t isOverrun(void){return SCIreg->SSR.BIT.ORER;};		// 受信する前にまた受信しちゃった
	void clearOverrun(void){SCIreg->SSR.BIT.ORER=0;};		//   クリア
	
	// 受信コールバック
	void (*TxEndCallBackFunc)(void);	//送信完了コールバック関数へのポインタ
	void (*RxCallBackFunc)(RingBuffer<uint8_t> * RxBuff);	//受信コールバック関数へのポインタ
	
	// 送信開始処理(isrTxを実行)してちょーだい(=TDRレジスタ空いてる&&isrTx処理中でない)
	bool_t needTxStartSequence(void){return fTdrEmpty&&(!fIsr);};
	
	//debug
	//uint8_t getSciRxData(void){return SCIreg->RDR;};
};


extern Sci_t Sci0;
extern Sci_t Sci1;
extern Sci_t Sci2;
extern Sci_t Sci5;


#endif