/**************************************************
 知能ロボコン2016 上部制御
概要：
 取得ボール、アーム位置管理
**************************************************/

#include "inrof2016_upper_ctrl.h"

inrof2016_upper_ctrl InrofUpper;

// ボール置き場のエンコーダカウント値
const uint8_t inrof2016_upper_ctrl::BallStockEnc[10] = {0x05,0x15,0x25,0x34,0x46,0x56,0x67,0x78,0x87,0x97};


// M系列エンコーダカウント値
const uint8_t inrof2016_upper_ctrl::MseqArray[] = {
/* +00h */	0x9B, 0xCD, 0xE6, 0xF3, 0x79, 0x3C, 0x1E, 0x8F, 0xC7, 0x63, 0xB1, 0x58, 0xAC, 0xD6, 0xEB, 0x75, 
/* +01h */	0x3A, 0x9D, 0x4E, 0x27, 0x13, 0x09, 0x84, 0xC2, 0xE1, 0xF0, 0x78, 0xBC, 0xDE, 0xEF, 0x77, 0xBB, 
/* +02h */	0xDD, 0xEE, 0xF7, 0x7B, 0xBD, 0x5E, 0x2F, 0x17, 0x0B, 0x05, 0x82, 0x41, 0x20, 0x10, 0x08, 0x04, 
/* +03h */	0x02, 0x81, 0x40, 0xA0, 0xD0, 0x68, 0xB4, 0xDA, 0xED, 0xF6, 0xFB, 0x7D, 0x3E, 0x9F, 0xCF, 0x67, 
/* +04h */	0xB3, 0xD9, 0xEC, 0x76, 0x3B, 0x1D, 0x8E, 0x47, 0xA3, 0xD1, 0xE8, 0x74, 0xBA, 0x5D, 0x2E, 0x97, 
/* +05h */	0xCB, 0x65, 0x32, 0x99, 0x4C, 0xA6, 0x53, 0xA9, 0x54, 0xAA, 0x55, 0x2A, 0x95, 0x4A, 0x25, 0x92, 
/* +06h */	0x49, 0x24, 0x12, 0x89, 0x44, 0xA2, 0x51, 0x28, 0x14, 0x0A, 0x85, 0x42, 0x21, 0x90, 0xC8, 0x64, 
/* +07h */	0xB2, 0x59, 0x2C, 0x16, 0x8B, 0xC5, 0xE2, 0xF1, 0xF8, 0x7C, 0xBE, 0x5F, 0xAF, 0xD7, 0x6B, 0xB5, 
/* +08h */	0x5A, 0x2D, 0x96, 0x4B, 0xA5, 0x52, 0x29, 0x94, 0xCA, 0xE5, 0xF2, 0xF9, 0xFC, 0x7E, 0x3F, 0x1F, 
/* +09h */	0x0F, 0x07, 0x03, 0x01, 0x80, 0xC0, 0x60, 0xB0, 0xD8, 0x6C, 0xB6, 0x5B, 0xAD, 0x56, 0x2B, 0x15, 
/* +0Ah */	0x8A, 0x45, 0x22, 0x91, 0x48, 0xA4, 0xD2, 0xE9, 0xF4, 0x7A, 0x3D, 0x9E, 0x4F, 0xA7, 0xD3, 0x69, 
/* +0Bh */	0x34, 0x1A, 0x8D, 0x46, 0x23, 0x11, 0x88, 0xC4, 0x62, 0x31, 0x98, 0xCC, 0x66, 0x33, 0x19, 0x8C, 
/* +0Ch */	0xC6, 0xE3, 0x71, 0x38, 0x1C, 0x0E, 0x87, 0xC3, 0x61, 0x30, 0x18, 0x0C, 0x06, 0x83, 0xC1, 0xE0, 
/* +0Dh */	0x70, 0xB8, 0xDC, 0x6E, 0x37, 0x1B, 0x0D, 0x86, 0x43, 0xA1, 0x50, 0xA8, 0xD4, 0x6A, 0x35, 0x9A, 
/* +0Eh */	0x4D, 0x26, 0x93, 0xC9, 0xE4, 0x72, 0x39, 0x9C, 0xCE, 0xE7, 0x73, 0xB9, 0x5C, 0xAE, 0x57, 0xAB, 
/* +0Fh */	0xD5, 0xEA, 0xF5, 0xFA, 0xFD, 0xFE, 0xFF, 0x7F};
	



// 20msec
void inrof2016_upper_ctrl::task0(void){


	// 入力
	
	
	/* 制御 */
	// 状態制御
	if(isEnableOut()){
		ctrlState();
	}
	
	
	/* 出力 */
	// アームのモーターを制御
	ctrlArmMotor();

	
	
	/* 次回用通信 */
	
	// アームエンコーダ取得要求
	reqArmEnc();
	// サーボデータ更新
	ServoGrab->updatePresentPosition();	// 角度
	ServoPitch->updatePresentPosition();	// x10[deg]
	ServoGrab->updatePresentSpeed();	// 速度
	ServoPitch->updatePresentSpeed();	// int16_t [deg/sec]
	ServoGrab->updatePresentCurrent();	// 電流
	ServoPitch->updatePresentCurrent();	// uint16_t[mA]
}

void inrof2016_upper_ctrl::task1(void){	// 200msec
	
	ColorSens->updateBallColor();	// ボール色判定
	
	
}


void inrof2016_upper_ctrl::begin(void){

	// アームモーター
	ArmMotor = GrEx.Motor3;
	ArmMotor->setPeriodCnt(GREX_MOTOR_PWMCNT_10KHz);
	//pid_ctrlArmMotor = new ctrl_pid(SAMPLETIME_CTRL_S, 50, 4, 2, -255, 255, -511, 511);
	pid_ctrlArmMotor = new ctrl_pid(SAMPLETIME_CTRL_S, 100, 0, 2, -255, 255, -511, 511);
	ratelimiter_ctrlArmMotor = new ctrl_ratelimiter_abs(SAMPLETIME_CTRL_S, 1000, 2000);	// 減速は速く
	
	// I2C
	
	// シリアルサーボ
	// SCI5
	ServoGrab = new futaba_cmd_servo(1, &Sci5, 115200);
	ServoPitch = new futaba_cmd_servo(2, &Sci5, 115200);
	
	// カラーセンサ
	ColorSens = new inrof2016_color_sens_t(&I2C0, ARM_I2C_ADDRESS);	
	
	ArmI2C = &I2C0;		// これの初期化どっかでやらなきゃね。カラーセンサのbeginでやるからいいんだけど。
	
	// 制御用
	// アームエンコーダ速度計算用ローパス
	CtrlArmEncSpdAverage = new ctrl_move_average(ARM_ENC_SPD_AVERAGE_NUM);
	
	
// 初期化
	//initialize(); //-> Inrofから呼ぶ
	
}

