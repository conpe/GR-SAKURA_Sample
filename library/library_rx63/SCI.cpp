
#include "SCI.h"


// Sci module 

//絶対beginするんだぞっ

Sci_t Sci0(SCI_SCI0);
Sci_t Sci1(SCI_SCI1);
Sci_t Sci2(SCI_SCI2);
Sci_t Sci5(SCI_SCI5);

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
		if(NULL==TxBuff) __heap_chk_fail();
		if(NULL==TxBuff) return -1;
	}

	if(UseRx){
		if(NULL!=RxBuff){
			delete RxBuff;
		}
		RxBuff = new RingBuffer<uint8_t>(SciConfig.RxBuffSize);
		if(NULL==RxBuff) __heap_chk_fail();
		if(NULL==RxBuff) return -1;
	}
	
	
	initRegister();
	setPinModeSci();
	
	// フラグ初期化
	fEmptySciTxBuff = true;
	fTxBuffOvf = false;
	fRxBuffOvf = false;
	
	State = SCI_BEGIN;
	return 0;
}


void Sci_t::initRegister(void){
	
	SYSTEM.PRCR.WORD = 0xA503u;
	switch(SciConfig.SciModule){
	case SCI_SCI0:
		MSTP(SCI0) = 0;
		break;	
	case SCI_SCI1:
		MSTP(SCI1) = 0;
		break;	
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
	enableTR();	// 送受信許可
	//setSciTxData(0xFF);	// 1フレーム分の1を出力すると送信可能状態になる
	
	SCIreg->SCR.BIT.TEIE = 0;	// 送信完了割り込みdisable(ラス1送った時にenableする)
	SCIreg->SCR.BIT.TIE = 1;	// 送信割り込みenable
	SCIreg->SCR.BIT.RIE = 1;	// 受信割り込みenable
	
	enableInterrupts();
	
	SCIreg->SSR.BYTE = 0xC0; 	//エラーフラグクリア
	
}

void Sci_t::enableIntTxBuffEmpty(void){
	switch(SciConfig.SciModule){
	case SCI_SCI0:	// シリアル通信ははリアルタイム性なくていいかな
		IEN(SCI0,TXI0) = 1;
		IPR(SCI0,TXI0) = 7;
		break;	
	case SCI_SCI1:
		IEN(SCI1,TXI1) = 1;
		IPR(SCI1,TXI1) = 7;
		break;	
	case SCI_SCI2:
		IEN(SCI2,TXI2) = 1;
		IPR(SCI2,TXI2) = 7;
		break;	
	case SCI_SCI5:
		IEN(SCI5,TXI5) = 1;
		IPR(SCI5,TXI5) = 0x07;
		break;
	}
}

void Sci_t::disableIntTxBuffEmpty(void){
	switch(SciConfig.SciModule){
	case SCI_SCI0:	// シリアル通信ははリアルタイム性なくていいかな
		IEN(SCI0,TXI0) = 0;
		break;	
	case SCI_SCI1:
		IEN(SCI1,TXI1) = 0;
		break;	
	case SCI_SCI2:
		IEN(SCI2,TXI2) = 0;
		break;	
	case SCI_SCI5:
		IEN(SCI5,TXI5) = 0;
		break;
	}
}

void Sci_t::enableIntTxEnd(void){
	switch(SciConfig.SciModule){
	case SCI_SCI0:	// シリアル通信ははリアルタイム性なくていいかな
		IEN(SCI0,TEI0) = 1;
		IPR(SCI0,TEI0) = 2;
		break;	
	case SCI_SCI1:
		IEN(SCI1,TEI1) = 1;
		IPR(SCI1,TEI1) = 2;
		break;	
	case SCI_SCI2:
		IEN(SCI2,TEI2) = 1;
		IPR(SCI2,TEI2) = 2;
		break;	
	case SCI_SCI5:
		IEN(SCI5,TEI5) = 1;
		IPR(SCI5,TEI5) = 0x07;
		break;
	}
}

void Sci_t::enableIntRx(void){
	switch(SciConfig.SciModule){
	case SCI_SCI0:	// シリアル通信ははリアルタイム性なくていいかな
		IEN(SCI0,RXI0) = 1;
		IPR(SCI0,RXI0) = 7;
		break;	
	case SCI_SCI1:
		IEN(SCI1,RXI1) = 1;
		IPR(SCI1,RXI1) = 7;
		break;	
	case SCI_SCI2:
		IEN(SCI2,RXI2) = 1;
		IPR(SCI2,RXI2) = 7;
		break;	
	case SCI_SCI5:
		IEN(SCI5,RXI5) = 1;
		IPR(SCI5,RXI5) = 0x0F;
		break;
	}
}

bool_t Sci_t::isEnableIntRx(void){
	switch(SciConfig.SciModule){
	case SCI_SCI0:
		return IEN(SCI0,RXI0);	
	case SCI_SCI1:
		return IEN(SCI1,RXI1);
	case SCI_SCI2:
		return IEN(SCI2,RXI2);
	case SCI_SCI5:
		return IEN(SCI5,RXI5);
	}
	
	return 0;
}

void Sci_t::enableInterrupts(void){
	enableIntTxBuffEmpty();
	enableIntTxEnd();
	enableIntRx();
}

void Sci_t::setPinModeSci(void){
	
	MPC.PWPR.BYTE = 0x00u;
	MPC.PWPR.BYTE = 0x40u;
	SYSTEM.PRCR.WORD = 0xA503u;
	
	switch(SciConfig.SciModule){
	case SCI_SCI0:
		if(SciConfig.TxEnable){
			setPinMode(P20, PIN_OUTPUT);	// TX output
			MPC.P20PFS.BIT.PSEL = 0x0A;		// TXD2
			PORT2.PMR.BIT.B0 = 1;			// not GPIO	
		}
		if(SciConfig.RxEnable){
			setPinMode(P21, PIN_INPUT_PULLUP);	// RX input pullup
			MPC.P21PFS.BIT.PSEL = 0x0A;		// RXD2 
			PORT2.PMR.BIT.B1 = 1;			// not GPIO
		}
		break;	
	case SCI_SCI1:
		if(SciConfig.TxEnable){
			setPinMode(P26, PIN_OUTPUT);	// TX output
			MPC.P26PFS.BIT.PSEL = 0x0A;		// TXD1
			PORT2.PMR.BIT.B6 = 1;			// not GPIO	
		}
		if(SciConfig.RxEnable){
			setPinMode(P30, PIN_INPUT_PULLUP);	// RX input pullup
			MPC.P30PFS.BIT.PSEL = 0x0A;		// RXD2 
			PORT3.PMR.BIT.B0 = 1;			// not GPIO
		}
		break;	
	case SCI_SCI2:
		if(SciConfig.TxEnable){
			setPinMode(P50, PIN_OUTPUT);	// TX output
			MPC.P50PFS.BIT.PSEL = 0x0A;		// TXD2
			PORT5.PMR.BIT.B0 = 1;			// not GPIO	
		}
		if(SciConfig.RxEnable){
			setPinMode(P52, PIN_INPUT_PULLUP);	// RX input pullup
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
			setPinMode(PC2, PIN_INPUT_PULLUP);	// RX input pullup
			MPC.PC2PFS.BIT.PSEL = 0x0A;		// RXD5 
			PORTC.PMR.BIT.B2 = 1;			// not GPIO	
		}
		break;	
	}
	
}


Sci_err Sci_t::transmit(uint8_t TrData){
	int8_t ack = SCI_OK;
	
	ack = TxBuff->add(TrData);
	if(BUFFER_NG == ack){
		fTxBuffOvf = true;
		return SCI_TX_ERR;
	}else{
		//if(isIdle()){	// TDRレジスタ開いてる もしくは モジュールがアイドル状態なら送る(何かの拍子にフラグ更新されなかった疑惑)
		if(fTdrEmpty){
			isrTx();	// Transmit one time
		}
		return SCI_OK;
	}
	
	return SCI_OK;
}

Sci_err Sci_t::receive(uint8_t *RcData){
	// フレーミングエラー
	if(isFramingErr()){
		clearFramingErr();
		RxBuff->clear();
		return SCI_RX_ERR;
	}
	// オーバーランエラー
	if(isOverrun()){
		clearOverrun();
		RxBuff->clear();
		return SCI_RX_ERR;
	}
	
	if(BUFFER_READ_OK == RxBuff->read(RcData)){
		return SCI_OK;
	}
	return SCI_NODATA;
}

uint8_t Sci_t::receive(void){
	uint8_t RcData = 0xff;
	
	receive(&RcData);
	
	return RcData;
}

void Sci_t::clearRxBuff(void){
	RxBuff->clear();
}


Sci_err Sci_t::print(char ch){
	return transmit(ch);
}


Sci_err Sci_t::print(int num){
	Sci_err ack = SCI_OK;
	int16_t Cnt;
	int16_t Num;
	
	char pStrCnst[SCI_MAX_STRING];
	char *pStr = pStrCnst;
	//char *pStr = new char[SCI_MAX_STRING];
	if(NULL==pStr){		// 確保できなかった
		return SCI_MEM_ERR;
	}
	
	Num = sprintf(pStr, "%d", num);
	
	for(Cnt=0; Cnt<Num; Cnt++){
		ack = transmit(*pStr++);
	}
	
	//delete[] pStr;
	return ack;
}
	
Sci_err Sci_t::print(const char *control, ...){
	Sci_err ack = SCI_OK;
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
		ack = transmit(*pStr++);
	}
	
	//delete[] pStr;
	
	return ack;
}

