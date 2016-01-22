
#include "SCI.h"


// Sci module 

//絶対beginするんだぞっ


Sci_t Sci5(SCI_SCI5);
Sci_t Sci2(SCI_SCI2);

// Constructor
Sci_t::Sci_t(sci_module SciModule)
{
	
	//レジスタ
	switch(SciModule){
	case SCI_SCI0:
		SCIreg = &SCI0;
		break;
	case SCI_SCI1:
		SCIreg = &SCI1;
		break;
	case SCI_SCI2:
		SCIreg = &SCI2;
		break;
	case SCI_SCI3:
		SCIreg = &SCI3;
		break;
	case SCI_SCI4:
		SCIreg = &SCI4;
		break;
	case SCI_SCI5:
		SCIreg = &SCI5;
		break;
	}
	
	SciConfig.SciModule = SciModule;
	
	fTdrEmpty = true;
	fTxEndInterruptEnable = false;
	TxEndCallBackFunc = NULL;
	RxCallBackFunc = NULL;
	
	
	TxBuff = NULL;
	RxBuff = NULL;
	
	fEmptySciTxBuff = true;
}

Sci_t::~Sci_t(void){
	// バッファ開放	
	delete TxBuff;
	delete RxBuff;
}

int8_t Sci_t::begin(uint32_t BaudRate){
	
	return begin(BaudRate, true, false);	// default:only tx
	
}

int8_t Sci_t::begin(uint32_t BaudRate, bool_t UseTx, bool_t UseRx){
	return begin(BaudRate, UseTx, UseRx, SCI_TX_BUFFER_SIZE_DEFAULT, SCI_RX_BUFFER_SIZE_DEFAULT);
}


int8_t Sci_t::begin(uint32_t BaudRate, bool_t UseTx, bool_t UseRx, int16_t TxBuffSize, int16_t RxBuffSize){
	SciConfig.BaudRate = BaudRate;
	SciConfig.TxEnable = UseTx;
	SciConfig.RxEnable = UseRx;
	SciConfig.TxBuffSize = TxBuffSize;
	SciConfig.RxBuffSize = RxBuffSize;


	// バッファ確保
	if(UseTx){	
		if(NULL!=TxBuff){
			delete TxBuff;
		}
		TxBuff = new RingBuffer<uint8_t>(SciConfig.TxBuffSize); 
		if(NULL==TxBuff) return -1;
	}

	if(UseRx){
		if(NULL!=RxBuff){
			delete RxBuff;
		}
		RxBuff = new RingBuffer<uint8_t>(SciConfig.RxBuffSize);
		if(NULL==RxBuff) return -1;
	}
	
	
	initRegister();
	setPinModeSci();
	
	// フラグ初期化
	fEmptySciTxBuff = true;
	
	State = SCI_BEGIN;
	return 0;
}


void Sci_t::initRegister(void){
	
	SYSTEM.PRCR.WORD = 0xA503u;
	switch(SciConfig.SciModule){
	case SCI_SCI2:
		MSTP(SCI2) = 0;
		break;	
	case SCI_SCI5:
		MSTP(SCI5) = 0;
		break;	
	}
	
	// 初期化
	SCIreg->SCR.BYTE = 0x00;
	
	// クロックセレクト
	SCIreg->SMR.BIT.CKS = 0;
	
	// ボーレート
	//ボーレート計算
	uint8_t n = SCIreg->SMR.BIT.CKS;
	
	if( (uint16_t)((float)getPCLK() / (unsigned long)((float)(32) * (float)pow(2,(2*n-1))*(float)SciConfig.BaudRate) - 1) > 255) {
		SCIreg->SEMR.BIT.ABCS = 0;
	}else{
		SCIreg->SEMR.BIT.ABCS = 1;
	}
		
	SCIreg->BRR = (uint8_t)( (float)getPCLK() / ((float)(((uint8_t)SCIreg->SEMR.BIT.ABCS&0x01)?32:64) * (float)pow(2, (2*n-1))*(float)SciConfig.BaudRate) - 1);
	/*
	// 115200bps
	SCIreg->SMR.BIT.CKS = 0;
	SCIreg->SEMR.BIT.ABCS = 0;
	SCIreg->BRR = 12;
	*/
	
	// 送受信許可
	// 割り込み有効, 優先度設定
	SCIreg->SCR.BYTE = 0x30;	// 送受信許可
	SCIreg->SCR.BIT.TEIE = 0;	// 送信完了割り込みdisable(ラス1送った時にenableする)
	SCIreg->SCR.BIT.TIE = 1;	// 送信割り込みenable
	SCIreg->SCR.BIT.RIE = 1;	// 受信割り込みenable
	
	enableInterrupts();
	
	SCIreg->SSR.BYTE &= 0x80; 	//エラーフラグクリア
	
}