void inrof2016_upper_ctrl::initialize(void){
	
	// シーケンス処理リセット
	SeqNum = 0;
	pCtrlSeqFunc = NULL;
	State = UPPER_STATE_IDLE;
	
	// 保持ボール初期化
	for(uint8_t i;i<NUM_BALL_STOCK;i++){
		BallStock[i] = INROF_NONE;
	}
	// 自由ボールセット
	BallStock[2] = INROF_YELLOW;
	//BallStock[3] = INROF_BLUE;
	
	
	
	// アームモーター
	ArmMotor->setDir(CCW);
	ArmMotor->setDuty(0);
	// サーボ
	ServoGrab->begin();
	ServoPitch->begin();
	ServoGrab->setLimit(SERVOPOS_MAX_G, SERVOPOS_MIN_G);
	ServoPitch->setLimit(SERVOPOS_MAX_P, SERVOPOS_MIN_P);
	
	// サーボon
	ServoGrab->enableTorque(CMDSV_TORQUE_ON);
	ServoPitch->enableTorque(CMDSV_TORQUE_ON);
	ServoGrab->setMaxTorque(100);
	ServoPitch->setMaxTorque(100);
	ServoGrab->setGoalTime(SERVO_GOALTIME_NORMAL);
	ServoPitch->setGoalTime(SERVO_GOALTIME_NORMAL);
	//ServoGrab->setGoalPosition(SERVOPOS_RUN_G);
	//ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
	ServoGrab->setGoalPosition(SERVOPOS_BALLUP2_G);
	ServoPitch->setGoalPosition(SERVOPOS_BALLUP2_P);
	
	//カラーセンサ
	ColorSens->begin();
	ColorSens->wakeup();	// モジュール起こす
	ColorSens->startMeasurement();	// 計測開始
	// debug.
	//ColorSens->enableMeasure();	// 計測開始
	
	
	// アームエンコーダ
	//ArmEncModeMseqIndex();	// M系列エンコードモードへ
	ArmEncModeRaw();	// こっちでエンコードモードへ
	ArmEnc = 0xFE;	// 未初期化状態
	ArmEncCorrect = ArmEnc;
	
	DutyTgt = 0;
	BallPosTgt = 4;
	EncCntTgt = 0x4B;
	
	CtrlMode = MODE_DUTY;
	
	// 動作許可
	enableAction();
}


// 状態制御
int8_t inrof2016_upper_ctrl::ctrlState(void){
	int8_t ret;
	
	if(NULL != pCtrlSeqFunc){
		
		ret = (this->*pCtrlSeqFunc)();
		
	}
	
	return ret;
}



// シーケンス処理関数セット
int8_t inrof2016_upper_ctrl::attachCtrlSeqFunc(int8_t (inrof2016_upper_ctrl::*pCtrlSeqFunc)(void)){
	
	SeqNum = 0;
	SeqNumLast = 0xFF;
	this->pCtrlSeqFunc = pCtrlSeqFunc;
	
	return 0;
};



/*********************
ボール捨てる準備要求
概要：
	指定した色のボールを掴んで、指定した角度まで持っていく
	
	移動時間をムダにしないため、目標角度から遠い方においてあるボールを選択する。
いじる変数：
	SeqArmEncObj
	SeqBallStockObj
	CtrlSeqFunc
	State
**********************/
int8_t inrof2016_upper_ctrl::reqBallTrashPre(inrof_color Col, float AngleRad){
	
	if((UPPER_STATE_IDLE == State) || (UPPER_STATE_ARM_TO_CENTER==State)){
		
		// 向かう角度設定(ボールを置くエンコード値)
		this->SeqArmEncObj = Rad2Enc(AngleRad);		// 目標エンコーダ位置
		
		// 何番目のボールを取るかを決定
		// 目標地点から遠いやつから順にとるよ
		
		// 0に近いやつを探す
		int8_t i;
		uint8_t NearCw = 0xFF;
		uint8_t NearCcw = 0xFF;
		for(i=0;i<NUM_BALL_STOCK;i++){	// けんさくけんさくぅ
			if(Col == BallStock[i]){	// 欲しい色のボールだったら
				NearCw = (uint8_t)i;
			}
		}
		for(i=NUM_BALL_STOCK-1;i>=0;i--){	// けんさくけんさくぅ
			if(Col == BallStock[i]){	// 欲しい色のボールだったら
				NearCcw = i;
			}
		}
		if((0xFF==NearCw) || (0xFF==NearCcw)){	// ボールなかったら
			State = UPPER_STATE_IDLE;
			return -1;			// 戻る
		}
		// 向かう場所から遠い方を目標ボール置き場に設定
		if( abs((int16_t)this->SeqArmEncObj - (int16_t)BallStockEnc[NearCw]) > abs((int16_t)this->SeqArmEncObj - (int16_t)BallStockEnc[NearCcw]) ){
			// CW側のほうが遠いのでそれを取ります。
			this->SeqBallStockObj = NearCw;		// 目標ボール置き場
		}else{
			this->SeqBallStockObj = NearCcw;	// 目標ボール置き場
		}
		
		
		
		// 関数登録
		attachCtrlSeqFunc(ctrlSeqBallTrashPre);
		// 状態更新
		State = UPPER_STATE_BALL_TRASH_PRE;
#ifdef DEBUGSCI0_UPPER
Sci0.print("reqBallTrashPre. Ang%f, BallPos%d, Col%d\r\n", AngleRad, this->SeqBallStockObj, Col);
#endif
		return 0;
	}else{
#ifdef DEBUGSCI0_UPPER
Sci0.print("reqBallTrashPre. n\r\n");
#endif
		return -1;
	}
}


