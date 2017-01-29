/**************************************************
inrof2016_lib.h
	知能ロボコン2016用のライブラリたち。
	
	
	
	
**************************************************/


#ifndef __INROF2016_H__
#define __INROF2016_H__

//#include <sdmmc.h>

#include "GR_define.h"
#include "inrof2016_common.h"

#include "AppliCtrl.h"

//#include "RIIC.h"
//#include "SCI.h"

#include "inrof2016_upper_ctrl.h"
#include "Inrof2016_aplSci.h"


#include "inrof2016_ball_sensor.h"
#include "inrof2016_sharp_dist.h"
#include "inrof2016_st_vl.h"
#include "inrof2016_color_sens.h"

#include "RoombaOi.h"

//#include "FutabaCmdServo.h"
//#include "OledDriverSSD1306.h"
//#include "SharpGP2Y0E.h"
#include "HamamatsuColorSensS11059.h"
#include "InvensenseMPU.h"


// define
//#define PI 3.1415
#define	INROF_LINETRACE_VEL_RATELIMIT	300.0	// [mm/s/s]
#define INROF_LINETRACE_ONLINECNT	50	// オンラインと判定する時間

typedef struct calc_odmetry{
	float REncCntDiff[2];
	float LEncCntDiff[2];
	float CosTh[2];
	float SinTh[2];
	float dth2[2];
}calc_odmetry;

enum inrof_state{
	INROF_INITIALIZE,	// 初期化中
	INROF_IDLE,
	INROF_TANSAKU,
	INROF_TOGOAL,
	INROF_TOSTART,
	INROF_STARTCALIB	// スタート時キャリブレーション
};
// 走行モード
enum inrof_drive_mode{
	INROF_DRIVE_AUTO,	// 自動
	INROF_DRIVE_PS3		// PS3コン。SCI0からのデータをそのまま流す
};

enum inrof_action_state{
	INROF_ACTION_IDLE,		// 0
	INROF_ACTION_LINE,		// 1
	INROF_ACTION_STRAIGHT,		// 2
	INROF_ACTION_TURN,		// 3
	INROF_ACTION_BALLGET,		// 4
	INROF_ACTION_BALLRELEASE,	// 5
	INROF_ACTION_OVERBALL,		// 6 ボール踏んだので下がって再開
	INROF_ACTION_TOFINISH,		// 7 スタートゾーン戻ったあとの処理
	INROF_ACTION_IMUCALIB,		// 8 MPU9250のキャリブレーション
	INROF_ACTION_GURUGURUTANSAKU	// 9 周囲見ながら探索
};
/*
enum inrof_action_state{
}
*/
// 走行向き
enum inrof_linedir{
	INROF_LINEDIR_NONE,
	INROF_LINEDIR_FORWARD,
	INROF_LINEDIR_BACKWARD
};

enum inrof_cross_index{
	INROF_CROSS_NONE	= 0,	// 0
	INROF_CROSS_START	= 1,	// 1
	INROF_CROSS_RED		= 2,	// 2
	INROF_CROSS_YELLOW	= 3,	// 3
	INROF_CROSS_BLUE	= 4,	// 4
	INROF_CROSS_BALLFIELD_BEGIN	= 5,	// 5
	INROF_CROSS_BALLFIELD_END	= 6,	// 6
	
	INROF_CROSS_GOAL_RED,			// 7
	INROF_CROSS_GOAL_YELLOW,		// 8
	INROF_CROSS_GOAL_BLUE,			// 9
	
	INROF_CROSS_BLUE_EDGE			// 10 (青ゴールの前。ライン無い所)
	
};

enum inrof_orientation{
	ORIENT_NONE, 
	ORIENT_EAST,
	ORIENT_NORTH,
	ORIENT_WEST,
	ORIENT_SOUTH,
};


