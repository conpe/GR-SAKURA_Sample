
#include "GR_Ex15_v3.h"



/****************************
拡張基板Ex15 メイン
****************************/
gr_ex::gr_ex(void){
	
	
}

gr_ex::~gr_ex(void){
	if(Sp) delete Sp;
	if(Motor0) delete Motor0;
	if(Motor1) delete Motor1;
	if(Motor2) delete Motor2;
	if(Motor3) delete Motor3;
	if(Enc0) delete Enc0;
	if(Enc1) delete Enc1;
}


void gr_ex::begin(void){
	
	
	// GPIO初期化
	setPinMode((pins)GREX_IO0, PIN_OUTPUT);
	setPinMode((pins)GREX_IO1, PIN_OUTPUT);
	setPinMode((pins)GREX_IO2, PIN_OUTPUT);
	setPinMode((pins)GREX_IO3, PIN_OUTPUT);
	setPinMode((pins)GREX_IO4, PIN_OUTPUT);
	setPinMode((pins)GREX_IO5, PIN_OUTPUT);
	setPinMode((pins)GREX_IO6, PIN_OUTPUT);
	setPinMode((pins)GREX_IO7, PIN_OUTPUT);
	
	
	// スピーカ初期化
	Sp = new gr_ex_sp();
	
	// モータ初期化
	Motor0 = new gr_ex_motor(MOTOR0);
	Motor1 = new gr_ex_motor(MOTOR1);
	Motor2 = new gr_ex_motor(MOTOR2);
	Motor3 = new gr_ex_motor(MOTOR3);
	
	// ロータリエンコーダ初期化
	Enc0 = new gr_ex_encoder(ENC0);
	Enc1 = new gr_ex_encoder(ENC1);
	
	if(NULL==Sp) __heap_chk_fail();
	if(NULL==Motor0) __heap_chk_fail();
	if(NULL==Motor1) __heap_chk_fail();
	if(NULL==Motor2) __heap_chk_fail();
	if(NULL==Motor3) __heap_chk_fail();
	if(NULL==Enc0) __heap_chk_fail();
	if(NULL==Enc1) __heap_chk_fail();
	
	// シリアルサーボ初期化
	
	
	
	// CAN初期化
	
	// シリアル初期化
	
	// I2C初期化 or MPU9250初期化
	
	
}


/*********************
 AD変換ポート初期化
概要：
 AD変換用ポート(IO8-15)をすべてAD変換用にして、連続スキャン開始する
 加算モードにして4回足した値がS12AD.ADDRxに入る。(左詰めモード。4回足してるので、左14ビットを使う)
引数：

**********************/
void gr_ex::startAD(void){
	SYSTEM.MSTPCRA.LONG &= ~(1 << 17);

	PORT4.PDR.BYTE = 0; // 全ポートを入力にする

	S12AD.ADCSR.BYTE = 0x4c;	// サンプリングクロックをPCLK(48MHz)に
								// 連続スキャンモード
	
	// 変換対象
	S12AD.ADANS0.BIT.ANS0 = 0xff;	// CH0-15 : on  // 0～7までしかないような気もするけど？
	S12AD.ADANS1.BIT.ANS1 = 0x00;	// CH16-20 : off
	
	// 加算処理
	S12AD.ADADS0.BIT.ADS0 = 0xff;	//　CH0-15 : on		// 加算モードにした場合、左14ビットが結果になる。
	S12AD.ADADS1.BIT.ADS1 = 0x00;	//  CH16-20 : off
	
	// 加算回数
	S12AD.ADADC.BIT.ADC = 0x03;	// 4回加算
	
	
	// AD変換開始
	S12AD.ADCSR.BIT.ADST = 1; // AD変換開始
}