void Sci_t::enableInterrupts(void){
	
	switch(SciConfig.SciModule){
	case SCI_SCI2:	// シリアル通信ははリアルタイム性なくていいかな
		IEN(SCI2,TXI2) = 1;
		IPR(SCI2,TXI2) = 7;
		IEN(SCI2,TEI2) = 1;
		IPR(SCI2,TEI2) = 2;
		IEN(SCI2,RXI2) = 1;
		IPR(SCI2,RXI2) = 7;
		break;	
	case SCI_SCI5:
		IEN(SCI5,TXI5) = 1;
		IPR(SCI5,TXI5) = 0x07;
		IEN(SCI5,TEI5) = 1;
		IPR(SCI5,TEI5) = 0x07;
		IEN(SCI5,RXI5) = 1;
		IPR(SCI5,RXI5) = 0x0F;
		break;	
	}
}

void Sci_t::setPinModeSci(void){
	
	MPC.PWPR.BYTE = 0x00u;
	MPC.PWPR.BYTE = 0x40u;
	SYSTEM.PRCR.WORD = 0xA503u;
	
	switch(SciConfig.SciModule){
	case SCI_SCI2:
		if(SciConfig.TxEnable){
			setPinMode(P50, PIN_OUTPUT);	// TX output
			MPC.P50PFS.BIT.PSEL = 0x0A;		// TXD2
			PORT5.PMR.BIT.B0 = 1;			// not GPIO	
		}
		if(SciConfig.RxEnable){
			setPinMode(P52, PIN_INPUT);		// RX input
			MPC.P52PFS.BIT.PSEL = 0x0A;		// RXD2 
			PORT5.PMR.BIT.B2 = 1;			// not GPIO	
		}
		break;	
	case SCI_SCI5:
		if(SciConfig.TxEnable){
			setPinMode(PC3, PIN_OUTPUT);	// TX output
			MPC.PC3PFS.BIT.PSEL = 0x0A;		// TXD5
			PORTC.PMR.BIT.B3 = 1;			// not GPIO	
		}
		if(SciConfig.RxEnable){
			setPinMode(PC2, PIN_INPUT);		// RX input
			MPC.PC2PFS.BIT.PSEL = 0x0A;		// RXD5 
			PORTC.PMR.BIT.B2 = 1;			// not GPIO	
		}
		break;	
	}
	
}


int8_t Sci_t::transmit(uint8_t TrData){
	int8_t ack;
	
	//if(this==&Sci5)	PORTD.PODR.BYTE = 0x44;
	ack = TxBuff->add(TrData);
	if(!ack){
	//	SCIreg->SCR.BIT.TIE = 1;	// 送信バッファ空き割り込みenable <- 常にenable
	//	SCIreg->SCR.BIT.TEIE = 0;	// 送信完了割り込みdisable
	}
	
	//if((fEmptySciTxBuff==true)||isIdle()){	//送信リングバッファが空 もしくは モジュールがアイドル状態なら送る
	if(needTxStartSequence()||isIdle()){	// TDRレジスタ開いてる もしくは モジュールがアイドル状態なら送る(何かの拍子にフラグ更新されなかった疑惑)
		isrTx();	// Transmit one time
	}
	//if(this==&Sci5)	PORTD.PODR.BYTE = 0x4F;
		
	State = SCI_TX_START;
	return ack;
}

int8_t Sci_t::receive(uint8_t *RcData){
	return RxBuff->read(RcData);
}

uint8_t Sci_t::receive(void){
	uint8_t RcData = 0xff;
	RxBuff->read(&RcData);
	return RcData;
}

void Sci_t::clearRxBuff(void){
	RxBuff->clear();
}


int8_t Sci_t::print(char ch){
	return transmit(ch);
}