// アームシーケンス
// ボール捨てる準備
int8_t inrof2016_upper_ctrl::ctrlSeqBallTrashPre(void){
	bool_t SeqInit = false;
	
	if(SeqNum!=SeqNumLast){	// 初回処理
		SeqInit = true;
	}
	SeqNumLast = SeqNum;
	
	switch(SeqNum){
	case 0:	// サーボを移動位置へ
		/* 初回処理 */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			// サーボを移動位置へ
			ServoGrab->setGoalTime(SERVO_GOALTIME_NORMAL);
			ServoPitch->setGoalTime(SERVO_GOALTIME_NORMAL);
			ServoGrab->setGoalPosition(SERVOPOS_BALLTRASH0_G);
			ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+2000<getTime_ms())){	// サーボ動作完了
			SeqNum ++;
		}
		break;
	
	case 1:	// ボール取りに向かう
		/* 初回処理 */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			// 目標位置設定
			moveArmBallPos(this->SeqBallStockObj);	// ボール取りに行くボール置き場を設定
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. Move\r\n");
#endif
		}
		
		/* 通常時処理 */
		/* do nothing (wait for ctrlArmMotor()) */
		
		
		/* 遷移処理 */
		if(isFinishCtrlArmMove() || (SeqStartTime_ms+10000<getTime_ms())){	// ボールの位置まで来た
			SeqNum ++;
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. MoveDone\r\n");
#endif
		}
		break;
	case 2:	// アーム下げる
		/* 初回処理 */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			ServoGrab->setGoalPosition(SERVOPOS_BALLTRASH0_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLTRASH0_P);
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. ArmDown\r\n");
#endif
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+3000<getTime_ms())){	// アーム下がった
#ifdef DEBUGSCI0_UPPER
Sci0.print("SCg%d,p%d\r\n", ServoGrab->getPresentCurrent(), ServoPitch->getPresentCurrent());
#endif
			SeqNum ++;
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. ArmDownDone\r\n");
#endif
		}
		break;
	case 3:	// ボール掴む
		/* 初回処理 */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			ServoGrab->setGoalPosition(SERVOPOS_BALLTRASH1_G);
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. Grab\r\n");
#endif
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+2000<getTime_ms())){	// アーム掴んだ
#ifdef DEBUGSCI0_UPPER
Sci0.print("SCg%d,p%d\r\n", ServoGrab->getPresentCurrent(), ServoPitch->getPresentCurrent());
#endif
			SeqNum ++;
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. GrabDone\r\n");
#endif
		}
		break;
	case 4:	// ボール持ち上げる
		/* 初回処理 */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. BallUp\r\n");
#endif
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+2000<getTime_ms())){	// ボール持ち上がった
			SeqNum ++;
			BallStock[this->SeqBallStockObj] = INROF_NONE;	// もうそこにボールはないの。
#ifdef DEBUGSCI0_UPPER
Sci0.print("SCg%d,p%d\r\n", ServoGrab->getPresentCurrent(), ServoPitch->getPresentCurrent());
Sci0.print("ctrlSeqBallTrashPre. BallUpDone\r\n");
#endif
		}
		break;
	case 5:	// 捨てる位置まで移動
		/* 初回処理 */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			// 目標位置設定
			moveArmEncCnt(this->SeqArmEncObj);	// ボール捨てる位置を指定
#ifdef DEBUGSCI0_UPPER
Sci0.print("ctrlSeqBallTrashPre. moveTrash\r\n");
#endif
		}
		
		/* 通常時処理 */
		/* wait for ctrlArmMotor() */
		
		
// ここでボール掴んでないじゃん！！
// てなったらもう一度取りに行く
// 3回くらいやってダメなら諦める。

		/* 遷移処理 */
		if(isFinishCtrlArmMove() || (SeqStartTime_ms+10000<getTime_ms())){	// 捨てる位置まで来た
#ifdef DEBUGSCI0_UPPER
Sci0.print("SCg%d,p%d\r\n", ServoGrab->getPresentCurrent(), ServoPitch->getPresentCurrent());
#endif
if(State != UPPER_STATE_IDLE){Sci0.print("ctrlSeqBallTrashPre. moveTrashDone\r\n");}
			State = UPPER_STATE_IDLE;
			dettachCtrlSeqFunc();	// 処理無し
		}
		break;
	default:
		/* なにかおかしいんだけど */
		return -1;
	}
	
	return 0;
}
	
	

/*********************
ボール捨てる要求
概要：
	アーム倒してボールぽいっ
いじる変数：
	CtrlSeqFunc
	State
**********************/
int8_t inrof2016_upper_ctrl::reqBallTrash(void){
	
	if((UPPER_STATE_IDLE == State) || (UPPER_STATE_ARM_TO_CENTER==State)){
		
		// 関数登録
		attachCtrlSeqFunc(ctrlSeqBallTrash);
		// 状態更新
		State = UPPER_STATE_BALL_TRASH;
	
		return 0;
	}else{
		return -1;
	}
}