// 8ビットで返す
uint16_t gr_ex::getAD8(grex_ad_ch AdCh){
	
	unsigned short *result = (unsigned short *)&S12AD.ADDR0;
	
	return (*(result+(int)AdCh))>>8;	// 加算モードなので左8ビット分を返す
}
// 10ビットで返す
uint16_t gr_ex::getAD10(grex_ad_ch AdCh){
	
	unsigned short *result = (unsigned short *)&S12AD.ADDR0;
	
	return (*(result+(int)AdCh))>>6;	// 加算モードなので左10ビット分を返す
}
// 12ビットで返す
uint16_t gr_ex::getAD12(grex_ad_ch AdCh){
	
	unsigned short *result = (unsigned short *)&S12AD.ADDR0;
	
	return (*(result+(int)AdCh))>>4;	// 加算モードなので左10ビット分を返す
}
// 12ビットで返す
uint16_t gr_ex::getAD(grex_ad_ch AdCh){
	
	unsigned short *result = (unsigned short *)&S12AD.ADDR0;
	
	return (*(result+(int)AdCh))>>4;	// 加算モードなので左12ビット分を返す
}

gr_ex GrEx;



/****************************
スピーカ
gr_ex_sp
	gr_ex.Sp

概要：
 PWMで音声再生
 バッファに用意した波形データを順次出力する
	
使用モジュール：
	MTU4	:	PWM生成 周期255(187.5kHz)
	TMR3	:	サンプリング周波数割り込み生成 

****************************/
gr_ex_sp::gr_ex_sp(void){
	SpBuff = new RingBuffer<uint8_t>(EXSP_BUFFSIZE);	// バッファサイズどうすっぺ。とりあえずデフォ。
	if(NULL==SpBuff) __heap_chk_fail();
	setupRegisterPwm();
}

gr_ex_sp::~gr_ex_sp(void){
	delete SpBuff;
}

// レジスタの初期化
//  MTU4をPWMモード1で。TGRDをバッファにしてMTIOC4A端子(PE2)から出力
void gr_ex_sp::setupRegisterPwm(void){
	
	// PWMの設定
	// PWMモード1
	
	// 書き込み許可
	SYSTEM.PRCR.WORD = 0xA503u;
	MTU.TRWER.BIT.RWE = 1;	// MTU関係レジスタ変更許可
	
	MSTP(MTU4) = 0;	// モジュール起動
	
	
	MTU.TSTR.BIT.CST4 = 0;
	
	MTU4.TCR.BIT.TPSC = 0;	// PCLK/1 -> 48MHz
	//MTU4.TCR.BIT.TPSC = 5;	// PCLK/1024 -> 46.875kHz -> 下で/255してるので 183Hz
	MTU4.TCR.BIT.CCLR = 1;	// TGRAのコンペアマッチでTCNTクリア
	
	MTU4.TMDR.BIT.BFA = 0;	// TGRA, Cは通常動作
	MTU4.TMDR.BIT.BFB = 1;	// TGRB, Dはバッファ動作
	
	MTU4.TMDR.BIT.MD = 2;	// PWMモード1

	MTU4.TBTM.BIT.TTSB = 1;	// TCNTクリア時にバッファからコピー
	
	MTU.TOER.BIT.OE4A = 1;	// 出力許可
	
	MTU4.TIORH.BIT.IOA = 6;	// High -> High
	MTU4.TIORH.BIT.IOB = 1;	// Low -> Low
	
	setPeriodCnt(255);	//周期	->	48MHz/255 = 187.5kHz
	//MTU4.TGRA = 255; 		//周期
	MTU4.TGRB = 127; 		//デューティー
	setDutyCnt(127); 		//デューティー
	//MTU4.TGRD = 127; 		//デューティー(バッファ ←ここにデューティー入れる。
	
	
	MTU.TSTR.BIT.CST4 = 1;	// カウントスタート
	
	
	// ポート出力設定
	MPC.PWPR.BYTE = 0x00u;
	MPC.PWPR.BYTE = 0x40u;
	SYSTEM.PRCR.WORD = 0xA503u;
	setPinMode(GREX_SPOUT, PIN_OUTPUT);	// output
	MPC.PE2PFS.BIT.PSEL = 1;			// MTIOC4A
	PORTE.PMR.BIT.B2 = 1;				// not GPIO
	
	
}

