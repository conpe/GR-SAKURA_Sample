/**************************************************
inrof2016_common.h
	知能ロボコン2015用のライブラリたち。
	
	
	
	
**************************************************/


#ifndef __inrof2016_COMMON_H__
#define __inrof2016_COMMON_H__


#define DEBUGSCI0		// 下半分のデバッグメーッセージ
//#define DEBUGSCI0_UPPER	// 上半身のデバッグメッセージ


#include <math.h>

#include "portReg.h"
#include "GR_Ex15_v3.h"
#include "GR_Ex15_OLED.h"

// inrof_libでもinrof_upperでもつかうもの
#include "aplMemCtrl.h"


// 制御周期
#define SAMPLETIME_CTRL_S 0.020	// 動作制御周期 20ms

// 制御定数
#define PI 3.1415

// バッテリやばい電圧 x10V
#define BATT_LOW 65	// 6.5V
#define BATT_LOW_RMBSOC 60	// ルンバのSOC 60%

// SdWav
#define SDWAV_MAXFILENAME 32


// 基本速度
#define VEL_NORMAL 200

// ラインセンサ
// 中心位置
#define LINESENS_POS_X	110.0F	// タイヤ位置から
#define LINESENS_POS_Y	0.0F
// 赤外LEDポート
#define LINESENS_PORT_LED	GREX_IO7
// センサ数
#define LINESENS_NUM_SENSE	4	// インデックスは左から順に0, 1, 2, 3
					// ADポートの0～3に対応	
// センサ閾値
//#define LINESENS_THRESHOLD_SIDE 40000
//#define LINESENS_THRESHOLD_CENTER 40000
#define LINESENS_THRESHOLD_SIDE 550
#define LINESENS_THRESHOLD_CENTER 500



// 測距センサ
//#define PSDSENS_NUM 3
// 左から右前を見るセンサ
#define PSDSENS_LEFT_ADRS		0x25		// 複数接続時に正常に設定できなくなるためデフォルト値(0x29)と同じにしないこと
#define PSDSENS_LEFT_POS_X		175.0F
#define PSDSENS_LEFT_POS_Y		60.0F
#define PSDSENS_LEFT_POS_TH		-45.0F*PI/180.0F
#define PSDSENS_LEFT_ENABLEPIN 		GREX_IO4		// 赤い線
// 左を見るセンサ
/*
#define PSDSENS_LEFTSIDE_ADRS	0x70		
#define PSDSENS_LEFTSIDE_POS_X	72.0F
#define PSDSENS_LEFTSIDE_POS_Y	40.0F
#define PSDSENS_LEFTSIDE_POS_TH	90.0F*PI/180.0F
*/
// 真ん中センサ
#define PSDSENS_FRONT_ADRS		0x26		// 複数接続時に正常に設定できなくなるためデフォルト値と同じにしないこと
#define PSDSENS_FRONT_POS_X		180.0F
//#define PSDSENS_FRONT_POS_X		48.0F
#define PSDSENS_FRONT_POS_Y		0.0F
#define PSDSENS_FRONT_POS_TH		0.0F
#define PSDSENS_FRONT_ENABLEPIN 	GREX_IO5	// 白い線

// 右を見るセンサ
/*
#define PSDSENS_RIGHTSIDE_ADRS		0x58
#define PSDSENS_RIGHTSIDE_POS_X		72.0F
#define PSDSENS_RIGHTSIDE_POS_Y		-40.0F
#define PSDSENS_RIGHTSIDE_POS_TH	-90.0F*PI/180.0F
*/
// 右から左前を見るセンサ
#define PSDSENS_RIGHT_ADRS		0x27		// 複数接続時に正常に設定できなくなるためデフォルト値と同じにしないこと
#define PSDSENS_RIGHT_POS_X		175.0F
#define PSDSENS_RIGHT_POS_Y		-60.0F
#define PSDSENS_RIGHT_POS_TH	45.0F*PI/180.0F
#define PSDSENS_RIGHT_ENABLEPIN 	GREX_IO6	// 緑の線