/****************************
 知能ロボコン2016

概要：
 制御周期は1ms, 10ms。
 メイン関数で使えるよう、200msecごとに立つフラグ(fCnt200)を用意
 
****************************/
class inrof2016{
public:
	// ルンバ
	RoombaOi* Rmb;
	// 通信
	//inrof2016_aplSci_t* ExtSci;	// SBDBT基板との通信(PC, PS3コン)
	
	// 通信系状態
	// ルンバ
	// I2C
	int8_t ComuStatusI2C;
	// サーボ
	int8_t ComuStatusSv;
	
	// フィールド
	// ボールの残り数
	uint8_t FieldBall;
	
	// センサ
	// 測距センサ
	//inrof2016_ball_sensor_t *PsdLeft;
	//inrof2016_ball_sensor_t *PsdCenter;
	//inrof2016_ball_sensor_t *PsdRight;
	inrof2016_st_vl_t *PsdCenter;
	inrof2016_st_vl_t *PsdRight;
	inrof2016_st_vl_t *PsdLeft;
	void resetBallSensors(void);
	
	// カラー
	//hamamatsu_S11059* ColorSens;
	
	// IMU
	invensense_MPU* Mpu;
	// エンコーダ
	gr_ex_encoder* EncR;
	gr_ex_encoder* EncL;
	
	
	bool_t fCnt200;
	// 目標速度, 角速度
	float SpdTgt, AngVelTgt;	//privateにしたいな
	float SpdTgtRated, AngVelTgtRated;	//privateにしたいな
	
	
	// 初期化
	inrof2016(void);
	void begin(void);	// オブジェクト生成
	void initialize(void);	// フラグ系リセット
	void reqInitPosture(void);	// 初期姿勢にする
	
	// モード
	void setMode(inrof_drive_mode NewMode);
	inrof_drive_mode getMode(void){return DriveMode;};
	
	// 処理
	void step(void);
	
	// 許可系
	void enableOutTyre(void){this->fMotorOut = true; InrofUpper.enableAction();};
	void disableOutTyre(void){this->fMotorOut = false; InrofUpper.disableAction();};
	bool_t isEnableOutTyre(void){return this->fMotorOut;};
	
	// ラインセンサ
	uint16_t getLineSensAd(uint8_t LineSensNum){return LineSensAd[LineSensNum];};
	void getLineSenseGlobalPos(position* Pos);
	//マシン状態取得
	float getRTireSpd(void){return RTireSpd_rps;};
	float getLTireSpd(void){return LTireSpd_rps;};
	position* getMachinePos(void){return &MachinePos;};
	
	// メインバッテリ、ルンババッテリーある？
	bool_t isBattLow(void){return fBattEmpty || ((BATT_LOW_RMBSOC > Rmb->getBatterySoc())&&(0!=Rmb->getBatterySoc()));};	//0(通信できてない)じゃなくてLOW_SOCより小さい
	// メインバッテリ電圧取得
	float getBattVoltage(void){return BattVoltage;};	// バッテリ電圧取得
	
	
	void setState(inrof_state State){ this->State = State; ActionState = INROF_ACTION_IDLE;};
	int8_t getState(void){return State;};
	
	
	// ネタ
	int8_t startClean(void){
		if(false != fCleaning){ return -1;};
		fCleaning = true;
		return Rmb->sendCommand(ROI_OPCODE_MOTORS, (uint8_t)0x03);
	};	// Vacume, SideBrush
	int8_t stopClean(void){
		if(false == fCleaning){ return -1;};
		fCleaning = false;
		return Rmb->sendCommand(ROI_OPCODE_MOTORS, (uint8_t)0x00);
	};
	bool_t isCleaning(void){return fCleaning;};
	
	// ユーティリティ
	//uint32_t getTime_ms(void){return Time_msec;};	// 起動からの時間
	