// アームシーケンス
// ボール捨てる
int8_t inrof2016_upper_ctrl::ctrlSeqBallTrash(void){
	bool_t SeqInit = false;
	
	if(SeqNum!=SeqNumLast){	// 初回処理
		SeqInit = true;
	}
	SeqNumLast = SeqNum;
	
	switch(SeqNum){
	case 0:	// サーボを捨てる位置へ
		/* 初回処理 */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			// アームを捨てる位置へ立ち上げる
			ServoGrab->setGoalPosition(SERVOPOS_BALLTRASH3_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLTRASH3_P);
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if((isFinishMoveServo() && (SeqStartTime_ms+1500<getTime_ms())) || (SeqStartTime_ms+2000<getTime_ms())){	// サーボ動作完了
			SeqNum ++;
		}
		break;
	case 1:	// アーム戻す
		/* 初回処理 */
		if(SeqInit){
			// 指放す
			ServoGrab->setGoalTime(1000);
			ServoGrab->setGoalPosition(SERVOPOS_RUN_G);
			ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
			// 開始時刻覚える
			SeqStartTime_ms = getTime_ms();
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if((isFinishMoveServo() && (SeqStartTime_ms+1500<getTime_ms())) || (SeqStartTime_ms+10000<getTime_ms())){	// サーボ動作完了 && ちょい待ち
			if(State != UPPER_STATE_IDLE){Sci0.print("ctrlSeqBallTrash. TrashDone\r\n");}
			//State = UPPER_STATE_BALL_TRASH_DONE;
			
			
			ServoGrab->setGoalTime(SERVO_GOALTIME_NORMAL);
			ServoGrab->setGoalPosition(SERVOPOS_RUN_G);
			ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
			
			State = UPPER_STATE_IDLE;		// ボール捨てた後ってなにもしないからIDLEにしよう
			dettachCtrlSeqFunc();	// 処理無し
		}
		break;
	default:
		/* なにかおかしいんだけど */
		return -1;
	}
	
	return 0;
}



/*********************
ボール拾う準備要求
概要：
	アーム倒してガバッと開く
引数：
	ボール拾う場所(角度)
いじる変数：
	CtrlSeqFunc
	State
**********************/
int8_t inrof2016_upper_ctrl::reqBallGetPre(float AngleRad){
	
	if((UPPER_STATE_IDLE == State) || (UPPER_STATE_ARM_TO_CENTER==State)){
		
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPre: req\r\n");
#endif
	
		// 向かう角度設定(ボールを拾うエンコード値)
		this->SeqArmEncObj = Rad2Enc(AngleRad);		// 目標エンコーダ位置
		
		// 関数登録
		attachCtrlSeqFunc(ctrlSeqBallGetPre);
		// 状態更新
		State = UPPER_STATE_BALL_GET_PRE;
		
		return 0;
	}else{
		return -1;
	}
}

// アームシーケンス
// ボール拾う準備
int8_t inrof2016_upper_ctrl::ctrlSeqBallGetPre(void){
	bool_t SeqInit = false;
	
	if(SeqNum!=SeqNumLast){	// 初回処理
		SeqInit = true;
	}
	SeqNumLast = SeqNum;
	
	switch(SeqNum){
	case 0:	// サーボを移動位置へ
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPre: SvMovePos\r\n");
#endif
			SeqStartTime_ms = getTime_ms();
			// アームを移動位置へ立ち上げる
			ServoGrab->setGoalTime(SERVO_GOALTIME_NORMAL);
			ServoPitch->setGoalTime(SERVO_GOALTIME_NORMAL);
			ServoGrab->setGoalPosition(SERVOPOS_RUN_G);
			ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// サーボ動作完了
			SeqNum ++;
		}
		break;
	case 1:	// アーム移動
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPre: ArmMove\r\n");
#endif
			// ボール拾う位置へ移動要求
			moveArmEncCnt(this->SeqArmEncObj);
			// 開始時刻覚える
			SeqStartTime_ms = getTime_ms();
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if(isFinishCtrlArmMove() || (SeqStartTime_ms+10000<getTime_ms())){	//拾う位置まで来た
			SeqNum ++;
		}
		break;
	case 2:	// サーボ下ろしてガバッと
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPre: SvOpen\r\n");
#endif
			// サーボを拾う位置へ下ろして、アームを広げる
			ServoGrab->setGoalPosition(SERVOPOS_BALLGET0_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLGET0_P);
			// 開始時刻覚える
			SeqStartTime_ms = getTime_ms();
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// サーボ動作完了
			SeqNum ++;
		}
		break;
	case 3:	// アーム閉じる
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPre: SvDown\r\n");
#endif
			// アーム閉じる
			ServoGrab->setGoalPosition(SERVOPOS_BALLGET1_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLGET1_P);
			// 開始時刻覚える
			SeqStartTime_ms = getTime_ms();
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// サーボ動作完了
			State = UPPER_STATE_IDLE;		// ボール拾う準備完了
			dettachCtrlSeqFunc();			// 処理無し
		}
		break;
	default:
		/* なにかおかしいんだけど */
#ifdef DEBUGSCI0_UPPER
		Sci0.print("<Sequence Error>inrof2016_upper_ctrl::ctrlSeqBallGetPre %d\r\n", SeqNum);
#endif
		return -1;
	}
	
	return 0;
}




/*********************
ボール拾う要求
概要：
	掴んで持ち上げる
	動いていい状態になったらシーケンス終了
	ボール色も判定
引数：
	なし
いじる変数：
	CtrlSeqFunc
	State
**********************/
int8_t inrof2016_upper_ctrl::reqBallGet(void){
	
	if(UPPER_STATE_IDLE == State){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGet req\r\n");
#endif
		
		// 関数登録
		attachCtrlSeqFunc(ctrlSeqBallGet);
		// 状態更新
		State = UPPER_STATE_BALL_GET;
		
		return 0;
	}else{
		return -1;
	}
}

// アームシーケンス
// ボール拾う
int8_t inrof2016_upper_ctrl::ctrlSeqBallGet(void){
	bool_t SeqInit = false;
	
	if(SeqNum!=SeqNumLast){	// 初回処理
		SeqInit = true;
	}
	SeqNumLast = SeqNum;
	
	switch(SeqNum){
	case 0:	// 掴む
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGet: Grab\r\n");
#endif
			SeqStartTime_ms = getTime_ms();
			// アームを掴む
			ServoGrab->setGoalTime(200);
			ServoGrab->setGoalPosition(SERVOPOS_BALLGET1_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLGET1_P);
			
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// サーボ動作完了
			
			ServoGrab->setGoalTime(SERVO_GOALTIME_NORMAL);
			// ボール掴んだ?
			if(checkHoldBall()){
				// 掴んでたらカラー判定開始
		//		ColorSens->enableMeasure();
			}
			// 掴んでる
			SeqNum ++;
		}
		break;
	case 1:	// 持ち上げる
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGet: Up\r\n");
#endif
			// アームを持ち上げる
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP0_G);
			ServoPitch->setGoalTime(2000);
			ServoPitch->setGoalPosition(SERVOPOS_BALLUP0_P);
			// 開始時刻覚える
			SeqStartTime_ms = getTime_ms();
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+2000<getTime_ms())){	// アーム持ち上げた
		
			ServoPitch->setGoalTime(1000);
			
			// ボール掴んだ?
			//if(checkHoldBall()){
			if(1){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGet: Grab Ball Got\r\n");
#endif
				// 掴んでる
				// こっちは勝手にボール置いたりしてるから
				// 下半身さんは移動していいよ。
				BallHoldNum ++;	// 獲得ボール数仮更新
				
				//State = UPPER_STATE_BALL_GET_POST;	// ボール載っける準備完了
				//attachCtrlSeqFunc(ctrlSeqBallGetPost);	// マシン上に載せに行く
				reqBallGetPost();
			}else{
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGet: Grab Ball no Got\r\n");
#endif
				// 掴んでない
				// しれっとシーケンス終わる
				ServoGrab->setGoalPosition(SERVOPOS_RUN_G);	// サーボは移動可能位置へ
				ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
				ColorSens->disableMeasure();
				// センターに戻ろう
				State = UPPER_STATE_IDLE;		// ボール拾う準備完了
				dettachCtrlSeqFunc();			// 処理無し
				//reqArmToCenter();
			}
			
			
		}
		break;
	default:
		/* なにかおかしいんだけど */