// カラーセンサ
// 白LEDのポート
//#define COLORSENS_PORT_LED	GREX_IO6

// アームマイコン
#define ARM_I2C_ADDRESS 0x09

// IMU(MPU9250)
#define IMU_I2C_ADDRESS 0x68


// ルンバ
// タイヤエンコーダ関係
//#define TIRE_TREAD_MM	235.0F		// トレッド
#define TIRE_TREAD_MM	248.0F		// トレッド
//#define TIRE_DIA_MM	70.0F		// 直径
#define TIRE_DIA_MM	72.0F		// 直径
#define GEAR_RATIO	1.0F		// ギアヘッドの減速比
#define ENC_CPR		490.0F		// エンコーダ一周あたりのカウント

// 最大値
#define RMB_ANGVEL_MAX 4.0F
#define RMB_VEL_MAX	400.0F

#define RMB_VEL_NORMAL	200.0F

#define SERVO_GOALTIME_NORMAL 800

#define ARM_MOTOR_CW	CW
#define ARM_MOTOR_CCW	CCW


// フィールド関係
#define FIELD_INROF_CROSS_START_X 0.0F
#define FIELD_INROF_CROSS_START_Y 0.0F
#define FIELD_INROF_CROSS_RED_X 0.0F
#define FIELD_INROF_CROSS_RED_Y 250.0F
#define FIELD_INROF_CROSS_YELLOW_X 0.0F
#define FIELD_INROF_CROSS_YELLOW_Y 900.0F
#define FIELD_INROF_CROSS_BLUE_X 300.0F
#define FIELD_INROF_CROSS_BLUE_Y 1550.0F
#define FIELD_INROF_CROSS_BALLFIELD_BEGIN_X 1040.0F
#define FIELD_INROF_CROSS_BALLFIELD_BEGIN_Y 1300.0F
#define FIELD_INROF_CROSS_BALLFIELD_END_X 1040.0F
#define FIELD_INROF_CROSS_BALLFIELD_END_Y 50.0F
#define FIELD_INROF_CROSS_GOAL_RED_X -250.0F
#define FIELD_INROF_CROSS_GOAL_RED_Y 250.0F
#define FIELD_INROF_CROSS_GOAL_YELLOW_X -250.0F
#define FIELD_INROF_CROSS_GOAL_YELLOW_Y 900.0F
#define FIELD_INROF_CROSS_GOAL_BLUE_X -250.0F
#define FIELD_INROF_CROSS_GOAL_BLUE_Y 1550.0F
#define FIELD_INROF_CROSS_BLUE_EDGE_X 0.0F
#define FIELD_INROF_CROSS_BLUE_EDGE_Y 1550.0F

#define FIELD_INROF_CROSS_JUDGE_THRESHOLD	150		// 交差点判定をするときにここまでのズレなら許容する
#define FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR	400		// 交差点判定をするときにここまでのズレなら許容する(他の線と離れてるところVer)
// ボール範囲
#define FIELD_BALLFIELD_NORTHLIMIT	1300
#define FIELD_BALLFIELD_SOUTHLIMIT	100
#define FIELD_BALLFIELD_EASTLIMIT	1400
#define FIELD_BALLFIELD_WESTLIMIT	680

// ボール探索
//#define BALLFIND_MAXDISTANCE_SIDE	270		// この距離以下でボールがあると判定する
//#define BALLFIND_MAXDISTANCE_SIDECLOSS	300		// この距離以下でボールがあると判定する
#define BALLFIND_MAXDISTANCE_SIDE	200		// この距離以下でボールがあると判定する
#define BALLFIND_MAXDISTANCE_SIDECLOSS	200		// この距離以下でボールがあると判定する
#define BALLFIND_MAXDISTANCE_FRONT 	100
//#define BALLFIND_MAXDISTANCE_SIDE	18		// この距離以下でボールがあると判定する
//#define BALLFIND_MAXDISTANCE_SIDECLOSS	18		// この距離以下でボールがあると判定する
//#define BALLFIND_MAXDISTANCE_FRONT 18