	// TPU6を使ったフリーカウンタ
	void setupFreeCounter(void);
	uint16_t getFreeCounter(void){return TPU6.TCNT;};
	bool_t isFreeCounterOvf(void){return TPU6.TSR.BIT.TCFV;};
	void clearFreeCounterOvfFlag(void){TPU6.TSR.BIT.TCFV = 0;};
	
	
	// 1ms割り込み
	void isr1ms(void);
	
	
	// シリアルコマンド
	static int8_t SerCom_resetBallSensors(uint8_t* Data);
	
	
	bool_t fFirstGuruGuruFin;			// 初めはぐるぐるやる。
private:
// private member
	// 足回り制御状態
	inrof_drive_mode DriveMode;	// プログラムで動かすか、PS3コンで動かすか
	
	bool_t fExeReqTask0;
	bool_t fExeReqTask1;
	bool_t fExeReqTask2;
	
	uint16_t CntTask1;
	uint16_t CntTask2;
	
	//uint32_t Time_msec;	// 起動からの時間
	
	// 動作状態
	inrof_state State;
	inrof_state StateLast;
	int8_t fFinishManageAction;
	inrof_action_state ActionState;
	inrof_action_state ActionStateLast;
	int8_t SubActionState;
	int8_t SubActionStateLast;
	int8_t fFinishAction;
	bool_t fNextAction;	// 次の状態へ移るフラブ
	bool_t fReqBallTrash;	// ボール捨てる要求出した
	
	// キャリブレーション
	bool_t fStartCalibed;	// 開始時キャリブ完了フラグ
	bool_t fGyroCalibed;	// ジャイロキャリブ完了フラグ
	
	// 向かってる場所
	inrof_goal Goal;
	
	// モーター出力許可
	bool_t fMotorOut;
	
	// ラインセンサ
	uint16_t LineSensAd[LINESENS_NUM_SENSE];	// ラインセンサAD値
	
	
	//// マシン状態 ////
	
	// バッテリ
	float BattVoltage;		// バッテリ電圧 [V]
	bool_t fBattEmpty;		// バッテリないよフラグ
	float updateBattVoltage(void);		// バッテリ電圧取得(BattVoltageにセット)
	
	//オドメトリ関係
	uint16_t REncCnt, LEncCnt;	// エンコーダ値
	int16_t REncCntDiff, LEncCntDiff;	// エンコーダ前回との差
	float RTirePos_r, LTirePos_r;		// タイヤ角度[rev]
	float RTireSpd_rps, LTireSpd_rps;		// タイヤ角速度[rps]
	position MachinePos;				// マシン位置
	float Spd;							// マシン速度
	position LastFindCrossPos;			// 最後に交点を発見した時のマシン位置
	calc_odmetry CalcOdmetry;			// オドメトリ計算用
	inrof_orientation MachineOrientation;			// マシンの向き(4方向)
	bool_t fAdjustMachinePos;
	void initMachinePos(float x = 0.0, float y = -200.0, float th = PI/2.0);	// マシン位置初期化
	
	// 走行状態
	inrof_linedir LineTraceDir;			// 走行向き
	/// 走行制御
	int16_t RTireDutyTgt, LTireDutyTgt;		// 現在duty
	ctrl_pid* PidCtrlRTire;				// タイヤPID
	ctrl_pid* PidCtrlLTire;				//  速度→デューティー
	ctrl_ratelimiter_abs* RateLimitSpdTgt;		// 車体速度レートリミッタ [mm/s/s]
	ctrl_ratelimiter_abs* RateLimitAngVelTgt;	// 車体回転速度レートリミッタ [rad/s/s]
	