Sci_err Sci_t::println(const char *control, ...){
	Sci_err ack = SCI_OK;
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
		ack = transmit(*pStr++);
	}
	transmit('\r');
	transmit('\n');
	
	//delete[] pStr;
	return ack;
}





/*********************
 送信割り込み処理
  TDRレジスタ空いてなくても書いちゃうので注意
**********************/
void Sci_t::isrTx(void){
	uint8_t Data;
	
	State = SCI_TX_TXI;
		fTdrEmpty = true;	// この関数に入ったってことはTDRはempty
	
	if(!fIsr){	// 処理中に割り込みとか来ても困るので！
		fIsr = true;
		
		if(!TxBuff->read(&Data)){	//バッファ読む。バッファあったらsetする
			// データ送信
			fTdrEmpty = false;		// 送信レジスタ空いてるフラグクリア	// これtrueに出来る？->次にTEI入ったときにtrue
			setSciTxData(Data);		// Transmit data
			
			State = SCI_TX_START;
			
			if(TxBuff->isEmpty()){	// ラス1送ったとこ
				SCIreg->SCR.BIT.TEIE = 1;	// 送信完了割り込みenable
			}else{
				SCIreg->SCR.BIT.TEIE = 0;	// 送信完了割り込みdisable	
			}
		}
		
		fIsr = false;
	}else{
		
	}
	
}