#ifdef DEBUGSCI0_UPPER
		Sci0.print("<Sequence Error>inrof2016_upper_ctrl::ctrlSeqBallGet %d\r\n", SeqNum);
#endif
		return -1;
	}
	
	return 0;
}




/*********************
ボールを載っける要求
概要：
	ボール拾ったやつをマシン上に置く
引数：
	なし
いじる変数：
	CtrlSeqFunc
	State
**********************/
int8_t inrof2016_upper_ctrl::reqBallGetPost(void){
	
	if(UPPER_STATE_BALL_GET == State){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost req\r\n");
#endif
		
		// 関数登録
		attachCtrlSeqFunc(ctrlSeqBallGetPost);
		// 状態更新
		State = UPPER_STATE_BALL_GET_POST;
		
		return 0;
	}else{
		return -1;
	}
}
int8_t inrof2016_upper_ctrl::ctrlSeqBallGetPost(void){
	bool_t SeqInit = false;
	
	if(SeqNum!=SeqNumLast){	// 初回処理
		SeqInit = true;
	}
	SeqNumLast = SeqNum;
	
	switch(SeqNum){
	case 0:	// サーボを移動位置へ
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: Sv MovePos\r\n");
#endif
			SeqStartTime_ms = getTime_ms();
			// アームを移動位置へ立ち上げる
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP0_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLUP0_P);
			
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// サーボ動作完了
			SeqNum ++;
		}
		break;
		
	case 1:	// アームをカラーセンサ設置位置へ移動
		if(SeqInit){
			// ボール拾う位置へ移動要求
			moveArmBallPos(INROF_UPPER_COLSENS_POS);
		}
		
		/* do nothing */
		// アーム移動待ち
		
		/* 遷移処理 */
		if(isFinishCtrlArmMove() || (SeqStartTime_ms+10000<getTime_ms())){	// 置く位置まで来た
			SeqNum ++;
		}
		
	case 2:	// カラーチェックのためボールを一旦置く
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: color check: Sv Put ball pos\r\n");
#endif
			// サーボを置く角度へ
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP1_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLUP1_P);
			// 開始時刻覚える
			SeqStartTime_ms = getTime_ms();
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// サーボ動作完了
			SeqNum ++;
		}
		break;
		
	case 3:	// ボール置く(カラーチェック)
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: color check: Sv put Ball \r\n");
#endif
			// ボールを放す (持ち替えるため)
			ServoGrab->setGoalTime(200);
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP2_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLUP2_P);
			// 開始時刻覚える
			SeqStartTime_ms = getTime_ms();
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if((isFinishMoveServo() && (SeqStartTime_ms+3000<getTime_ms())) || (SeqStartTime_ms+5000<getTime_ms())){
			// サーボ動作完了&&ボール落ちるの待つ
			// カラーチェック開始
			ColorSens->enableMeasure();
			
			ServoGrab->setGoalTime(SERVO_GOALTIME_NORMAL);
			SeqNum ++;
		}
		break;
		
	case 4:	// ボールつかむ+カラーチェック待ち
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: color check: Sv get Ball \r\n");
#endif
			// カラーセンサ上のボールを掴む
			ServoGrab->setGoalPosition(SERVOPOS_BALLTRASH1_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLTRASH1_P);
			// 開始時刻覚える
			SeqStartTime_ms = getTime_ms();
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if((isFinishMoveServo() && (SeqStartTime_ms+3000<getTime_ms())) || (SeqStartTime_ms+5000<getTime_ms())){
			// サーボ動作完了&&ボール落ちるの待つ
			// 色確定
			HoldingBallColor = ColorSens->getBallColor();
			
			// ルンバに表示
			switch(HoldingBallColor){
			case INROF_RED:
				Rmb.outLedDispAscii("RED");
				break;
			case INROF_YELLOW:
				Rmb.outLedDispAscii("YLOW");
				break;
			case INROF_BLUE:
				Rmb.outLedDispAscii("BLUE");
				break;
			default:
				Rmb.outLedDispAscii("MISS");
				
				break;
			}
			// カラーチェック終わり
			ColorSens->disableMeasure();
			
			// 判定できなかった
			if(INROF_NONE == HoldingBallColor){
				//HoldingBallColor = INROF_BLUE;	//とりあえず青のとこに入れる
				
				// 掴んでない
				// しれっとシーケンス終わる
				ServoGrab->setGoalPosition(SERVOPOS_RUN_G);	// サーボは移動可能位置へ
				ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
				ColorSens->disableMeasure();
				// センターに戻ろう
				State = UPPER_STATE_IDLE;		// ボール拾う準備完了
				dettachCtrlSeqFunc();			// 処理無し
				//reqArmToCenter();
				
			}
			
			
			
			SeqNum ++;
		}
		break;
		
	case 5:	// ボール持ち上げる
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: color check done (%d): Sv up Ball \r\n", HoldingBallColor);
#endif
			// ボールを移動可能状態に持ち上げる
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP0_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLUP0_P);
			// 開始時刻覚える
			SeqStartTime_ms = getTime_ms();
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+5000<getTime_ms())){
			// サーボ動作完了
			SeqNum ++;
		}
		break;
		
	case 6:	// ボールを置く位置へアーム移動
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: ArmMove\r\n");
#endif
			
			// マシン上のいい感じの位置を決める
			// 今のアーム位置から遠い位置(なるべくボールが置いてあるところをアームが移動しないように)
			int8_t Sel;
			uint8_t NearCw = 0xFF;
			uint8_t NearCcw = 0xFF;
			// CW方向で遠い位置
			for(Sel=0;Sel<NUM_BALL_STOCK;Sel+=2){	// 偶数箇所にしか置かない
				if(BallStock[Sel] == INROF_NONE){	// 開いてる
					NearCw = Sel;
					break;
				}
			}
			for(Sel=8;Sel>=0;Sel-=2){	// 偶数箇所にしか置かない
				if(BallStock[Sel] == INROF_NONE){	// 開いてる
					NearCcw = Sel;
					break;
				}
			}
			
			// 置く場所無いんだけど
			if((NearCw==0xFF) && (NearCcw==0xFF)){
				// あとで考える
			}
			
			// CWCCWの遠い方を目標ボール置き場に設定
			if( fabs(((float)NUM_BALL_STOCK-1.0F)/2.0F - (float)NearCw) > fabs(((float)NUM_BALL_STOCK-1.0F)/2.0F - (float)NearCcw) ){
				// CW側のほうが遠いのでそこに置きます。
				this->SeqBallStockObj = NearCw;		// 目標ボール置き場
			}else{
				this->SeqBallStockObj = NearCcw;	// 目標ボール置き場
			}
			
			// ボール拾う位置へ移動要求
			moveArmBallPos(this->SeqBallStockObj);
			
			
			
			// 開始時刻覚える
			SeqStartTime_ms = getTime_ms();
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if(isFinishCtrlArmMove() || (SeqStartTime_ms+10000<getTime_ms())){	// 置く位置まで来た
			SeqNum ++;
		}
		break;
	case 7:	// ボール置く角度へ
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: Sv Put ball pos\r\n");
#endif
			// サーボを置く角度へ
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP2_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLUP2_P);
			// 開始時刻覚える
			SeqStartTime_ms = getTime_ms();
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+2000<getTime_ms())){	// サーボ動作完了
			SeqNum ++;
		}
		break;
	case 8:	// ボール置く
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: Sv put Ball \r\n");
#endif
			// ボールを放す
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP3_G);
			ServoPitch->setGoalPosition(SERVOPOS_BALLUP3_P);
			// 開始時刻覚える
			SeqStartTime_ms = getTime_ms();
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if((isFinishMoveServo() && (SeqStartTime_ms+1000<getTime_ms())) || (SeqStartTime_ms+5000<getTime_ms())){
			// サーボ動作完了&&ボール落ちるの待つ
			SeqNum ++;
		}
		break;
	case 9:	// サーボを移動可能位置へ
		/* 初回処理 */
		if(SeqInit){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqBallGetPost: Sv free pos\r\n");
#endif
			// サーボを移動角度へ
			ServoGrab->setGoalPosition(SERVOPOS_BALLUP3_G);		// Grabは開いたまま。
			ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
			// 開始時刻覚える
			SeqStartTime_ms = getTime_ms();
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// サーボ動作完了
			// 置いたボールを登録
			BallStock[this->SeqBallStockObj] = HoldingBallColor;
			// ボール数更新(念のため)
			updateBallHoldNum();
			
			State = UPPER_STATE_IDLE;		// ボール置いた
			dettachCtrlSeqFunc();			// 処理無し
			//reqArmToCenter();			// 置いたら戻る、真ん中へ
		}
		break;
	default:
		/* なにかおかしいんだけど */
