/************************************************/
/*	浜松フォトニクス			*/
/*		デジタルカラーセンサS11059(I2C)	*/
/*			for RX63n @ CS+		*/
/*			Wrote by conpe_		*/
/*				2016/05/26	*/
/************************************************/

#include "Inrof2016_color_sens.h"




/*********************
カラーセンサ初期化
コンストラクタ
I2C初期化
引数：	I2C
**********************/
inrof2016_color_sens_t::inrof2016_color_sens_t(I2c_t* I2Cn, uint8_t ArmAdrs):hamamatsu_S11059(I2Cn){
	this->ArmAdrs = ArmAdrs;
	this->ArmI2C = I2Cn;
	BallColorTmp = INROF_NONE;
	BallColor = INROF_NONE;
}
/*********************
カラーセンサ初期化
デストラクタ
**********************/
inrof2016_color_sens_t::~inrof2016_color_sens_t(void){
}


/*********************
カラーセンサ動作開始
**********************/
void inrof2016_color_sens_t::begin(void){
	hamamatsu_S11059::begin();
	I2Cn->begin(400, 32);
	
	offBallColorLed();
	fJudgeColor = false;
}

// debug 
#include "Sci.h"

/*********************
ボールの色更新
概要：
 HSV空間で色を判定。
 LEDon,off時で比較し、
 反射光が弱いときはINROF_UNKNOWNとする。
 関数呼び出しごとにon, off。
 2回以上同じ判定色なら確定する。
 200msecより速い周期だとおかしくなるっぽい
**********************/
// debug
//#include "SCI.h"
int8_t inrof2016_color_sens_t::updateBallColor(void){
	uint16_t BallColorValDiff[3];
	int8_t ret;
	
	if(fJudgeColor){
		if(BallColorLed){
			getLuminance(&BallColorVal[0][1], &BallColorVal[1][1], &BallColorVal[2][1]);
		}else{
			getLuminance(&BallColorVal[0][0], &BallColorVal[1][0], &BallColorVal[2][0]);
		}
		// 明るい時と暗い時の差をとる
		BallColorValDiff[0] = abs(BallColorVal[0][1] - BallColorVal[0][0]);
		BallColorValDiff[1] = abs(BallColorVal[1][1] - BallColorVal[1][0]);
		BallColorValDiff[2] = abs(BallColorVal[2][1] - BallColorVal[2][0]);
		
		// 色判定
		if((BallColorValDiff[0]>COLSENS_BALL_EXIST_LUM)||(BallColorValDiff[1]>COLSENS_BALL_EXIST_LUM)||(BallColorValDiff[2]>COLSENS_BALL_EXIST_LUM)){	// 反射光あり
			
			uint16_t SensRed;
			uint16_t SensGreen;
			uint16_t SensBlue;
			uint16_t RgbMax;
			uint16_t RgbMin;
			color MaxCol;
			float H;
			
			SensRed = BallColorValDiff[0];
			SensGreen = BallColorValDiff[1];
			SensBlue = BallColorValDiff[2];
			
			if(SensRed > SensGreen){
				RgbMax = SensRed;
				MaxCol = red;
			}else{
				RgbMax = SensGreen;
				MaxCol = green;
			}
			if(RgbMax < SensBlue){
				RgbMax = SensBlue;
				MaxCol = blue;
			}
			
			if(SensRed < SensGreen){
				RgbMin = SensRed;
			}else{
				RgbMin = SensGreen;
			}
			if(RgbMin > SensBlue){
				RgbMin = SensBlue;
			}
			
			if(RgbMax != RgbMin){
				switch(MaxCol){
				case red:
					H = (60.0f * ((float)SensBlue-(float)SensGreen))/(float)(RgbMax-RgbMin);
					break;
				case green:
					H = 120.0f + (60.0f*((float)SensRed-(float)SensBlue))/(float)(RgbMax-RgbMin);
					break;
				case blue:
					H = 240.0f + (60.0f*((float)SensGreen-(float)SensRed))/(float)(RgbMax-RgbMin);
					break;
				default:
					H = 0.0f;
					break;
				}
				
				while(H<0){
					H = H+360.0f;
				}
				
				//if(H<60 || H>300){
				if(H<80.0f || H>340.0f){
					BallColorTmp = INROF_RED;
				//}else if(H>=60 && H<180){
				}else if(H<180.0F){
					BallColorTmp = INROF_YELLOW;
				}else{
					BallColorTmp = INROF_BLUE;
				}
				
			}else{
				H = 0.0f;
				BallColorTmp = INROF_NONE;
			}
			
			//Sci0.print("Col %d = %d, R%d, G%d, B%d, DIFF=R%d, G%d, B%d\r\n", BallColor, (uint16_t)H, SensRed, SensGreen, SensBlue,  BallColorValDiff[0], BallColorValDiff[1], BallColorValDiff[2]);
			
		}else{
			//Sci0.print("Col NONE DIFF=R%d, G%d, B%d\r\n", BallColorValDiff[0], BallColorValDiff[1], BallColorValDiff[2]);
			BallColorTmp = INROF_NONE;
		}
		if(BallColorLed){
			ret = offBallColorLed();
		}else{
			ret = onBallColorLed();
		}
		
		
		// 色確定処理
		if(INROF_NONE != BallColorTmp){		// ボール検知あり
			if(BallColorTmpLast == BallColorTmp){	// 前回値と同じ
				JudgeCnt++;
				if(COLSENS_BALL_JUDGE_CNT <= JudgeCnt){			// カウントまっくす。
					BallColor = BallColorTmp;			// ボール色確定
				}else{
					BallColor = INROF_NONE;				 // 不定
				}
			}else{			// 前回と違う色
				JudgeCnt = 1;	// カウントリセット(色はあるので1で。)
				BallColor = INROF_NONE; 				// 不定
			}
		}else{
			// ボール検知できなかったときは即判定
			JudgeCnt = 0;
			BallColor = INROF_NONE;
		}
		
		
		ret = updateSens();	// カラーセンサ値取得要求
	}else{
		
		JudgeCnt = 0;
		BallColorTmp = INROF_NONE;
		BallColor = INROF_NONE;
		
		ret = offBallColorLed();
	}
	
	BallColorTmpLast = BallColorTmp;
	
	return ret;
}

/*********************
カラー判定LED on
概要：
　I2Cでアーム基板に渡す
**********************/
int8_t inrof2016_color_sens_t::onBallColorLed(void){
	//uint8_t TxData;
	
	//TxData = 0x01;
	BallColorLed = true;
	
	//return ArmI2C->attach(ArmAdrs, &TxData, 1);
	
	outPin(COLSENS_LED_PIN, 1);
	
	return 0;
}

/*********************
カラー判定LED off
概要：
　I2Cで渡す
**********************/
int8_t inrof2016_color_sens_t::offBallColorLed(void){
	//uint8_t TxData;
	
	//TxData = 0x00;
	BallColorLed = false;
	
	//return ArmI2C->attach(ArmAdrs, &TxData, 1);
	outPin(COLSENS_LED_PIN, 0);
	
	return 0;
}

