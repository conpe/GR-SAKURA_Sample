/**************************************************
inrof2016_upper_ctrl.h
	知能ロボコン2015用のライブラリたち。
	
	
	10個ある置き場のうち、ボールを置くのは偶数箇所のみ。
	
	
**************************************************/


#ifndef __inrof2016_UPPERCTRL_H__
#define __inrof2016_UPPERCTRL_H__


#include "inrof2016_common.h"

#include "FutabaCmdServo.h"
#include "inrof2016_color_sens.h"

#include "AppliCtrl.h"
#include "RIIC.h"

#include "RoombaOi.h"

extern RoombaOi Rmb;


#define ENC_CNT_MAX 155	// M系列信号の長さ
#define INROF_UPPER_BALLHOLD_CURRENT_TH_mA	30	// ボール持ってる判定するサーボの電流[mA]
//#define INROF_UPPER_BALLHOLD_CURRENT_TH_mA	0	// ボール持ってる判定するサーボの電流[mA]

#define INROF_UPPER_CNT_FAIL_MAX 40

#define ARM_ENC_SPD_AVERAGE_NUM	5U	// アームエンコーダ速度の移動平均とる数

#define NUM_BALL_STOCK 10	// ボール置き場の数
//#define NUM_BALL_STOCK_MAX 5	// 上部におけるボール数
#define NUM_BALL_STOCK_MAX 1	// 上部におけるボール数

#define INROF_UPPER_COLSENS_POS	5	// カラーセンサ設置箇所

enum upper_state{
	UPPER_STATE_IDLE,
	UPPER_STATE_BALL_TRASH_PRE,	// ボール捨てる準備中
	UPPER_STATE_BALL_TRASH,		// ボール捨ててるちゅう
	UPPER_STATE_BALL_GET_PRE,	// ボール取る準備中
	UPPER_STATE_BALL_GET,		// ボールとってる
	UPPER_STATE_BALL_GET_POST,	// ボールとった後処理
	UPPER_STATE_ARM_TO_CENTER,	// 真ん中へ移動中
};



/****************************
 知能ロボコン2016 上部制御
概要：
 取得ボール、アーム位置管理
****************************/
class inrof2016_upper_ctrl{
public: 
	
	// アクチュエータ
	// アームモータ
	gr_ex_motor* ArmMotor;
	uint8_t ArmPosTgt;
	
	
	// サーボ
	futaba_cmd_servo* ServoGrab;
	futaba_cmd_servo* ServoPitch;
	// カラーセンサ
	inrof2016_color_sens_t *ColorSens;
	// アームエンコーダ
	uint8_t ArmEnc;		// アーム位置(0～ENC_CNT_MAX-1, 0xFE:未初期化, 0xFF無効値)
	uint8_t ArmEncRaw;	// エンコーダ生値
	uint8_t ArmEncLast;
	uint8_t NoDecideCnt;	// アーム位置確定しないカウント
	float ArmEncSpd;	// アーム速度(エンコーダ速度) [cnt/sec]	ローパス付き
	
	
	// アームモーター
	// アームエンコーダ
	static inrof2016_upper_ctrl* ReqObject;	// staticな受信関数からArmEncを特定するため
	
	
	// 
	void begin(void);
	void initialize(void);
	// 
	void task0(void);	// 20msec
	void task1(void);	// 200msec
	
	
	/*********************
	// 動作許可
	**********************/
	bool_t fAction;
	void enableAction(void){fAction = true;};
	void disableAction(void){fAction = false;};
	bool_t isEnableOut(void){return fAction;};
	
	/*********************
	// 動作状態取得
	**********************/
	upper_state getState(void){return State;};
	
	/*********************
	// 持ってるボール関係
	**********************/
	// 持ってるボール数
	uint8_t numBall(void){return BallHoldNum;};
	// 持ってるボール数(色指定)
	uint8_t numBallColor(inrof_color Col);
	// 青いボール持ってる数
	uint8_t numBlue(void){return numBallColor(INROF_BLUE);};
	// 黄色いボール持ってる数
	uint8_t numYellow(void){return numBallColor(INROF_YELLOW);};
	// 赤いボール持ってる数
	uint8_t numRed(void){return numBallColor(INROF_RED);};
	// もういっぱい？
	bool_t isBallFull(void){return (NUM_BALL_STOCK_MAX<=BallHoldNum);};
	
	// 動作処理
	int8_t ctrlState(void);		 // 処理シーケンスを実行
	int8_t attachCtrlSeqFunc(int8_t (inrof2016_upper_ctrl::*pCtrlSeqFunc)(void));	// 新しい処理シーケンスをセットする
	int8_t dettachCtrlSeqFunc(void){pCtrlSeqFunc = NULL; return 0;};
	/*********************
	ボール捨てる準備
	概要：
		指定した色のボールを掴んで、指定した角度まで持っていく
	**********************/
	// 要求
	int8_t reqBallTrashPre(inrof_color Col, float AngleRad);
	// 処理シーケンス
	int8_t ctrlSeqBallTrashPre(void);
	
	/*********************
	ボール捨てる要求
	概要：
		アーム立ててボール離してアーム上げる
	**********************/
	// 要求
	int8_t reqBallTrash(void);
	// 処理シーケンス
	int8_t ctrlSeqBallTrash(void);
	
	
	
	/*********************
	ボール拾う準備
	概要：
		アームを指定位置でガバッと開く
		終わったらIDLE
	**********************/
	// 要求
	int8_t reqBallGetPre(float AngleRad);
	// 処理シーケンス
	int8_t ctrlSeqBallGetPre(void);
	