#ifdef DEBUGSCI0_UPPER
		Sci0.print("<Sequence Error>inrof2016_upper_ctrl::ctrlSeqBallGetPost %d\r\n", SeqNum);
#endif
		return -1;
	}
	
	return 0;
}




/*********************
アーム真ん中へ移動要求
概要：
	アーム中立にして真ん中へ移動
	終わったらUPPER_STATE_IDLEになる
**********************/
// 要求
int8_t inrof2016_upper_ctrl::reqArmToCenter(void){
	
	if(State == UPPER_STATE_IDLE){
#ifdef DEBUGSCI0_UPPER
	Sci0.print("ctrlSeqArmToCenter req\r\n");
#endif
		// 関数登録
		attachCtrlSeqFunc(ctrlSeqArmToCenter);
		// 状態更新
		State = UPPER_STATE_ARM_TO_CENTER;
		
		return 0;
	}else{
		return -1;
	}
}

// 処理シーケンス
int8_t inrof2016_upper_ctrl::ctrlSeqArmToCenter(void){
	bool_t SeqInit = false;
	
	if(SeqNum!=SeqNumLast){	// 初回処理
		SeqInit = true;
	}
	SeqNumLast = SeqNum;
	
	switch(SeqNum){
	case 0:	// サーボを移動位置へ
		/* 初回処理 */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			// サーボを移動位置へ
			ServoGrab->setGoalTime(500);
			ServoPitch->setGoalTime(500);
			ServoGrab->setGoalPosition(SERVOPOS_RUN_G);
			ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
		}
		
		/* 通常時処理 */
		/* do nothing (wait for servo move) */
		
		
		/* 遷移処理 */
		if(isFinishMoveServo() || (SeqStartTime_ms+1000<getTime_ms())){	// サーボ動作完了
			ServoGrab->setGoalTime(SERVO_GOALTIME_NORMAL);
			ServoPitch->setGoalTime(SERVO_GOALTIME_NORMAL);
			SeqNum ++;
		}
		break;
	
	case 1:	// 中央へ向かう
		/* 初回処理 */
		if(SeqInit){
			SeqStartTime_ms = getTime_ms();
			// 目標位置設定
			moveArmAngle(PI/2);	// ボール取りに行くボール置き場を設定
		}
		
		/* 通常時処理 */
		/* do nothing (wait for ctrlArmMotor()) */
		
		
		/* 遷移処理 */
		if(isFinishCtrlArmMove()  || (SeqStartTime_ms+10000<getTime_ms())){	// 中央位置まで来た
			State = UPPER_STATE_IDLE;
			dettachCtrlSeqFunc();	// 処理無し
		}
		break;
	default:
		/* なにかおかしいんだけど */
		return -1;
	}
	
	return 0;
}




/*********************
ボール持ってる？
戻り値：
	0:なし
	1:前で持ってる
	2:後ろで持ってる
**********************/
uint8_t inrof2016_upper_ctrl::checkHoldBall(void){
	return 1;
	if(INROF_UPPER_BALLHOLD_CURRENT_TH_mA <= ServoGrab->getPresentCurrent()){
		if( 0 < ServoGrab->updatePresentPosition()){	// 前
			return 1;
		}else{	//後ろ
			return 2;
		}
	}else{	// 持ってない
		return 0;
	}
	
	return 0;
}


