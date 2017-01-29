
#include "Inrof2016_lib.h"

inrof2016 Inrof;
inrof2016_aplSci_t ExtSci(&Sci0);	// SBDBT基板との通信(PC, PS3コン)


// ルンバ
// SCI2
RoombaOi Rmb = RoombaOi();

inrof2016::inrof2016(void){
	
}


void inrof2016::begin(void){
	uint16_t i;
	
	GrEx.begin();		// 拡張基板初期化
	GrExOled.begin();	// 拡張基板のOLED初期化

// 外部通信
	Sci0.begin(115200, true, true, 1024, 128);	// PC use Tx, Rx
	//Sci1.begin(115200, true, true, 255, 128);	// デバッガ use Tx, Rx
	
// オブジェクト用意
	// ルンバ
	Rmb = &::Rmb;
	Rmb->begin(&Sci2);
		
	// エンコーダ
	EncR = GrEx.Enc1;
	EncL = GrEx.Enc0;
	
	// MPU6050
	Mpu = new invensense_MPU(&I2C0, IMU_I2C_ADDRESS);
	
	// 測距センサ
	beginBallSensor();
	
//初期化	
	
	// IMU
	ComuStatusI2C |= Mpu->setup();
	
	// ラインセンサ
	setPinMode(LINESENS_PORT_LED, PIN_OUTPUT);	// LED光らせるポート
	// AD変換開始
	GrEx.startAD();
		
	
	// ルンバ初期化
	Inrof.Rmb->setMode(ROI_MODE_FULL);	// フルモードにする
	// ルンバモード変更待ち
	for(i=0;i<65534;i++);	// ちょい待ち
	for(i=0;i<65534;i++);	// ちょい待ち
	// 受信したいデータ設定
	ROI_SENSPACKET_ID SensDataReq[] = {
				ROI_SENSPACKET_BUMPS_WHEELDROPS,
				ROI_SENSPACKET_BATTERY_CHARGE,
				ROI_SENSPACKET_BATTERY_CAPACITY,
				ROI_SENSPACKET_ENCODER_COUNTS_LEFT,
				ROI_SENSPACKET_ENCODER_COUNTS_RIGHT,
				ROI_SENSPACKET_BUTTONS,
				ROI_SENSPACKET_CLIFF_LEFT,
				ROI_SENSPACKET_CLIFF_FRONT_LEFT,
				ROI_SENSPACKET_CLIFF_FRONT_RIGHT,
				ROI_SENSPACKET_CLIFF_RIGHT,
				ROI_SENSPACKET_CLIFF_LEFT_SIGNAL,
				ROI_SENSPACKET_CLIFF_FRONT_LEFT_SIGNAL,
				ROI_SENSPACKET_CLIFF_FRONT_RIGHT_SIGNAL,
				ROI_SENSPACKET_CLIFF_RIGHT_SIGNAL,
				ROI_SENSPACKET_SONG_PLAYING
	};
	Inrof.Rmb->setRcvStream(sizeof(SensDataReq) / sizeof(SensDataReq[0]), SensDataReq);
	
	
	
	// 上部初期化
	InrofUpper.begin();
	
	// フラグ系リセット
	initialize();
	
	
	setup1msInterrupt();	//1msごと割り込み設定
}

void inrof2016::initialize(void){
	float tmp[3];
	
	// データフラッシュ初期化
	initMem();
	
	// 制御用
	// タイヤ
	//PidCtrlRTire = new ctrl_pid(SAMPLETIME_CTRL_S, 10.0F, 0.0F, 0.1F, -255,255, -1000, 1000);	// 速度->デューティー
	//PidCtrlLTire = new ctrl_pid(SAMPLETIME_CTRL_S, 10.0F, 0.0F, 0.1F, -255,255, -1000, 1000);
	if(PidCtrlRTire){	delete PidCtrlRTire; }
	PidCtrlRTire = new ctrl_pid(SAMPLETIME_CTRL_S, 20.0F, 0.0F, 1.0F, -255,255, -500, 500);	// 速度->デューティー
	if(PidCtrlLTire){	delete PidCtrlLTire; }
	PidCtrlLTire = new ctrl_pid(SAMPLETIME_CTRL_S, 20.0F, 0.0F, 1.0F, -255,255, -500, 500);
	// 速度レートリミッタ
	if(RateLimitSpdTgt){	delete RateLimitSpdTgt; }
	RateLimitSpdTgt = new ctrl_ratelimiter_abs(SAMPLETIME_CTRL_S, 400.0F, 1000.0F);	// 500 mm/s/s
	
	if(RateLimitAngVelTgt){ delete RateLimitAngVelTgt; }
	RateLimitAngVelTgt = new ctrl_ratelimiter_abs(SAMPLETIME_CTRL_S, 3.0F, 3.0F);		// 0.3 rad/s/s	
	
	// ライントレース
	readMem(MEM_LINETRACE_PID_KP, &tmp[0]);
	readMem(MEM_LINETRACE_PID_KI, &tmp[1]);
	readMem(MEM_LINETRACE_PID_KD, &tmp[2]);
	
	if(PidCtrlLineTrace){ delete PidCtrlLineTrace; }
	PidCtrlLineTrace = new ctrl_pid(SAMPLETIME_CTRL_S, 
					tmp[0], 
					tmp[1], 
					tmp[2], 
					-RMB_ANGVEL_MAX,RMB_ANGVEL_MAX, -RMB_ANGVEL_MAX/10.0F, RMB_ANGVEL_MAX/10.0F);
	
			
	// ボールセンサモジュール初期化
	initBallSensor();
	
	// 上部初期化
	InrofUpper.initialize();
	
	// フラグ系リセット
	// 初回は全タスク実行する
	fExeReqTask0 = true;
	fExeReqTask1 = true;
	fExeReqTask2 = true;
	
	fBattEmpty = false;
	fCnt200 = false;
	
	fResetEncoder = true;
	
	SpdTgt = 0;
	AngVelTgt = 0;
	RTireDutyTgt = 0;
	LTireDutyTgt = 0;
	
	
	// 車体初期位置
	MachinePos.X = 0.0;
	MachinePos.Y = -200.0;
	MachinePos.dX = 0.0;
	MachinePos.dY = 0.0;
	MachinePos.Th = PI/2.0;
	MachinePos.dTh = 0.0;
	
	// 前に交差点発見した場所
	LastFindCrossPos.X = 0.0;
	LastFindCrossPos.Y = -200.0;
	Cross = INROF_CROSS_NONE;
	
	
	CalcOdmetry.REncCntDiff[0] = 0.0;
	CalcOdmetry.LEncCntDiff[0] = 0.0;
	CalcOdmetry.CosTh[0] = 0.0;
	CalcOdmetry.SinTh[0] = 1.0;
	CalcOdmetry.dth2[0] = 0.0;
	CalcOdmetry.REncCntDiff[1] = 0.0;
	CalcOdmetry.LEncCntDiff[1] = 0.0;
	CalcOdmetry.CosTh[1] = 0.0;
	CalcOdmetry.SinTh[1] = 1.0;
	CalcOdmetry.dth2[1] = 0.0;
	
	// フィールド
	PosCrossStart.X = FIELD_INROF_CROSS_START_X;
	PosCrossStart.Y = FIELD_INROF_CROSS_START_Y;
	PosCrossRed.X = FIELD_INROF_CROSS_RED_X;
	PosCrossRed.Y = FIELD_INROF_CROSS_RED_Y;
	PosCrossYellow.X = FIELD_INROF_CROSS_YELLOW_X;
	PosCrossYellow.Y = FIELD_INROF_CROSS_YELLOW_Y;
	PosCrossBlue.X = FIELD_INROF_CROSS_BLUE_X;
	PosCrossBlue.Y = FIELD_INROF_CROSS_BLUE_Y;
	PosCrossBallFieldBegin.X = FIELD_INROF_CROSS_BALLFIELD_BEGIN_X;
	PosCrossBallFieldBegin.Y = FIELD_INROF_CROSS_BALLFIELD_BEGIN_Y;
	PosCrossBallFieldEnd.X = FIELD_INROF_CROSS_BALLFIELD_END_X;
	PosCrossBallFieldEnd.Y = FIELD_INROF_CROSS_BALLFIELD_END_Y;
	PosCrossGoalRed.X = FIELD_INROF_CROSS_GOAL_RED_X;
	PosCrossGoalRed.Y = FIELD_INROF_CROSS_GOAL_RED_Y;
	PosCrossGoalYellow.X = FIELD_INROF_CROSS_GOAL_YELLOW_X;
	PosCrossGoalYellow.Y = FIELD_INROF_CROSS_GOAL_YELLOW_Y;
	PosCrossGoalBlue.X = FIELD_INROF_CROSS_GOAL_BLUE_X;
	PosCrossGoalBlue.Y = FIELD_INROF_CROSS_GOAL_BLUE_Y;
	
	// のこりボール数
	FieldBall = FIELD_BALL_NUM_INIT;
	
	
	State = INROF_INITIALIZE;
	ActionState = INROF_ACTION_IDLE;
	
	DriveMode = INROF_DRIVE_AUTO;
	
	fFinishManageAction = false;
	fFinishAction = false;
	
	fAdjustMachinePos = true;
	
	
	Goal = INROF_GOAL_YELLOW;
	
	Time_msec = 0;
}

void inrof2016::beginBallSensor(void){
	//uint16_t cnt;
	
	PsdCenter = new inrof2016_st_vl_t(	&I2C0,		PSDSENS_FRONT_ADRS,	PSDSENS_FRONT_ENABLEPIN);
	PsdRight = new inrof2016_st_vl_t(	&I2C0,		PSDSENS_RIGHT_ADRS,	PSDSENS_RIGHT_ENABLEPIN);
	PsdLeft = new inrof2016_st_vl_t(	&I2C0,		PSDSENS_LEFT_ADRS,	PSDSENS_LEFT_ENABLEPIN);
	
	//for(cnt=0;cnt<65500;cnt++);	// ちょっとまつ
	// I2Cアドレスの設定があるので、ひとまず無効化
	PsdLeft->disableModule();
	PsdCenter->disableModule();
	PsdRight->disableModule();
	
}

void inrof2016::initBallSensor(void){
	uint16_t cnt;
	
	// I2Cアドレスの設定があるので、ひとまず無効化
	PsdLeft->disableModule();
	PsdCenter->disableModule();
	PsdRight->disableModule();
	
	
	// センサ位置セット
	PsdLeft->setSensPos(	PSDSENS_LEFT_POS_X,	PSDSENS_LEFT_POS_Y,	PSDSENS_LEFT_POS_TH);
	PsdCenter->setSensPos(	PSDSENS_FRONT_POS_X,	PSDSENS_FRONT_POS_Y,	PSDSENS_FRONT_POS_TH);
	PsdRight->setSensPos(	PSDSENS_RIGHT_POS_X,	PSDSENS_RIGHT_POS_Y,	PSDSENS_RIGHT_POS_TH);
	
	// 順に起動して
	// センサ動作開始(ここでI2Cアドレス設定)
	PsdLeft->enableModule();		// モジュールリセット解除
	for(cnt=0;cnt<65500;cnt++);		// 起きるのを待つ
	PsdLeft->begin();			// 新しいI2Cアドレスをセットして動作開始
	for(cnt=0;cnt<65500;cnt++);		// 設定完了を待つ
	//while(!PsdLeft->isIdleComu());	// 設定完了を待つ
	PsdCenter->enableModule();
	for(cnt=0;cnt<65500;cnt++);		// 起きるのを待つ
	PsdCenter->begin();
	for(cnt=0;cnt<65500;cnt++);		// 設定完了を待つ
	//while(!PsdCenter->isIdleComu());	// 設定完了を待つ
	PsdRight->enableModule();		// 起きるのを待つ
	for(cnt=0;cnt<65500;cnt++);
	PsdRight->begin();
	for(cnt=0;cnt<65500;cnt++);		// 設定完了を待つ
	//while(!PsdRight->isIdleComu());	// 設定完了を待つ
	
	
}





void inrof2016::step(void){
	if(fExeReqTask0){
		stepTask0();
		fExeReqTask0 = false;
	}
	if(fExeReqTask1){
		stepTask1();
		fExeReqTask1 = false;
	}
	if(fExeReqTask2){	// 10msのより先にやらないと、カラーセンサのI2Cできない(new失敗する)？？？
		stepTask2();
		fExeReqTask2 = false;
	}
}

// 1msごとに実行する関数
void inrof2016::isr1ms(void){
	GR_cnt();
	Time_msec++;
	
	fExeReqTask0 = true;	// 1msごと
	
	if(CntTask1<19){		// 20msごと
		CntTask1 ++;
	}else{
		CntTask1 = 0;
		fExeReqTask1 = true;
	}
	
	if(CntTask2<199){	// 200msごと
		CntTask2++;
	}else{
		CntTask2 = 0;
		fExeReqTask2 = true;
	}
	
}