// Dutyをセット
//	タイマカウント値で指定
void gr_ex_sp::setDutyCnt(uint16_t DutyCnt){
	
	if(MTU4.TGRA==DutyCnt){	// duty最大の時はゼロのままになってしまうことがあるので、常にhighになるように+1する。
		MTU4.TGRD = DutyCnt+1;
	}else{
		MTU4.TGRD = DutyCnt;
	}
}

// 周期をセット
//	タイマカウント値で指定
void gr_ex_sp::setPeriodCnt(uint16_t PeriodCnt){
	_Freq = PCLK/(PeriodCnt);	// Hz
	
	MTU4.TGRA = PeriodCnt;
}


// 再生用の設定
//  SamplingFreqでの割り込みを設定
//  TMR3使用(コンペアマッチAで割り込み)
//   ->割り込みごとにbufferから読んでDutyをセットする。
void gr_ex_sp::setupPlay(uint16_t SamplingFreq){	
	uint8_t CntSetting = 2;		// 0:1/1, 1:1/2, 2:1/8, 3:32, 4:64
	uint8_t cnt = 255;
	
	
	SYSTEM.PRCR.WORD = 0xA503u;		// 書き込み許可
	MTU.TRWER.BIT.RWE = 1;	// MTU関係レジスタ変更許可
	MSTP(TMR3) = 0;					// モジュール起動
	
	TMR3.TCR.BYTE = 0x00;
	TMR3.TCR.BIT.CCLR = 1;			// コンペアマッチAでカウンタクリア
	TMR3.TCR.BIT.CMIEA = 1;			// コンペアマッチA割り込み許可(CMIAn)
	
	
	TMR3.TCSR.BYTE = 0x00;
	// カウンタ分周選択
	if(256 > PCLK/1/SamplingFreq){
		CntSetting = 0;
		cnt = PCLK/1/SamplingFreq;
	}else if(256 > PCLK/2/SamplingFreq){
		CntSetting = 1;
		cnt = PCLK/2/SamplingFreq;
	}else if(256 > PCLK/8/SamplingFreq){
		CntSetting = 2;
		cnt = PCLK/8/SamplingFreq;
	}else if(256 > PCLK/32/SamplingFreq){
		CntSetting = 3;
		cnt = PCLK/32/SamplingFreq;
	}else if(256 > PCLK/64/SamplingFreq){
		CntSetting = 4;
		cnt = PCLK/64/SamplingFreq;
	}else if(256 > PCLK/1024/SamplingFreq){
		CntSetting = 5;
		cnt = PCLK/1024/SamplingFreq;
	}else{
		CntSetting = 6;
		cnt = PCLK/8192/SamplingFreq;
	}
	
	
	// カウンタ
	//uint8_t cnt = PCLK/8/SamplingFreq;
	TMR3.TCNT = 0;
	TMR3.TCORA = cnt;		//コンペアマッチ用(使わない)
	TMR3.TCORB = cnt+1;		//コンペアマッチ用(使わない)
	
	// 割り込み許可
	IEN(TMR3,CMIA3) = 1;
	IPR(TMR3,CMIA3) = 6;
	
	// クロック設定
	TMR3.TCCR.BYTE = 0x00;
	TMR3.TCCR.BIT.CKS = CntSetting;	// クロック選択			PCLK/8
	TMR3.TCCR.BIT.CSS = 0x01;	// クロックソース選択 分周クロック
	
	// 動作止めるのはこれしかない？
	//TMR3.TCCR.BIT.CKS = 0x00;	// クロック選択			PCLK/8
	//TMR3.TCCR.BIT.CSS = 0x00;
	
	
}

void gr_ex_sp::stopPlay(void){	
	//TMR3.TCCR.BIT.CKS = 0x00;	// クロックなし
	//TMR3.TCCR.BIT.CSS = 0x00;
	
	// 止めようかと思ったけど、プッツンなるので止めないことに。
	
}

