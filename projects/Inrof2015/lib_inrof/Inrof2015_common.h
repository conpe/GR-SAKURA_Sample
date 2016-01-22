/**************************************************
Inrof2015_common.h
	知能ロボコン2015用のライブラリたち。
	
	
	
	
**************************************************/


#ifndef __INROF2015_COMMON_H__
#define __INROF2015_COMMON_H__

#include <math.h>


// 制御周期
#define SAMPLETIME_CTRL_S 0.001	// 動作制御周期 1ms

// 制御定数
//#define PI 3.1415

// バッテリやばい電圧 x10V
#define BATT_LOW 65	// 6.5V

// SdWav
#define SDWAV_MAXFILENAME 32


// 基本速度
#define VEL_NORMAL 200

// ラインセンサ
// 中心位置
#define LINESENS_POS_X	72.0	//だいたい72
#define LINESENS_POS_Y	0.0
// 赤外LEDポート
#define LINESENS_PORT_LED	GREX_IO7
// センサ数
#define LINESENS_NUM_SENSE	4			// インデックスは左から順に0, 1, 2, 3
// センサ閾値
#define LINESENS_THRESHOLD_SIDE 25000
#define LINESENS_THRESHOLD_CENTER 50000



// 測距センサ
#define PSDSENS_NUM 5
// 左から右前を見るセンサ
#define PSDSENS_LEFT_ADRS		0x78
#define PSDSENS_LEFT_POS_X		58.0
#define PSDSENS_LEFT_POS_Y		60.0
#define PSDSENS_LEFT_POS_TH		-30.0*PI/180.0
// 左を見るセンサ
#define PSDSENS_LEFTSIDE_ADRS	0x70		
#define PSDSENS_LEFTSIDE_POS_X	72.0
#define PSDSENS_LEFTSIDE_POS_Y	40.0
#define PSDSENS_LEFTSIDE_POS_TH	90.0*PI/180.0
// 真ん中センサ
#define PSDSENS_FRONT_ADRS		0x60		
#define PSDSENS_FRONT_POS_X		90.0
//#define PSDSENS_FRONT_POS_X		48.0
#define PSDSENS_FRONT_POS_Y		0.0
#define PSDSENS_FRONT_POS_TH	0.0*PI/180.0
// 右を見るセンサ
#define PSDSENS_RIGHTSIDE_ADRS		0x58
#define PSDSENS_RIGHTSIDE_POS_X		72.0
#define PSDSENS_RIGHTSIDE_POS_Y		-40.0
#define PSDSENS_RIGHTSIDE_POS_TH	-90.0*PI/180.0
// 右から左前を見るセンサ
#define PSDSENS_RIGHT_ADRS		0x50	
#define PSDSENS_RIGHT_POS_X		58.0
#define PSDSENS_RIGHT_POS_Y		-60.0
#define PSDSENS_RIGHT_POS_TH	30.0*PI/180.0


// カラーセンサ
// 白LEDのポート
#define COLORSENS_PORT_LED	GREX_IO6

// タイヤエンコーダ関係
#define TIRE_TREAD_MM	176.7		// トレッド
#define TIRE_DIA_MM	56.0		// 直径
#define GEAR_RATIO	21.3		// ギアヘッドの減速比
#define ENC_CPR		1336		// エンコーダ一周あたりのカウント(4逓倍済み)
#define RTIRE_DIR_FW	CW
#define RTIRE_DIR_BK	CCW
#define LTIRE_DIR_FW	CW
#define LTIRE_DIR_BK	CCW


// フィールド関係
#define FIELD_INROF_CROSS_START_X 0.0
#define FIELD_INROF_CROSS_START_Y 0.0
#define FIELD_INROF_CROSS_RED_X 0.0
#define FIELD_INROF_CROSS_RED_Y 250.0
#define FIELD_INROF_CROSS_YELLOW_X 0.0
#define FIELD_INROF_CROSS_YELLOW_Y 900.0
#define FIELD_INROF_CROSS_BLUE_X 300.0
#define FIELD_INROF_CROSS_BLUE_Y 1550.0
#define FIELD_INROF_CROSS_BALLFIELD_BEGIN_X 1040.0
#define FIELD_INROF_CROSS_BALLFIELD_BEGIN_Y 1300.0
#define FIELD_INROF_CROSS_BALLFIELD_END_X 1040.0
#define FIELD_INROF_CROSS_BALLFIELD_END_Y 50.0
#define FIELD_INROF_CROSS_GOAL_RED_X -250.0
#define FIELD_INROF_CROSS_GOAL_RED_Y 250.0
#define FIELD_INROF_CROSS_GOAL_YELLOW_X -250.0
#define FIELD_INROF_CROSS_GOAL_YELLOW_Y 900.0
#define FIELD_INROF_CROSS_GOAL_BLUE_X -250.0
#define FIELD_INROF_CROSS_GOAL_BLUE_Y 2550.0

#define FIELD_INROF_CROSS_JUDGE_THRESHOLD	150		// 交差点判定をするときにここまでのズレなら許容する
#define FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR	400		// 交差点判定をするときにここまでのズレなら許容する(他の線と離れてるところVer)
// ボール範囲
#define FIELD_BALLFIELD_NORTHLIMIT	1300
#define FIELD_BALLFIELD_SOUTHLIMIT	100
#define FIELD_BALLFIELD_EASTLIMIT	1400
#define FIELD_BALLFIELD_WESTLIMIT	680

// ボール探索
#define BALLFIND_MAXDISTANCE_SIDE	270		// この距離以下でボールがあると判定する
#define BALLFIND_MAXDISTANCE_SIDECLOSS	200		// この距離以下でボールがあると判定する
#define BALLFIND_MAXDISTANCE_FRONT 250
//#define BALLFIND_MAXDISTANCE_SIDE	18		// この距離以下でボールがあると判定する
//#define BALLFIND_MAXDISTANCE_SIDECLOSS	18		// この距離以下でボールがあると判定する
//#define BALLFIND_MAXDISTANCE_FRONT 18

#define PSD_MIN 20							// PSDセンサを有効とみなす最小値

// ボール捕獲
#define BALLGET_DISTANCE_FRONT 130			// フロントセンサからこの距離だけ離れてアーム下ろす

// サーボ位置
//#define SERVOPOS_START_F 0
//#define SERVOPOS_START_R -900
#define SERVOPOS_RUN_F 0
#define SERVOPOS_RUN_R -50
#define SERVOPOS_BALLGET_F -940
#define SERVOPOS_BALLGET_R 400
#define SERVOPOS_BALLUP_F -550
#define SERVOPOS_BALLUP_R 300
#define SERVOPOS_BALLTRASH_F -900
#define SERVOPOS_BALLTRASH_R -200	// サーボオフにするので重要ではない
#define SERVOPOS_COLORCHECK_F 450
#define SERVOPOS_COLORCHECK_R -550	//-600








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