/*********************
サーボ移動完了？
 速度 10deg/sec以内
 位置 2deg以内
**********************/
bool_t inrof2016_upper_ctrl::isFinishMoveServo(void){
	
	if(abs(ServoPitch->getPresentSpeed()) < 10){
		if(abs(ServoPitch->GoalPosition - ServoPitch->getPresentPosition()) < 20){	// 20*0.1度
			if(abs(ServoGrab->getPresentSpeed()) < 10){
				if(abs(ServoGrab->GoalPosition - ServoGrab->getPresentPosition()) < 20){	// 20*0.1度
					return 1;
				}
			}
		}
	}
	return 0;
}



/*********************
持ってるボール数更新
概要：
	配列を数える
**********************/
void inrof2016_upper_ctrl::updateBallHoldNum(void){
	uint8_t i;
	uint8_t cnt=0;
	for(i=0;i<NUM_BALL_STOCK;i++){
		if(BallStock[i]!=INROF_NONE){
			cnt++;
		}
	}
	BallHoldNum = cnt;
}
/*********************
ボール何個持ってる？
引数：
	色
戻り値：
	個数
**********************/
uint8_t inrof2016_upper_ctrl::numBallColor(inrof_color Col){
	uint8_t cnt = 0;
	uint8_t i;
	
	for(i = 0; i<NUM_BALL_STOCK; i++){
		if(Col == BallStock[i]){
			cnt++;
		}
	}
	
	return cnt;
}



/*********************
アームエンコーダ エンコードモード
　フォトインタラプタ へ遷移
概要：
**********************/
int8_t inrof2016_upper_ctrl::ArmEncModeRaw(void){
	uint8_t TxData;
	
	ArmEncMode = ARM_ENC_RAW;
	TxData = 0x02;
	return ArmI2C->attach(ARM_I2C_ADDRESS, &TxData, 1);
}

/*********************
アームエンコーダ エンコードモード
　エンコード済み へ遷移
概要：
**********************/
int8_t inrof2016_upper_ctrl::ArmEncModeMseqIndex(void){
	uint8_t TxData;
	
	ArmEncMode = ARM_ENC_MSEQINDEX;
	TxData = 0x03;
	return ArmI2C->attach(ARM_I2C_ADDRESS, &TxData, 1);
}

/*********************
アームエンコーダ情報を要求
概要：
 I2Cで受ける。
 1バイト受信
**********************/
int8_t inrof2016_upper_ctrl::reqArmEnc(void){
	uint8_t dummy;
	
	ReqObject = this;
	//return ArmI2C->attach(ARM_I2C_ADDRESS, &dummy, 0, 1, isrArmEnc);
	return ArmI2C->attach(ARM_I2C_ADDRESS, &dummy, 0, 2, isrArmEnc);
}

/*********************
I2C受信処理
static
概要：
 I2Cで受ける。
 1バイト受信
**********************/
inrof2016_upper_ctrl* inrof2016_upper_ctrl::ReqObject;
void inrof2016_upper_ctrl::isrArmEnc(I2c_comu_t *Comu){
	uint8_t tmp[2];
	uint8_t ArmEncLast = ReqObject->ArmEnc;
	
	if(!Comu->Err){
		 tmp[0] = Comu->RxData[0];	// 処理済み
		 tmp[1] = Comu->RxData[1];	// raw
	}else{
		 tmp[0] = 0xFE;	// 通信失敗
		 tmp[1] = 0xFE;	// 通信失敗
	}
	
	if(ARM_ENC_RAW == ReqObject->ArmEncMode){			// 生値を受信
		
		ReqObject->ArmEncRaw = tmp[1];
		
		if(tmp[0] == 0xFF){		// フォトインタラプタ光ってないor全部白判定(正常ならありえない)
			ReqObject->ArmEnc = 0xFF;	// 一度FFになっちゃうと復帰に手間取るのでなくした。
			//return -1;
		}else if(Comu->Err){		// 通信失敗
			Sci0.print("Mseq Comu Error. %d\r\n", I2C0.isError());
			ReqObject->ArmEnc = 0xFE;
			//return -1;
		}else{
			ReqObject->updateArmEnc(ReqObject->ArmEncRaw);	// 自前でエンコード
		}
	}else if(ARM_ENC_MSEQINDEX == ReqObject->ArmEncMode){	// エンコード済みを受信
		
		ReqObject->ArmEnc = tmp[0];
		ReqObject->ArmEncRaw = tmp[1];
		
	}
	
	// 速度更新
	if(((ReqObject->ArmEnc)<=ENC_CNT_MAX) && (ArmEncLast<=ENC_CNT_MAX)){
		ReqObject->ArmEncSpd = ReqObject->CtrlArmEncSpdAverage->average( (ReqObject->ArmEnc - ArmEncLast)/SAMPLETIME_CTRL_S );
	}
}

/*********************
m系列信号からインデックスを更新
概要：順番考慮。変な値になったら更新しない。
 更新されなかったら-1を返す
**********************/
int8_t inrof2016_upper_ctrl::updateArmEnc(uint8_t Mseq){
	uint8_t i;
	uint8_t Kouho = 0xFF;
	
	uint8_t Window = 5;	// 前回値からこれだけのズレは許可
	const uint8_t NoDecideCntMax = 5;	// 前回値からこれだけのズレは許可
	
	/*
	// 2015.05.19
	// 大きい値に飛んじゃった時に復帰できなくなるので削除。
	if((ArmEnc<5)||(ArmEnc==0xFE)){	// 未初期化か小さい値
		i = 0;			// だったら、0から探索スタート
	}else{
		i = ArmEnc-5;
	}
	*/
	/*
	// debug. 
	if(Mseq == 0xFF){	// フォトインタラプタ光ってないor全部白判定(正常ならありえない)
		//ArmEnc = 0xFF;	// 一度FFになっちゃうと復帰に手間取るのでなくした。
		return -1;
	}else if(Mseq == 0xFE){	// 通信失敗
	Sci0.print("Mseq Comu Error.\r\n");
		ArmEnc = 0xFE;
		return -1;
	}
	*/
	
	// インデックスに変換(該当なしなら0xFFのまま)
	for(i=0;i<ENC_CNT_MAX;i++){
		if(Mseq == MseqArray[i]){
			Kouho = i;
			break;
		}
	}
	
	//if(Kouho!=0xFF){	// 値有効
		// 未初期化もしくは、(前回値と比較してプラスマイナス1の範囲にある)なら更新
		// 前回が0xFFとかFEだったら即復活？下の回復ロジックで回復するなら良いか。
		if((((ArmEnc==0xFE) || (ArmEnc==0xFF))&&(Kouho<ENC_CNT_MAX)) || ((Kouho<=(ArmEnc+Window))&&((Kouho+Window)>=ArmEnc))){
			ArmEnc = Kouho;
			NoDecideCnt = 0;
			
			return 0;
		}
		
		// ↓ちゃんと動いてない感ある。
		
		// 動いてるのに連続しておかしければ直すべき。
		// (未初期化時に適用した値が堺のとこだったとかで、
		// 変な値が確定されちゃうことがあるので。)
		if(ArmEncLast != Kouho){	// 動いている
			if(NoDecideCnt > NoDecideCntMax){	// nマス動いてだめなら、
				ArmEnc = Kouho;	// 今回のを確定させちゃう
						// 前回値ArmEncLastとの差はみなくていい？
						// これで次回ちゃんとした値になればCntはリセットされる
				Sci0.print("ArmEncAdj = 0x%02X\r\n", ArmEnc);
			}else{
				NoDecideCnt++;
				Sci0.print("ArmEncCnt = 0x%02d\r\n", NoDecideCnt);
			}
		}
		
		ArmEncLast = Kouho;
	//}
	return -1;
}