// バッファから順次デューティーに設定する
// バッファなければやらぬ。
void gr_ex_sp::isrSpPlay(void){
	/*
	// ノコギリ波テスト
	static uint8_t cnt;
	cnt++;	// 255で一周する	->	173Hz
	setDutyCnt(cnt);
	*/
	uint8_t dat;
	
	if(!SpBuff->read(&dat)){
		setDutyCnt(dat);
	}

}





/****************************
モーター
gr_ex_motor
	gr_ex.Motor0 (PWM非対応 未実装)
	gr_ex.Motor1 (PWM非対応 未実装)
	gr_ex.Motor2 (PWM対応)
	gr_ex.Motor3 (PWM対応)

使用モジュール
	TPU3, MTU3	:	PWM生成
		周期： TPU3 TGRC
		デューティ:
			//motor0:	TIOCB3 TGRB (PWMモード2)
			//motor1:	TIOCA3 TGRA (PWMモード2)
		回転方向, 停止制御
			GPIO x各2
			
		周期: MTU3 TGRA
		デューティ:
			motor2:	MTIOC3A TGRB (PWMモード1)
			motor3:	MTIOC3C TGRD (PWMモード1)
		回転方向, 停止制御
			GPIO x各2
****************************/
gr_ex_motor::gr_ex_motor(gr_ex_motor_num MotorNum){
	_Motor = MotorNum;	// モーターセット
	
	
	setRegister();		// レジスタ設定
	// カウンタ初期値
	setDir(FREE);
	setPeriodCnt(GREX_MOTOR_PWMCNT_1KHz);	//周期	->	48MHz/47999 = 1kHz
	switch(MotorNum){
	case MOTOR0:	// PWM非対応なので、デューティーはマックス扱い
	case MOTOR1:
		setDutyCnt(GREX_MOTOR_PWMCNT_1KHz); 		//デューティー
		break;
	case MOTOR2:	// 停止状態で開始
	case MOTOR3:
		setDutyCnt(0); 		//デューティー
		break;
	}
}

