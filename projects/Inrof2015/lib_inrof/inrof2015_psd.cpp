
#include "inrof2015_psd.h"




/****************************
 知能ロボコン2015
  PSDセンサ
概要：
 PSDセンサを継承して, センサ位置とかの情報追加した。
****************************/

/*********************
コンストラクタ
引数：
	I2Cインスタンス
	I2Cアドレス
**********************/
inrof2015_psd::inrof2015_psd(I2c_t* I2Cn, uint8_t I2cAddress, float PosX, float PosY, float PosTh) : sharp_GP2Y0E(I2Cn, I2cAddress)
{
	PosX_ = PosX;
	PosY_ = PosY;
	PosTh_ = PosTh;
}
/*********************
マシン位置をセット
引数：
	マシンの位置
**********************/
void inrof2015_psd::setMachinePos(position* MachinePos){
	MachinePosX_ = MachinePos->X;
	MachinePosY_ = MachinePos->Y;
	MachinePosTh_ = MachinePos->Th;
}

/*********************
センサ値が示す点をグローバル座標系で取得
引数：
	マシンの位置
	(返り)*X
	(返り)*Y
**********************/
int8_t inrof2015_psd::getGlobalObjPos(float* PosXw, float* PosYw){
	int16_t Dist = getDistance();
	
	float cosMTh = cosf(MachinePosTh_);
	float sinMTh = sinf(MachinePosTh_);
	
	if(Dist>=0){	// 負は無効値
		*PosXw = MachinePosX_ + (PosX_*cosMTh - PosY_*sinMTh) + Dist * cosf(PosTh_ + MachinePosTh_);
		*PosYw = MachinePosY_ + (PosX_*sinMTh + PosY_*cosMTh) + Dist * sinf(PosTh_ + MachinePosTh_);
	}else{
		*PosXw = 0;
		*PosYw = 0;
		
		return -1;
	}
	return 0;
}

/*********************
センサ位置をグローバル座標系で取得
引数：
	マシンの位置
	(返り)*X
	(返り)*Y
**********************/
void inrof2015_psd::getGlobalPos(float* PosXw, float* PosYw){
	float cosMTh = cosf(MachinePosTh_);
	float sinMTh = sinf(MachinePosTh_);
	
	*PosXw = MachinePosX_ + (PosX_*cosMTh - PosY_*sinMTh);
	*PosYw = MachinePosY_ + (PosX_*sinMTh + PosY_*cosMTh);
	
}

/*********************
見てる物がボールエリア内か判定する
引数：
	なし
戻値：
	エリア内ならtrue
**********************/
bool_t inrof2015_psd::isObjectBallArea(void){
	float x, y;
	if(!getGlobalObjPos(&x, &y)){	// 値取れてる		
		if( (x<=FIELD_BALLFIELD_EASTLIMIT)		// 東の端より左
			&& (x>=FIELD_BALLFIELD_WESTLIMIT)	// 西の端より右
			&& (y<=FIELD_BALLFIELD_NORTHLIMIT)	// 北の端より下
			&& (y>=FIELD_BALLFIELD_SOUTHLIMIT)	// 南の端より上
		){
			return true;
		}
		
	}
	
	return false;
}