	/*********************
	ボール拾う
	概要：
		掴んで持ち上げる
		終わったら自動でGetPost処理実行
	**********************/
	// 要求
	int8_t reqBallGet(void);
	// 処理シーケンス
	int8_t ctrlSeqBallGet(void);
	
	/*********************
	ボール拾った後処理
	概要：
		色チェックして、
		ボールをいい感じのマシン上に置く。
		もう車体は動いていいよ。
		終わったら自動でセンターへ戻る
	**********************/
	// 要求
	int8_t reqBallGetPost(void);
	// 処理シーケンス
	int8_t ctrlSeqBallGetPost(void);
	
	
	/*********************
	アーム真ん中へ要求
	概要：
		アーム中立にして真ん中へ移動
		終わったらUPPER_STATE_IDLEになる
	**********************/
	// 要求
	int8_t reqArmToCenter(void);
	// 処理シーケンス
	int8_t ctrlSeqArmToCenter(void);
	
	
	/*********************
	アーム移動要求(位置or速度)
	概要：
	 アームを移動する
	 位置指定系はいずれも内部的には目標エンコーダ値に変換する
	引数：
	 int16_t Duty : +-255
	 int16_t EncCnt : 0-ENC_CNT_MAX
	 uint8_t BallPos : ボール位置(CW端が0, CCW端が9)
	 float Rad : 角度で指定
	**********************/
	void moveArmDuty(int16_t Duty);		// デューティー指定
	void moveArmEncCnt(uint8_t EncCnt);	// エンコーダ値
	void moveArmBallPos(uint8_t BallPos);	// ボールストック位置
	void moveArmAngle(float Rad);		// 角度
	// アーム移動完了？
	bool_t isFinishCtrlArmMove(void);	// 目的地(エンコーダ値)に近い かつ 速度が小さい
	
	
	/*********************
	ボール持ってる？
	 0:なし
	 1:前で持ってる
	 2:後ろで持ってる
	**********************/
	uint8_t checkHoldBall(void);
	
	/*********************
	サーボ移動完了？
	**********************/
	bool_t isFinishMoveServo(void);
	
	
private:
	enum ctrl_mode_t{
		MODE_DUTY,
		MODE_ENCCNT,
		MODE_BALLPOS,
		MODE_RAD
	};
	
	enum arm_enc_mode{
		ARM_ENC_RAW,
		ARM_ENC_MSEQINDEX
	};
	
	// 上部の状態
	upper_state State;
	
	// アーム移動制御(位置or速度)
	ctrl_mode_t CtrlMode;	// アーム制御モード(目標値指定方法)
	int16_t DutyTgt;	// デューティ指定値
	uint8_t EncCntTgt;	// エンコーダ位置指定値
	uint8_t BallPosTgt;	// ボールの位置指定値
	uint8_t RadTgt;		// 角度指定値 [rad]
	
	// ボール色
	inrof_color BallColor;		// 判定ボール色
	inrof_color HoldingBallColor;	// 手で持ってるボール色
	
	// エンコーダ値
	static const uint8_t BallStockEnc[NUM_BALL_STOCK];	// ボール置き場のエンコーダ値
	static const uint8_t MseqArray[];	// エンコーダ値とインデックスの対応
	
	// 持ってるボール
	inrof_color BallStock[NUM_BALL_STOCK];
	int8_t BallHoldNum;	// 確保してるボール数
	void updateBallHoldNum(void);	// ボール数更新	// ctrlSeqBallGet()で仮に増やしたりするので、実際に乗っけた時に改めて数え直す
	
	/* シーケンス処理用 */
	int8_t (inrof2016_upper_ctrl::*pCtrlSeqFunc)(void);	//処理するシーケンス関数
	// 各シーケンス中の処理番号
	uint8_t SeqNum;
	uint8_t SeqNumLast;
	// 目標ボール置き場
	uint8_t SeqBallStockObj;	// 目標ボール置き場
	uint8_t SeqArmEncObj;		// 目標エンコーダ位置
	// 各シーケンス開始時刻
	uint32_t SeqStartTime_ms;	
	
	/*********************
	アームエンコーダ
	概要：	
	**********************/
	I2c_t* ArmI2C;
	bool_t ArmEncMode;
	int8_t ArmEncModeRaw(void);	// 生値受信モードへ
	int8_t ArmEncModeMseqIndex(void);	// M系列エンコード済み受信モードへ
	int8_t reqArmEnc(void);	// 通信要求
	static void isrArmEnc(I2c_comu_t *Comu);	// 受信処理
	int8_t updateArmEnc(uint8_t);	// m系列信号からインデックス(ArmEnc)更新
	ctrl_move_average* CtrlArmEncSpdAverage;	// アームエンコーダ速度移動平均処理
	
	
	/*********************
	アーム移動制御
	概要：	
	**********************/
	int8_t ctrlArmMotor(void);
	
	ctrl_pid* pid_ctrlArmMotor;
	ctrl_ratelimiter_abs* ratelimiter_ctrlArmMotor;
	ctrl_mode_t CtrlModeLast;
	uint8_t CntFail;
	uint8_t ArmEncCorrect;
	
	
	/*********************
	こまいの
	**********************/
	// rad -> エンコーダ値
	uint8_t Rad2Enc(float Rad);
	uint8_t Deg2Enc(float Deg);
	
};



extern inrof2016_upper_ctrl InrofUpper;

#endif