extern uint8_t motorset;
void gr_ex_motor::setRegister(void){
	
	
	switch(_Motor){
	case MOTOR0:
			
		// PWMの設定
		
		// 書き込み許可
		
		/*
		SYSTEM.PRCR.WORD = 0xA503u;
		
		MSTP(TPU3) = 0;	// モジュール起動
		TPUA.TSTR.BIT.CST3 = 0;
		TPU3.TCR.BIT.TPSC = 0;	// PCLK/1 -> 48MHz
		
		TPU3.TCR.BIT.CCLR = 5;	// TGRCレジスタのコンペアマッチでTCNTカウンタクリア
		TPU3.TMDR.BIT.MD = 3;	// PWMモード2

		TPU3.TIORH.BIT.IOB = 5;	// High -> Low
		
		TPUA.TSTR.BIT.CST3 = 1;	// カウントスタート
		
		// ポート出力設定
		MPC.PWPR.BYTE = 0x00u;
		MPC.PWPR.BYTE = 0x40u;
		SYSTEM.PRCR.WORD = 0xA503u;
		setPinMode(GREX_PIN_MOTOR0P, PIN_OUTPUT);
		MPC.P20PFS.BIT.PSEL = 3;			// TIOCB3
		PORT2.PMR.BIT.B0 = 1;				// not GPIO
		*/
		setPinMode(GREX_PIN_MOTOR0A, PIN_OUTPUT);
		PORT0.PMR.BIT.B5 = 0;				// GPIO
		setPinMode(GREX_PIN_MOTOR0B, PIN_OUTPUT);
		PORT0.PMR.BIT.B5 = 0;				// GPIO
		
		break;
	case MOTOR1:
			
		// PWMの設定
		
		/*
		// 書き込み許可
		SYSTEM.PRCR.WORD = 0xA503u;
		MSTP(TPU3) = 0;	// モジュール起動
		
		TPUA.TSTR.BIT.CST3 = 0;
		TPU3.TCR.BIT.TPSC = 0;	// PCLK/1 -> 48MHz
		
		TPU3.TCR.BIT.CCLR = 5;	// TGRCレジスタのコンペアマッチでTCNTカウンタクリア
		TPU3.TMDR.BIT.MD = 3;	// PWMモード2

		TPU3.TIORH.BIT.IOA = 5;	// High -> Low
		
		TPUA.TSTR.BIT.CST3 = 1;	// カウントスタート
		
		// ポート出力設定
		MPC.PWPR.BYTE = 0x00u;
		MPC.PWPR.BYTE = 0x40u;
		SYSTEM.PRCR.WORD = 0xA503u;
		setPinMode(GREX_PIN_MOTOR1P, PIN_OUTPUT);
		MPC.P21PFS.BIT.PSEL = 3;			// TIOCA3
		PORT2.PMR.BIT.B1 = 1;				// not GPIO
		*/
		setPinMode(GREX_PIN_MOTOR1A, PIN_OUTPUT);
		PORT1.PMR.BIT.B7 = 0;				// GPIO
		setPinMode(GREX_PIN_MOTOR1B, PIN_OUTPUT);
		PORT5.PMR.BIT.B1 = 0;				// GPIO
		
		break;
	case MOTOR2:
		
		// 書き込み許可
		SYSTEM.PRCR.WORD = 0xA503u;
		MSTP(MTU3) = 0;	// モジュール起動
		MTU.TRWER.BIT.RWE = 1;	// MTU関係レジスタ変更許可
		
		MTU.TSTR.BIT.CST3 = 0;
		MTU3.TCR.BIT.TPSC = 0;	// PCLK/1 -> 48MHz
		
		MTU3.TCR.BIT.CCLR = 1;	// TGRAのコンペアマッチでクリア
		MTU3.TMDR.BIT.MD = 2;	// PWMモード1
		
		MTU3.TMDR.BIT.BFA = 0;	// 非バッファモード

		MTU3.TIORH.BIT.IOA = 6;	// High -> High
		MTU3.TIORH.BIT.IOB = 5;	// High -> Low
		
		MTU.TSTR.BIT.CST3 = 1;	// カウントスタート
		
		// ポート出力設定
		MPC.PWPR.BYTE = 0x00u;
		MPC.PWPR.BYTE = 0x40u;
		SYSTEM.PRCR.WORD = 0xA503u;
		setPinMode(GREX_PIN_MOTOR2P, PIN_OUTPUT);
		MPC.PC1PFS.BIT.PSEL = 1;			// MTIOC3A
		PORTC.PMR.BIT.B1 = 1;				// not GPIO
		setPinMode(GREX_PIN_MOTOR2A, PIN_OUTPUT);
		PORT5.PMR.BIT.B4 = 0;				// GPIO
		setPinMode(GREX_PIN_MOTOR2B, PIN_OUTPUT);
		PORT5.PMR.BIT.B5 = 0;				// GPIO
		
		break;
	case MOTOR3:
		// 書き込み許可
		SYSTEM.PRCR.WORD = 0xA503u;
		MSTP(MTU3) = 0;	// モジュール起動
		MTU.TRWER.BIT.RWE = 1;	// MTU関係レジスタ変更許可
		
		MTU.TSTR.BIT.CST3 = 0;
		MTU3.TCR.BIT.TPSC = 0;	// PCLK/1 -> 48MHz
		
		MTU3.TCR.BIT.CCLR = 1;	// TGRAのコンペアマッチでクリア
		MTU3.TMDR.BIT.MD = 2;	// PWMモード1
		
		MTU3.TMDR.BIT.BFB = 0;	// 非バッファモード
	
		MTU3.TIORL.BIT.IOC = 6;	// High -> High
		MTU3.TIORL.BIT.IOD = 5;	// High -> Low
		
		MTU.TSTR.BIT.CST3 = 1;	// カウントスタート
		
		// ポート出力設定
		MPC.PWPR.BYTE = 0x00u;
		MPC.PWPR.BYTE = 0x40u;
		SYSTEM.PRCR.WORD = 0xA503u;
		setPinMode(GREX_PIN_MOTOR3P, PIN_OUTPUT);
		//PORTJ.ODR0.BIT.B6 = 0;	// オープドレインoff
		MPC.PJ3PFS.BIT.PSEL = 1;			// MTIOC3C
		PORTJ.PMR.BIT.B3 = 1;				// not GPIO
		setPinMode(GREX_PIN_MOTOR3A, PIN_OUTPUT);
		PORTC.PMR.BIT.B4 = 0;				// GPIO
		setPinMode(GREX_PIN_MOTOR3B, PIN_OUTPUT);
		PORTE.PMR.BIT.B0 = 0;				// GPIO
		
		break;
	
	}
	
}