int8_t Sci_t::print(int num){
	int16_t Cnt;
	int16_t Num;
	
	char pStrCnst[SCI_MAX_STRING];
	char *pStr = pStrCnst;
	//char *pStr = new char[SCI_MAX_STRING];
	if(NULL==pStr){		// 確保できなければそのまま出しちゃう
		return -1;	
	}
	
	Num = sprintf(pStr, "%d", num);
	
	for(Cnt=0; Cnt<Num; Cnt++){
		transmit(*pStr++);
	}
	
	//delete[] pStr;
	return 0;
}
	
int8_t Sci_t::print(const char *control, ...){
	int16_t Cnt;
	int16_t Num;
	
	//char *pStr = new char[SCI_MAX_STRING];	// 動的に確保してたら他のとこ侵食してた。なんでだろ。
	char pStrCnst[SCI_MAX_STRING];
	char *pStr = pStrCnst;
	
	if(NULL==pStr){		// 確保できなければそのまま出しちゃう
		pStr = (char *)control;	
		Num = strlen(control);
	}else{
		va_list arg;
		va_start(arg, control);
		Num = vsprintf(pStr, control, arg);
		va_end(arg);
	}
	for(Cnt=0; Cnt<Num; Cnt++){
		transmit(*pStr++);
	}
	
	//delete[] pStr;
	
	return 0;
}

int8_t Sci_t::println(const char *control, ...){
	int16_t Cnt;
	int16_t Num;
	
	//char *pStr = new char[SCI_MAX_STRING];
	char pStrCnst[SCI_MAX_STRING];
	char *pStr = pStrCnst;
	
	if(NULL==pStr){		// 確保できなければそのまま出しちゃう
		pStr = (char *)control;	
		Num = strlen(control);
	}else{
		va_list arg;
		va_start(arg, control);
		Num = vsprintf(pStr, control, arg);
		va_end(arg);
	}
	
	for(Cnt=0; Cnt<Num; Cnt++){
		transmit(*pStr++);
	}
	transmit('\n');
	
	//delete[] pStr;
	return 0;
}





/*********************
 送信割り込み処理
  TDRレジスタ空いてなくても書いちゃうので注意
**********************/
void Sci_t::isrTx(void){
	uint8_t Data;
	if(!fIsr){	// 処理中に割り込みとか来ても困るので！
		fIsr = true;
	//	if(this==&Sci5) PORTD.PODR.BYTE = 0x4a;
		
		if(!TxBuff->read(&Data)){	//バッファ読む。ちゃんと読めたらsetする
		
	//		if(this==&Sci5)	PORTD.PODR.BYTE = 0x40;
		
			
			if(fTxEndInterruptEnable && (TxBuff->isEmpty())){	// 送信完了割り込み有りで、ラス1送るので送信割り込みoff
				SCIreg->SCR.BIT.TIE = 0;
			}else{
				SCIreg->SCR.BIT.TIE = 1;
			}
				
			// データ送信
			fTdrEmpty = false;		// 送信レジスタ空いてるフラグクリア
			
			setSciTxData(Data);		// Transmit data
	//		if(this==&Sci5) PORTD.PODR.BYTE = Data;
			// 割り込み設定
			// 最後の送信だったら、送信完了割り込みを使う
			if(fTxEndInterruptEnable && (TxBuff->isEmpty())){	// 送信完了割り込み, ラス1送ったとこ
				SCIreg->SCR.BIT.TIE = 0;	// 送信バッファエンプティ割り込み停止
				//SCIreg->SSR.BIT.TEND = 0;	// 初め0になってるので、TEIEを1にした瞬間に割り込み入っちゃうのを阻止する。
				//							// ハードウェアマニュアル"35.11 割り込み要因"参照
											// -> read専用だったので、割り込み先でSSR.TENDを確認するように。
											// -> また、データ送信して(ここでTENDが0になるはず)から割り込み設定をすることでも回避
											
				SCIreg->SCR.BIT.TEIE = 1;	// 送信完了割り込みenable
	//		if(this==&Sci5) PORTD.PODR.BYTE = 0xE0;
			}else{
				SCIreg->SCR.BIT.TIE = 1;
				SCIreg->SCR.BIT.TEIE = 0;
	//		if(this==&Sci5) PORTD.PODR.BYTE = 0xE1;
			}
			
			
		}else{	// もうバッファ無し
	//		if(this==&Sci5) PORTD.PODR.BYTE = 0x41;
			
			fTdrEmpty = true;	// 送信レジスタ空いてるフラグセット
		}
		
		
		fIsr = false;
	}else{
	//	if(this==&Sci5) PORTD.PODR.BYTE = 0x4b;
	}
	
	//if(this==&Sci5) PORTD.PODR.BYTE = 0xFF;
	
	State = SCI_TX_TXI;
}

