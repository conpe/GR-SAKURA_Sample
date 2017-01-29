/**************************************************
inrof2016_aplSci.h
	知能ロボコン2016用のライブラリたち。
  SCI関連処理クラス
	
	
	
	
**************************************************/


#ifndef __INROF2016_APLSCI_H__
#define __INROF2016_APLSCI_H__

#include "SCI.h"


/****************************
 知能ロボコン2016用
  SCI関連処理クラス
概要：
	外部とのやり取りを管理。
	PC通信モードとPS3コン通信モード
****************************/
#define SBDBT_PS3CON_DATALENGTH 57
#define SBDBT_PS3CON_DISABLECNT 25	// task何回でコントローラ操作を無効判定するか

//class inrof2016_aplSci_t:public Sci_t{
class inrof2016_aplSci_t{
	
public :
	enum mode_t{
		PC = 1,
		PC_THROUGH,	// PCからの通信をそのまま使う
		PS3		// PS3コンからの情報として使う
	};
	
	union ps3data_t{
		uint8_t Data[SBDBT_PS3CON_DATALENGTH];
		struct{
			uint8_t Rcb3_Head;		// 0バイト目
			uint8_t Rcb3_Square	:1;	// 1バイト目
			uint8_t Rcb3_L1	:1;
			uint8_t Rcb3_L2	:1;
			uint8_t Rcb3_R1	:1;
			uint8_t Rcb3_R2	:1;
			uint8_t Rcb3_Dummy0	:3;
			uint8_t Rcb3_Up	:1;	// 2バイト目
			uint8_t Rcb3_Down	:1;		// 上下両方共1ならStart
			uint8_t Rcb3_Right	:1;
			uint8_t Rcb3_Left	:1;		// 左右両方とも1ならSelect
			uint8_t Rcb3_Triangle	:1;
			uint8_t Rcb3_Cross	:1;
			uint8_t Rcb3_Circle	:1;
			uint8_t Rcb3_Dummy1	:1;
			uint8_t Rcb3_LStickHorizontal;	// 3バイト目	センター 0x40
			uint8_t Rcb3_LStickVertical;	// 4バイト目
			uint8_t Rcb3_RStickHorizontal;	// 5バイト目
			uint8_t Rcb3_RStickVertical;	// 6バイト目
			uint8_t Rcb3_CheckSum;		// 7バイト目
			uint8_t ReportId;		// 8バイト目
			uint8_t Dummy0;			// 9バイト目
			uint8_t Select	:1;		// 10バイト目
			uint8_t L3	:1;
			uint8_t R3	:1;
			uint8_t Start	:1;
			uint8_t Up	:1;
			uint8_t Right	:1;
			uint8_t Down	:1;
			uint8_t Left	:1;
			uint8_t L2	:1;		// 11バイト目
			uint8_t R2	:1;
			uint8_t L1	:1;
			uint8_t R1	:1;
			uint8_t Triangle	:1;
			uint8_t Circle	:1;
			uint8_t Cross	:1;
			uint8_t Square	:1;
			uint8_t Playstation	:1;	// 12バイト目
			uint8_t Dummy12	:7;
			uint8_t Dummy13;		//13バイト目
			uint8_t LStickHrizontal;	//14バイト目	センター 0x80
			uint8_t LStickVertical;		//15バイト目
			uint8_t RStickHrizontal;	//16バイト目
			uint8_t RStickVertical;		//17バイト目
			uint8_t Dummy18;		//18バイト目	// 何らかのデータではあるっぽい
			uint8_t Dummy19;		//19バイト目
			uint8_t Dummy20;		//20バイト目
			uint8_t Dummy21;		//21バイト目
			uint8_t AnalogUp;		//22バイト目
			uint8_t AnalogRight;		//23バイト目
			uint8_t AnalogDown;		//24バイト目
			uint8_t AnalogLeft;		//25バイト目
			uint8_t AnalogL2;		//26バイト目
			uint8_t AnalogR2;		//27バイト目
			uint8_t AnalogL1;		//28バイト目
			uint8_t AnalogR1;		//29バイト目
			uint8_t AnalogTriangle;		//30バイト目
			uint8_t AnalogCircle;		//31バイト目
			uint8_t AnalogCross;		//32バイト目
			uint8_t AnalogSquare;		//33バイト目
			uint8_t Dummy34;		//34バイト目
			uint8_t Dummy35;		//35バイト目
			uint8_t Dummy36;		//36バイト目
			uint8_t Status;		//37バイト目
			uint8_t Batt0;		//38バイト目
			uint8_t Batt1;		//39バイト目
			uint8_t Dummy40;	//40バイト目
			uint8_t ComLevel;	//41バイト目
			uint8_t Dummy42;	//42バイト目
			uint8_t Dummy43;	//43バイト目
			uint8_t Dummy44;	//44バイト目
			uint8_t Dummy45;	//45バイト目
			uint8_t Dummy46;	//46バイト目
			uint8_t MotorSmall	:1;	//47バイト目
			uint8_t Dummy47		:7;	
			uint8_t Dummy48		:7;	//48バイト目	
			uint8_t MotorBig	:1;
			uint16_t Gyro0;		//49,50バイト目
			uint16_t Gyro1;		//51,52バイト目
			uint16_t Gyro2;		//53,54バイト目
			uint16_t Gyro3;		//55,56バイト目	//ジャイロと加速度と思われる
		}button;
	};
	
public:
	ps3data_t ControllerData;
	
	
	inrof2016_aplSci_t(Sci_t *SciObject);
	// 通信データ解析
	// モードに応じて受信バッファを解釈する
	// 定期的に呼び出すこと
	void task(void);
	
	// モードをセット
	void setMode(mode_t NewMode);
	// 現在モードを取得
	mode_t getMode(void){return Mode;}
	
	// PS3コン使える？
	bool_t isPs3ConAvailable(void){return Ps3ConAvailable;};
	
private:
	Sci_t *Sci;
	mode_t Mode;
	bool_t Ps3ConAvailable;
	uint8_t Ps3ConDisableCnt;
	
};



#endif