void gr_ex_motor::setPeriodCnt(uint16_t PeriodCnt){
	
	PeriodCnt_ = PeriodCnt;
	
	switch(_Motor){
	case MOTOR0:
	case MOTOR1:
	//	TPU3.TGRC = PeriodCnt;
		break;
	case MOTOR2:
	case MOTOR3:
		MTU3.TGRA = PeriodCnt;
		MTU3.TGRC = PeriodCnt;
		break;
	}
}

void gr_ex_motor::setDutyCnt(uint16_t DutyCnt){
	
	switch(_Motor){
	case MOTOR0:
	case MOTOR1:
	//	if(TPU3.TGRC==DutyCnt){	// duty最大の時はゼロのままになってしまうことがあるので、常にhighになるように+1する。
	//		DutyCnt = TPU3.TGRC+1;
	//	}
		if(DutyCnt>0){
			DutyCnt = GREX_MOTOR_PWMCNT_1KHz;
			setDir(_Dir);
		}else{
			DutyCnt = 0;
			setDir(FREE);
		}
		break;
	case MOTOR2:
	case MOTOR3:
		if(MTU3.TGRA==DutyCnt){	// duty最大の時はゼロのままになってしまうことがあるので、常にhighになるように+1する。
			DutyCnt = MTU3.TGRA+1;
		}
		break;
	}
	
	switch(_Motor){
	case MOTOR0:
	//	TPU3.TGRB = DutyCnt;
		break;
	case MOTOR1:
	//	TPU3.TGRA = DutyCnt;
		break;
	case MOTOR2:
		MTU3.TGRB = DutyCnt;
		break;
	case MOTOR3:
		MTU3.TGRD = DutyCnt;
		break;
	}
}

void gr_ex_motor::setDir(gr_ex_motor_dir Dir){
	_Dir = Dir;
	bool_t OutA, OutB;
	
	switch(Dir){
	case CW:
		OutA = 1;
		OutB = 0;
		break;
	case CCW:
		OutA = 0;
		OutB = 1;
		break;
	case BRAKE:
		OutA = 1;
		OutB = 1;
		break;
	case FREE:
		OutA = 0;
		OutB = 0;
		break;
	}
	
	
	switch(_Motor){
	case MOTOR0:
		outPin(GREX_PIN_MOTOR0A, OutA);
		outPin(GREX_PIN_MOTOR0B, OutB);
		break;
	case MOTOR1:
		outPin(GREX_PIN_MOTOR1A, OutA);
		outPin(GREX_PIN_MOTOR1B, OutB);
		break;
	case MOTOR2:
		outPin(GREX_PIN_MOTOR2A, OutA);
		outPin(GREX_PIN_MOTOR2B, OutB);
		break;
	case MOTOR3:
		outPin(GREX_PIN_MOTOR3A, OutA);
		outPin(GREX_PIN_MOTOR3B, OutB);
		break;
	}
	
}

// +-255でデューティ設定
void gr_ex_motor::setDuty(int16_t Duty){

	if(Duty<0){
		setDir(CCW);
		Duty = -Duty;
	}else{
		setDir(CW);
	}
	
	if(Duty>255){
		Duty = 255;
	}
	
	
	setDutyCnt(Duty * PeriodCnt_ / 255);
	
}


