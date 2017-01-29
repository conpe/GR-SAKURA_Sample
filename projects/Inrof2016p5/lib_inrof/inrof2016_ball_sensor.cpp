
#include "inrof2016_ball_sensor.h"




/****************************
 知能ロボコン2016
  ボールセンサ
概要：
****************************/

/*********************
センサ位置登録
引数：
**********************/
void inrof2016_ball_sensor_t::setSensPos(float PosX, float PosY, float PosTh)
{
	Pos.X = PosX;
	Pos.Y = PosY;
	Pos.Th = PosTh;
}
void inrof2016_ball_sensor_t::setSensPos(position* Pos)
{
	this->Pos = *Pos;
}



/*********************
(このメソッド要らないなぁ)
マシン位置をセット
引数：
	マシンの位置
**********************/
/*
void inrof2016_ball_sensor_t::setMachinePos(position* MachinePos){
	this->MachinePos = *MachinePos;
}
*/
/*********************
センサ値が示す点をグローバル座標系で取得
引数：
	マシンの位置
	(返り)*X
	(返り)*Y
**********************/
int8_t inrof2016_ball_sensor_t::getGlobalObjPos(position* MachinePos, float* PosXw, float* PosYw){
	int16_t Dist = getDistance();
	
	this->MachinePos = *MachinePos;	// とりあえず
	
	float cosMTh = cosf(MachinePos->Th);
	float sinMTh = sinf(MachinePos->Th);
	
	if(Dist>=0){
		*PosXw = MachinePos->X + (Pos.X*cosMTh - Pos.Y*sinMTh) + Dist * cosf(Pos.Th + MachinePos->Th);
		*PosYw = MachinePos->Y + (Pos.X*sinMTh + Pos.Y*cosMTh) + Dist * sinf(Pos.Th + MachinePos->Th);
	}else{	// 負は無効値
		*PosXw = 0;
		*PosYw = 0;
		
		return -1;
	}
	return 0;
}
int8_t inrof2016_ball_sensor_t::getGlobalObjPos(position* MachinePos, position* ObjPos){
	int16_t Dist = getDistance();
	
	this->MachinePos = *MachinePos;	// とりあえず
	
	float cosMTh = cosf(MachinePos->Th);
	float sinMTh = sinf(MachinePos->Th);
	
	if(Dist>=0){
		ObjPos->X = MachinePos->X + (Pos.X*cosMTh - Pos.Y*sinMTh) + Dist * cosf(Pos.Th + MachinePos->Th);
		ObjPos->Y = MachinePos->Y + (Pos.X*sinMTh + Pos.Y*cosMTh) + Dist * sinf(Pos.Th + MachinePos->Th);
	}else{	// 負は無効値
		ObjPos->X = 0;
		ObjPos->Y = 0;
		
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
void inrof2016_ball_sensor_t::getGlobalPos(position* MachinePos, float* PosXw, float* PosYw){
	float cosMTh = cosf(MachinePos->Th);
	float sinMTh = sinf(MachinePos->Th);
	
	this->MachinePos = *MachinePos;	// とりあえず
	
	*PosXw = MachinePos->X + (Pos.X*cosMTh - Pos.Y*sinMTh);
	*PosYw = MachinePos->Y + (Pos.X*sinMTh + Pos.Y*cosMTh);
	
}
/*********************
センサ位置をグローバル座標系で取得
引数：
	マシンの位置
	(返り)*センサ位置
**********************/
void inrof2016_ball_sensor_t::getGlobalPos(position* MachinePos, position* SensPos){
	float cosMTh = cosf(MachinePos->Th);
	float sinMTh = sinf(MachinePos->Th);
	
	this->MachinePos = *MachinePos;	// とりあえず
	
	SensPos->X = MachinePos->X + (Pos.X*cosMTh - Pos.Y*sinMTh);
	SensPos->Y = MachinePos->Y + (Pos.X*sinMTh + Pos.Y*cosMTh);
}

/*********************
見てる物がボールエリア内か判定する
引数：
	なし
戻値：
	エリア内ならtrue
**********************/
bool_t inrof2016_ball_sensor_t::isObjectBallArea(position* MachinePos){
	float x, y;
	if(!getGlobalObjPos(MachinePos, &x, &y)){	// 値取れてる		
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


/*********************
対象物の車体中央からの距離を取得
引数：
	マシンの位置
戻値：
	対象物の車体中央からの距離
**********************/
int16_t inrof2016_ball_sensor_t::getObjDistFromMachine(position* MachinePos){
	position ObjPos;
	float cosMTh = cosf(MachinePos->Th);
	float sinMTh = sinf(MachinePos->Th);
	
	this->MachinePos = *MachinePos;	// とりあえず
	getGlobalObjPos(MachinePos, &ObjPos);	// 対象物の位置を取得
	
	return (int16_t)(ObjPos - *MachinePos);	// 対象物と車体中央の距離を返す
}


