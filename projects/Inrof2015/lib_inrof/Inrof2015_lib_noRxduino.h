/**************************************************
Inrof2015_lib.h
	知能ロボコン2015用のライブラリたち。
	
	
	
	
**************************************************/


#ifndef __INROF2015_H__
#define __INROF2015_H__

//#include <sdmmc.h>

#include "GR_define.h"
#include "inrof2015_common.h"

#include "AppliCtrl.h"

#include "RIIC.h"
#include "SCI.h"

#include "portReg.h"
#include "GR_Ex15.h"
#include "GR_Ex15_OLED.h"
#include "inrof2015_psd.h"

#include "FutabaCmdServo.h"
#include "OledDriverSSD1306.h"
#include "SharpGP2Y0E.h"
#include "HamamatsuColorSensS11059.h"
#include "InvensenseMPU.h"


#define PI 3.1415


typedef struct calc_odmetry{
	float REncCntDiff[2];
	float LEncCntDiff[2];
	float CosTh[2];
	float SinTh[2];
	float dth2[2];
}calc_odmetry;

enum inrof_state{
	INROF_IDLE,
	INROF_TANSAKU,
	INROF_TOGOAL,
	INROF_TOSTART
};
enum inrof_action_state{
	INROF_ACTION_IDLE,		// 0
	INROF_ACTION_LINE,		// 1
	INROF_ACTION_STRAIGHT,	// 2
	INROF_ACTION_TURN,		// 3
	INROF_ACTION_BALLGET,	// 4
	INROF_ACTION_BALLRELEASE	// 5
};
/*
enum inrof_action_state{
}
*/
enum inrof_color{
	INROF_NONE,
	INROF_RED,
	INROF_YELLOW,
	INROF_BLUE
};
// 走行向き
enum inrof_linedir{
	INROF_LINEDIR_NONE,
	INROF_LINEDIR_FORWARD,
	INROF_LINEDIR_BACKWARD
};

enum inrof_cross_index{
	INROF_CROSS_NONE,	// 0
	INROF_CROSS_START,	// 1
	INROF_CROSS_RED,	// 2
	INROF_CROSS_YELLOW,	// 3
	INROF_CROSS_BLUE,	// 4
	INROF_CROSS_BALLFIELD_BEGIN,	// 5
	INROF_CROSS_BALLFIELD_END,		// 6
	INROF_CROSS_GOAL_RED,			// 7
	INROF_CROSS_GOAL_YELLOW,		// 8
	INROF_CROSS_GOAL_BLUE			// 9
};

enum inrof_orientation{
	ORIENT_NONE, 
	ORIENT_EAST,
	ORIENT_NORTH,
	ORIENT_WEST,
	ORIENT_SOUTH,
};


/****************************
 知能ロボコン2015

概要：
 制御周期は1ms, 10ms。
 メイン関数で使えるよう、200msecごとに立つフラグ(fCnt200)を用意
 
****************************/
class inrof2015{
public:
	
	// アクチュエータ
	// モータ
	gr_ex_motor* MotorR;
	gr_ex_motor* MotorL;
	// サーボ
	futaba_cmd_servo* ServoF;
	futaba_cmd_servo* ServoR;
	// センサ
	// 測距センサ
	inrof2015_psd *PsdLeft;
	inrof2015_psd *PsdLeftSide;
	inrof2015_psd *PsdFront;
	inrof2015_psd *PsdRightSide;
	inrof2015_psd *PsdRight;
	// カラーセンサ
	hamamatsu_S11059 *ColorSens;
	// IMU
	invensense_MPU* Mpu;
	// エンコーダ
	gr_ex_encoder* EncR;
	gr_ex_encoder* EncL;
	
	
	bool_t fCnt200;
	bool_t fMotorOut;
	// 目標速度, 角速度
	float SpdTgt, AngVelTgt;	//privateにしたいな
	
	
	// 初期化
	inrof2015(void);
	void begin(void);
	
	// 処理
	void step(void);
	
	
	
	// ラインセンサ
	uint16_t getLineSensAd(uint8_t LineSensNum){return LineSensAd[LineSensNum];};
	void getLineSenseGlobalPos(position* Pos);
	//マシン状態取得
	float getRTireSpd(void){return RTireSpd_rps;};
	float getLTireSpd(void){return LTireSpd_rps;};
	position* getMachinePos(void){return &MachinePos;};
	
	
	void setState(inrof_state State){this->State = State;};
	int8_t getState(void){return State;};
	
	
	// ユーティリティ
	uint32_t getTime(void){return Time_msec;};	// 起動からの時間
	bool_t isEmptyBatt(void){return fBattEmpty;};	// バッテリ空?
	