/****************************
エンコーダ
gr_ex_encoder
	gr_ex.Enc0
	gr_ex.Enc1

使用モジュール
	MTU1, MTU2
		Enc0 : MTU1 / MTCLKA, MTCLKB
		Enc1 : MTU2 / MTCLKC, MTCLKD
****************************/
gr_ex_encoder::gr_ex_encoder(gr_ex_encoder_num EncNum){
	Enc_ = EncNum;	// エンコーダセット
	
	setRegister();		// レジスタ設定
	
	setCnt(65535/2);	// カウンタ初期値
	CntLast_ = 65535/2;	
	
	stopCnt();
}

void gr_ex_encoder::setRegister(void){
	
	switch(Enc_){
	case ENC0:
	
		// 書き込み許可
		SYSTEM.PRCR.WORD = 0xA503u;
		MSTP(MTU1) = 0;	// モジュール起動
		MTU.TSTR.BIT.CST1 = 0;
		
		
		MTU1.TMDR.BIT.MD = 4;		// 位相計数モード1(4逓倍)

		//MTU.TSTR.BIT.CST1 = 1;	// カウントスタート
		
		// ポート出力設定
		MPC.PWPR.BYTE = 0x00u;
		MPC.PWPR.BYTE = 0x40u;
		SYSTEM.PRCR.WORD = 0xA503u;
		setPinMode(GREX_PIN_ENC0A, PIN_INPUT_PULLUP);
		MPC.P24PFS.BIT.PSEL = 2;			// MTCLKA
		PORT2.PMR.BIT.B4 = 1;				// not GPIO
		setPinMode(GREX_PIN_ENC0B, PIN_INPUT_PULLUP);
		MPC.P25PFS.BIT.PSEL = 2;			// MTCLKB
		PORT2.PMR.BIT.B5 = 1;				// not GPIO
		
		// 割り込み許可
		MTU1.TIER.BIT.TCIEV = 1;	// オーバーフロー
		MTU1.TIER.BIT.TCIEU = 1;	// アンダーフロー
		EN(MTU1,TCIV1) = 1;			// オーバーフロー(ICU)
		EN(MTU1,TCIU1) = 1;			// アンダーフロー(ICU)
		IEN(ICU,GROUP1) = 1;
		IPR(ICU,GROUP1) = 1;
	
		break;
	case ENC1:
	
		// 書き込み許可
		SYSTEM.PRCR.WORD = 0xA503u;
		MSTP(MTU2) = 0;	// モジュール起動
		MTU.TSTR.BIT.CST2 = 0;
		
		MTU2.TMDR.BIT.MD = 4;		// 位相計数モード1(4逓倍)

		//MTU.TSTR.BIT.CST2 = 1;	// カウントスタート
		
		// ポート出力設定
		MPC.PWPR.BYTE = 0x00u;
		MPC.PWPR.BYTE = 0x40u;
		SYSTEM.PRCR.WORD = 0xA503u;
		setPinMode(GREX_PIN_ENC1A, PIN_INPUT_PULLUP);
		MPC.P22PFS.BIT.PSEL = 2;			// MTCLKC
		PORT2.PMR.BIT.B2 = 1;				// not GPIO
		setPinMode(GREX_PIN_ENC1B, PIN_INPUT_PULLUP);
		MPC.P23PFS.BIT.PSEL = 2;			// MTCLKD
		PORT2.PMR.BIT.B3 = 1;				// not GPIO
		
		// 割り込み許可
		MTU2.TIER.BIT.TCIEV = 1;	// オーバーフロー
		MTU2.TIER.BIT.TCIEU = 1;	// アンダーフロー
		EN(MTU2,TCIV2) = 1;			// オーバーフロー(ICU)
		EN(MTU2,TCIU2) = 1;			// アンダーフロー(ICU)
		IEN(ICU,GROUP2) = 1;
		IPR(ICU,GROUP2) = 1;
		
		break;
	}
	
	
}