/*********************
 送信完了割り込み処理
**********************/
void Sci_t::isrTxEnd(void){
	
	//if(this==&Sci5) PORTD.PODR.BYTE = 0x42;
	
	if(SCIreg->SSR.BIT.TEND){
		if(TxBuff->isEmpty()){	// この処理までにバッファ入ってきてるか判定(始めの1つめの送信後はすぐ入ってきてる可能性あり)
			// バッファ入ってないです。
			fTdrEmpty = true;			// 送信レジスタ空いてるフラグクリア
			SCIreg->SCR.BIT.TEIE = 0;	// 送信完了割り込み無効
			
			if(NULL!=TxEndCallBackFunc){		// コールバック関数がattachされていたら
				TxEndCallBackFunc();	// コールバック関数を実行
			}
		}else{
			// すぐにバッファたまってた！
			SCIreg->SCR.BIT.TEIE = 0;	// 送信完了割り込み無効
			isrTx();				
		}
	}
	
	//if(this==&Sci5) PORTD.PODR.BYTE = 0xFF;
	State = SCI_TX_TEI;
}


/*********************
 受信完了割り込み処理
**********************/
void Sci_t::isrRx(void){
	//if(this==&Sci5) PORTD.PODR.BYTE = 0x43;
	
	RxBuff->add(getSciRxData());	// バッファに貯めこむ
		
	if(NULL!=RxCallBackFunc){		// コールバック関数がattachされていたら
		RxCallBackFunc(RxBuff);	// コールバック関数を実行
	}
	
	//if(this==&Sci5) PORTD.PODR.BYTE = 0xFF;
	State = SCI_RX_RXI;
}








// interrupt
#pragma section IntPRG
// tx
#ifndef TARGET_BOARD	// GR-SAKURA用プロジェクトの時は使えない
#pragma interrupt (Excep_SCI2_TXI2(vect=VECT(SCI2,TXI2), enable))
void Excep_SCI2_TXI2(void){
	setpsw_i();	//多重割り込み許可
	Sci2.isrTx();
}
#endif

#pragma interrupt (Excep_SCI5_TXI5(vect=VECT(SCI5,TXI5), enable))
void Excep_SCI5_TXI5(void)
{
	setpsw_i();	//多重割り込み許可
	// PORTD.PODR.BYTE = 0x4c;
	Sci5.isrTx();
	// PORTD.PODR.BYTE = 0x4d;
}

// 送信完了割り込み
#ifndef TARGET_BOARD	// GR-SAKURA用プロジェクトの時は使えない
#pragma interrupt (Excep_SCI2_TEI2(vect=VECT(SCI2,TEI2), enable))
void Excep_SCI2_TEI2(void)
{
	setpsw_i();	//多重割り込み許可
	Sci2.isrTxEnd();
}
#endif

#pragma interrupt (Excep_SCI5_TEI5(vect=VECT(SCI5,TEI5), enable))
void Excep_SCI5_TEI5(void)
{
	setpsw_i();	//多重割り込み許可
	Sci5.isrTxEnd();
}

//rx
#ifndef TARGET_BOARD	//
#pragma interrupt (Excep_SCI2_RXI2(vect=VECT(SCI2,RXI2), enable))
void Excep_SCI2_RXI2(void){
	setpsw_i();	//多重割り込み許可
	Sci2.isrRx();
}
#endif

#pragma interrupt (Excep_SCI5_RXI5(vect=VECT(SCI5,RXI5), enable))
void Excep_SCI5_RXI5(void){
	setpsw_i();	//多重割り込み許可
	// PORTD.PODR.BYTE = 0x4e;
	Sci5.isrRx();
	// PORTD.PODR.BYTE = 0x4f;
}

// TXI0, RXI0, RXI2はどっかで使われる(割り込みベクターの設定がある)っぽいので要確認
#pragma section 



