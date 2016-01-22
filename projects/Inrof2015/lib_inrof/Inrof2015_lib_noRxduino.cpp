
#include "Inrof2015_lib_noRxduino.h"

inrof2015 Inrof;
//SDMMC MMC;

inrof2015::inrof2015(void){
//	SdWav = new sd_wav;
	
	fBattEmpty = false;
	fCnt200 = false;
	fJudgeColor = true;
	
	SpdTgt = 0;
	AngVelTgt = 0;
	
	
	MachinePos.X = 0.0;
	MachinePos.Y = -115.0;
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
	
	
	BallColorFixed = INROF_YELLOW;	// 自由ボールを置きに行く(どこのゴールでもおｋ)
	
	State = INROF_IDLE;
	ActionState = INROF_ACTION_IDLE;
	
	fFinishManageAction = false;
	fFinishAction = false;
	
	fAdjustMachinePos = true;
	
	Time_msec = 0;
}


void inrof2015::begin(void){
	
	GrEx.begin();		// 拡張基板初期化
	GrExOled.begin();	// 拡張基板のOLED初期化

// オブジェクト用意
	// エンコーダ
	EncR = GrEx.Enc1;
	EncL = GrEx.Enc0;
	
	// モーター
	MotorR = GrEx.Motor1;
	MotorL = GrEx.Motor0;
	
	// I2C
	
	// SCI
	// シリアルサーボ
	ServoF = new futaba_cmd_servo(1, &Sci5, 115200);
	ServoR = new futaba_cmd_servo(2, &Sci5, 115200);
	
	// 測距センサ
	PsdLeft = new inrof2015_psd(&I2C0, PSDSENS_LEFT_ADRS, PSDSENS_LEFT_POS_X, PSDSENS_LEFT_POS_Y, PSDSENS_LEFT_POS_TH);
	PsdLeftSide = new inrof2015_psd(&I2C0, PSDSENS_LEFTSIDE_ADRS, PSDSENS_LEFTSIDE_POS_X, PSDSENS_LEFTSIDE_POS_Y, PSDSENS_LEFTSIDE_POS_TH);
	PsdFront = new inrof2015_psd(&I2C0, PSDSENS_FRONT_ADRS, PSDSENS_FRONT_POS_X, PSDSENS_FRONT_POS_Y, PSDSENS_FRONT_POS_TH);
	PsdRightSide = new inrof2015_psd(&I2C0, PSDSENS_RIGHTSIDE_ADRS, PSDSENS_RIGHTSIDE_POS_X, PSDSENS_RIGHTSIDE_POS_Y, PSDSENS_RIGHTSIDE_POS_TH);
	PsdRight = new inrof2015_psd(&I2C0, PSDSENS_RIGHT_ADRS, PSDSENS_RIGHT_POS_X, PSDSENS_RIGHT_POS_Y, PSDSENS_RIGHT_POS_TH);
	
	// カラーセンサ
	ColorSens = new hamamatsu_S11059(&I2C0);
	// MPU6050
	//Mpu = new invensense_MPU(&I2C0, 0x68);
	
// 初期化
	//エンコーダ計測開始
	EncR->startCnt();
	EncL->startCnt();
	
	
	ServoF->begin();
	ServoR->begin();
	// センサ類設定
	PsdLeftSide->begin();
	PsdLeft->begin();
	PsdFront->begin();
	PsdRightSide->begin();
	PsdRight->begin();
	PsdLeft->setMedianFilter(9);
	PsdLeftSide->setMedianFilter(9);
	PsdFront->setMedianFilter(9);
	PsdRightSide->setMedianFilter(9);
	PsdRight->setMedianFilter(9);
	//カラーセンサ
	ColorSens->begin();
	setPinMode(COLORSENS_PORT_LED, PIN_OUTPUT);
	outPin(COLORSENS_PORT_LED, 1);
	ColorSens->wakeup();	// モジュール起こす
	ColorSens->startMeasurement();	// 計測開始
	//Mpu->setup();
	
	
	// AD変換開始
	GrEx.startAD();
	
	
	// 制御用
	RateLimitSpdTgt = new ctrl_ratelimiter_abs(SAMPLETIME_CTRL_S, 500, 1000);			// 500 mm/s/s
	RateLimitAngVelTgt = new ctrl_ratelimiter_abs(SAMPLETIME_CTRL_S, 0.5);	// 1 rev/s/s	
	
	
	// PSDセンサのI2Cアドレス書き換える
	//Oled.println("%d", PsdLeft->setI2cAddress(0x70, PD0));
	//while(1);
	
	setup1msInterrupt();	//1msごと割り込み設定
}

void inrof2015::step(void){
	if(fExeReq1ms){
		step1ms();
		fExeReq1ms = false;
	}
	if(fExeReq10ms){
		step10ms();
		fExeReq10ms = false;
	}
	if(fExeReq200ms){	// 10msのより先にやらないと、カラーセンサのI2Cできない(new失敗する)
		step200ms();
		fExeReq200ms = false;
	}
}

// 1msごとに実行する関数
void inrof2015::isr1ms(void){
	
	Time_msec++;
	
	fExeReq1ms = true;
	
	if(Cnt200ms<199){
		Cnt200ms++;
	}else{
		fCnt200 = 1;
		Cnt200ms = 0;
		fExeReq200ms = true;
	}
	
	
	if(Cnt10ms<9){
		Cnt10ms ++;
	}else{
		Cnt10ms = 0;
		fExeReq10ms = true;
	}
	
	
	
}

void inrof2015::step1ms(void){
	int32_t RTireDutyTgt, LTireDutyTgt;
	
	// test
	// 1msごとにチカチカ
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

	/***********
	 入力処理
	************/
	// ラインセンサ更新
	updateLineSense();
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
	
	// PSDセンサ/マシン位置更新
	PsdLeft->setMachinePos(&MachinePos);
	PsdLeftSide->setMachinePos(&MachinePos);
	PsdFront->setMachinePos(&MachinePos);
	PsdRightSide->setMachinePos(&MachinePos);
	PsdRight->setMachinePos(&MachinePos);
	
	// ラインセンサ交点判定
	IsCrossing = judgeCross();
	// 走行向き判定
	judgeRunOrient();
	// ボール見つけた判定
	IsBallFinding = judgeBallFind();
	
	//SpdTgt = 200;
	//AngVelTgt = 0.000005*(getLineSensAd(1) - getLineSensAd(2));	// ライントレース
	
	// 状態管理
	// 動作管理
	// 制御 (SpdTgtとAngVelTgtを決定する)
	manageState();
	
	/***********
	 出力
	************/
	//SpdTgt = 200;
	//AngVelTgt = 0.000005*(getLineSensAd(1) - getLineSensAd(2));	
	
	
	//RateLimitSpdTgt->setRate(500, 1000);	// 減速側は強めで。
	//RateLimitAngVelTgt->setRate(0.5);
	RateLimitSpdTgt->limitRate(&SpdTgt);
	RateLimitAngVelTgt->limitRate(&AngVelTgt);
	
	// 速度制御で目標速度を満たすデューティー比計算
	ctrlRun(SpdTgt, AngVelTgt, &RTireDutyTgt, &LTireDutyTgt);	// 速度制御
	
	// デューティー出力
	// バッテリないときは出力しない
	if(!fBattEmpty){	// バッテリ上がってなければ
		outTyres(RTireDutyTgt, LTireDutyTgt);				// デューティー出力
	}else{
		fMotorOut = false;
		outTyres(0, 0);	//モーターoff
	}
}
	