// 1msごと
void inrof2016::stepTask0(void){
	
	// test
	// 1sごとにチカチカ
	static uint16_t cnt;
	static bool_t f;
	if(1000 < cnt++){
		if(f){
			outPin(GR_LED0, 0);
			f = 0;
		}else{
			outPin(GR_LED0, 1);
			f = 1;
		}
		cnt = 0;
	}
	
	
	static uint8_t Col=0;
	if(!fBattEmpty){
		if(0==cnt%300){
			Col+=10;
			Inrof.Rmb->outLedBattColor(Col,0xFF);
		}
	}
}

//20msごと
void inrof2016::stepTask1(void){
	
	// EEPROM書き込み
	drvDataFlash_WriteTask();
	
	// ルンバmusic
	Inrof.Rmb->playMusicTask();
	
	InrofUpper.task0();	// 上部制御タスク
	
	
	// PC・PS3コン通信処理
	ExtSci.task();
		
	/***********
	 入力処理
	************/
	// ラインセンサ更新
	updateLineSense();

	// 情報取得要求
	//Mpu->measure();
	
	if(checkInitialize()){	// 初期化チェック(ルンバ通信など))// 初期化終わったら StateをINITIALIZEより上にする。
	//if(1){	// 初期化チェック// 初期化終わったら StateをINITIALIZEより上にする。
		
		
		// 物理バンパー当たったら止まる
		//Rmb->driveWheels(250*(!Rmb->isBumpRight()),250*(!Rmb->isBumpLeft()));
		
		/***********
		 入力処理
		************/
		// エンコーダ更新
		updateEncoder();
		
		/***********
		 制御
		************/
		// タイヤ状態更新
		updateTyre();
		
		// オドメトリ更新
		//adjustMachinePos();	// 補正 <- ライントレース内で実施
		updateOdmetry();
		
		
		// ラインセンサ交点判定
		IsCrossing = judgeCross();
		// 走行向き判定
		judgeRunOrient();
		// ボール見つけた判定
		IsBallFinding = judgeBallFind();
		
		
		// 状態管理
		// 動作管理
		// 制御 (SpdTgtとAngVelTgtを決定する)
		manageState();
		
		/***********
		 出力
		************/
		
		// ラインセンサデモ
	//	SpdTgt = 100;
	//	AngVelTgt = 0.5*(min(getLineSensAd(2), getLineSensAd(3)) - min(getLineSensAd(0),getLineSensAd(1)));
		
		
		SpdTgtRated = RateLimitSpdTgt->limitRate(SpdTgt);
		AngVelTgtRated = RateLimitAngVelTgt->limitRate(AngVelTgt);
		
		// 速度制御で目標速度を満たすデューティー比計算
		ctrlRun(SpdTgtRated, AngVelTgtRated, &RTireDutyTgt, &LTireDutyTgt);	// 速度制御
		
		// デューティー出力
		// バッテリないときは出力しない
		if(!fBattEmpty && !I2C0.isError()){	// バッテリ上がってなければ
							// かつ、I2C正常ならば
			// タイヤ
			if(INROF_DRIVE_PS3 == DriveMode){	// PS3モード
				drivePs3Con();
			}else{	
				outTyres(RTireDutyTgt, LTireDutyTgt, fMotorOut);		// デューティー出力
				//outTyres(SpdTgt+AngVelTgt, SpdTgt-AngVelTgt, fMotorOut);	// 速度出力
			}
			
		}else{
			//fMotorOut = false;
			disableOutTyre();
			outTyres(0, 0, fMotorOut);	//モーターoff
		}
	}else{
	}
	
	
	
	// センサ更新要求
	ComuStatusI2C |= PsdLeft->measure();
	ComuStatusI2C |= PsdCenter->measure();
	ComuStatusI2C |= PsdRight->measure();
	PsdLeft->checkAlive();
	PsdCenter->checkAlive();
	PsdRight->checkAlive();
	
	
	//Sci0.print("ToFAt = %d\r\n", PsdCenter->measure());
	
}
	
// 200msごと
void inrof2016::stepTask2(void){
	
	fCnt200 = true;
	
	InrofUpper.task1();	// 上部
		
	
	
	// バッテリ情報更新
//	getBattVoltage();

// バッテリーチェック
	bool_t fBattEmptyLast = fBattEmpty;
	fBattEmpty = isBattLow();
	if(!fBattEmptyLast){	// これまで空じゃなかった
		if(fBattEmpty){
			// 今回空になった判定された
			Rmb->outLedCheckRobot(true);	// チェックランプ点灯
			Rmb->outLedBattColor(0xFF, 0xFF);	// 赤、明るさmax
		}
	}else{	// これまで空だった
		if(!fBattEmpty){	// 今回は元気判定
			// 今回元気になった判定された
			Rmb->outLedCheckRobot(false);	// チェックランプ消灯
			Rmb->outLedBattColor(0x00, 0x00);	// 緑、明るさmin
		}
	}
}


/*********************
モード変更
概要：
 
引数：

**********************/
void inrof2016::setMode(inrof_drive_mode NewMode){
	
	if(NewMode != DriveMode){	// 異なるモードだったら
		switch(NewMode){
		case INROF_DRIVE_AUTO:
			
			break;
		case INROF_DRIVE_PS3:
			// 今の位置を目標位置にする
			GrabAngleTgt = InrofUpper.ServoGrab->getPresentPosition();
			PitchAngleTgt = InrofUpper.ServoPitch->getPresentPosition();
			break;
		}
	}
	
	DriveMode = NewMode;
	
}
	
/*********************
ラインセンサアップデート
概要：
 エンコーダ値をアップデート
引数：

**********************/
void inrof2016::updateLineSense(void){
	static bool_t fLineSensOn;				// LED光らせてるフラグ
	static uint16_t LineSensAdOn[LINESENS_NUM_SENSE];	// ラインセンサAD値 LED on時 10bit
	static uint16_t LineSensAdOff[LINESENS_NUM_SENSE];	// ラインセンサAD値 LED off時 10bit
	
	
	if(fLineSensOn){	// onだった
		for(uint8_t i=0; i<LINESENS_NUM_SENSE; i++){
			LineSensAdOn[i] = GrEx.getAD10((grex_ad_ch)i);	// 10bit
		}
		// LED消す
		outPin(LINESENS_PORT_LED, 0); 
		fLineSensOn = 0;
	}else{
		for(uint8_t i=0; i<LINESENS_NUM_SENSE; i++){
			LineSensAdOff[i] = GrEx.getAD10((grex_ad_ch)i);	// 10bit
		}
		
		// LED光らす
		outPin(LINESENS_PORT_LED, 1);
		fLineSensOn = 1;
		
	}
	
	// off時-on時でセンサ値とする
	for(uint8_t i=0; i<LINESENS_NUM_SENSE; i++){
		LineSensAd[(grex_ad_ch)i] = LineSensAdOff[(grex_ad_ch)i] - LineSensAdOn[(grex_ad_ch)i];
	}
	
}
/*********************
エンコーダアップデート
概要：
 エンコーダカウントの前回との差を計算
引数：

**********************/
void inrof2016::updateEncoder(void){
	uint16_t REncCntLast, LEncCntLast;
	
	REncCntLast = this->REncCnt;
	LEncCntLast = this->LEncCnt;
	
	// ルンバからエンコーダ値取得
	Rmb->getEncoderCounts(&this->LEncCnt, &this->REncCnt);	// ルンバから見たら逆向き
	
	if(fResetEncoder){	// 初回は同じの入れる。差をゼロにするため。
		REncCntLast = this->REncCnt;
		LEncCntLast = this->LEncCnt;
		fResetEncoder = false;
	}
	
	// 差を計算
	REncCntDiff = REncCntLast - REncCnt;	// ルンバから見たら逆向き
	LEncCntDiff = LEncCntLast - LEncCnt;	
	
	// ↑オーバーフローとかでいい感じに計算される
	// int16_t = uint16_t - uint16_t
	// int16_tで計算できる値より大きい差がでたら逆方向に回転したと判断した結果になる。
}

/*********************
タイヤ状態アップデート
概要：
 エンコーダ値から角度と角速度をアップデート
引数：

**********************/
void inrof2016::updateTyre(void){
	
	// タイヤ角速度[rps]
	RTireSpd_rps = (float)REncCntDiff/ENC_CPR/GEAR_RATIO/SAMPLETIME_CTRL_S;
	LTireSpd_rps = (float)LEncCntDiff/ENC_CPR/GEAR_RATIO/SAMPLETIME_CTRL_S;
	
	// タイヤ角度[rev]
	RTirePos_r = RTirePos_r + (float)REncCntDiff/ENC_CPR/GEAR_RATIO;
	LTirePos_r = LTirePos_r + (float)LEncCntDiff/ENC_CPR/GEAR_RATIO;
	
}
	
/*********************
自己位置更新
**********************/
void inrof2016::updateOdmetry(void){
	CalcOdmetry.REncCntDiff[1] = CalcOdmetry.REncCntDiff[0];
	CalcOdmetry.REncCntDiff[0] = REncCntDiff;
	CalcOdmetry.LEncCntDiff[1] = CalcOdmetry.LEncCntDiff[0];
	CalcOdmetry.LEncCntDiff[0] = LEncCntDiff;
	
	// マシン角度
	float ThDiff = (CalcOdmetry.REncCntDiff[0] - CalcOdmetry.LEncCntDiff[0])*TIRE_DIA_MM*PI/TIRE_TREAD_MM/ENC_CPR/GEAR_RATIO;		//[rad]
	MachinePos.dTh = ThDiff / SAMPLETIME_CTRL_S;	// [rad/s]
	MachinePos.Th = MachinePos.Th + ThDiff;			// [rad]
	
	CalcOdmetry.CosTh[1] = CalcOdmetry.CosTh[0];
	CalcOdmetry.SinTh[1] = CalcOdmetry.SinTh[0];
	CalcOdmetry.CosTh[0] = cosf(MachinePos.Th);
	CalcOdmetry.SinTh[0] = sinf(MachinePos.Th);
	
	CalcOdmetry.dth2[1] = CalcOdmetry.dth2[0];
	CalcOdmetry.dth2[0] = (CalcOdmetry.REncCntDiff[0]+CalcOdmetry.REncCntDiff[1]+CalcOdmetry.LEncCntDiff[0]+CalcOdmetry.LEncCntDiff[1])*PI/ENC_CPR/GEAR_RATIO;
	
	// マシン速度(フィールド座標)
	float XDiff = (CalcOdmetry.dth2[0] * CalcOdmetry.CosTh[0] + CalcOdmetry.dth2[1] * CalcOdmetry.CosTh[1]) * TIRE_DIA_MM / 8;
	float YDiff = (CalcOdmetry.dth2[0] * CalcOdmetry.SinTh[0] + CalcOdmetry.dth2[1] * CalcOdmetry.SinTh[1]) * TIRE_DIA_MM / 8;		// [mm]
	MachinePos.dX = XDiff / SAMPLETIME_CTRL_S;	
	MachinePos.dY = YDiff / SAMPLETIME_CTRL_S;	// [mm/s]
	// マシン位置
	MachinePos.X = MachinePos.X + XDiff;
	MachinePos.Y = MachinePos.Y + YDiff;		// [mm]
	
	// マシン速度
	this->Spd = (CalcOdmetry.REncCntDiff[0] + CalcOdmetry.LEncCntDiff[0] + CalcOdmetry.REncCntDiff[1] + CalcOdmetry.LEncCntDiff[1])/2/2/ENC_CPR/GEAR_RATIO*(TIRE_DIA_MM*PI)/SAMPLETIME_CTRL_S;
	// マシン方向
	float NormalizedTh = MachinePos.getNormalizeTh();
	if((NormalizedTh < PI/4)&&(NormalizedTh >= -PI/4))	this->MachineOrientation = ORIENT_EAST;
	else if((NormalizedTh < PI*3/4)&&(NormalizedTh >= PI/4))	this->MachineOrientation = ORIENT_NORTH;
	else if((NormalizedTh >= PI*3/4)||(NormalizedTh < -PI*3/4))	this->MachineOrientation = ORIENT_WEST;
	else if((NormalizedTh < -PI/4)&&(NormalizedTh >= -PI*3/4))	this->MachineOrientation = ORIENT_SOUTH;
}


/*********************
交点判定
概要：交点にいることを判定する
引数：
戻値：交点にいるか
**********************/
bool_t inrof2016::onCross(void){
	return ((getLineSensAd(0)<LINESENS_THRESHOLD_SIDE) && (getLineSensAd(3)<LINESENS_THRESHOLD_SIDE));
}