	// manageAction用
	position MachinePosActionStart;		// マシン位置 動作開始時
	int8_t ManageActionTansakuState_BallFind;	//	探索モード状態 ボール探索用 
	int8_t ManageActionTansakuState_BallFind_Last;	//	 前回値
	int8_t ManageActionTansakuState_BallFind_turn;	//	探索モード状態 ボール探索時のターン用 
	int8_t ManageActionTansakuState_BallFind_turn_Last;	//	探索モード状態 ボール探索用 
	int8_t ManageActionTansakuState_BallRelease;	//	探索モード状態 ボール投げる用 
	int8_t ManageActionTansakuState_BallReleaseLast;	//	探索モード状態 ボール投げる用 
	int8_t ManageActionTansakuState_BallOver;	//	探索モード状態 ボール探索用 
	int8_t ManageActionTansakuState_BallOver_Last;	//	 前回値
	int8_t ManageActionToStartState_ToFinish;	//	スタートへ戻る
	int8_t ManageActionToStartState_ToFinish_Last;	//	 前回値
	int8_t ManageActionSub;
	int8_t ManageActionSub_Last;
	uint32_t SeqStartTime_ms;
	
	bool_t BallFindEnable;				// ボール探索有効
	bool_t IsBallFinding;				// ボール見つけた瞬間にtrue
	inrof_linedir ManageActionLineTraceDir;		// 動作入った時の走行向き
	bool_t fGuruGuruSearch;				// ぐるぐる回りながら探索する
	position GuruGuruStartPos;			// ぐるぐる開始位置 (ライン上で北もしくは南を向いている位置)
	uint8_t GuruGuruStatus;				// ぐるぐるここまで見たフラグ
	bool_t GuruGuruStartPosSetReq;			// ぐるぐる開始位置設定要求
	// ぐるぐる探索する条件
	bool_t jdgGuruGuruSearch(void){return (fGuruGuruSearch && MachinePos.isBallArea());};
	//bool_t fFirstGuruGuruFin;			// 初めはぐるぐるやる。
	
	bool_t GoToGoalFlg;				// 時間とか経ってたらもう捨てに行くフラグ
	
	
	// ボール取得失敗カウント
	uint8_t BallGetMissCnt;
	position BallGetMissPos;
	
	// ライントレース関係
	uint16_t OnLineCnt;				// ライン上にいるカウント
	uint32_t LineTraceTimeLast;			// 最後にライントレースした時刻
	
	//交点判定
	bool_t IsCrossing;				// 交点入った判定
	inrof_cross_index Cross;	// どこの交差点を過ぎたところか
	
	// ボール取得処理用
	bool_t BallDir;				// ボールのある方向 0:右, 1:左
	position MachinePosActionManage;		// マシン位置 動作開始時
	position MachinePosActionManageSub;		// マシン位置 動作開始時
	float BallFindThStart;		// ボール見つけはじめた角度
	float BallFindTh;		// ボール見つけた角度
	uint32_t StartTimeActionManage;		// 動作開始時刻
	uint32_t StartTimeSubActionManage;	// 動作開始時刻
	int16_t BallFindDistance;		// 見つけたボールの距離
	bool_t BallGetMissFlgTmp;	// ボール取得ミスフラグ
	float NearestBallFindTh;		// 旋回中、一番近くにあるボールの角度
	int16_t NearestBallFindDistance;	// 旋回中、一番近いボールの距離
	bool_t fTurnReverse;		// 回りすぎたときとかの反転フラグ
	
	// ボールセンサ
	// 初期化
	void beginBallSensor(void);	// 開始
	void initBallSensor(void);	// 初期化(I2Cアドレス設定)
	
	
	
	
	
	// ネタ
	// お掃除
	bool_t fCleaning;
	
	// フィールド(staticでいいんだけど…)
	position PosCrossStart;
	position PosCrossRed;
	position PosCrossYellow;
	position PosCrossBlue;
	position PosCrossBallFieldBegin;
	position PosCrossBallFieldEnd;
	position PosCrossGoalRed;
	position PosCrossGoalYellow;
	position PosCrossGoalBlue;
	
// private function
	void stepTask0(void);	// 1msごと処理
	void stepTask1(void);	// 10msごと処理
	void stepTask2(void);	// 200msごと処理
	