void inrof2015::step10ms(void){
	
	
	// センサ更新
	PsdLeft->measure();
	PsdLeftSide->measure();
	PsdFront->measure();
	PsdRightSide->measure();
	PsdRight->measure();
	
	//Mpu->measure();
	
	// サーボデータ更新
	ServoF->updatePresentPosition();	// おーけー (やってなくてもgetpresentpositionには影響なし)
	ServoR->updatePresentPosition();	
	
	outPin(GR_LED3, readPin(OLED_SW3));
}

void inrof2015::step200ms(void){
	// ボール色更新
	updateBallColor();
	ColorSens->updateSens();
	
	// バッテリ情報更新
	getBattVoltage();
}

/*********************
ラインセンサアップデート
概要：
 エンコーダ値をアップデート
引数：

**********************/
void inrof2015::updateLineSense(void){
	static bool_t fLineSensOn;							// LED光らせてるフラグ
	static uint16_t LineSensAdOn[LINESENS_NUM_SENSE];	// ラインセンサAD値 LED on時
	static uint16_t LineSensAdOff[LINESENS_NUM_SENSE];	// ラインセンサAD値 LED off時
	
	
	if(fLineSensOn){	// onだった
		for(uint8_t i=0; i<LINESENS_NUM_SENSE; i++){
			LineSensAdOn[i] = GrEx.getAD((grex_ad_ch)i);
		}
		outPin(LINESENS_PORT_LED, 0); 
		fLineSensOn = 0;
	}else{
		for(uint8_t i=0; i<LINESENS_NUM_SENSE; i++){
			LineSensAdOff[i] = GrEx.getAD((grex_ad_ch)i);
		}
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
 エンコーダ値をアップデート
引数：

**********************/
void inrof2015::updateEncoder(void){
	
	REncCntDiff = EncR->getCntDiff();
	LEncCntDiff = EncL->getCntDiff();
}

/*********************
タイヤ状態アップデート
概要：
 エンコーダ値から角度と角速度をアップデート
引数：

**********************/
void inrof2015::updateTyre(void){
	
	// タイヤ角速度[rps]
	RTireSpd_rps = -(float)REncCntDiff/ENC_CPR/GEAR_RATIO/SAMPLETIME_CTRL_S;
	LTireSpd_rps =  (float)LEncCntDiff/ENC_CPR/GEAR_RATIO/SAMPLETIME_CTRL_S;
	
	// タイヤ角度[rev]
	RTirePos_r = RTirePos_r + (float)REncCntDiff/ENC_CPR/GEAR_RATIO;
	LTirePos_r = LTirePos_r - (float)LEncCntDiff/ENC_CPR/GEAR_RATIO;
	
}
	
/*********************
自己位置更新
**********************/
void inrof2015::updateOdmetry(void){
	CalcOdmetry.REncCntDiff[1] = CalcOdmetry.REncCntDiff[0];
	CalcOdmetry.REncCntDiff[0] = -REncCntDiff;
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
ボールの色更新
概要：
 色センサの各色の強い色とする。
 微妙なときはINROF_UNKNOWNとする。
 センサ更新と同期して呼んでね。(LED光らせるタイミングの都合)
**********************/
void inrof2015::updateBallColor(void){
	int16_t BallColorValDiff[3];
	
	if(fJudgeColor){
		
		if(BallColorLed){
			ColorSens->getLuminance(&BallColorVal[0][1], &BallColorVal[1][1], &BallColorVal[2][1]);
		}else{
			ColorSens->getLuminance(&BallColorVal[0][0], &BallColorVal[1][0], &BallColorVal[2][0]);
		}
		// 明るい時と暗い時の差をとる
		BallColorValDiff[0] = abs(BallColorVal[0][1] - BallColorVal[0][0]);
		BallColorValDiff[1] = abs(BallColorVal[1][1] - BallColorVal[1][0]);
		BallColorValDiff[2] = abs(BallColorVal[2][1] - BallColorVal[2][0]);
		
		if((BallColorValDiff[0]>50)||(BallColorValDiff[1]>50)||(BallColorValDiff[2]>50)){
			
			if(BallColorValDiff[0] > BallColorValDiff[1]){
				if(BallColorValDiff[0] > BallColorValDiff[2]){
					BallColor = INROF_RED;	// Red
				}else{
					BallColor = INROF_BLUE;	// Blue
				}
			}else if(BallColorValDiff[1] > BallColorValDiff[2]){
				if(BallColorValDiff[1] > BallColorValDiff[0]){
					BallColor = INROF_YELLOW;	// Green
				}else{
					BallColor = INROF_RED;	// Red
				}
			}else{
				if(BallColorValDiff[2] > BallColorValDiff[1]){
					BallColor = INROF_BLUE;	// Blue
				}else{
					BallColor = INROF_YELLOW;	// Green
				}
			}
			outPin(GR_LED1, 1);
		}else{
			outPin(GR_LED1, 0);
			BallColor = INROF_NONE;
		}
		
		if(BallColorLed){
			
			outPin(COLORSENS_PORT_LED, 0);
			BallColorLed = false;
		}else{
			
			outPin(COLORSENS_PORT_LED, 1);
			BallColorLed = true;
		}
	}else{
		
		BallColor = INROF_NONE;
		outPin(COLORSENS_PORT_LED, 0);
		BallColorLed = false;
	}
	
}



/*********************
交点判定
概要：交点に来たことを判定する
		新しい交点に来た時に交点インデックスを更新し、trueを返す
		それ以外ではfalseを返す。
引数：
戻値：新しい交差点に来た
**********************/
bool_t inrof2015::judgeCross(void){
	bool_t IsNewCross = false;
	position LineSensPos;
	
	// 交差点判定
	if((getLineSensAd(0)<LINESENS_THRESHOLD_SIDE) && (getLineSensAd(3)<LINESENS_THRESHOLD_SIDE)){
		
		if(100.0<(MachinePos - LastFindCrossPos)){	// 前見た線より100mm以上遠いところだったら(連続判定防止)
			LastFindCrossPos = MachinePos;	// 線を見つけた場所覚えておく
			IsNewCross = true;
			//どこの交差点だい？
			getLineSenseGlobalPos(&LineSensPos);	// ラインセンサのグローバルな位置を取得
			if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossStart)){		// ラインセンサと、フィールド上の交差点の位置が近ければ
				Cross = INROF_CROSS_START;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossRed)){
				Cross = INROF_CROSS_RED;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossYellow)){
				Cross = INROF_CROSS_YELLOW;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR>(LineSensPos - PosCrossBlue)){
				Cross = INROF_CROSS_BLUE;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR>(LineSensPos - PosCrossBallFieldBegin)){
				Cross = INROF_CROSS_BALLFIELD_BEGIN;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR>(LineSensPos - PosCrossBallFieldEnd)){
				Cross = INROF_CROSS_BALLFIELD_END;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossGoalRed)){
				Cross = INROF_CROSS_GOAL_RED;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossGoalYellow)){
				Cross = INROF_CROSS_GOAL_YELLOW;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossGoalBlue)){
				Cross = INROF_CROSS_GOAL_BLUE;
			}else{
				// どことも近くないのにライン上？？？？
				Cross = INROF_CROSS_NONE;
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
bool_t inrof2015::judgeBallFind(void){
	
	
	if(true==BallFindEnable){	// enableのとき。
		//if((INROF_CROSS_BALLFIELD_BEGIN == Cross) || (INROF_CROSS_BALLFIELD_END == Cross)){	// ボールゾーン内
			if( (1500.0 > MachinePos.Y) && ( (PI/6 > fabs(MachinePos.getNormalizeTh()-PI/2))||(PI/6 > fabs(MachinePos.getNormalizeTh()+PI/2) ) ) ){		// ボールゾーン内, ほぼ上下向いている
				
				// 左横センサ
				if( (BALLFIND_MAXDISTANCE_SIDE > PsdLeftSide->getDistance())		// 指定した距離より近い
					&& (PSD_MIN < PsdLeftSide->getDistance())						// 有効範囲
					&& PsdLeftSide->isObjectBallArea()								// ボールはフィールド内(センサ値取得できてなければfalse返る)
				){
					return true;		
				}
				// 右横センサ
				if( (BALLFIND_MAXDISTANCE_SIDE > PsdRightSide->getDistance())		// 指定した距離より近い
					&& (PSD_MIN < PsdRightSide->getDistance())						// 有効範囲
					&& PsdRightSide->isObjectBallArea()								// ボールはフィールド内(センサ値取得できてなければfalse返る)
				){
					return true;		
				}
				// 左クロスセンサ
				if( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdLeft->getDistance())		// 指定した距離より近い
					&& (PSD_MIN < PsdLeft->getDistance())						// 有効範囲
					&& PsdLeft->isObjectBallArea()								// ボールはフィールド内(センサ値取得できてなければfalse返る)
				){
					return true;		
				}
				// 右クロスセンサ
				if( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdRight->getDistance())		// 指定した距離より近い
					&& (PSD_MIN < PsdRight->getDistance())						// 有効範囲
					&& PsdRight->isObjectBallArea()								// ボールはフィールド内(センサ値取得できてなければfalse返る)
				){
					return true;		
				}
				
			}
		//}
	}
	
	return false;
}
	