/*********************
交点判定
概要：交点に来たことを判定する
		新しい交点に来た時に交点インデックスを更新し、trueを返す
		それ以外ではfalseを返す。
引数：
戻値：新しい交差点に来た
**********************/
bool_t inrof2016::judgeCross(void){
	bool_t IsNewCross = false;
	position LineSensPos;
	
	// 交差点判定
	if(onCross()){	// 両端ライン触ってる
		
		if(100.0<(MachinePos - LastFindCrossPos)){	// 前見た線より100mm以上遠いところだったら(連続判定防止)
			LastFindCrossPos = MachinePos;	// 線を見つけた場所覚えておく
			IsNewCross = true;
			//どこの交差点だい？
			getLineSenseGlobalPos(&LineSensPos);	// ラインセンサのグローバルな位置を取得
			if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossStart)){		// ラインセンサと、フィールド上の交差点の位置が近ければ
				Cross = INROF_CROSS_START;

#ifdef DEBUGSCI0
Sci0.print("judgeCross start\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossRed)){
				Cross = INROF_CROSS_RED;

#ifdef DEBUGSCI0
Sci0.print("judgeCross red\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossYellow)){
				Cross = INROF_CROSS_YELLOW;
				
#ifdef DEBUGSCI0
Sci0.print("judgeCross yellow\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR>(LineSensPos - PosCrossBlue)){
				Cross = INROF_CROSS_BLUE;

#ifdef DEBUGSCI0
Sci0.print("judgeCross blue\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR>(LineSensPos - PosCrossBallFieldBegin)){
				Cross = INROF_CROSS_BALLFIELD_BEGIN;
#ifdef DEBUGSCI0
Sci0.print("judgeCross ballbegin\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR>(LineSensPos - PosCrossBallFieldEnd)){
				Cross = INROF_CROSS_BALLFIELD_END;
#ifdef DEBUGSCI0
Sci0.print("judgeCross ballend\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossGoalRed)){
				Cross = INROF_CROSS_GOAL_RED;
#ifdef DEBUGSCI0
Sci0.print("judgeCross goalred\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossGoalYellow)){
				Cross = INROF_CROSS_GOAL_YELLOW;
#ifdef DEBUGSCI0
Sci0.print("judgeCross goalyellow\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossGoalBlue)){
				Cross = INROF_CROSS_GOAL_BLUE;
#ifdef DEBUGSCI0
Sci0.print("judgeCross goalblue\r\n");
#endif
			}else{
				// どことも近くないのにライン上？？？？
				Cross = INROF_CROSS_NONE;
#ifdef DEBUGSCI0
Sci0.print("st mX%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, PosCrossYellow.X, PosCrossYellow.Y, PosCrossYellow.Th);							
Sci0.print("judgeCross dokodoko??\r\n");
#endif
			}
		}
			
		
	}else{
		
	}
	
	return IsNewCross;
}


/*********************
ボール見つけた判定
概要：	ボールを見つけた時に1になる。
		条件：ライントレース中、ボール探索フィールド上、左右センサが壁より近い値、左右斜めセンサがマシンぶつかるくらい近い値
引数：
戻値：新しいボール発見した
**********************/
bool_t inrof2016::judgeBallFind(void){
	bool_t BallExist_last = this->BallExist;
	
	this->BallExist = false;	// リセット
	
	//if((INROF_CROSS_BALLFIELD_BEGIN == Cross) || (INROF_CROSS_BALLFIELD_END == Cross)){	// ボールゾーン内
	//if( (1500.0 > MachinePos.Y) && ( (PI/6 > fabs(MachinePos.getNormalizeTh()-PI/2))||(PI/6 > fabs(MachinePos.getNormalizeTh()+PI/2) ) ) ){		// ボールゾーン内, ほぼ上下向いている
	//if( 600.0 < MachinePos.X ){		// ボールゾーン内
						// 変なとこに転がってた時用に全域にしたい。
		
		// 左クロスセンサ
		
		if( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdLeft->getDistance())		// 指定した距離より近い
			&& (PSD_MIN < PsdLeft->getDistance())				// 有効範囲
			&& PsdLeft->isObjectBallArea(&MachinePos)			// ボールはフィールド内(センサ値取得できてなければfalse返る)
		){
			this->BallExist = true;		
		}
		// 右クロスセンサ
		if( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdRight->getDistance())		// 指定した距離より近い
			&& (PSD_MIN < PsdRight->getDistance())				// 有効範囲
			&& PsdRight->isObjectBallArea(&MachinePos)			// ボールはフィールド内(センサ値取得できてなければfalse返る)
		){
			this->BallExist = true;		
		}
		
		// 去年は中央は使ってなかった。
		// 中央センサ
		if( (BALLFIND_MAXDISTANCE_FRONT > PsdCenter->getDistance())		// 指定した距離より近い
			&& (PSD_MIN < PsdCenter->getDistance())				// 有効範囲
			&& PsdCenter->isObjectBallArea(&MachinePos)			// ボールはフィールド内(センサ値取得できてなければfalse返る) // スタートゾーンで動作確認したいため無効
		){
			this->BallExist = true;		
		}
		
	//}
		//}
		
	if(BallExist_last != this->BallExist){	// 結果変化時
		this->Rmb->outLedDustFull(this->BallExist);	// ボールあるときはルンバのゴミマークつける
	}
	
	this->Rmb->outLedDebris(this->BallFindEnable);	// 探索有効中
		
	if(BallFindEnable){	// enableのときは結果そのまま返す
		return BallExist;
	}
	
	return false;
}
	
	
/*********************
ボールセンサをリセット
概要：	ToFのセンサが固まっちゃうんでリセット掛ける
引数：
戻値：
**********************/
void inrof2016::resetBallSensors(void){
//	Inrof.PsdLeft->resetModule();
//	Inrof.PsdCenter->resetModule();
//	Inrof.PsdRight->resetModule();
	// なぜか2回送らないと再開しない
//	Inrof.PsdLeft->resetModule();
//	Inrof.PsdCenter->resetModule();
//	Inrof.PsdRight->resetModule();

#ifdef DEBUGSCI0
	Sci0.print("Reset Ball Sensors\r\n");
#endif
	initBallSensor();
}
	
void inrof2016::judgeRunOrient(void){
	
	// ほぼ確実なときだけ更新する
	if(MachinePos.Y < 1450){
		if((MachinePos.X < 100.0F) && (MachinePos.X > -100.0F)){	// 左のライン上にいる
			if((MachinePos.getNormalizeTh() > PI/4)&&(MachinePos.getNormalizeTh() < PI/4*3)){
				LineTraceDir = INROF_LINEDIR_FORWARD;
			}else if((MachinePos.getNormalizeTh() < -PI/4)&&(MachinePos.getNormalizeTh() > -PI/4*3)){
				LineTraceDir = INROF_LINEDIR_BACKWARD;
			}
		}else if((MachinePos.X < 1240.0F) && (MachinePos.X > 940.0F)){	// 右のライン上にいる
			if((MachinePos.getNormalizeTh() > 0)&&(MachinePos.getNormalizeTh() < PI)){			// 北向いてる
				LineTraceDir = INROF_LINEDIR_BACKWARD;
			//}else if((MachinePos.getNormalizeTh() < -PI/4)&&(MachinePos.getNormalizeTh() > -PI/4*3)){	// 南向いてる
			}else{	// 南向いてる
				LineTraceDir = INROF_LINEDIR_FORWARD;
			}
		}
	}else{	// 上のライン上にいる
		
		if((MachinePos.getNormalizeTh() < PI/2)&&(MachinePos.getNormalizeTh() > -PI/2)){			// 東向いてる
			LineTraceDir = INROF_LINEDIR_FORWARD;
		//}else if((MachinePos.getNormalizeTh() > PI/4*3)||(MachinePos.getNormalizeTh() < -PI/4*3)){	// 西向いてる
		}else{	// 西向いてる
			LineTraceDir = INROF_LINEDIR_BACKWARD;
		}
	}
	
}


// マシン位置修正
// 交差点座標を使う

void inrof2016::adjustMachinePos(void){
	
	if(IsCrossing){	// 交差点入ったなう
		switch(Cross){
		case INROF_CROSS_YELLOW:	// 黄色ゾーンの交差点なう
			if(ORIENT_NORTH==MachineOrientation){			// 北向いてる
#ifdef DEBUGSCI0
				outPin(GR_LED2, 1);
				Sci0.print("adjust Yel North mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossYellow.X;	
				MachinePos.Y = PosCrossYellow.Y - LINESENS_POS_X - 9;	// 位置とラインセンサの位置とライン幅
				MachinePos.Th = PI/2;
				
#ifdef DEBUGSCI0
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}else if(ORIENT_SOUTH==MachineOrientation){	// 南向いてる
				outPin(GR_LED2, 1);
				
#ifdef DEBUGSCI0
				Sci0.print("adjust Yel South mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossYellow.X;
				MachinePos.Y = PosCrossYellow.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;
				
#ifdef DEBUGSCI0
			Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif			
			}
			break;
		case INROF_CROSS_BLUE:	// 青ゾーンの交差点なう
			if(ORIENT_WEST==MachineOrientation){		// 西向いてる
				outPin(GR_LED2, 1);
#ifdef DEBUGSCI0
				Sci0.print("adjust Bul West mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossBlue.X + LINESENS_POS_X + 9;	
				MachinePos.Y = PosCrossBlue.Y;
				MachinePos.Th = PI;

#ifdef DEBUGSCI0	
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}else if(ORIENT_EAST==MachineOrientation){	// 東向いてる	// 曲がって直後なので注意
				outPin(GR_LED2, 1);
#ifdef DEBUGSCI0
				Sci0.print("adjust Bul East mX%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossBlue.X - LINESENS_POS_X - 9;	
				MachinePos.Y = PosCrossBlue.Y;
				MachinePos.Th = 0;

#ifdef DEBUGSCI0		
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}
			break;
		case INROF_CROSS_RED:	// 赤ゾーンの交差点なう
			if(ORIENT_SOUTH==MachineOrientation){			// 南向いてる

#ifdef DEBUGSCI0
				outPin(GR_LED2, 1);
				Sci0.print("adjust Red South mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossRed.X;	
				MachinePos.Y = PosCrossRed.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;
				
#ifdef DEBUGSCI0
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif			
			}else if(ORIENT_NORTH==MachineOrientation){			// 北向いてる
				outPin(GR_LED2, 1);
#ifdef DEBUGSCI0
				Sci0.print("adjust Red North mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossRed.X;	
				MachinePos.Y = PosCrossRed.Y - LINESENS_POS_X - 9;
				MachinePos.Th = PI/2;

#ifdef DEBUGSCI0		
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}
			break;
		case INROF_CROSS_BALLFIELD_BEGIN:	// ボールゾーン開始の交差点なう
			if(ORIENT_NORTH==MachineOrientation){			// 北向いてる
				outPin(GR_LED2, 1);

#ifdef DEBUGSCI0
				Sci0.print("adjust BalSt North mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
	#endif
				MachinePos.X = PosCrossBallFieldBegin.X;	
				MachinePos.Y = PosCrossBallFieldBegin.Y - LINESENS_POS_X - 9;
				MachinePos.Th = PI/2;

#ifdef DEBUGSCI0
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}else if(ORIENT_SOUTH==MachineOrientation){	// 南向いてる	// 曲がって直後なので注意

#ifdef DEBUGSCI0
outPin(GR_LED2, 1);
				Sci0.print("adjust BalSt South mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossBallFieldBegin.X;	
				MachinePos.Y = PosCrossBallFieldBegin.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;

#ifdef DEBUGSCI0
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}
			break;
		case INROF_CROSS_BALLFIELD_END:	// ボールゾーン終わりの交差点なう
			if(ORIENT_SOUTH==MachineOrientation){	// 南向いてる
#ifdef DEBUGSCI0
				outPin(GR_LED2, 1);
				Sci0.print("adjust BalEnd South mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossBallFieldEnd.X;
				MachinePos.Y = PosCrossBallFieldEnd.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;

#ifdef DEBUGSCI0		
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}
			break;
		case INROF_CROSS_START:	// ボールゾーン終わりの交差点なう
			if(ORIENT_SOUTH==MachineOrientation){	// 南向いてる

#ifdef DEBUGSCI0
				outPin(GR_LED2, 1);
				Sci0.print("adjust START South mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.Y = PosCrossStart.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;

#ifdef DEBUGSCI0				
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}else if(ORIENT_NORTH==MachineOrientation){	// 北向いてる

#ifdef DEBUGSCI0
			outPin(GR_LED2, 1);
				Sci0.print("adjust START South mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.Y = PosCrossStart.Y + LINESENS_POS_X - 9;
				MachinePos.Th = PI/2;

#ifdef DEBUGSCI0			
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}
			break;
		}
	}else{
	//	outPin(GR_LED2, 0);
	}
	
}




//状態管理
int8_t inrof2016::manageState(void){
	
	// ひたすらボールとってゴールいくだけだから今のとこいらないなぁ。
	
	fFinishManageAction = manageAction(State);
	return 0;
}
	
// 動作管理
// 引数
//  目的地とか探索モードとか
bool_t inrof2016::manageAction(inrof_state State){
	
	
	switch(State){
	case INROF_IDLE:
		SpdTgt = 0;
		AngVelTgt = 0;
		break;
	case INROF_TANSAKU:
		return manageAction_tansaku();
		break;
	case INROF_TOGOAL:
		return manageAction_goToGoal();
		break;
	case INROF_TOSTART:
		return manageAction_goToStart();
		break;
	case INROF_LINECALIB:
		return manageAction_calibLine();
		break;
	}
	
	return false;
}



/*********************
探索モード処理
概要：
 どこにいても探索場へ行き、探索する。
 終了条件
  ボールを取ってゴールの方向く
 戻値： bool_t 動作完了
 
基本はINROF_ACTION_LINEの状態で探索。

**********************/
bool_t inrof2016::manageAction_tansaku(void){
	
	bool_t SeqInitSub = false;
	bool_t SeqInit = false;
	
	
	// 状態遷移(交点)
	if(IsCrossing){
		if(Cross==INROF_CROSS_BALLFIELD_END){	// フィールドの端にいたらちょっと下がって90°回転する
			// 180°回転する
			ActionState = INROF_ACTION_TURN;
			ManageActionTansakuState_BallFind_turn = 0;	// ちょっと下がる
			ManageActionTansakuState_BallFind_turn_Last = 0xFF;
			MachinePosActionManageSub = MachinePos;
		}else if(Cross==INROF_CROSS_BALLFIELD_BEGIN){
			if((INROF_ACTION_LINE == ActionState) && (fabs(MachinePos.getNormalizeTh() - PI/2)) < 0.5 ){	// ライントレースしてて上むいてる時にボールフィールドの上の線を越えた
				ActionState = INROF_ACTION_TURN;
				ManageActionTansakuState_BallFind_turn = 1;	// 下がらなくて良いので1から
				ManageActionTansakuState_BallFind_turn_Last = 0xFF;
				MachinePosActionManageSub = MachinePos;
			}
		}else if(Cross==INROF_CROSS_START){
			
		}
		if((INROF_ACTION_LINE == ActionState) && ((Cross==INROF_CROSS_START) || (Cross==INROF_CROSS_YELLOW) || (Cross==INROF_CROSS_RED))){
			if(fabs(MachinePos.getNormalizeTh() + PI/2) < 0.5 ){	// ライントレースしてて下むいてる時にスタートゾーン	// 何故か来ちゃった的な
				ActionState = INROF_ACTION_TURN;
				ManageActionTansakuState_BallFind_turn = 1;	// 下がらなくて良いので1から
				MachinePosActionManageSub = MachinePos;
			}
		}
	}
	
	
	
	// フィールド端っこで壁にぶつかりそう
	// ボールセンサで検知
	
	if((INROF_ACTION_LINE == ActionState) && (Cross==INROF_CROSS_BALLFIELD_BEGIN)){
		if(MachinePos.Y < 230.0F){
			float PosXw,PosYw;
			bool_t fFieldEnd = false;
			bool_t valid;
			
			valid = !PsdCenter->getGlobalObjPos(&MachinePos, &PosXw, &PosYw);
			if(valid){
				fFieldEnd |= (PosYw<(FIELD_INROF_CROSS_BALLFIELD_END_Y-10.0F));	// ボールゾーン外に物体有り
			}
			valid = !PsdRight->getGlobalObjPos(&MachinePos, &PosXw, &PosYw);
			if(valid){
				fFieldEnd |= (PosYw<(FIELD_INROF_CROSS_BALLFIELD_END_Y-10.0F));
			}
			valid = !PsdLeft->getGlobalObjPos(&MachinePos, &PosXw, &PosYw);
			if(valid){
				fFieldEnd |= (PosYw<(FIELD_INROF_CROSS_BALLFIELD_END_Y-10.0F));
			}
			
			if( fFieldEnd ){
				// 180°回転する
				ActionState = INROF_ACTION_TURN;
				ManageActionTansakuState_BallFind_turn = 0;	// ちょっと下がる
				ManageActionTansakuState_BallFind_turn_Last = 0xFF;
				MachinePosActionManageSub = MachinePos;
			}
		}
	}
	
	// 状態遷移(ボール見つける処理)
	if(IsBallFinding){	// ボール見つけた
#ifdef DEBUGSCI0
	Sci0.print("ball find.\r\n");
#endif
		ActionState = INROF_ACTION_BALLGET;		// ボール取得ステートへ移行
		BallFindEnable = false;					// ボール探索やめる
		MachinePosActionManage = MachinePos;	// 
#ifdef DEBUGSCI0
	Sci0.print("nowpos %f %f %f\r\n",MachinePosActionManage.X, MachinePosActionManage.Y, MachinePosActionManage.Th);
	#endif
		MachinePosActionManageSub = MachinePos;
		ManageActionTansakuState_BallFind = 0;	// ボール探索用ステート初期化
		ManageActionTansakuState_BallFind_Last = 0xFF;
		// どのセンサ？
		BallFindDistance = 200;
		/*
		if( (BALLFIND_MAXDISTANCE_SIDE > PsdLeftSide->getDistance()) && PsdLeftSide->isObjectBallArea(&MachinePos) ){
			if(BallFindDistance > PsdLeftSide->getDistance()){	// 他のセンサより近ければそれを適用
				BallDir = 1;	// 左回り
				BallFindDistance = PsdLeftSide->getDistance();
			}
		}
		*/
		if( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdRight->getDistance()) && PsdRight->isObjectBallArea(&MachinePos) ){
		// 左横か、右クロスだったら左回りする

#ifdef DEBUGSCI0
Sci0.print("Ball find RightSensor\r\n");
#endif
			if(BallFindDistance > PsdRight->getDistance()){	// 他のセンサより近ければそれを適用
				BallDir = 1;	// 右
				BallFindDistance = PsdRight->getDistance() * 0.5;
			}
		}
		if( (BALLFIND_MAXDISTANCE_FRONT > PsdCenter->getDistance()) && PsdCenter->isObjectBallArea(&MachinePos) ){
		// センターはとりあえず左回りする
			
#ifdef DEBUGSCI0
Sci0.print("Ball find CenterSensor\r\n");
#endif
			if(BallFindDistance > PsdCenter->getDistance()){	// 他のセンサより近ければそれを適用
				BallDir = 1;
				BallFindDistance = PsdCenter->getDistance();
			}
		}
		if ( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdLeft->getDistance()) && PsdLeft->isObjectBallArea(&MachinePos) ){
		// 右横か、左クロスだったら右回りする

#ifdef DEBUGSCI0
			Sci0.print("Ball find LeftSensor\r\n");
#endif
if(BallFindDistance > PsdLeft->getDistance()){	
				BallDir = 0;	// 右
				BallFindDistance = PsdLeft->getDistance() * 0.5;
			}
		}
	}
	
	if(ActionStateLast != ActionState){
		SeqInit = true;
	}
	ActionStateLast = ActionState;
	
	
	if(SeqInit){
		switch(ActionState){
		case INROF_ACTION_LINE:	// サーチ！
			Rmb->outLedDispAscii("Srch");
			break;
		case INROF_ACTION_BALLGET:	// ボール見つけた。取る
			Rmb->outLedDispAscii("Find");
			break;
		
		}
#ifdef DEBUGSCI0
		Sci0.print("%08d INROF_manageAction_tansaku:newSeq %d\r\n", getTime_ms(), ActionState);
		#endif
	}
	
	
	// 動作
	switch(ActionState){
	case INROF_ACTION_IDLE:		// まずはここに入る
		ActionState = INROF_ACTION_LINE;
		setTgtVel_LineTrace();
		ManageActionTansakuState_BallFind = 0;
		
		// ボール探索中の姿勢へ
		InrofUpper.reqArmToCenter();
		
		break;
	case INROF_ACTION_TURN:		// 180°回る
		
		if(ManageActionTansakuState_BallFind_turn!=ManageActionTansakuState_BallFind_turn_Last){	// 初回処理
			SeqInitSub = true;
			resetBallSensors();
#ifdef DEBUGSCI0
			Sci0.print("%08d INROF_ACTION_TURN:newSeq %d\r\n", getTime_ms(), ManageActionTansakuState_BallFind);
		#endif
		}
		ManageActionTansakuState_BallFind_turn_Last = ManageActionTansakuState_BallFind_turn;
		
		//adjustMachinePos();	// ライントレースではないけど交差点で判定することが多いので
		switch(ManageActionTansakuState_BallFind_turn){
		case 0:	// ちょっと下がる
			if(SeqInitSub){
#ifdef DEBUGSCI0
				Sci0.print("Turn\r\n");
				#endif
			}
			if(80.0 < (MachinePos-MachinePosActionManageSub)){	// 80mm以上離れた
				ManageActionTansakuState_BallFind_turn ++;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{		// 離れるまで下がる
				this->SpdTgt = -60;		// [mmps]
				this->AngVelTgt = 0;	// [rps]
				BallFindEnable = false;
			}
			break;
		case 1:	//180度旋回
			if(fabs(MachinePos.Th - (MachinePosActionManageSub.Th + PI)) < 0.1){
				//fFinishAction = 1;				// 180°回ったので次のステートへ
				ManageActionTansakuState_BallFind_turn = 0;
				ActionState = INROF_ACTION_LINE;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [radps]
				BallFindEnable = true;
			}else{		// 180°回るまで回転
				this->SpdTgt = 0;		// [mmps]
				this->AngVelTgt = 0.10F * ((MachinePosActionManageSub.Th + PI) - MachinePos.Th);	// [rad/s]
				
				// 速すぎないようにする
				if(0.05F<this->AngVelTgt){
					this->AngVelTgt = 0.05;
				}else if(-0.05F>AngVelTgt){
					this->AngVelTgt = -0.05;
				}
			
				BallFindEnable = false;
			}
			break;
		}
		break;
	case INROF_ACTION_BALLGET:	// ボールがいたので掴まえる
		if(ManageActionTansakuState_BallFind!=ManageActionTansakuState_BallFind_Last){	// 初回処理
			SeqInitSub = true;
			//resetBallSensors();
			#ifdef DEBUGSCI0
			Sci0.print("%08dms INROF_ACTION_BALLGET:newSeq %d\r\n", getTime_ms(), ManageActionTansakuState_BallFind);
			#endif
		}
		ManageActionTansakuState_BallFind_Last = ManageActionTansakuState_BallFind;
		
		switch(ManageActionTansakuState_BallFind){
		case 0:		// ボールなくなるまで反対向く(もともと目の前にボールがあった時用)
			if(SeqInitSub){
				MachinePosActionManageSub = MachinePos;
			}
			
			if(fabs(MachinePos.Th - MachinePosActionManageSub.Th) > PI/2){		// 90度以上回ったらやめて探索再開
				ManageActionTansakuState_BallFind = 40;
			}else{
			//if(UPPER_STATE_IDLE != InrofUpper.getState()){	// アームが安全な状態か確認			
			//	InrofUpper.reqArmToCenter();		// じゃなければ探索状態の位置にする
			//}else{
				if((BallFindDistance+30 < PsdCenter->getDistance()) || (0>PsdCenter->getDistance())){ 	// 指定値より大きい or 無効値
					ManageActionTansakuState_BallFind = 1;				// 20°回ったので次のステートへ
					this->SpdTgt = 0;		// [mmps]
					this->AngVelTgt = 0.0;	// [rps]
				}else{
					this->SpdTgt = 0;		// [mmps]
					if(BallDir){	// ボールは左方向
						this->AngVelTgt = -0.02;	// 右回転[rps]
					}else{
						this->AngVelTgt = 0.02;	// [rps]
					}
				}
			//}
			}
			break;
			
		case 1:		// ボール見つけるまで回る
		
			if(fabs(MachinePos.Th - MachinePosActionManageSub.Th) > PI/2){		// 90度以上回ったらやめて探索再開
				ManageActionTansakuState_BallFind = 40;
				
			//}else if(( BallFindDistance+30 > PsdCenter->getDistance()) && PsdCenter->isObjectBallArea()){		//指定値より小さい and 有効範囲
			}else if(( (BallFindDistance+30) > PsdCenter->getDistance()) && (0 < PsdCenter->getDistance())){		// ボール見つけた(指定値より小さい and 有効範囲)
			//}else if(( BallFindDistance+30 > PsdCenter->getDistance())){		//指定値より小さい -> 無効値入ったら反応しちゃうのでボツ
					// 対象のボール位置より小さければ見つけたと判定。ちょっと余裕持って+30mmしてる
				ManageActionTansakuState_BallFind = 2;				// ボール見つけたので次のステートへ
				BallFindTh = MachinePos.Th;
				
				
			}else{
				if(BallDir){	// ボールは左方向
					//旋回
					this->SpdTgt = 0;		// [mmps]
					this->AngVelTgt = 0.02;	// [rps]
				}else{			// ボールは右方向
				//-旋回
					this->SpdTgt = 0;		// [mmps]
					this->AngVelTgt = -0.02;	// [rps]
				}
			}
			break;
			
			
		case 2:		// フロントセンサでボール見つけた。ボールなくなるまで更に回るよ
			if(fabs(MachinePos.Th - MachinePosActionManageSub.Th) > (PI*1/2)){		// 90度以上回ったらやめて探索再開
				ManageActionTansakuState_BallFind = 40;
				
			}else if((BallFindDistance+30 < PsdCenter->getDistance()) || (0>PsdCenter->getDistance())){ 	// 指定値より遠い(違うボールを見ないため) or 無効値(見えないくらい遠い)
				ManageActionTansakuState_BallFind = 3;				// ボールなくなったので次のステートへ
				BallFindTh = (MachinePos.Th + BallFindTh)/2;
				
				// ToF測距センサ用調整
				if(BallDir){	// 調整 マシンの左にある
				//	BallFindTh = BallFindTh + 0.05F;	// ちょい左
				}else{		// マシンの右にある
				//	BallFindTh = BallFindTh - 0.10F;	// ちょい右
				}
				
				this->SpdTgt = 0;		// [mmps]
				this->AngVelTgt = (BallFindTh - MachinePos.Th)/12;	// [rps]
				
			}else{
				if(BallDir){	// ボールは左方向
					//旋回
					this->SpdTgt = 0;		// [mmps]
					this->AngVelTgt = 0.02;	// [rps]
				}else{			// ボールは右方向
					this->SpdTgt = 0;		// [mmps]
					this->AngVelTgt = -0.02;	// [rps]
				}
			}
			break;
			
			
		case 3:		// ボールなくなるまで回った。BallFindThがちょうどボールの真ん中だよ。たぶんね。とりあえずそっち向くよ。
			//旋回
			if(fabs(BallFindTh - MachinePos.Th) < 0.02){ 	// ボールのほう向いた
				ManageActionTansakuState_BallFind = 4;				// ボールのほう向いたので次のステートへ
				this->SpdTgt = 0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{
				this->SpdTgt = 0;		// [mmps]
				//if(BallDir){
				//	this->AngVelTgt = -0.1 * (BallFindTh - MachinePos.Th);
				//}else{
					this->AngVelTgt = 0.2 * (BallFindTh - MachinePos.Th);	// [rps]
				//}
			}
			
			if(this->AngVelTgt > 0.02){
				this->AngVelTgt = 0.02;
			}else if(this->AngVelTgt< -0.02){
				this->AngVelTgt = -0.02;
			}
			break;
			
			
		case 4:		// 前後距離合わせるよ
			if(SeqInitSub){
				MachinePosActionManageSub = MachinePos;
			}
			//旋回
			if((BALLGET_MAXDISTANCE_FRONT<PsdCenter->getDistance()) || (0>PsdCenter->getDistance() ) || (MachinePosActionManageSub-MachinePos)>300){
				// ボール見失った or 200mm以上下がった(センサ固まったとかで)
				ManageActionTansakuState_BallFind = 40;
			}else if((fabs(PsdCenter->getDistance() - BALLGET_DISTANCE_FRONT_ARMDOWN) < 5.0F) && (InrofUpper.getState() == UPPER_STATE_IDLE)){ 
				// ボールとの距離合わせた && 上半身ボール取得可能状態
				
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
				// アーム下ろす
				InrofUpper.reqBallGetPre(PI/2);	// 前方
				ManageActionTansakuState_BallFind ++;				// 距離あわせたので次のステートへ
				StartTimeActionManage = getTime_ms();	// 時刻
								
			}else{
				// フロント距離センサでボールとの距離を合わせる
				this->SpdTgt = 5 * (PsdCenter->getDistance() - BALLGET_DISTANCE_FRONT_ARMDOWN);		// [mmps]
				// 最低速度規制？
				if(100<SpdTgt){
					this->SpdTgt = 100;
				}else if(-100>SpdTgt){
					this->SpdTgt = -100;
				}
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
			
			
		case 5:		// アーム下ろすの待ち
			if((UPPER_STATE_IDLE == InrofUpper.getState()) || (StartTimeActionManage + 12000 < getTime_ms()) ){ 	// サーボ下げた
				ManageActionTansakuState_BallFind ++;
				MachinePosActionManageSub = MachinePos;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
			}
			break;
			
			
		case 6:		// 前進して、ボール掴む
			if((BALLGET_DISTANCE_FRONT_ARMDOWN-BALLGET_DISTANCE_FRONT_GRAB) < (MachinePosActionManageSub - MachinePos)){ 	// アーム掴む位置まで進む前進完
				ManageActionTansakuState_BallFind = 9;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				StartTimeActionManage = getTime_ms();	// 時刻
				
				// アームつかむ
				InrofUpper.reqBallGet();
				
			}else{
				this->SpdTgt = 1.0*((BALLGET_DISTANCE_FRONT_ARMDOWN) - (MachinePosActionManageSub - MachinePos));		// [mmps]
				//SpdTgt = 30;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
			
		case 9:		// ボール持ち上げて移動可能な状態になるのを待つ
			if((UPPER_STATE_BALL_GET != InrofUpper.getState()) || ((StartTimeActionManage + 5000) < getTime_ms())){ 	// アーム持ち上げた
				ManageActionTansakuState_BallFind ++;
#ifdef DEBUGSCI0
	Sci0.print("base: ballget and back\r\n");
	#endif
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
				// フラグリセット
				fNextAction = false;
				
			}else{
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
			}
			break;
			
			
		case 10:	// ボールとった！！！！ラインへ戻るよ
			if(SeqInitSub){
#ifdef DEBUGSCI0
				Sci0.print("TgtPos %f, %f, %f\r\n", MachinePosActionManage.X, MachinePosActionManage.Y, MachinePosActionManage.Th);
				Sci0.print("NowPos %f, %f, %f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
	#endif
			}
			if( fNextAction ){	// ライン上へ戻った
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
				// ボールとったことにする。
				FieldBall--;
				
				
				// ここで、ボールを十分確保した
				// もしくは、フィールドにボールがない
				// 場合は、ボールを置きに帰る
				if(InrofUpper.isBallFull() || (FieldBall<=0)){
#ifdef DEBUGSCI0
	Sci0.print("base: to goal\r\n");
	Sci0.print("base: turn North\r\n");
	#endif
					ManageActionTansakuState_BallFind = 11;
				}else{
#ifdef DEBUGSCI0
	Sci0.print("base: re search\r\n");
	#endif
					// そうでない場合は、
					// 元の向きを向いて再び探索する。
					ManageActionTansakuState_BallFind = 41;
				}
				
				fNextAction = false; // フラグリセット
			}else{
				// まっすぐバック
				/*
				if((MachinePos.getNormalizeTh() > PI/2) || (MachinePos.getNormalizeTh() < -PI/2)){	// 左向いてる
					SpdTgt = -3 * (MachinePosActionManage.X - MachinePos.X);		// [mmps]
				}else{																				// 右向いてる
					SpdTgt = 3 * (MachinePosActionManage.X - MachinePos.X);
				}*/
				if(ORIENT_WEST == MachineOrientation){	// 西向いてる
					this->SpdTgt = -3.0F * (MachinePosActionManage.X - MachinePos.X);		// [mmps]
					if(fabs(MachinePosActionManage.X - MachinePos.X) < 5.0){
					//if(MachinePosActionManage.X < MachinePos.X){
						fNextAction = true;
					}
				}else if(ORIENT_EAST == MachineOrientation){	// 東																		// 右向いてる
					this->SpdTgt = 3.0F * (MachinePosActionManage.X - MachinePos.X);
					if(fabs(MachinePosActionManage.X - MachinePos.X) < 5.0){
					//if(fabs(MachinePosActionManage.X - MachinePos.X) < 5.0){
						fNextAction = true;
					}
				}else if(ORIENT_SOUTH == MachineOrientation){	// 南向いてる
					this->SpdTgt = -3.0F * (MachinePosActionManage.Y - MachinePos.Y);		// [mmps]
					if(fabs(MachinePosActionManage.Y - MachinePos.Y) < 5.0){
					//if(MachinePosActionManage.Y < MachinePos.Y){
						fNextAction = true;
					}
				}else{											// 北向いてる
					this->SpdTgt = 3.0F * (MachinePosActionManage.Y - MachinePos.Y);
					if(fabs(MachinePosActionManage.Y - MachinePos.Y) < 5.0){
					//if(MachinePosActionManage.Y > MachinePos.Y){
						fNextAction = true;
					}
				}
				
				if(this->SpdTgt>100.0F){
					this->SpdTgt = 100.0F;
				}else if(this->SpdTgt< -100.0F){
					this->SpdTgt = -100.0F;
				}
				
				
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
		case 11:		// 北向く
			if( fabs(MachinePos.getNormalizeTh()-PI/2) < 0.2F){	// だいたい北向いた[rad]
				ManageActionTansakuState_BallFind ++;
#ifdef DEBUGSCI0
	Sci0.print("base: north\r\n");
#endif
			}else{
				// 北を向く(P制御)
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.20 * (PI/2 - ((MachinePos.getNormalizeTh()< -PI/2)?MachinePos.getNormalizeTh()+2*PI:MachinePos.getNormalizeTh()));	// [rps]
				//回転速度に制限
				if(0.04<AngVelTgt){
					this->AngVelTgt = 0.04;
				}else if(-0.04>AngVelTgt){
					this->AngVelTgt = -0.04;
				}
				
				if(0<AngVelTgt){	// 次のシーケンスでの回転向き設定
					BallDir = 1;
				}else{
					BallDir = 0;
				}
			}
			
			break;
		
		case 12:	// ライン乗るまでさらに回る
			if((getLineSensAd(1)<LINESENS_THRESHOLD_CENTER)||(getLineSensAd(2)<LINESENS_THRESHOLD_CENTER)){	// 中央２つのセンサいずれかが黒くなるまで
				
				ManageActionTansakuState_BallFind = 13;
				fNextAction = false; // フラグリセット
			}else{
				//ラインなければまわる
				//  回りすぎちゃったらどうしよう
				this->SpdTgt = 0.0;		// [mmps]
				if(BallDir){	// 左のボールをとった
					// もっと回る(左回り)
					this->AngVelTgt = 0.02;
				}else{
					// もっと回る(右回り)
					this->AngVelTgt = -0.02;
				}
			}
			break;
		
		case 13:	// ボール判定完了待ち。ゴールへ移動処理
			//if((StartTimeActionManage + 2000) < getTime_ms()){	// ボール色センサ起動して2000ms以上経っていたらちぇっく
			if(UPPER_STATE_IDLE==InrofUpper.getState()){	// ボール色センサ起動して2000ms以上経っていたらちぇっく
			//ボール色チェック
				//BallColorFixed = BallColor;
				
				// ボール持ってなかったら！！！
				// また探索！！！
				if(InrofUpper.numBall()){
					// ボール持ってるから置きに行く
					this->SpdTgt = 0.0;		// [mmps]
					this->AngVelTgt = 0.0;	// [rps]
					
					ManageActionLineTraceDir = LineTraceDir;
					setState(INROF_TOGOAL);	// 次の状態へ。ボールを置きに行く。
					ManageActionTansakuState_BallFind = 0;
				}else{
					// もう無いってことだからto startかな
					setState(INROF_TOSTART);	// 次の状態へ。ボールを置きに行く。
					
					ManageActionTansakuState_BallFind = 0x00;
				}
				
				//fJudgeColor = false;	// 色判定終了
			}else{
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;
			}
			break;
// 再探索用ステート	
//  まだボール乗っかる
//  ボール見つけたけど見失った
		case 40:	//ボール見つけたと思ったけど見失った
			// ライン上に戻る
			if( fNextAction ){
			
				ManageActionTansakuState_BallFind ++;	// 回転する
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
				fNextAction = false; // フラグリセット
			}else{
				if(ORIENT_WEST == MachineOrientation){	// 西向いてる
					this->SpdTgt = -3 * (MachinePosActionManage.X - MachinePos.X);		// [mmps]
					if(fabs(MachinePosActionManage.X - MachinePos.X) < 5.0){
						fNextAction = true;
					}
				}else if(ORIENT_EAST == MachineOrientation){	// 東																		// 右向いてる
					this->SpdTgt = 3 * (MachinePosActionManage.X - MachinePos.X);
					if(fabs(MachinePosActionManage.X - MachinePos.X) < 5.0){
						fNextAction = true;
					}
				}else if(ORIENT_SOUTH == MachineOrientation){	// 南向いてる
					this->SpdTgt = -3 * (MachinePosActionManage.Y - MachinePos.Y);		// [mmps]
					if(fabs(MachinePosActionManage.Y - MachinePos.Y) < 5.0){
						fNextAction = true;
					}
				}else{											// 北向いてる
					this->SpdTgt = 3 * (MachinePosActionManage.Y - MachinePos.Y);
					if(fabs(MachinePosActionManage.Y - MachinePos.Y) < 5.0){
						fNextAction = true;
					}
				}
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
		case 41:	//続・ボール見つけたと思ったけど違った
			// 向きを元に戻して探索再開
			if(0.5 > fabs(MachinePosActionManage.Th - MachinePos.Th)){
				ManageActionTansakuState_BallFind ++;
			}else{
				SpdTgt = 0.0;		// [mmps]
				AngVelTgt = 0.15*(MachinePosActionManage.Th - MachinePos.Th);	// [rps]
				
				// 速すぎないようにする
				if(0.04F<this->AngVelTgt){
					this->AngVelTgt = 0.04;
				}else if(-0.04F>AngVelTgt){
					this->AngVelTgt = -0.04;
				}
			}
			break;
		case 42:	// ライン乗るまでさらに回る
			if((getLineSensAd(1)<LINESENS_THRESHOLD_CENTER)||(getLineSensAd(2)<LINESENS_THRESHOLD_CENTER)){	// 中央２つのセンサいずれかが黒くなるまで
				
				ActionState = INROF_ACTION_IDLE;	// 引き続き探索します(IDLE行って初期化してから)
				ManageActionTansakuState_BallFind = 0;
				ManageActionTansakuState_BallFind_Last = 0xFF;
			}else{
				//ラインなければまわる
				//  回りすぎちゃったらどうしよう
				this->SpdTgt = 0.0;		// [mmps]
				if(BallDir){	// 左のボールをとった
					// もっと回る(右回り)
					this->AngVelTgt = -0.02;
				}else{
					// もっと回る(左回り)
					this->AngVelTgt = 0.02;
				}
			}
			break;
			
			
			
			
		case 99:	// 一時停止	
			if(StartTimeActionManage + 1000 < getTime_ms() ){
				//ManageActionTansakuState_BallFind = ManageActionTansakuState_BallFind_last+1;
			}else{
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
		}
		break;
		
		
	case INROF_ACTION_LINE:	// ライントレース
		float x, y;
		
		if(SeqInit){
#ifdef DEBUGSCI0
			Sci0.print("%08d LineTrace \r\n", getTime_ms());
#endif
			StartTimeActionManage = getTime_ms();
		}
		
		
		PsdCenter->getGlobalPos(&MachinePos, &x, &y);
		if((x > 580.0F) && (y < 1500.0F)){	// フィールド上ではゆっくり＆ボール探索有効
			BallFindEnable = true;
			setTgtVel_LineTrace(RMB_VEL_NORMAL-50.0F);
		}else{
			BallFindEnable = false;	// フィールド外ではボール探索しない
			setTgtVel_LineTrace(RMB_VEL_NORMAL);
		}
		
		
		// 3秒に一回やる
		if(StartTimeActionManage + 3000 < getTime_ms()){
			resetBallSensors();
			StartTimeActionManage = getTime_ms();
		}	
		
		
		// ボール踏んだ！！！
		if(Rmb->isCliff()){
			ActionState = INROF_ACTION_OVERBALL;
			ManageActionTansakuState_BallFind = 0;
			ManageActionTansakuState_BallFind_Last = 0xFF;
			ManageActionTansakuState_BallOver = 0;
		}
		
		break;
	case INROF_ACTION_OVERBALL:	
		// マシンがボールに乗り上げた時の処理
		if(ManageActionTansakuState_BallOver!=ManageActionTansakuState_BallOver_Last){	// 初回処理
			SeqInitSub = true;
		}
		ManageActionTansakuState_BallOver_Last = ManageActionTansakuState_BallOver;
		
		switch(ManageActionTansakuState_BallOver){
		case 0:
			if(SeqInitSub){
				MachinePosActionManage = MachinePos;
			}
			this->SpdTgt = -50.0;		// [mmps]
			this->AngVelTgt = 0.0;	// [rps]
			
			if(!Rmb->isCliff() || ((MachinePosActionManage - MachinePos)>300)){	// 踏まなくなるか300mm下がった
			
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				ActionState = INROF_ACTION_LINE;
				ManageActionTansakuState_BallFind_Last = 0xFF;
				
				ManageActionTansakuState_BallOver = 0;
				ManageActionTansakuState_BallOver_Last = 0xFF;
				
			}
		break;
		}
	}
	
	
	
	return false;
}
	
	
// ボールを置きに行く処理
bool_t inrof2016::manageAction_goToGoal(void){
	bool_t SeqInit = false;
	bool_t SeqEnd = false;
	
	ActionStateLast = ActionState;
	
	// 動作
	switch(ActionState){
	case INROF_ACTION_IDLE:	// 最初に入る
		ActionState = INROF_ACTION_BALLRELEASE;
		ManageActionTansakuState_BallRelease = 0;
		ManageActionTansakuState_BallReleaseLast = 0xff;
		MachinePosActionManage = MachinePos;	// この状態に入った時の場所からxxmm以内は場所補正しない用
		ManageActionLineTraceDir = LineTraceDir;
		Goal = INROF_GOAL_NONE;		// まだ行き先決まらない
		
		// 続けて下を実行
	case INROF_ACTION_BALLRELEASE:
			
		if(ManageActionTansakuState_BallReleaseLast != ManageActionTansakuState_BallRelease){	// 初回フラグ立てる
			SeqInit = true;
#ifdef DEBUGSCI0
Sci0.print("manageAction_goToGoal %d\r\n", ManageActionTansakuState_BallRelease);
#endif
		}
		ManageActionTansakuState_BallReleaseLast = ManageActionTansakuState_BallRelease;
		
		switch(ManageActionTansakuState_BallRelease){
		case 0:	// ゴール位置までライントレース、アーム移動要求
			/* 初回処理 */
			if(SeqInit){
				Goal = INROF_GOAL_NONE;
				
#ifdef DEBUGSCI0
				Sci0.print("Forward? %d\r\n", ManageActionLineTraceDir);
#endif
			}
		
			/* 通常時処理 */
			// 基本はライントレースで目標ゴール位置の交点を目指す
			
			// 目標ゴール決定処理
			if((Goal == INROF_GOAL_NONE)&&(UPPER_STATE_IDLE == InrofUpper.getState())){	// まだ決まってない && やっとアーム暇になった
			// アームさんに何色のボール持ってるか聞く
				inrof_color TrashColor;
				
				if(INROF_LINEDIR_FORWARD == ManageActionLineTraceDir){
					if(0 < InrofUpper.numRed()){	// 赤チェック
						TrashColor = INROF_RED;		// 捨てるボールの色
						this->Goal = INROF_GOAL_RED;		// 向かうゴール
					}else if(0 < InrofUpper.numYellow()){	// 黄チェック
						TrashColor = INROF_YELLOW;	// 捨てるボールの色
						this->Goal = INROF_GOAL_YELLOW;		// 向かうゴール
					}else if(0 < InrofUpper.numBlue()){	// 青チェック
						TrashColor = INROF_BLUE;	// 捨てるボールの色
						this->Goal = INROF_GOAL_BLUE;		// 向かうゴール
					}else{	// ボール無し -> 処理完了
						TrashColor = INROF_NONE;
						this->Goal = INROF_GOAL_NONE;		// 向かうゴール
						
						// 探索再開
						setState(INROF_TANSAKU);
						break;
					}
				}else{	
					if(0 < InrofUpper.numBlue()){		// 青チェック
						TrashColor = INROF_BLUE;	// 捨てるボールの色
						this->Goal = INROF_GOAL_BLUE;		// 向かうゴール
					}else if(0 < InrofUpper.numYellow()){	// 黄チェック
						TrashColor = INROF_YELLOW;	// 捨てるボールの色
						this->Goal = INROF_GOAL_YELLOW;		// 向かうゴール
					}else if(0 < InrofUpper.numRed()){	// 赤チェック
						TrashColor = INROF_RED;	// 捨てるボールの色
						this->Goal = INROF_GOAL_RED;		// 向かうゴール
					}else{	// ボール無し -> 処理完了
						TrashColor = INROF_NONE;
						this->Goal = INROF_GOAL_NONE;		// 向かうゴール
						
						// 探索再開
						setState(INROF_TANSAKU);
						break;
					}
				}
				
				// アームさんにボールを用意するよう要求
				if(INROF_LINEDIR_FORWARD == ManageActionLineTraceDir){	// ライン正方向ならマシン左側に捨てる
					InrofUpper.reqBallTrashPre(TrashColor, PI);
				}else{							// ライン逆方向ならマシン右側に捨てる
					if(INROF_GOAL_BLUE == Goal){			// ライン逆方向で青ならマシン前方に捨てる
						InrofUpper.reqBallTrashPre(TrashColor, PI/2);
					}else{
						InrofUpper.reqBallTrashPre(TrashColor, 0);
					}
				}
				
				Sci0.print("ArmReqBallTrash. Col %d\r\n", Goal);
				
				switch(Goal){
				case INROF_GOAL_BLUE:
					Rmb->outLedDispAscii("BLUE");
					break;
				case INROF_GOAL_YELLOW:
					Rmb->outLedDispAscii("YLOW");
					break;
				case INROF_GOAL_RED:
					Rmb->outLedDispAscii("RED");
					break;
				}
				
			}
			
			
			
			// 移動処理
			
			if(200.0f > (MachinePosActionManage - MachinePos)){	// ライントレースし始まって200mm以下しか進んでなければ位置補正しない。
				fAdjustMachinePos = false;
			}else{
				fAdjustMachinePos = true;
			}
			if(INROF_CROSS_START==Cross || INROF_CROSS_NONE==Cross){
				// 赤交差点よりスタート位置側だったら直進
				SpdTgt = RMB_VEL_NORMAL;
				AngVelTgt = 0.0;
				//fAdjustMachinePos = true;
				
			}else if(INROF_GOAL_BLUE == Goal){
				// 目的地が青の場合の処理
				if(INROF_LINEDIR_FORWARD == ManageActionLineTraceDir){
				// Forward時：	黄ライン踏むまでと、黄ライン通過後(450-センサ分)mmはライントレース
				//		その後は直進。黄ラインから650mm進むまで
					
					if(INROF_CROSS_YELLOW == Cross){	// 黄色い線踏んで以降
					
							
						if((MachinePos - PosCrossYellow) > 650.0F){	/* 処理順 3 */	// 黄線から650mm進んだ
							// ここがボール置く位置。
#ifdef DEBUGSCI0
							Sci0.print("se mX%f Y%f, yX%f Y%f\r\n", MachinePos.X, MachinePos.Y, PosCrossYellow.X, PosCrossYellow.Y);
#endif
							SeqEnd = true;
							
						}else if((MachinePos - PosCrossYellow) > (450.0F - LINESENS_POS_X)){	/* 処理順 2 */	// 黄線から450mm-センサ分進んだ
							
							SpdTgt = RMB_VEL_NORMAL;
							AngVelTgt = 0.0;		// 直進
							
#ifdef DEBUGSCI0
							Sci0.print("st mX%f Y%f, yX%f Y%f\r\n", MachinePos.X, MachinePos.Y, PosCrossYellow.X, PosCrossYellow.Y);
#endif							
						}else{		/* 処理順 1 */
							setTgtVel_LineTrace(RMB_VEL_NORMAL);	// ライントレース
						}
					}else{
						setTgtVel_LineTrace(RMB_VEL_NORMAL);	// ライントレース
					}
					
				}else{
				// Backward時：	青ライン通過後(150-センサ分)mmはライントレース
				//		その後は直進。青ラインから300mm進むまで
					
					if((INROF_CROSS_BLUE == Cross)||(MachinePos.X<FIELD_INROF_CROSS_BLUE_X)){	// 青い線踏んで以降
						if((MachinePos - PosCrossBlue) > 300.0F){		// 青線から300mm進んだ
							// ここがボール置く位置。
							SeqEnd = true;
						}else if((MachinePos - PosCrossBlue) > (150.0F - LINESENS_POS_X)){	// 青線から150mm進んだ(ラインなくなる辺り)
															// 数値上、青い線踏んでちょっと進むまで直進しちゃうけどまぁいっか
							SpdTgt = RMB_VEL_NORMAL;
							AngVelTgt = 0.0;		// 直進
							
						}else{
							setTgtVel_LineTrace(RMB_VEL_NORMAL);	// ライントレース
						}
					}else{
						setTgtVel_LineTrace(RMB_VEL_NORMAL);	// ライントレース
					}
				}
				
			}else{
				// 目的地が黄または赤、もしくは決まっていないの時の処理
				setTgtVel_LineTrace(RMB_VEL_NORMAL);	// ライントレース
				
				// 青捨てた後だったら…
				
				
				//fAdjustMachinePos = true;
				if((INROF_GOAL_RED == Goal) && (INROF_CROSS_RED == Cross)){	// 目的のライン越えた
					if((MachinePos - PosCrossRed) < 40.0F){	// 目的ライン上まで進んだ
						SeqEnd = true;					// そこが停止位置
					}
				}else if((INROF_GOAL_YELLOW == Goal) && (INROF_CROSS_YELLOW == Cross)){	// 目的のライン越えた
					if((MachinePos - PosCrossYellow) < 40.0F){	// 目的ライン上まで進んだ
						SeqEnd = true;					// そこが停止位置
					}
				}else{
				}
				
			}
			
			
			/* 遷移処理 */
			// ゴールの交差点に来たー
			if(	/*onCross() 	// 2回目以上連続して捨てるとき、ラインいきすぎてると走り出しちゃうのでライン上であることは検知しない。→中途半端な位置の時や、次向かうゴールが向いてる方じゃない時注意
				&&*/
				SeqEnd
			){	
				//adjustMachinePos();	// 角度が、-PI〜PIの範囲になるので注意
				
				ManageActionTansakuState_BallRelease ++;	// 次の処理へ
				MachinePosActionManageSub = MachinePos;		// ゴールの場所についた時の位置を覚える
				
				ManageActionLineTraceDir = LineTraceDir;
				
#ifdef DEBUGSCI0
Sci0.print("Obj Goal Pos. \r\n");
#endif
			}
			break;
			
			
		case 1:	// アーム移動
			/* 初回処理 */
			if(SeqInit){
				
				// マシン止める
				SpdTgt = 0.0;
				AngVelTgt = 0.0;
#ifdef DEBUGSCI0
Sci0.print("Stop machine. \r\n");
#endif
				
			}
			/* 通常時処理 */
			// notthing to do (wait for arm move. )
			
			
			/* 遷移処理 */
			// アーム移動完了した
			if(UPPER_STATE_IDLE == InrofUpper.getState()){
				ManageActionTansakuState_BallRelease ++;	// 次の処理へ
#ifdef DEBUGSCI0
Sci0.print("UPPER_STATE_BALL_TRASH_PRE_DONE\r\n");
#endif
			}
			break;
			
		case 2:	// ボール捨てる
			/* 初回処理 */
			if(SeqInit){
				
				Rmb->outLedDispAscii("TRSH");
				
				// ボール捨てる要求
				InrofUpper.reqBallTrash();
				
			}
			/* 通常時処理 */
			// notthing to do
			
			
			/* 遷移処理 */
			// ボール捨てた
			if(UPPER_STATE_IDLE == InrofUpper.getState()){
				
				if(InrofUpper.numBlue()
				 || InrofUpper.numYellow()
				 || InrofUpper.numRed()){	// まだ捨てるものあり
					
					if((INROF_GOAL_BLUE == Goal) && (InrofUpper.numBlue()==0)){	// 今青捨てた。青は捨て終わった。
						
					#ifdef DEBUGSCI0
						Sci0.print("trash bule fin.\r\n");
						#endif
						ManageActionTansakuState_BallRelease  = 3;	// 赤とか、黄色のゴールを向く
					}else{
#ifdef DEBUGSCI0
						Sci0.print("trash same ball.\r\n");
#endif
						ManageActionTansakuState_BallRelease = 0;	// また捨てる
					}
				}else{	// もう捨てるものなし				
					// フィールド上もなし
					if(FieldBall==0){
						if(INROF_GOAL_BLUE == Goal){
#ifdef DEBUGSCI0
							Sci0.print("to start. from blue\r\n");
							#endif
							ManageActionTansakuState_BallRelease = 3;	// 赤とか、黄色のゴールを向く
						}else{
#ifdef DEBUGSCI0
							Sci0.print("to start. from yellow or red\r\n");
							#endif
							// スタートへ
							setState(INROF_TOSTART);
						}
					}else{
						ManageActionTansakuState_BallRelease = 4;	// また探索するために上を向く
						
					}
				}
			}
			break;
		case 3: // 青から赤とかのほう向く
			
			this->SpdTgt = 0;
			
			if(INROF_LINEDIR_FORWARD == ManageActionLineTraceDir){	// スタート側から来た時
				// 元の向きの逆を向く
				if(fabs(fabs(MachinePos.Th - (MachinePosActionManageSub.Th))-PI) < 0.1){
					if((MachinePosActionManageSub - MachinePos) > 200.0F){	// ボール捨てたとこからライン載ってるだろうとこまで進んだ
						SeqEnd = true;
					}else{	// ライン乗るまで進む
						this->SpdTgt = RMB_VEL_NORMAL;
						this->AngVelTgt = 0.0F;
					}
				}else{	
					// 逆向くまで回転
					this->AngVelTgt = 0.15F * ( (MachinePosActionManageSub.Th - PI) - MachinePos.Th );  // 右180°回転
				}
			}else{	// フィールド側から来た
				// 元の向き+90°を向く
				
				if(fabs(fabs(MachinePos.Th - MachinePosActionManageSub.Th) - PI/2.0F) < 0.1F){
					if((MachinePosActionManageSub - MachinePos) > 200.0F){	// ボール捨てたとこからライン載ってるだろうとこまで進んだ
						SeqEnd = true;
					}else{
						this->SpdTgt = RMB_VEL_NORMAL;
						this->AngVelTgt = 0.0F;
					}
				}else{
					// 元の+1/2PI向くまで回転
					this->AngVelTgt =  0.15F * ( (MachinePosActionManageSub.Th + PI/2.0F) - MachinePos.Th );  // 左90°回転
				}
			}
			//回転速度に制限
			if(0.04<AngVelTgt){
				this->AngVelTgt = 0.04;
			}else if(-0.04>AngVelTgt){
				this->AngVelTgt = -0.04;
			}
			
			if(SeqEnd){
				if(InrofUpper.numBlue()
				 || InrofUpper.numYellow()
				 || InrofUpper.numRed()){
					
					ManageActionTansakuState_BallRelease = 0;	// 赤とか黄色を捨てる
					
#ifdef DEBUGSCI0
					Sci0.print("next trash red or yellow. \r\n");
					#endif
				}else{	// もう捨てるものなし
					
					// フィールド上もなし
					// スタートへ
					setState(INROF_TOSTART);
#ifdef DEBUGSCI0
					Sci0.print("to start. \r\n");
					#endif
				}
			}
			
			break;
		case 4: 	// ボール捨て終わったからフィールドを向いて探索再開
			/* 初回処理 */
			if(SeqInit){
				InrofUpper.reqArmToCenter();	// アーム収納
			}
			/* 通常時処理 */
		
			// 以下未検討
			
			this->SpdTgt = 0.0;
			
			// 終了判定 兼 旋回制御
			if(INROF_GOAL_BLUE == Goal){
				if(INROF_LINEDIR_BACKWARD == ManageActionLineTraceDir){	// フィールド側から来た時
					// 元の向きと逆を向く
					if(fabs(fabs(MachinePos.Th - (MachinePosActionManageSub.Th))-PI) < 0.1){
						if((MachinePosActionManageSub - MachinePos) > 200.0F){	// ボール捨てたとこからライン載ってるだろうとこまで進んだ
							SeqEnd = true;
						}else{
							this->SpdTgt = RMB_VEL_NORMAL;
							this->AngVelTgt = 0.0F;
						}
					}else{	
						// 逆向くまで回転
						this->AngVelTgt = 0.15F * ( (MachinePosActionManageSub.Th - PI) - MachinePos.Th );  // 右180°回転
					}
				}else{	// スタート側から来た
					// 元の向き-90°を向く
					if(fabs(fabs(MachinePos.Th - MachinePosActionManageSub.Th) - PI/2.0F) < 0.1F){
						if((MachinePosActionManageSub - MachinePos) > 200.0F){	// ボール捨てたとこからライン載ってるだろうとこまで進んだ
							SeqEnd = true;
						}else{
							this->SpdTgt = RMB_VEL_NORMAL;
							this->AngVelTgt = 0.0F;
						}
					}else{
						// 元の-1/2PI向くまで回転
						this->AngVelTgt = 0.15F * ( (MachinePosActionManageSub.Th - PI/2.0F) - MachinePos.Th );  // 右90°回転
					}
				}
				
			}else{	// 赤 or 黄
				// ライン上にいるので、旋回するだけでok
				if(((fabs(fabs(MachinePos.Th - (MachinePosActionManageSub.Th))-PI) < 0.1)&&(INROF_LINEDIR_BACKWARD == ManageActionLineTraceDir))		// フィールドから来た時：元の向きの反対を向く
					|| (INROF_LINEDIR_FORWARD == ManageActionLineTraceDir)){		// スタートから来た時：元の向きと同じ
					
					// かつ、ライン上なら。でいいかも。
					
					SeqEnd = true;
				
				}else{
					
					this->AngVelTgt = 0.15 * ( (MachinePosActionManageSub.Th - PI) - MachinePos.Th );  // 右180°回転
			
				}
			}
	
			if(0.04<AngVelTgt){
				this->AngVelTgt = 0.04;
			}else if(-0.04>AngVelTgt){
				this->AngVelTgt = -0.04;
			}
			
			
			/* 遷移処理 */
			// 右回転完了
			if( 
				SeqEnd
			){
				// 探索再開
				setState(INROF_TANSAKU);
				
				// アーム中央へ
				//InrofUpper.reqArmToCenter();
				
			}
			break;
		
		}
		break;
		
	}
	
	
	
	return false;
}

bool_t inrof2016::manageAction_goToStart(void){
	bool_t SeqInit = false;
	bool_t SubSeqInit = false;
	bool_t SeqEnd = false;
	
	
	if(ActionStateLast != ActionState){
		SeqInit = true;
	}
	ActionStateLast = ActionState;
	
	if(ManageActionToStartState_ToFinish_Last != ManageActionToStartState_ToFinish){
		SubSeqInit = true;
	}
	ManageActionToStartState_ToFinish_Last = ManageActionToStartState_ToFinish;
	
	
	switch(ActionState){
	case INROF_ACTION_IDLE:	// 初期化
		Sci0.print("To Start \r\n");
		ActionState = INROF_ACTION_LINE;
		
		InrofUpper.reqArmToCenter();	// アーム真ん中にするよ
		
		ManageActionToStartState_ToFinish_Last = 0xff;
		ManageActionToStartState_ToFinish = 0x00;
		
		
		Rmb->outLedDispAscii("FIN.");
		
		break;
		
	case INROF_ACTION_LINE:	// ライントレース
		
		if(SeqInit){
			Sci0.print("LineTrace \r\n");
			StartTimeActionManage = getTime_ms();
		}
		
		setTgtVel_LineTrace(RMB_VEL_NORMAL);
		
		
		// 遷移条件
		if(MachinePos.Y < -200.0F){	// スタートゾーン入った
			Sci0.print("StartZone in \r\n");
			ActionState = INROF_ACTION_TOFINISH;
			ManageActionToStartState_ToFinish = 0;
			ManageActionToStartState_ToFinish_Last = 0xFF;
		}
		
		break;
	case INROF_ACTION_TOFINISH:	// スタートゾーン戻った
		switch(ManageActionToStartState_ToFinish){
		case 0:	
			if(SubSeqInit){
				Sci0.print("StartZone \r\n");
				StartTimeActionManage = getTime_ms();
				
				MachinePosActionManageSub = MachinePos;
				
			}
			
			// その場で回る
			this->SpdTgt = 0.0F;
			
			if(fabs(fabs(MachinePos.Th - (MachinePosActionManageSub.Th))-PI) < 0.1){
				this->AngVelTgt = 0.0F;
				SeqEnd = true;
			}else{	
				// 逆向くまで回転
				this->AngVelTgt = 0.2F * ( (MachinePosActionManageSub.Th - PI) - MachinePos.Th );  // 右180°回転
			}
			
			if(0.04<AngVelTgt){
				this->AngVelTgt = 0.04;
			}else if(-0.04>AngVelTgt){
				this->AngVelTgt = -0.04;
			}
			
			// 向き終わった
			if(SeqEnd){
				
				// 完了SE
				
				Inrof.Rmb->outNoteSingle(0, 69, 8);
				
				// ルンバ起きてるかわからないため、全部つける
				Inrof.Rmb->outLeds(0xFF);	// 全点灯
				Inrof.Rmb->outSchedulingLeds(0xFF);	// 全点灯
				Inrof.disableOutTyre();
				
				setState(INROF_IDLE);
			}
			break;
			
		default:
			break;
		}
		
		
		break;
	}	
	
	return false;
}

// ラインセンサのキャリブレーション
// 白、黒の差の間を、線有り無しのしきい値にしてeepromに設定
bool_t inrof2016::manageAction_calibLine(void){
	
	setState(INROF_IDLE);
	
	return false;
}





/*************
ライントレースするように目標速度設定
ラインがない、もしくはスタートの交差点過ぎたとこなら直進
*************/
void inrof2016::setTgtVel_LineTrace(void){
	
	
	setTgtVel_LineTrace(VEL_NORMAL);
	
}
void inrof2016::setTgtVel_LineTrace(float SpdTgt){
	
	
	this->SpdTgt = SpdTgt;
	
	
	if(INROF_CROSS_START==Cross || INROF_CROSS_NONE == Cross){	// スタートラインを超えたとこ。
		this->AngVelTgt = 0;	
		PidCtrlLineTrace->resetStatus(getLineSensAd(2) - getLineSensAd(1));	//pidはリセットしておく
	}else{
		
		
		
		if((getLineSensAd(1)<LINESENS_THRESHOLD_CENTER)||(getLineSensAd(2)<LINESENS_THRESHOLD_CENTER)){
			// 中央2つのセンサがライン乗ってる
			this->AngVelTgt = PidCtrlLineTrace->calc(getLineSensAd(2) - getLineSensAd(1));	// ライントレース
			if(fAdjustMachinePos){
				adjustMachinePos();
			}
		}else if((getLineSensAd(0)<LINESENS_THRESHOLD_SIDE)||(getLineSensAd(3)<LINESENS_THRESHOLD_SIDE)){
			// 左右2つのセンサどちらかライン乗ってる
			this->AngVelTgt = PidCtrlLineTrace->calc(getLineSensAd(3) - getLineSensAd(0));	// ライントレース
			this->AngVelTgt = this->AngVelTgt * 2.0F;
			this->SpdTgt = SpdTgt*0.5F;
		}else{
			// どっちも乗ってないのでまっすぐ
			this->AngVelTgt = 0;	
			
			PidCtrlLineTrace->resetStatus(getLineSensAd(2) - getLineSensAd(1));	//pidはリセットしておく
		}
		/*
		if(	(getLineSensAd(1)<LINESENS_THRESHOLD_CENTER)||(getLineSensAd(2)<LINESENS_THRESHOLD_CENTER)
			||(getLineSensAd(0)<LINESENS_THRESHOLD_SIDE)||(getLineSensAd(3)<LINESENS_THRESHOLD_SIDE)
		){
			this->AngVelTgt = 0.5*(min(getLineSensAd(2), getLineSensAd(3)) - min(getLineSensAd(0),getLineSensAd(1)));
		}else{
			// どれも乗ってないのでまっすぐ
			this->AngVelTgt = 0;	
		}
		*/
		
	}
	
}
/*************
指定半径でターンする目標速度設定
概要
 謎の1/4はなぞの定数。これかけないと曲がり過ぎちゃうの。
引数：
 直進速度 [mm/s]
 旋回半径(正で左回転, 負で右回転) [mm]
*************/
void inrof2016::setTgtVel_Turn(float SpdTgt, float Radius){
	
	this->SpdTgt = SpdTgt;							// [mm/s]
	//this->AngVelTgt = SpdTgt / Radius / (2*PI) /4.0F ;	// [rev/s]
	this->AngVelTgt = this->Spd / Radius / (2.0F*PI) /2.0F;	// [rev/s]
	
}








/*********************
走行制御
概要：
 PID制御で、直進成分±回転成分のDutyを出力する。
 Dutyは、signedのプラスマイナスで前進後進を表す。
**********************/
void inrof2016::ctrlRun(float SpdTgt_mmps, float AngVelTgt_rps, int16_t* RTireDutyTgt, int16_t* LTireDutyTgt){
	float RTireSpdTgt_rps, LTireSpdTgt_rps;
	
	// 各タイヤの目標速度
	//  目標直進速度[mm/s]と角速度[r/s]から各タイヤの目標角速度[r/s]を求める
	RTireSpdTgt_rps = SpdTgt_mmps/(PI*TIRE_DIA_MM) + AngVelTgt_rps * 4.0F * TIRE_TREAD_MM/TIRE_DIA_MM;
	LTireSpdTgt_rps = SpdTgt_mmps/(PI*TIRE_DIA_MM) - AngVelTgt_rps * 4.0F * TIRE_TREAD_MM/TIRE_DIA_MM;

	// フィードバック制御
	if(fMotorOut){
		// PID制御
		*RTireDutyTgt += (int16_t)PidCtrlRTire->calc(RTireSpdTgt_rps - RTireSpd_rps);
		*LTireDutyTgt += (int16_t)PidCtrlLTire->calc(LTireSpdTgt_rps - LTireSpd_rps);
	}else{
		// モーター出力無効時は、PID制御にリセットをかけ、目標デューティーを0とする。
		PidCtrlRTire->resetStatus(RTireSpdTgt_rps - RTireSpd_rps);
		PidCtrlLTire->resetStatus(LTireSpdTgt_rps - LTireSpd_rps);
		*RTireDutyTgt = 0;
		*LTireDutyTgt = 0;
	}
	
}



/*********************
モーター出力(PWM duty)
概要：
 Duty正負(+-255)に応じて正転逆転を切り替える。
 fMotorOutがfalseの時、モーターをフリー状態とする。
**********************/
void inrof2016::outTyres(int16_t RTireDutyTgt, int16_t LTireDutyTgt, bool_t fMotorOut){
	
	if(fMotorOut){
		// ルンバ的にはバックなので逆にする
		Rmb->drivePwm(-LTireDutyTgt, -RTireDutyTgt);
	}else{
		Rmb->drivePwm(0, 0);	// 止まれ！
	}
	
}



/*********************
ラインセンサ位置をグローバル座標系で取得
引数：
	(返り)*X
	(返り)*Y
**********************/
void inrof2016::getLineSenseGlobalPos(position* Pos){
	float cosMTh = cosf(MachinePos.Th);
	float sinMTh = sinf(MachinePos.Th);
	
	Pos->X = MachinePos.X + (LINESENS_POS_X*cosMTh - LINESENS_POS_Y*sinMTh);
	Pos->Y = MachinePos.Y + (LINESENS_POS_X*sinMTh + LINESENS_POS_Y*cosMTh);
	
}







/*********************
バッテリ電圧を取得＆BattVoltage,fBattEmptyにセット
サーボから取得した情報を使います
引数：
**********************/
void inrof2016::getBattVoltage(void){
	
	this->BattVoltage = InrofUpper.ServoPitch->getPresentVolts();	// 電圧計測値取得
	if(BATT_LOW>=(this->BattVoltage)){
		this->fBattEmpty = true;
	}else{
		this->fBattEmpty = false;
	}
	
	InrofUpper.ServoPitch->updatePresentVolts();	// 次の更新(計測要求
	
}




/*********************
初期化チェック
引数：なし
戻値：初期化完了条件揃ってる
初期化完了条件
　通信出来てる
　受信要求だしたやつ全部受信した
**********************/
bool_t inrof2016::checkInitialize(void){
	bool_t Init = true;
	
	// 通信チェック
	Init = Init && Rmb->isConnect();	// 通信出来てる 
	Init = Init && Rmb->isRcvOnceAtLeast();	// 受信要求だしたやつ全部受信した
	
	if(Init && (INROF_INITIALIZE==this->State)){
		this->State = INROF_IDLE;
	}
	
	
	return Init;
}




void inrof2016::drivePs3Con(void){
	int16_t outDutyR,outDutyL;	// -255 ~ 255
	
	
	
	if(ExtSci.isPs3ConAvailable()){
		
		// 掃除モーター駆動on/off
		// STARTボタン(立ち上がり)
		if(ExtSci.ControllerData.button.Start && !Ps3ConDataLast.button.Start){
			if(isCleaning()){
				stopClean();
			//	Inrof.ArmMotor->setDuty(200);
			}else{
				startClean();
			//	Inrof.ArmMotor->setDuty(-200);
			}
		}
		// Fullモードへ
		// ○ボタン(立ち上がり)
		if(ExtSci.ControllerData.button.Circle && !Ps3ConDataLast.button.Circle){
			Rmb->setMode(ROI_MODE_PASSIVE);
			Rmb->setMode(ROI_MODE_FULL);
		}
		// パッシブモードへ
		// Selectボタン(立ち上がり)
		if(ExtSci.ControllerData.button.Select && !Ps3ConDataLast.button.Select){
			Rmb->setMode(ROI_MODE_PASSIVE);
		}
		// お家へ帰る
		// □ボタン(立ち上がり)
		if(ExtSci.ControllerData.button.Square && !Ps3ConDataLast.button.Square){
			Rmb->sendCommand(ROI_OPCODE_FORCE_SEEKING_DOG);
		}
		
		
		// つかむ
		//-100 ~ 150
		GrabAngleTgt += ((int16_t)ExtSci.ControllerData.button.AnalogRight - (int16_t)ExtSci.ControllerData.button.AnalogLeft)/20;
		if(GrabAngleTgt>SERVOPOS_MAX_G){
			GrabAngleTgt = SERVOPOS_MAX_G;
		}else if(GrabAngleTgt<SERVOPOS_MIN_G){
			GrabAngleTgt = SERVOPOS_MIN_G;
		}
		InrofUpper.ServoGrab->setGoalPosition(GrabAngleTgt);
		
		// 持ち上げる
		//-700 ~ 380
		PitchAngleTgt += ((int16_t)ExtSci.ControllerData.button.AnalogUp - (int16_t)ExtSci.ControllerData.button.AnalogDown)/20;
		if(PitchAngleTgt>SERVOPOS_MAX_P){
			PitchAngleTgt = SERVOPOS_MAX_P;
		}else if(PitchAngleTgt<SERVOPOS_MIN_P){
			PitchAngleTgt = SERVOPOS_MIN_P;
		}
		InrofUpper.ServoPitch->setGoalPosition(PitchAngleTgt);
		
		
		// アームモーター
		// R2：CW
		// L2：CCW
		if(ExtSci.ControllerData.button.R1 && !Ps3ConDataLast.button.R1){
			Ps3ConArmMode = 1;
			
			if(ArmPosTgt>0){
				ArmPosTgt--;
			}
		}
		if(ExtSci.ControllerData.button.L1 && !Ps3ConDataLast.button.L1){
			Ps3ConArmMode = 1;
			
			if(ArmPosTgt<9){
				ArmPosTgt++;
			}
		}
		
		if((ExtSci.ControllerData.button.AnalogL2 > 5) || (Ps3ConDataLast.button.AnalogR2 > 5)){
			Ps3ConArmMode = 0;
		}
		
		switch(Ps3ConArmMode){
		case 0:	// アナログ的に動かすモード
			InrofUpper.moveArmDuty((int16_t)ExtSci.ControllerData.button.AnalogL2 - (int16_t)ExtSci.ControllerData.button.AnalogR2);
			break;
		case 1:	
			InrofUpper.moveArmBallPos(ArmPosTgt);
			break;
		default:
			InrofUpper.moveArmDuty(0);
		}
		
		
		// 左右スティックで走る(+-128 -> +-255)
		//outDutyL = ((int16_t)0x0080 - (int16_t)ExtSci.ControllerData.button.LStickVertical)*2;
		//outDutyR = ((int16_t)0x0080 - (int16_t)ExtSci.ControllerData.button.RStickVertical)*2;
		
		
		int16_t Straight = ((int16_t)0x0080 - (int16_t)ExtSci.ControllerData.button.RStickVertical);
		int16_t Turn = ((int16_t)0x0080 - (int16_t)ExtSci.ControllerData.button.LStickHrizontal)/1.5;
		
		if((Straight<20) && (Straight>-20)){
			Straight = 0;
		}
		if((Turn<30) && (Turn>-30)){
			Turn = 0;
		}
		
		outDutyL = Straight - Turn;
		outDutyR = Straight + Turn;
		// 中央に不感帯
		if((outDutyL<30) && (outDutyL>-30)){
			outDutyL = 0;
		}
		if((outDutyR<30) && (outDutyR>-30)){
			outDutyR = 0;
		}
		
		outTyres(outDutyR, outDutyL, fMotorOut);
		
		// 今回のボタン情報を記憶(立ち上がり判定のため)
		Ps3ConDataLast = ExtSci.ControllerData;
	}else{
		InrofUpper.ArmMotor->setDuty(0);
		outTyres(0, 0, 0);
	}
}










// 1msごとの割り込み設定
// TMR0
// 割り込み周期 46 / (PCLK/1024) = 0.981msec
void inrof2016::setup1msInterrupt(void){
	uint16_t SamplingFreq = 1000;

	SYSTEM.PRCR.WORD = 0xA503u;		// 書き込み許可
	MSTP(TMR0) = 0;					// モジュール起動
	
	TMR0.TCR.BYTE = 0x00;
	TMR0.TCR.BIT.CCLR = 1;			// コンペアマッチAでカウンタクリア
	TMR0.TCR.BIT.CMIEA = 1;			// コンペアマッチA割り込み許可(CMIAn)
	
	
	TMR0.TCSR.BYTE = 0x00;
	
	
	// カウンタ
	uint16_t cnt = PCLK/1024/SamplingFreq;
	TMR0.TCNT = 0;
	TMR0.TCORA = cnt;		//コンペアマッチ用(使わない)
	TMR0.TCORB = cnt+1;		//コンペアマッチ用(使わない)
	
	// 割り込み許可
	IEN(TMR0,CMIA0) = 1;
	IPR(TMR0,CMIA0) = 2;
	
	// クロック設定
	TMR0.TCCR.BYTE = 0x00;
	TMR0.TCCR.BIT.CKS = 0x05;	// クロック選択			PCLK/1024
	TMR0.TCCR.BIT.CSS = 0x01;	// クロックソース選択 分周クロック
	
	// 動作止めるのはこれしかない？
	//TMR0.TCCR.BIT.CKS = 0x00;
	//TMR0.TCCR.BIT.CSS = 0x00;	
}

/*************
TPU6を使ったフリーカウンタ。
時間計測用
16bit, PCLK/1 -> 48MHz/64 = 750kHz
タイマカウンタ : TPU6.TCNT
オーバーフローすると TPU6.TSR.BIT.TCFVが立つ
*************/
void inrof2016::setupFreeCounter(void){
	MSTP(TPU6) = 0;					// モジュール起動
	TPU6.TCR.BIT.TPSC = 3;	// PCLK/4
	TPU6.TCR.BIT.CKEG = 0;	// 立ち上がりエッジでカウント
	TPU6.TCR.BIT.CCLR = 0;	// クリアなし
	TPU6.TMDR.BIT.MD = 0;	// 通常動作
	
	TPUB.TSTR.BIT.CST6 = 1;	// カウント開始
	
}


//extern void isr1ms(void);
// 割り込み関数の設定
#pragma interrupt (Excep_TMR0_CMIA0(vect=VECT(TMR0,CMIA0), enable))
void Excep_TMR0_CMIA0(void){
	
	Inrof.isr1ms();
	//isr1ms();
	
}