void inrof2016_upper_ctrl::moveArmDuty(int16_t Duty){
	DutyTgt = Duty;
	CtrlMode = MODE_DUTY;
}

void inrof2016_upper_ctrl::moveArmEncCnt(uint8_t EncCnt){
	EncCntTgt = EncCnt;
	CtrlMode = MODE_ENCCNT;
}
void inrof2016_upper_ctrl::moveArmBallPos(uint8_t BallPos){
	
	// BollPosからエンコーダカウント値へ変換
	EncCntTgt = BallStockEnc[BallPos];
	
	BallPosTgt = BallPos;
	CtrlMode = MODE_BALLPOS;
}


void inrof2016_upper_ctrl::moveArmAngle(float Rad){
	
	// 0x1B Rad=0
	// 0x80 Rad=pi
	
	// Radianからエンコーダカウント値へ変換
	//EncCntTgt = (0x80 - 0x1B)/3.1415F * Rad + 0x1B;
	EncCntTgt = Rad2Enc(Rad);
	
	RadTgt = Rad;
	CtrlMode = MODE_RAD;
}


/*********************
移動完了？(シーケンス制御)
 ・目標エンコーダ値1digit以内
 ・0.5count/sec以内の速度になった
引数：
	目標エンコーダ値
戻り値：
	おわった 1
**********************/
bool_t inrof2016_upper_ctrl::isFinishCtrlArmMove(void){
	
	// 速度小さい
	if(fabs(ArmEncSpd) < 0.5F){
		if( (MODE_ENCCNT==CtrlMode) || (MODE_BALLPOS==CtrlMode) || (MODE_RAD==CtrlMode) ){	// 位置制御系
			// 位置制御の場合は更に位置の条件
			if(abs((int16_t)ArmEnc - (int16_t)EncCntTgt) < 2U){
				return 1;
			}else{
				return 0;
			}
		}else{	// 速度制御とか
			return 1;
		}
	}
	return 0;
}



/*********************
アームモータ制御
概要：
 指定エンコーダ位置へ移動する
 エンコーダ値がしばらく変わらないようなら
**********************/
int8_t inrof2016_upper_ctrl::ctrlArmMotor(void){
	
	if(CtrlModeLast != CtrlMode){
		pid_ctrlArmMotor->resetStatus();
	}
	
	
	// エンコーダカウント値からデューティーへ変換
	if(MODE_DUTY != CtrlMode){	// 位置制御系
		if(ArmEnc<=ENC_CNT_MAX){	
			// エンコーダ有効値
			// PIDで目標エンコーダ位置へ
			DutyTgt = pid_ctrlArmMotor->calc(EncCntTgt - ArmEnc);
			DutyTgt = ratelimiter_ctrlArmMotor->limitRate(DutyTgt);
			CntFail = 0;
			ArmEncCorrect = ArmEnc;	// 正しく通信できていた時の値を覚えておく
			
			
		}else{ // 通信途絶えてる
			// 
			if(INROF_UPPER_CNT_FAIL_MAX<CntFail){
				
				if(ArmEnc!=0xFE){
					if(fabs(ArmEncSpd)<0.5){	// 止まっちゃってる
								// 端ぶつかっちゃってる時はどうすっぺ
						if(EncCntTgt > ArmEncCorrect){	// 目標とずれたところで止まっちゃってたらちょっと動かしちゃえ
							DutyTgt = -100;	// あえて逆向きに回す
						}else{
							DutyTgt = 100;
						}
					}
				}else{
					DutyTgt = 0;	// 通信途絶は止める
				}
			}else{
				// カウントマックスになるまでは前のを出力
				CntFail++;
			}
		}
	}
	
	// 端っこ判定
	// それ以上行かないようにする
	
	if(ArmEnc<=ENC_CNT_MAX){	// 有効値の時のみ
		if(ArmEnc > ENC_CNT_MAX-3){
			if(DutyTgt>0){
				DutyTgt = 0;
			}
		}
		if(ArmEnc < 3){
			if(DutyTgt<0){
				DutyTgt = 0;
			}
		}
	}
	
	
	
	if(fAction){	// 動作許可
		ArmMotor->setDuty( (int16_t)DutyTgt );
	}else{
		ArmMotor->setDuty( 0 );
		ArmMotor->setDir(FREE);
	}
	
	CtrlModeLast = CtrlMode;
	return 0;
}



/*********************
rad -> エンコーダ値
概要：
**********************/
uint8_t inrof2016_upper_ctrl::Rad2Enc(float Rad){
	
	
	if(Rad < -1.0F/3.0F*3.14159F){
		Rad = -1.0F/3.0F*3.14159F;
	}
	if(Rad > 4.0F/3.0F*3.14159F){
		Rad = 4.0F/3.0F*3.14159F;
	}
	
	// 270degで146count 0度でカウント値は30
	// 146/270 * rad/pi*180 + 30;
	float tmp = Rad * 30.98F + 30.0F;
	
	if(tmp<0){
		tmp = 0;
	}else if(tmp>ENC_CNT_MAX){
		tmp = ENC_CNT_MAX;
	}
	
	return (uint8_t)tmp;
}