void inrof2015::judgeRunOrient(void){
	
	// ほぼ確実なときだけ更新する
	if(MachinePos.Y < 1400){
		if((MachinePos.X < 100) && (MachinePos.X > -100)){	// 左のライン上にいる
			if((MachinePos.getNormalizeTh() > PI/4)&&(MachinePos.getNormalizeTh() < PI/4*3)){
				LineTraceDir = INROF_LINEDIR_FORWARD;
			}else if((MachinePos.getNormalizeTh() < -PI/4)&&(MachinePos.getNormalizeTh() > -PI/4*3)){
				LineTraceDir = INROF_LINEDIR_BACKWARD;
			}
		}else if((MachinePos.X < 100) && (MachinePos.X > -100)){	// 右のライン上にいる
			if((MachinePos.getNormalizeTh() > PI/4)&&(MachinePos.getNormalizeTh() < PI/4*3)){			// 北向いてる
				LineTraceDir = INROF_LINEDIR_BACKWARD;
			}else if((MachinePos.getNormalizeTh() < -PI/4)&&(MachinePos.getNormalizeTh() > -PI/4*3)){	// 南向いてる
				LineTraceDir = INROF_LINEDIR_FORWARD;
			}
		}
	}else{	// 上のライン上にいる
		
		if((MachinePos.getNormalizeTh() < PI/4)&&(MachinePos.getNormalizeTh() > -PI/4)){			// 東向いてる
			LineTraceDir = INROF_LINEDIR_FORWARD;
		}else if((MachinePos.getNormalizeTh() > PI/4*3)||(MachinePos.getNormalizeTh() < -PI/4*3)){	// 西向いてる
			LineTraceDir = INROF_LINEDIR_BACKWARD;
		}
	}
	
}


// マシン位置修正
// 交差点座標を使う

void inrof2015::adjustMachinePos(void){
	
	if(IsCrossing){	// 交差点入ったなう
		switch(Cross){
		case INROF_CROSS_YELLOW:	// 黄色ゾーンの交差点なう
			if(ORIENT_NORTH==MachineOrientation){			// 北向いてる
				MachinePos.X = PosCrossYellow.X;	
				MachinePos.Y = PosCrossYellow.Y - LINESENS_POS_X - 9;
				MachinePos.Th = PI/2;
		
				outPin(GR_LED2, 1);
				
			}else if(ORIENT_SOUTH==MachineOrientation){	// 南向いてる
				MachinePos.X = PosCrossYellow.X;
				MachinePos.Y = PosCrossYellow.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;
		
				outPin(GR_LED2, 1);
			}
			break;
		case INROF_CROSS_BLUE:	// 青ゾーンの交差点なう
			if(ORIENT_WEST==MachineOrientation){			// 西向いてる
				MachinePos.X = PosCrossBlue.X + LINESENS_POS_X + 9;	
				MachinePos.Y = PosCrossBlue.Y;
				MachinePos.Th = PI;
		
				outPin(GR_LED2, 1);
			}else if(ORIENT_EAST==MachineOrientation){			// 東向いてる
				MachinePos.X = PosCrossBlue.X - LINESENS_POS_X - 9;	
				MachinePos.Y = PosCrossBlue.Y;
				MachinePos.Th = 0;
		
				outPin(GR_LED2, 1);
			}
			break;
		case INROF_CROSS_RED:	// 赤ゾーンの交差点なう
			if(ORIENT_SOUTH==MachineOrientation){			// 南向いてる
				MachinePos.X = PosCrossRed.X;	
				MachinePos.Y = PosCrossRed.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;
		
				outPin(GR_LED2, 1);
			}else if(ORIENT_NORTH==MachineOrientation){			// 北向いてる
				MachinePos.X = PosCrossRed.X;	
				MachinePos.Y = PosCrossRed.Y - LINESENS_POS_X - 9;
				MachinePos.Th = PI/2;
		
				outPin(GR_LED2, 1);
				
			}
			break;
		case INROF_CROSS_BALLFIELD_BEGIN:	// ボールゾーン開始の交差点なう
			if(ORIENT_NORTH==MachineOrientation){			// 北向いてる
				MachinePos.X = PosCrossBallFieldBegin.X;	
				MachinePos.Y = PosCrossBallFieldBegin.Y - LINESENS_POS_X - 9;
				MachinePos.Th = PI/2;
		
				outPin(GR_LED2, 1);
			}else if(ORIENT_SOUTH==MachineOrientation){	// 南向いてる
			//	MachinePos.X = PosCrossBallFieldBegin.X;
			//	MachinePos.Y = PosCrossBallFieldBegin.Y + LINESENS_POS_X + 9;
			//	MachinePos.Th = -PI/2;
		
			//	outPin(GR_LED2, 1);
			}
			break;
		case INROF_CROSS_BALLFIELD_END:	// ボールゾーン終わりの交差点なう
			if(ORIENT_SOUTH==MachineOrientation){	// 南向いてる
				MachinePos.X = PosCrossBallFieldEnd.X;
				MachinePos.Y = PosCrossBallFieldEnd.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;
		
				outPin(GR_LED2, 1);
			}
			break;
		}
	}else{
		outPin(GR_LED2, 0);
	}
	
}




