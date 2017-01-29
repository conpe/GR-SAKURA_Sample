/**************************************************
inrof2016_common.h
	知能ロボコン2015用のライブラリたち。
	
	
	
	
**************************************************/


#ifndef __inrof2016_COMMON_H__
#define __inrof2016_COMMON_H__


#define DEBUGSCI0		// 下半分のデバッグメーッセージ
#define DEBUGSCI0_UPPER	// 上半身のデバッグメッセージ

//#define IMU_ENABLE	// IMU有効


#include <math.h>

#include "portReg.h"
#include "GR_Ex15_v3.h"
#include "GR_Ex15_OLED.h"

// inrof_libでもinrof_upperでもつかうもの
#include "aplMemCtrl.h"		// メモリ制御
#include "aplSerialCom.h"	// シリアル通信


// 制御周期
#define SAMPLETIME_CTRL_S 0.020	// 動作制御周期 20ms

// 制御定数
#define PI 3.1415

// 角度
#define DEG_TO_RAD 0.017453F	// [rad/deg]
#define RAD_TO_DEG 57.2957F	// [deg/rad]

// バッテリやばい電圧 x10V
#define BATT_LOW 11.1F		// 定格11.1V
//#define BATT_AD_COEF	4.3F	// 330kΩと100kΩで分圧
#define BATT_AD_COEF	4.5F	// 330kΩと100kΩで分圧(調整)
#define BATT_LOW_RMBSOC 60	// ルンバのSOC 60%

// SdWav
#define SDWAV_MAXFILENAME 32


// ラインセンサ
// 中心位置
#define LINESENS_POS_X	110.0F	// タイヤ位置からの距離
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
#define PSDSENS_MAX_DIST		2000		// 最大測定距離[mm]
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
#define RMB_VEL_MAX	250.0F

// 走行基本速度
#define RMB_VEL_MAX_CORNER	200.0F
#define RMB_VEL_NORMAL		250.0F
#define RMB_VEL_BALL_FIELD	200.0F
#define RMB_VEL_MAX_BALL	250.0F

// サーボ標準動作速度 [msec]
#define SERVO_GOALTIME_NORMAL 400

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
#define FIELD_BALLFIELD_NORTHLIMIT	1400	//ホントは1300
#define FIELD_BALLFIELD_SOUTHLIMIT	100
#define FIELD_BALLFIELD_EASTLIMIT	1400
#define FIELD_BALLFIELD_WESTLIMIT	680

// ボール
#define FIELD_BALL_NUM_INIT 15	// フィールド上のボール数

// ボール捕獲
#define BALLGET_MAXDISTANCE_FRONT 	300U		// ボール確保中の最大距離
#define BALLGET_DISTANCE_FRONT_ARMDOWN	200U		// フロントセンサからこの距離だけ離れてアーム下ろす
#define BALLGET_DISTANCE_FRONT_GRAB	110U		// フロントセンサからこの距離だけ離れて掴む(持ち上げる)

// ボール探索
// この距離以下でボールがあると判定する
#define BALLFIND_MAXDISTANCE_SIDE	200		// 真横センサ
#define BALLFIND_MAXDISTANCE_SIDECLOSS	400		// 斜めセンサ
#define BALLFIND_MAXDISTANCE_FRONT 	150		// 中央センサ 基本は左右のセンサで見つける(先に前を見つけると、それ取りに行って左右の見逃して乗り上げてしまうため。)。

#define PSD_MIN 10							// PSDセンサを有効とみなす最小値


// サーボ位置
// 最大角度
#define SERVOPOS_MAX_G 500
#define SERVOPOS_MIN_G -300
#define SERVOPOS_MAX_P 300
#define SERVOPOS_MIN_P -900
// 基本位置
#define SERVOPOS_START_G 80		// 初期位置
#define SERVOPOS_START_P -200
#define SERVOPOS_RUN_G 80		// 移動中位置
#define SERVOPOS_RUN_P -300		// (持ってるボール、フィールドのボールに当たらない)
// ボール捕獲
#define SERVOPOS_BALLGET0_G 400		// ボール捕獲 開く	// 周りのボールに当たらないように狭めだけど、位置いまいち決まらないようならもっと開く
#define SERVOPOS_BALLGET0_P -750	// 850だと下擦っちゃって動けなくなる
#define SERVOPOS_BALLGET1_G -100		// ボール捕獲 掴む
#define SERVOPOS_BALLGET1_P -750	// 850だと下擦っちゃって動けなくなる
// ボール捕獲後
#define SERVOPOS_BALLUP0_G 0		// ボール捕獲 持ち上げる(移動中)
#define SERVOPOS_BALLUP0_P SERVOPOS_RUN_P
#define SERVOPOS_BALLUP1_G 0		// ボール捕獲 コロンッ
#define SERVOPOS_BALLUP1_P 260	
#define SERVOPOS_BALLUP2_G -300		// ボール捕獲 ボール離す
#define SERVOPOS_BALLUP2_P 260
#define SERVOPOS_BALLUP3_G -300		// ボール離す
#define SERVOPOS_BALLUP3_P 200
// ボールを捨てに行く
#define SERVOPOS_BALLTRASH0_G -300	// マシン上のをつかむ位置
#define SERVOPOS_BALLTRASH0_P 300	
#define SERVOPOS_BALLTRASH1_G 200	// 掴む
#define SERVOPOS_BALLTRASH1_P SERVOPOS_BALLTRASH0_P
#define SERVOPOS_BALLTRASH2_G 200	// 持ち上げ状態
#define SERVOPOS_BALLTRASH2_P SERVOPOS_RUN_P
#define SERVOPOS_BALLTRASH3_G -50	// 捨てる
#define SERVOPOS_BALLTRASH3_P -900


#define SERVOPOS_BALL_NOT_CONTACT 100			// ボール移動時、これだけ上げてからなら動き出して大丈夫(最終的な角度になるまでにアーム上がりきる) (これより小さいならok)
#define SERVOPOS_BALL_NOT_CONTACT_AFTER_PUT 0	// ボール置いたあと、アーム単体が置いているボールに当たらない角度 (これより小さいならok)

enum inrof_color{
	INROF_NONE,
	INROF_RED,
	INROF_YELLOW,
	INROF_BLUE,
	INROF_UNKNOWN
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
	
	bool_t isBallArea(void){
		if(Y > FIELD_BALLFIELD_NORTHLIMIT){
			return false;
		}
		if(Y < FIELD_BALLFIELD_SOUTHLIMIT){
			return false;
		}
		if(X > FIELD_BALLFIELD_EASTLIMIT){
			return false;
		}
		if(X < FIELD_BALLFIELD_WESTLIMIT){
			return false;
		}
		return true;
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