/*********************
 送信完了割り込み処理
**********************/
void Sci_t::isrTxEnd(void){
	
	//	if(TxBuff->isEmpty()){	// この処理までにバッファ入ってきてるか判定(始めの1つめの送信後はすぐ入ってきてる可能性あり)
			// バッファにもう無いので送信終了
			
			fTdrEmpty = true;		// 送信レジスタ空いてるフラグセット
			SCIreg->SCR.BIT.TEIE = 0;	// 送信完了割り込み無効
			
			if(NULL!=TxEndCallBackFunc){	// コールバック関数がattachされていたら
				TxEndCallBackFunc();	// コールバック関数を実行
			}
			
	//	}else{
			// すぐにバッファたまってた！(複数送信の一発目はここの処理に入る)
	//		isrTx();
	//	}
	
	
	State = SCI_TX_TEI;
}


/*********************
 受信完了割り込み処理
**********************/
void Sci_t::isrRx(void){
	
	State = SCI_RX_RXI;
	
	
	fRxBuffOvf = RxBuff->add(getSciRxData());	// バッファに貯めこむ
	
	// フレーミングエラー
	if(isFramingErr()){
		clearFramingErr();
		RxBuff->clear();
		return;
	}
	// オーバーランエラー
	if(isOverrun()){
		clearOverrun();
		RxBuff->clear();
		return;
	}
	
	if(NULL!=RxCallBackFunc){	// コールバック関数がattachされていたら
		RxCallBackFunc(RxBuff);	// コールバック関数を実行
	}
	
}