#define PSD_MIN 10							// PSDセンサを有効とみなす最小値


// ボール
#define FIELD_BALL_NUM_INIT 3	// フィールド上のボール

// ボール捕獲
#define BALLGET_MAXDISTANCE_FRONT 	200.0F		// ボール確保中の最大距離
#define BALLGET_DISTANCE_FRONT_ARMDOWN	170.0F		// フロントセンサからこの距離だけ離れてアーム下ろす
#define BALLGET_DISTANCE_FRONT_GRAB	90.0F		// フロントセンサからこの距離だけ離れて掴む

// サーボ位置
// 最大角度
#define SERVOPOS_MAX_G 500
#define SERVOPOS_MIN_G -150
#define SERVOPOS_MAX_P 280
#define SERVOPOS_MIN_P -900
// 基本位置
#define SERVOPOS_START_G 80		// 初期位置
#define SERVOPOS_START_P -200
#define SERVOPOS_RUN_G 80		// 移動中位置
#define SERVOPOS_RUN_P -200		// (持ってるボール、フィールドのボールに当たらない)
// ボール捕獲
#define SERVOPOS_BALLGET0_G 400		// ボール捕獲 開く	// 周りのボールに当たらないように狭めだけど、位置いまいち決まらないようならもっと開く
#define SERVOPOS_BALLGET0_P -750	// 850だと下擦っちゃって動けなくなる
#define SERVOPOS_BALLGET1_G 80		// ボール捕獲 掴む
#define SERVOPOS_BALLGET1_P -750	// 850だと下擦っちゃって動けなくなる
// ボール捕獲後
#define SERVOPOS_BALLUP0_G 80		// ボール捕獲 持ち上げる(移動中)
#define SERVOPOS_BALLUP0_P -200
#define SERVOPOS_BALLUP1_G 80		// ボール捕獲 置く位置
#define SERVOPOS_BALLUP1_P -200
#define SERVOPOS_BALLUP2_G 80		// ボール捕獲 コロンッ
#define SERVOPOS_BALLUP2_P 250
#define SERVOPOS_BALLUP3_G -150		// ボール離す
#define SERVOPOS_BALLUP3_P 200
// ボールを捨てに行く
#define SERVOPOS_BALLTRASH0_G -150	// マシン上のをつかむ位置
#define SERVOPOS_BALLTRASH0_P 280	
#define SERVOPOS_BALLTRASH1_G 80		// 掴む
#define SERVOPOS_BALLTRASH1_P 280
#define SERVOPOS_BALLTRASH2_G 80		// 持ち上げ状態
#define SERVOPOS_BALLTRASH2_P -200
#define SERVOPOS_BALLTRASH3_G 0		// 捨てる
#define SERVOPOS_BALLTRASH3_P -900


enum inrof_color{
	INROF_NONE,
	INROF_RED,
	INROF_YELLOW,
	INROF_BLUE
};

enum inrof_goal{
	INROF_GOAL_NONE,
	INROF_GOAL_RED,
	INROF_GOAL_YELLOW,
	INROF_GOAL_BLUE
};





// 位置だけのクラスと
// 位置と速度のクラスと
// 回転系も合わせたクラス作りたい

class position{
public:
	float X, Y;
	float dX, dY;
	float Th;			// [rad]
	float dTh;
	
	// 引き算は 距離を返す
	float operator - (position Pos){
		return sqrtf(powf((this->Y - Pos.Y), 2) + powf((this->X - Pos.X), 2));
	}
	
	// +-PI[rad]に正規化
	float getNormalizeTh(void){	return normalize(Th); };
private:
	float normalize(float Th){
		/*
		if(Th > 3.1415){
			return normalize(Th - 2*3.1415);
		}else if(Th< -3.1415){
			return normalize(Th + 2*3.1415);
		}
		*/
		while(Th > 3.1415){
			Th -= (2*3.1415);
		}
		while(Th < -3.1415){
			Th += (2*3.1415);
		}


		return Th;
	}
};




#endif