	// TPU6を使ったフリーカウンタ
	void setupFreeCounter(void);
	uint16_t getFreeCounter(void){return TPU6.TCNT;};
	bool_t isFreeCounterOvf(void){return TPU6.TSR.BIT.TCFV;};
	void clearFreeCounterOvfFlag(void){TPU6.TSR.BIT.TCFV = 0;};
	
	
	// 1ms割り込み
	void isr1ms(void);
	
private:
// private member
	bool_t fExeReq1ms;
	bool_t fExeReq10ms;
	bool_t fExeReq200ms;
	
	uint16_t Cnt10ms;
	uint16_t Cnt200ms;
	
	uint32_t Time_msec;	// 起動からの時間
	
	// 動作状態
	inrof_state State;
	int8_t fFinishManageAction;
	inrof_action_state ActionState;
	int8_t fFinishAction;
	
	
	// ラインセンサ
	uint16_t LineSensAd[LINESENS_NUM_SENSE];	// ラインセンサAD値
	
	
	//// マシン状態 ////
	
	// バッテリ
	uint8_t BattVoltage;		// バッテリ電圧 [x10 V]
	bool_t fBattEmpty;		// バッテリないよフラグ
	void getBattVoltage(void);		// バッテリ電圧取得(BattVoltageにセット)
	
	//オドメトリ関係
	int16_t REncCntDiff, LEncCntDiff;	// エンコーダ前回との差
	float RTirePos_r, LTirePos_r;		// タイヤ角度[rev]
	float RTireSpd_rps, LTireSpd_rps;		// タイヤ角速度[rps]
	position MachinePos;				// マシン位置
	float Spd;							// マシン速度
	position LastFindCrossPos;			// 最後に交点を発見した時のマシン位置
	calc_odmetry CalcOdmetry;			// オドメトリ計算用
	inrof_orientation MachineOrientation;			// マシンの向き(4方向)
	bool_t fAdjustMachinePos;
	
	// 走行状態
	inrof_linedir LineTraceDir;			// 走行向き
	/// 走行制御
	ctrl_ratelimiter_abs* RateLimitSpdTgt;
	ctrl_ratelimiter_abs* RateLimitAngVelTgt;
	
	// manageAction用
	position MachinePosActionStart;		// マシン位置 動作開始時
	int8_t ManageActionTansakuState_BallFind;	//	探索モード状態 ボール探索用 
	int8_t ManageActionTansakuState_BallFind_turn;	//	探索モード状態 ボール探索時のターン用 
	int8_t ManageActionTansakuState_BallFind_last;	//	探索モード状態 ボール探索用 
	int8_t ManageActionTansakuState_BallRelease;	//	探索モード状態 ボール投げる用 
	
	bool_t BallFindEnable;					// ボール探索有効
	bool_t IsBallFinding;				// ボール見つけた瞬間にtrue
	inrof_linedir ManageActionLineTraceDir;			// 動作入った時の走行向き
	
	//交点判定
	bool_t IsCrossing;				// 交点入った判定
	inrof_cross_index Cross;	// どこの交差点を過ぎたところか
	
	// ボール取得処理用
	bool_t BallDir;				// ボールのある方向 0:右, 1:左
	position MachinePosActionManage;		// マシン位置 動作開始時
	position MachinePosActionManageSub;		// マシン位置 動作開始時
	float BallFindTh;		// ボール見つけた角度
	uint32_t StartTimeActionManage;		// 動作開始時刻
	int16_t BallFindDistance;		// 見つけたボールの距離
	
	//ボール判定
	bool_t fJudgeColor;
	inrof_color BallColor;		// 判定ボール色
	inrof_color BallColorFixed;		// [確定]判定ボール色
	bool_t BallColorLed;		// LED光らせてるか
	int16_t BallColorVal[3][2];	// センサ値記憶
	
	
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
	void step1ms(void);	// 1msごと処理
	void step10ms(void);	// 10msごと処理
	void step200ms(void);	// 200msごと処理
	
	void setup1msInterrupt(void);
	
/***********
 入力処理
************/
	void updateLineSense(void);
	void updateEncoder(void);
	void updateBallColor(void);
	/*********************
	タイヤパラメータ更新
	概要：
	引数：
	**********************/
	void updateTyre(void);

	/*********************
	オドメトリ更新
	概要：
	引数：
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
	void ctrlRun(float SpdTgt_mmps, float AngVelTgt_rps, int32_t* RTireDutyTgt, int32_t* LTireDutyTgt);
	
	/*********************
	測距センサ値をブローバル座標へ
	概要：
	引数：
	**********************/
	void updatePsdObjPosGlobal(void);
	
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
	bool_t manageAction_goToGoal(inrof_color Color);
	bool_t manageAction_goToStart(void);
	
	
	//
	void setTgtVel_LineTrace(void);
	void setTgtVel_LineTrace(float SpdTgt);
	void setTgtVel_Turn(float SpdTgt, float Radius);
	
	
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
	void outTyres(int32_t RTireDutyTgt, int32_t LTireDutyTgt);
	
	
	
	
	
	
};


extern inrof2015 Inrof;
//extern SDMMC MMC;


#endif