	void setup1msInterrupt(void);
	
/***********
 入力処理
************/
	/*********************
	ラインセンサ更新
	概要：AD変換値をセット
	**********************/
	void updateLineSense(void);
	/*********************
	エンコーダ更新
	概要：	エンコーダの前回値との差分をセット
		初回実行時は差分0をセットする
	**********************/
	void updateEncoder(void);
	bool_t fResetEncoder;	// 立ってる間差分0になる
	
	
	/*********************
	タイヤパラメータ更新
	概要：	角度[rev]、角速度[rps]
	引数：
	**********************/
	void updateTyre(void);

	/*********************
	オドメトリ更新
	概要：
	引数：
	メンバ変数：MachinePos,MachineOrientation,Spd,CalcOdmetry
	**********************/
	void updateOdmetry(void);
	
/***********
 制御
************/
	
	
	/*********************
	走行制御
	概要：
	 目標速度、角速度に追従する
	引数：
	 目標速度[mmps]、角速度[rps]
	**********************/
	void ctrlRun(float SpdTgt_mmps, float AngVelTgt_rps, int16_t* RTireDutyTgt, int16_t* LTireDutyTgt);
	
	/*********************
	測距センサ値をブローバル座標へ
	概要：
	引数：
	**********************/
	void updatePsdObjPosGlobal(void);
	
	/*********************
	交点判定
	概要：交点にいる
	引数：
	戻値：交点にいるか
	**********************/
	bool_t onCross(void);
	
	/*********************
	交点判定
	概要：
	引数：
	戻値：新しい交差点に来た
	**********************/
	bool_t judgeCross(void);
	
	/*********************
	ボール見つけた判定
	概要：	ボールを見つけた時に1になる。
			条件：ボール探索フィールド上、左右センサが壁より近い値、左右斜めセンサがマシンぶつかるくらい近い値
	引数：
	戻値：新しいボール発見した
	**********************/
	bool_t judgeBallFind(void);
	bool_t BallExist;
	
	/*********************
	走行向き判定
	概要：	
	引数：なし
	戻値：なし
	**********************/
	void judgeRunOrient(void);
	
	/*********************
	座標補正
	概要：ライントレース中は自機座標補正する
	引数：なし
	戻値：なし
	**********************/
	void adjustMachinePos(void);
	
	// 状態管理
	int8_t manageState(void);
	
	// 動作管理
	// 引数
	//  目的地とか探索モードとか
	bool_t manageAction(inrof_state State);
	bool_t manageAction_tansaku(void);
	bool_t manageAction_goToGoal(void);
	bool_t manageAction_goToStart(void);
	bool_t manageAction_StartCalib(void);
	
	// ライントレース
	ctrl_pid* PidCtrlLineTrace;
	bool_t setTgtVel_LineTrace(float SpdTgt = RMB_VEL_NORMAL);
	void setTgtVel_Turn(float SpdTgt, float Radius);
	
	// 捨てるボール判定
	inrof_goal TrashGoal;
	inrof_goal reqTrashPre(void); 	// 捨てるボールを判定してアームに要求出す
	
/***********
 出力
************/
	/*********************
	モーター出力
	概要：
	 PWM値を出力する
	引数：
	 Duty
	**********************/
	void outTyres(int16_t RTireDutyTgt, int16_t LTireDutyTgt, bool_t fMotorOut);
	
	
/***********
 マシン状態
************/
	
	
	bool_t checkInitialize(void);
	
	
/***********
 PS3コン関係
************/
	inrof2016_aplSci_t::ps3data_t Ps3ConDataLast;
	uint8_t Ps3ConArmMode;
	uint8_t ArmPosTgt;
	int16_t GrabAngleTgt;
	int16_t PitchAngleTgt;
	void drivePs3Con(void);
	float Ps3ArmDutyOut;	// +-255
	float Ps3StrOut;	// +-255
	float Ps3TurnOut;	// +-255
	
};


extern inrof2016 Inrof;
extern inrof2016_aplSci_t ExtSci;


#endif