#pragma section IntPRG
// interrupt
/* SCI0 */
#ifndef TARGET_BOARD	// GR-SAKURA用プロジェクトの時は使えない
#pragma interrupt (Excep_SCI0_TXI0(vect=VECT(SCI0,TXI0), enable))
void Excep_SCI0_TXI0(void){
	setpsw_i();	//多重割り込み許可
	Sci0.isrTx();
}
#pragma interrupt (Excep_SCI0_TEI0(vect=VECT(SCI0,TEI0), enable))
void Excep_SCI0_TEI0(void){
	setpsw_i();	//多重割り込み許可
	Sci0.isrTxEnd();
}

#pragma interrupt (Excep_SCI0_RXI0(vect=VECT(SCI0,RXI0), enable))
void Excep_SCI0_RXI0(void){
	setpsw_i();	//多重割り込み許可
	Sci0.isrRx();
}
#endif


/* SCI1 */
#pragma interrupt (Excep_SCI1_TXI1(vect=VECT(SCI1,TXI1), enable))
void Excep_SCI1_TXI1(void){
	setpsw_i();	//多重割り込み許可
	Sci1.isrTx();
	}
#pragma interrupt (Excep_SCI1_TEI1(vect=VECT(SCI1,TEI1), enable))
void Excep_SCI1_TEI1(void){
	setpsw_i();	//多重割り込み許可
	Sci1.isrTxEnd();
}
#pragma interrupt (Excep_SCI1_RXI1(vect=VECT(SCI1,RXI1), enable))
void Excep_SCI1_RXI1(void){
	setpsw_i();	//多重割り込み許可
	Sci1.isrRx();
}


/* SCI2 */
#ifndef TARGET_BOARD	// GR-SAKURA用プロジェクトの時は使えない
#pragma interrupt (Excep_SCI2_TXI2(vect=VECT(SCI2,TXI2), enable))
void Excep_SCI2_TXI2(void){
	setpsw_i();	//多重割り込み許可
	Sci2.isrTx();
}
#pragma interrupt (Excep_SCI2_TEI2(vect=VECT(SCI2,TEI2), enable))
void Excep_SCI2_TEI2(void){
	setpsw_i();	//多重割り込み許可
	Sci2.isrTxEnd();
}
#pragma interrupt (Excep_SCI2_RXI2(vect=VECT(SCI2,RXI2), enable))
void Excep_SCI2_RXI2(void){
	setpsw_i();	//多重割り込み許可
	Sci2.isrRx();
}
#endif

/* SCI5 */
#pragma interrupt (Excep_SCI5_TXI5(vect=VECT(SCI5,TXI5), enable))
void Excep_SCI5_TXI5(void){
	setpsw_i();	//多重割り込み許可
	Sci5.isrTx();
	}
#pragma interrupt (Excep_SCI5_TEI5(vect=VECT(SCI5,TEI5), enable))
void Excep_SCI5_TEI5(void){
	setpsw_i();	//多重割り込み許可
	Sci5.isrTxEnd();
}
#pragma interrupt (Excep_SCI5_RXI5(vect=VECT(SCI5,RXI5), enable))
void Excep_SCI5_RXI5(void){
	setpsw_i();	//多重割り込み許可
	Sci5.isrRx();
}

// TXI0, RXI0, RXI2はどっかで使われる(割り込みベクターの設定がある)っぽいので要確認
#pragma section 