void gr_ex_encoder::startCnt(void){
	switch(Enc_){
	case ENC0:
		MTU.TSTR.BIT.CST1 = 1;	// カウントスタート
		break;
	case ENC1:
		MTU.TSTR.BIT.CST2 = 1;	// カウントスタート
		break;
	}
}

void gr_ex_encoder::stopCnt(void){
	switch(Enc_){
	case ENC0:
		MTU.TSTR.BIT.CST1 = 0;	// カウントスタート
		break;
	case ENC1:
		MTU.TSTR.BIT.CST2 = 0;	// カウントスタート
		break;
	}
}


void gr_ex_encoder::setCnt(uint16_t Cnt){
	switch(Enc_){
	case ENC0:
		MTU1.TCNT = Cnt;
		break;
	case ENC1:
		MTU2.TCNT = Cnt;
		break;
	}
}

uint16_t gr_ex_encoder::getCnt(void){
	switch(Enc_){
	case ENC0:
		return MTU1.TCNT;
		break;
	case ENC1:
		return MTU2.TCNT;
		break;
	}
	
	return 0;
}

int16_t gr_ex_encoder::getCntDiff(void){
	uint16_t Cnt;
	int16_t Diff;	
	bool_t fUpCnt;	// カウント方向(アップカウントで1)
	
	switch(Enc_){
	case ENC0:
		Cnt = MTU1.TCNT;
		fUpCnt = MTU1.TSR.BIT.TCFD;
		break;
	case ENC1:
		Cnt = MTU2.TCNT;
		fUpCnt = MTU2.TSR.BIT.TCFD;
		break;
	}
	
	if(fUpCnt){
		if(fOverflow_){
			Diff = Cnt + (0xFFFF - CntLast_) + 1;
		}else{
			Diff = Cnt - CntLast_;
		}
	}else{
		if(fUnderflow_){
			Diff = (Cnt - 0xFFFF) + CntLast_ - 1;
		}else{
			Diff = Cnt - CntLast_;
		}
	}
	
	CntLast_ = Cnt;
	
	fOverflow_ = 0;
	fUnderflow_ = 0;
	
	return Diff;
}
















// 割り込み関数の設定
// スピーカ
#pragma interrupt (Excep_TMR3_CMIA3(vect=VECT(TMR3,CMIA3), enable))
void Excep_TMR3_CMIA3(void){
	
	GrEx.Sp->isrSpPlay();
	
}


// MTU0 オーバーフロー
// MTU1 オーバーフロー
//		アンダーフロー
#pragma interrupt (Excep_ICU_GROUP1(vect=VECT(ICU,GROUP1), enable))
void Excep_ICU_GROUP1(void){
	// オーバーフロー
	if(IS(MTU1,TCIV1)){			// 割り込みステータスフラグチェック
		CLR(MTU1,TCIV1) = 1;	// 割り込み要求クリア
		GrEx.Enc0->fOverflow_ = 1;	// 
	}
	// アンダーフロー
	if(IS(MTU1,TCIU1)){			// 割り込みステータスフラグチェック
		CLR(MTU1,TCIU1) = 1;	// 割り込み要求クリア
		GrEx.Enc0->fUnderflow_ = 1;	// 
	}
}

// MTU2 オーバーフロー
//		アンダーフロー
// MTU3 オーバーフロー
#pragma interrupt (Excep_ICU_GROUP2(vect=VECT(ICU,GROUP2), enable))
void Excep_ICU_GROUP2(void){
	// オーバーフロー
	if(IS(MTU2,TCIV2)){			// 割り込みステータスフラグチェック
		CLR(MTU2,TCIV2) = 1;	// 割り込み要求クリア
		GrEx.Enc1->fOverflow_ = 1;	// 
	}
	// アンダーフロー
	if(IS(MTU2,TCIU2)){			// 割り込みステータスフラグチェック
		CLR(MTU2,TCIU2) = 1;	// 割り込み要求クリア
		GrEx.Enc1->fUnderflow_ = 1;	// 
	}
}