//状態管理
int8_t inrof2015::manageState(void){
	
	// ひたすらボールとってゴールいくだけだから今のとこいらないなぁ。
	
	fFinishManageAction = manageAction(State);
	return 0;
}
	
// 動作管理
// 引数
//  目的地とか探索モードとか
bool_t inrof2015::manageAction(inrof_state State){
	
	
	switch(State){
	case INROF_IDLE:
		SpdTgt = 0;
		AngVelTgt = 0;
		break;
	case INROF_TANSAKU:
		return manageAction_tansaku();
		break;
	case INROF_TOGOAL:
		return manageAction_goToGoal(BallColorFixed);
		break;
	case INROF_TOSTART:
		return manageAction_goToStart();
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
bool_t inrof2015::manageAction_tansaku(void){
	
	
	// 状態遷移(交点)
	if(IsCrossing){
		if(Cross==INROF_CROSS_BALLFIELD_END){	// フィールドの端にいたらちょっと下がって90°回転する
			// 180°回転する
			ActionState = INROF_ACTION_TURN;
			ManageActionTansakuState_BallFind_turn = 0;	// ちょっと下がる
			MachinePosActionManageSub = MachinePos;
		}else if(Cross==INROF_CROSS_BALLFIELD_BEGIN){
			if((INROF_ACTION_LINE == ActionState) && (fabs(MachinePos.getNormalizeTh() - PI/2)) < 0.5 ){	// ライントレースしてて上むいてる時にボールフィールドの上の線を越えた
				ActionState = INROF_ACTION_TURN;
				ManageActionTansakuState_BallFind_turn = 1;
				MachinePosActionManageSub = MachinePos;
			}
		}else if(Cross==INROF_CROSS_START){
			if((INROF_ACTION_LINE == ActionState) && (fabs(MachinePos.getNormalizeTh() + PI/2)) < 0.5 ){	// ライントレースしてて下むいてる時にスタートゾーン	// 多分使わない
				ActionState = INROF_ACTION_TURN;
				MachinePosActionManageSub = MachinePos;
			}
		}
	}
	
	// 状態遷移(ボール見つける処理)
	if(IsBallFinding){	// ボール見つけた
		ActionState = INROF_ACTION_BALLGET;		// ボール取得ステートへ移行
		BallFindEnable = false;					// ボール探索終了
		MachinePosActionManage = MachinePos;	// 
		MachinePosActionManageSub = MachinePos;
		ManageActionTansakuState_BallFind = 0;	// ボール探索用ステート初期化
		// どのセンサ？
		BallFindDistance = 999;
		if( (BALLFIND_MAXDISTANCE_SIDE > PsdLeftSide->getDistance()) && PsdLeftSide->isObjectBallArea() ){
			if(BallFindDistance > PsdLeftSide->getDistance()){	// 他のセンサより近ければそれを適用
				BallDir = 1;	// 左回り
				BallFindDistance = PsdLeftSide->getDistance();
			}
		}
		if( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdRight->getDistance()) && PsdRight->isObjectBallArea() ){
		// 左横か、右クロスだったら左回りする
			if(BallFindDistance > PsdRight->getDistance()){	// 他のセンサより近ければそれを適用
				BallDir = 1;
				BallFindDistance = PsdRight->getDistance();
			}
		}
		if( (BALLFIND_MAXDISTANCE_SIDE > PsdRightSide->getDistance()) && PsdRightSide->isObjectBallArea() ){
			if(BallFindDistance > PsdRightSide->getDistance()){
				BallDir = 0;	// 右
				BallFindDistance = PsdRightSide->getDistance();
			}
		}
		if ( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdLeft->getDistance()) && PsdLeft->isObjectBallArea() ){
		// 右横か、左クロスだったら右回りする
			if(BallFindDistance > PsdLeft->getDistance()){	
				BallDir = 0;	// 右
				BallFindDistance = PsdLeft->getDistance();
			}
		}
	}
	
	
	
	// 動作
	switch(ActionState){
	case INROF_ACTION_IDLE:		// 
		ActionState = INROF_ACTION_LINE;
		setTgtVel_LineTrace();
		ManageActionTansakuState_BallFind = 0;
		break;
	case INROF_ACTION_TURN:		// 180°回る
		//adjustMachinePos();	// ライントレースではないけど交差点で判定することが多いので
		switch(ManageActionTansakuState_BallFind_turn){
		case 0:	// ちょっと下がる
			if(100.0 < (MachinePos-MachinePosActionManageSub)){	// 100mm以上離れた
				ManageActionTansakuState_BallFind_turn = 1;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{		// 離れるまで下がる
				this->SpdTgt = -100;		// [mmps]
				this->AngVelTgt = 0;	// [rps]
				BallFindEnable = false;
			}
			break;
		case 1:	//180度旋回
			if(0.05 > fabs(MachinePos.Th - (MachinePosActionManageSub.Th + PI))){
				//fFinishAction = 1;				// 180°回ったので次のステートへ
				ManageActionTansakuState_BallFind_turn = 0;
				ActionState = INROF_ACTION_LINE;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				BallFindEnable = true;
			}else{		// 180°回るまで回転
				this->SpdTgt = 0;		// [mmps]
				this->AngVelTgt = 0.2 * ((MachinePosActionManageSub.Th + PI) - MachinePos.Th);	// [rps]
				BallFindEnable = false;
			}
			break;
		}
		break;
	case INROF_ACTION_BALLGET:	// ボールがいたので掴まえる
		switch(ManageActionTansakuState_BallFind){
		case 0:		// ボールなくなるまで反対向く(もともと目の前にボールがあった時用)
			if((BallFindDistance+30 < PsdFront->getDistance()) || (0>PsdFront->getDistance())){ 	// 指定値より大きい or 無効値
				ManageActionTansakuState_BallFind = 1;				// 20°回ったので次のステートへ
				this->SpdTgt = 0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{
				this->SpdTgt = 0;		// [mmps]
				if(BallDir){	// ボールは左方向
					this->AngVelTgt = -0.05;	// 右回転[rps]
				}else{
					this->AngVelTgt = 0.05;	// [rps]
				}
			}
			
			break;
			
		case 1:		// ボール見つけるまで回る
		
			if(fabs(MachinePos.Th - MachinePosActionManageSub.Th) > PI/2){		// 90度以上回ったらやめて探索再開
				ManageActionTansakuState_BallFind = 40;
		
			//}else if(( BallFindDistance+30 > PsdFront->getDistance()) && PsdFront->isObjectBallArea()){		//指定値より小さい and 有効範囲
			}else if(( (BallFindDistance+30) > PsdFront->getDistance()) && (0 < PsdFront->getDistance())){		// ボール見つけた(指定値より小さい and 有効範囲)
			//}else if(( BallFindDistance+30 > PsdFront->getDistance())){		//指定値より小さい -> 無効値入ったら反応しちゃうのでボツ
					// 対象のボール位置より小さければ見つけたと判定。ちょっと余裕持って+30mmしてる
				ManageActionTansakuState_BallFind = 2;				// ボール見つけたので次のステートへ
				BallFindTh = MachinePos.Th;
			}else{
				if(BallDir){	// ボールは左方向
					//旋回
					this->SpdTgt = 0;		// [mmps]
					this->AngVelTgt = 0.04;	// [rps]
				}else{			// ボールは右方向
				//-旋回
					this->SpdTgt = 0;		// [mmps]
					this->AngVelTgt = -0.04;	// [rps]
				}
			}
			break;
			
			
		case 2:		// フロントセンサでボール見つけた。ボールなくなるまで更に回るよ
			if(fabs(MachinePos.Th - MachinePosActionManageSub.Th) > (PI*2/3)){		// 120度以上回ったらやめて探索再開
				ManageActionTansakuState_BallFind = 40;
		
			}else if((BallFindDistance+30 < PsdFront->getDistance()) || (0>PsdFront->getDistance())){ 	// 指定値より遠い or 無効値(見えないくらい遠い)
				ManageActionTansakuState_BallFind = 3;				// ボールなくなったので次のステートへ
				BallFindTh = (MachinePos.Th + BallFindTh)/2;
				if(BallDir){	// 調整
					BallFindTh = BallFindTh - 0.05;
				}else{
					BallFindTh = BallFindTh + 0.05;
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
			if(fabs(BallFindTh - MachinePos.Th) < 0.01){ 	// ボールのほう向いた
				ManageActionTansakuState_BallFind = 4;				// ボールのほう向いたので次のステートへ
				this->SpdTgt = 0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{
				this->SpdTgt = 0;		// [mmps]
				if(!BallDir){
					this->AngVelTgt = 0.02;	// [rps]
				}else{
					this->AngVelTgt = -0.02;
				}
			}
			break;
			
			
		case 4:		// ボールのほう向いた。前後距離合わせるよ
			//旋回
			if((BALLFIND_MAXDISTANCE_FRONT<PsdFront->getDistance()) || (0>PsdFront->getDistance() )){	// ボール無い or 無効値
				ManageActionTansakuState_BallFind = 40;
			}else if(fabs(PsdFront->getDistance() - BALLGET_DISTANCE_FRONT) < 10){ 	// ボールとの距離合わせた
				ManageActionTansakuState_BallFind = 5;				// 距離あわせたので次のステートへ
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				StartTimeActionManage = getTime();	// 時刻
				
				// アーム下ろす
				Inrof.ServoF->setGoalTime(50);
				Inrof.ServoR->setGoalTime(50);
				ServoF->setGoalPosition(SERVOPOS_BALLGET_F);
				ServoR->setGoalPosition(SERVOPOS_BALLGET_R);
				
			}else{
				// フロント距離センサでボールとの距離を合わせる
				this->SpdTgt = 5 * (PsdFront->getDistance() - BALLGET_DISTANCE_FRONT);		// [mmps]
				// 最低速度規制？
				if(100<SpdTgt && 0<SpdTgt){
					this->SpdTgt = 100;
				}else if(-100>SpdTgt && 0>SpdTgt){
					this->SpdTgt = -100;
				}
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
			
			
		case 5:		// 前後距離合わせた。アーム下ろすよ
			if(StartTimeActionManage + 800 < getTime() /* サーボ動作完(500msec) */){ 	// サーボ下げた
				ManageActionTansakuState_BallFind = 6;
				MachinePosActionManageSub = MachinePos;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
			}
			break;
			
			
		case 6:		// アーム下ろした。前進するよ
			if((BALLGET_DISTANCE_FRONT-20) < (MachinePosActionManageSub - MachinePos)){ 	// 前進完
				ManageActionTansakuState_BallFind = 7;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				StartTimeActionManage = getTime();	// 時刻
				
				// アーム上げる
				Inrof.ServoF->setGoalTime(50);
				Inrof.ServoR->setGoalTime(200);
				ServoF->setGoalPosition(SERVOPOS_BALLUP_F);
				ServoR->setGoalPosition(SERVOPOS_BALLUP_R);
				
			}else{
				this->SpdTgt = 1.0*(BALLGET_DISTANCE_FRONT - (MachinePosActionManageSub - MachinePos));		// [mmps]
				//SpdTgt = 30;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
			
			
		case 7:		// 前後距離合わせた。アーム上げるよ
			if(StartTimeActionManage + 800 < getTime()){ 	// サーボ上げた
				ManageActionTansakuState_BallFind = 8;
				this->SpdTgt = -10.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
				// 走行姿勢 -> ボール色判定姿勢
				Inrof.ServoF->setGoalTime(50);
				Inrof.ServoR->setGoalTime(50);
				//Inrof.ServoF->setGoalPosition(SERVOPOS_RUN_F);
				//Inrof.ServoR->setGoalPosition(SERVOPOS_RUN_R);
				Inrof.ServoF->setGoalPosition(SERVOPOS_COLORCHECK_F);
				Inrof.ServoR->setGoalPosition(SERVOPOS_COLORCHECK_R);
				
				// 色判定開始
				fJudgeColor = true;
				StartTimeActionManage = getTime();	//判定のための時間測定用
				
			}else{
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
				// アーム上げる
				
			}
			break;
			
			
		case 8:		// ボールとった！！！！ラインへ戻るよ
			if( 5.0 > (((ORIENT_WEST == MachineOrientation) || (ORIENT_EAST == MachineOrientation))?fabs(MachinePosActionManage.X - MachinePos.X):fabs(MachinePosActionManage.Y - MachinePos.Y)) ){
			//if( ((MachinePos.X > MachinePosActionManage.X) && ((MachinePos.getNormalizeTh()>PI/2) || (MachinePos.getNormalizeTh()<(-PI/2))))
			//	||  ((MachinePos.X < MachinePosActionManage.X) && ((MachinePos.getNormalizeTh()<=PI/2) && (MachinePos.getNormalizeTh()>=(-PI/2))))
			//){ 	// ライン上に戻ってきた
				ManageActionTansakuState_BallFind = 9;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
				
			}else{
				// まっすぐバック
				/*
				if((MachinePos.getNormalizeTh() > PI/2) || (MachinePos.getNormalizeTh() < -PI/2)){	// 左向いてる
					SpdTgt = -3 * (MachinePosActionManage.X - MachinePos.X);		// [mmps]
				}else{																				// 右向いてる
					SpdTgt = 3 * (MachinePosActionManage.X - MachinePos.X);
				}*/
				if(ORIENT_WEST == MachineOrientation){	// 西向いてる
					this->SpdTgt = -3 * (MachinePosActionManage.X - MachinePos.X);		// [mmps]
				}else if(ORIENT_EAST == MachineOrientation){	// 東																		// 右向いてる
					this->SpdTgt = 3 * (MachinePosActionManage.X - MachinePos.X);
				}else if(ORIENT_SOUTH == MachineOrientation){	// 南向いてる
					this->SpdTgt = -3 * (MachinePosActionManage.Y - MachinePos.Y);		// [mmps]
				}else{											// 北向いてる
					this->SpdTgt = 3 * (MachinePosActionManage.Y - MachinePos.Y);
				}
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
		case 9:		// 北向く
			if( (fabs(MachinePos.getNormalizeTh()-PI/2) < 0.15) && (StartTimeActionManage + 1000 < getTime())){	// だいたい北向いた＆ボール判定時間経った。							// かつ、ボール色センサ起動して1000ms以上経っていること。
				ManageActionTansakuState_BallFind = 10;
				
				//return true;		
			}else{
				// 北を向く(P制御)
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.2 * (PI/2 - ((MachinePos.getNormalizeTh()< -PI/2)?MachinePos.getNormalizeTh()+2*PI:MachinePos.getNormalizeTh()));	// [rps]
				//回転速度に制限
				if(0.2<AngVelTgt){
					this->SpdTgt = 0.2;
				}else if(-0.2>AngVelTgt){
					this->SpdTgt = -0.2;
				}
			}
			
			break;
		
		case 10:	// ライン乗るまでさらに回る
		
				// ラインあるかチェック
			if((getLineSensAd(1)<LINESENS_THRESHOLD_CENTER)||(getLineSensAd(2)<LINESENS_THRESHOLD_CENTER)){	// 中央２つのセンサいずれかが黒くなるまで
				//ボール色チェック
				BallColorFixed = BallColor;
				
				// ボール持ってなかったら！！！
				// また探索！！！
				if(INROF_NONE == BallColorFixed){
					ManageActionTansakuState_BallFind = 40;
				}else{
					// ボール持ってるから置きに行く
					this->SpdTgt = 0.0;		// [mmps]
					this->AngVelTgt = 0.0;	// [rps]
					Inrof.ServoF->setGoalTime(500);
					Inrof.ServoR->setGoalTime(500);
					Inrof.ServoF->setGoalPosition(SERVOPOS_RUN_F);
					Inrof.ServoR->setGoalPosition(SERVOPOS_RUN_R);
					
					
					State = INROF_TOGOAL;	// 次の状態へ。ボールを置きに行く。
					ActionState = INROF_ACTION_IDLE;
					ManageActionTansakuState_BallFind = 0;
				}
				
				fJudgeColor = false;
			}else{
				//ラインなければまわる
				//  回りすぎちゃったらどうしよう
				this->SpdTgt = 0.0;		// [mmps]
				if(BallDir){	// 左のボールをとった
					// もっと回る(左回り)
					this->AngVelTgt = 0.05;
				}else{
					// もっと回る(右回り)
					this->AngVelTgt = -0.05;
				}
			}
			break;
			
// ボール見つけたけど見失った時用ステート
		case 40:	//ボール見つけたと思ったけど見失った
			// ライン上に戻る
			if( 5.0 > (((ORIENT_WEST == MachineOrientation) || (ORIENT_EAST == MachineOrientation))?fabs(MachinePosActionManage.X - MachinePos.X):fabs(MachinePosActionManage.Y - MachinePos.Y)) ){
			//if( 5.0 > fabs(MachinePosActionManage.X - MachinePos.X) ){
			//if( ((5.0 > fabs(MachinePosActionManage.X - MachinePos.X)) && ((MachinePos.getNormalizeTh()>PI/2) || (MachinePos.getNormalizeTh()<(-PI/2))))
			//	||  ((MachinePos.X < MachinePosActionManage.X) && ((MachinePos.getNormalizeTh()<=PI/2) && (MachinePos.getNormalizeTh()>=(-PI/2))))
		//	){ 	// ライン上に戻ってきた
				ManageActionTansakuState_BallFind = 41;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{
				/*
				if((MachinePos.getNormalizeTh() > PI/2) || (MachinePos.getNormalizeTh() < -PI/2)){	// 左向いてる
					SpdTgt = -5 * (MachinePosActionManage.X - MachinePos.X);		// [mmps]
				}else{																				// 右向いてる
					SpdTgt = 5 * (MachinePosActionManage.X - MachinePos.X);
				}
				*/
				if(ORIENT_WEST == MachineOrientation){	// 西向いてる
					this->SpdTgt = -3 * (MachinePosActionManage.X - MachinePos.X);		// [mmps]
				}else if(ORIENT_EAST == MachineOrientation){	// 東																		// 右向いてる
					this->SpdTgt = 3 * (MachinePosActionManage.X - MachinePos.X);
				}else if(ORIENT_SOUTH == MachineOrientation){	// 南向いてる
					this->SpdTgt = -3 * (MachinePosActionManage.Y - MachinePos.Y);		// [mmps]
				}else{											// 北向いてる
					this->SpdTgt = 3 * (MachinePosActionManage.Y - MachinePos.Y);
				}
				
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
		case 41:	//続・ボール見つけたと思ったけど違った
			// 向きを元に戻して探索再開
			if(0.05 > fabs(MachinePosActionManage.Th - MachinePos.Th)){
				ActionState = INROF_ACTION_LINE;	// 引き続き探索します
				ManageActionTansakuState_BallFind = 0;
			}else{
				SpdTgt = 0.0;		// [mmps]
				AngVelTgt = 0.15*(MachinePosActionManage.Th - MachinePos.Th);	// [rps]
			}
			break;
			
			
		case 99:	// 一時停止	
			if(StartTimeActionManage + 1000 < getTime() ){
				ManageActionTansakuState_BallFind = ManageActionTansakuState_BallFind_last+1;
			}else{
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
		}
		break;
		
		
	case INROF_ACTION_LINE:	// ライントレース
		float x, y;
		PsdFront->getGlobalPos(&x, &y);
		if((x > 980) && (y < 1500)){	// フィールド上ではゆっくり＆ボール探索有効
			BallFindEnable = true;
			setTgtVel_LineTrace(200.0);
		}else{
			setTgtVel_LineTrace(240.0);
		}
		
		
		// あれっ？ボール持ってるよ！
		
		
		break;
	
		
	
	}
	
	
	
	return false;
}
	
bool_t inrof2015::manageAction_goToGoal(inrof_color Color){
	
	// 動作
	switch(ActionState){
	case INROF_ACTION_IDLE:	// 最初に入る
		ActionState = INROF_ACTION_BALLRELEASE;
		ManageActionTansakuState_BallRelease = 0;
		MachinePosActionManage = MachinePos;	// この状態に入った時の場所からxxmm以内は場所補正しない用
		//break;
		/*
	case INROF_ACTION_LINE:		// 
		setTgtVel_LineTrace();	// とりあえずライントレース
		break;
		*/
	case INROF_ACTION_BALLRELEASE:
		switch(ManageActionTansakuState_BallRelease){
		case 0:	// 目標交差点までライントレース
			
			// ゴールの交差点に来たー
			if(	IsCrossing 
				&&(	((INROF_RED == BallColorFixed) && (INROF_CROSS_RED == Cross))
					|| ((INROF_YELLOW == BallColorFixed) && (INROF_CROSS_YELLOW == Cross))
					|| ((INROF_BLUE == BallColorFixed) && (INROF_CROSS_BLUE == Cross))
				)
			){	
				adjustMachinePos();	// 角度が、-PI～PIの範囲になるので注意
				
				ManageActionTansakuState_BallRelease = 1;
				MachinePosActionManageSub = MachinePos;
				
				ManageActionLineTraceDir = LineTraceDir;
				
				
			}else{	
				if(200 > (MachinePosActionManage - MachinePos)){	// ライントレースし始まって200mm以下しか進んでなければ位置補正しない。
					fAdjustMachinePos = false;
				}
				if(INROF_CROSS_START==Cross || INROF_CROSS_NONE==Cross){
					// 赤交差点よりスタート位置側だったら直進
					SpdTgt = 150.0;
					AngVelTgt = 0.0;
					fAdjustMachinePos = true;
				}else{
					setTgtVel_LineTrace(240.0);	// ライントレース
					fAdjustMachinePos = true;
				}
			}
			break;
			
		case 1:	// 赤or黄色の交差点に来た。BackWordなら右ターンするよ。Forwardなら左ターンだよ(自由ボール時)
			// -90°回る or 青なら捨てる(すぐ次の状態へ)
			//if( ((fabs(fabs(MachinePos.Th - MachinePosActionManageSub.Th)-PI/2) < 0.1)&&(INROF_LINEDIR_FORWARD != ManageActionLineTraceDir))
			if( (fabs(fabs(MachinePos.Th - MachinePosActionManageSub.Th)-PI/2) < 0.1)
				//|| (((MachinePos.Th - MachinePosActionManageSub.Th) > (PI/2 - 0.1))&&(INROF_LINEDIR_BACKWARD != ManageActionLineTraceDir))
				|| (INROF_BLUE == BallColorFixed)
				//|| (INROF_YELLOW == BallColorFixed)
			){
				ManageActionTansakuState_BallRelease = 2;
				StartTimeActionManage = getTime();
				
				SpdTgt = 0.0;
				AngVelTgt = 0.0;
				
				Inrof.ServoR->enableTorque(CMDSV_TORQUE_OFF);
				Inrof.ServoF->setGoalTime(50);
				//Inrof.ServoR->setGoalTime(40);
				Inrof.ServoF->setGoalPosition(SERVOPOS_BALLTRASH_F);
				Inrof.ServoR->setGoalPosition(SERVOPOS_BALLTRASH_R);
				
		outPin(GR_LED3, 0);
			}else{
		outPin(GR_LED3, 1);
				if(INROF_LINEDIR_FORWARD == ManageActionLineTraceDir){	// ライン正方向なら
					setTgtVel_Turn(VEL_NORMAL, LINESENS_POS_X+50);		// 左旋回
				}else{
					setTgtVel_Turn(VEL_NORMAL, -(LINESENS_POS_X+50));	// 速度、旋回半径指定
				}
			}
			break;
			
		case 2:	// 赤or黄色の交差点でターン完了した。ボール放出
			if( StartTimeActionManage + 1500 < getTime() ){	// ボール投げた
				ManageActionTansakuState_BallRelease = 3;
				SpdTgt = 0.0;
				AngVelTgt = 0.0;
				
				
				Inrof.ServoR->enableTorque(CMDSV_TORQUE_ON);
				Inrof.ServoF->setGoalTime(500);
				Inrof.ServoR->setGoalTime(500);
				Inrof.ServoF->setGoalPosition(SERVOPOS_RUN_F);
				Inrof.ServoR->setGoalPosition(SERVOPOS_RUN_R);
				
			}else{
				//setTgtVel_LineTrace();
				this->SpdTgt = 0.0;
				this->AngVelTgt = 0.0;
			}
			break;
		case 3: 	// ボール捨てたから下がりながら右回転する
			if( ((fabs(fabs(MachinePos.Th - (MachinePosActionManageSub.Th))-PI) < 0.1)&&(INROF_LINEDIR_FORWARD != ManageActionLineTraceDir))		// 北(青以外)・東(青)を向く(元の向きの反対を向く)
				|| (((fabs(MachinePos.Th - MachinePosActionManageSub.Th) < 0.1))&&(INROF_LINEDIR_BACKWARD != ManageActionLineTraceDir))	// 北向く(FORWARDなので元の向きと同じ)
			){
				ActionState = INROF_ACTION_LINE;			// 探索再開
				
				State = INROF_TANSAKU;
				ActionState = INROF_ACTION_IDLE;
				ManageActionTansakuState_BallRelease = 0;
				
			}else{
				if(INROF_BLUE == BallColorFixed){
					this->SpdTgt = 0.0;
					this->AngVelTgt = 0.2 * ( (MachinePosActionManageSub.Th - PI) - MachinePos.Th );  // 右180°回転
					if(0.2<AngVelTgt){
						this->SpdTgt = 0.2;
					}else if(-0.2>AngVelTgt){
						this->SpdTgt = -0.2;
					}
				}else{
					setTgtVel_Turn(-VEL_NORMAL, (LINESENS_POS_X+50));	// 下がりながら右回転。
				}
			}
			break;
			
		
		}
		break;
		
	}
	
	return false;
}

bool_t inrof2015::manageAction_goToStart(void){
	
	return false;
}


/*************
ライントレースするように目標速度設定
ラインがない、もしくはスタートの交差点過ぎたとこなら直進
*************/
void inrof2015::setTgtVel_LineTrace(void){
	
	
	setTgtVel_LineTrace(VEL_NORMAL);
	
}
void inrof2015::setTgtVel_LineTrace(float SpdTgt){
	
	
	this->SpdTgt = SpdTgt;
	
	
	if(INROF_CROSS_START==Cross){
		this->AngVelTgt = 0;	
	}else{
		if((getLineSensAd(1)<LINESENS_THRESHOLD_CENTER)||(getLineSensAd(2)<LINESENS_THRESHOLD_CENTER)){
			// 中央2つのセンサがライン乗ってる
			this->AngVelTgt = 0.000005*(getLineSensAd(1) - getLineSensAd(2));	// ライントレース
			
			if(fAdjustMachinePos){
				adjustMachinePos();
			}
			
		}else if((getLineSensAd(0)<LINESENS_THRESHOLD_SIDE)||(getLineSensAd(3)<LINESENS_THRESHOLD_SIDE)){
			// 左右2つのセンサどちらかライン乗ってる
			this->AngVelTgt = 0.000004*(getLineSensAd(0) - getLineSensAd(3));	// ライントレース
			
			this->SpdTgt = SpdTgt/2;
		}else{
			// どっちも乗ってないのでまっすぐ
			this->AngVelTgt = 0;	
		}
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
void inrof2015::setTgtVel_Turn(float SpdTgt, float Radius){
	
	this->SpdTgt = SpdTgt;							// [mm/s]
	//this->AngVelTgt = SpdTgt / Radius / (2*PI) /4 ;	// [rev/s]
	this->AngVelTgt = this->Spd / Radius / (2*PI) /2;	// [rev/s]
	
}








/*********************
走行制御
概要：
 PID制御で、直進成分±回転成分のDutyを出力する。
 Dutyは、signedのプラスマイナスで前進後進を表す。
**********************/
void inrof2015::ctrlRun(float SpdTgt_mmps, float AngVelTgt_rps, int32_t* RTireDutyTgt, int32_t* LTireDutyTgt){
	static ctrl_pid PidCtrlRTire(SAMPLETIME_CTRL_S, 200000,1000,1, -MotorR->getPeriodCnt(),MotorR->getPeriodCnt(), -1000, 1000);
	static ctrl_pid PidCtrlLTire(SAMPLETIME_CTRL_S, 200000,1000,1, -MotorL->getPeriodCnt(),MotorL->getPeriodCnt(), -1000, 1000);
	float RTireSpdTgt_rps, LTireSpdTgt_rps;
	
	// 各タイヤの目標速度
	//  目標直進速度[mm/s]と角速度[r/s]からタイヤの目標角速度[r/s]を求める
	RTireSpdTgt_rps = SpdTgt_mmps/(PI*TIRE_DIA_MM) + AngVelTgt_rps * 4 * TIRE_TREAD_MM/TIRE_DIA_MM;
	LTireSpdTgt_rps = SpdTgt_mmps/(PI*TIRE_DIA_MM) - AngVelTgt_rps * 4 * TIRE_TREAD_MM/TIRE_DIA_MM;
	
	// フィードバック制御
	if(fMotorOut){
		// PID制御
		*RTireDutyTgt = (int32_t)PidCtrlRTire.calc(RTireSpdTgt_rps - RTireSpd_rps);
		*LTireDutyTgt = (int32_t)PidCtrlLTire.calc(LTireSpdTgt_rps - LTireSpd_rps);
	}else{
		// モーター出力無効時は、PID制御にリセットをかけ、目標デューティーを0とする。
		PidCtrlRTire.resetStatus(RTireSpdTgt_rps - RTireSpd_rps);
		PidCtrlLTire.resetStatus(LTireSpdTgt_rps - LTireSpd_rps);
		*RTireDutyTgt = 0;
		*LTireDutyTgt = 0;
	}
}



/*********************
モーター出力
概要：
 Duty正負に応じて正転逆転を切り替える。
 fMotorOutがfalseの時、モーターをフリー状態とする。
**********************/
void inrof2015::outTyres(int32_t RTireDutyTgt, int32_t LTireDutyTgt){
	
	if(fMotorOut){
		if(RTireDutyTgt<0){
			MotorR->setDir(RTIRE_DIR_BK);
			MotorR->setDutyCnt(-RTireDutyTgt);
		}else{
			MotorR->setDir(RTIRE_DIR_FW);
			MotorR->setDutyCnt(RTireDutyTgt);
		}
		if(LTireDutyTgt<0){
			MotorL->setDir(LTIRE_DIR_BK);
			MotorL->setDutyCnt(-LTireDutyTgt);
		}else{
			MotorL->setDir(LTIRE_DIR_FW);
			MotorL->setDutyCnt(LTireDutyTgt);
		}
	}else{
		MotorR->setDir(FREE);
		MotorR->setDutyCnt(0);
		MotorL->setDir(FREE);
		MotorL->setDutyCnt(0);
	}
	
}



/*********************
ラインセンサ位置をグローバル座標系で取得
引数：
	(返り)*X
	(返り)*Y
**********************/
void inrof2015::getLineSenseGlobalPos(position* Pos){
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
void inrof2015::getBattVoltage(void){
	
	this->BattVoltage = ServoF->getPresentVolts();	// 電圧計測値取得
	if(BATT_LOW>=(this->BattVoltage)){
		this->fBattEmpty = true;
	}else{
		this->fBattEmpty = false;
	}
	
	ServoF->updatePresentVolts();	// 次の更新(計測要求
	
}





// 1msごとの割り込み設定
// TMR0
// 割り込み周期 46 / (PCLK/1024) = 0.981msec
void inrof2015::setup1msInterrupt(void){
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
void inrof2015::setupFreeCounter(void){
	MSTP(TPU6) = 0;					// モジュール起動
	TPU6.TCR.BIT.TPSC = 3;	// PCLK/4
	TPU6.TCR.BIT.CKEG = 0;	// 立ち上がりエッジでカウント
	TPU6.TCR.BIT.CCLR = 0;	// クリアなし
	TPU6.TMDR.BIT.MD = 0;	// 通常動作
	
	TPUB.TSTR.BIT.CST6 = 1;	// カウント開始
	
}


extern void isr1ms(void);
// 割り込み関数の設定
#pragma interrupt (Excep_TMR0_CMIA0(vect=VECT(TMR0,CMIA0), enable))
void Excep_TMR0_CMIA0(void){
	
	Inrof.isr1ms();
	//isr1ms();
	
}
