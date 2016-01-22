/********************************************/
/*		Application/Control					*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/05/09		*/
/********************************************/


//【更新履歴】
// 2015.05.09 新規作成


#ifndef __APPLI_CTRL_H__
#define __APPLI_CTRL_H__



class ctrl_pid
{
public:
	ctrl_pid(float dt, float Kp, float Kd, float Ki, float OutMin, float OutMax, float SumMin, float SumMax);
	float calc(float Err);
	
	void resetStatus(void);
	void resetStatus(float Err);

private:
	float dt;			//
	float Kp, Kd, Ki;	//
	// ステータス
	float StErr_1;		// 前回誤差
	float StSum;		// 積分値 (St:state)
	
	float OutMin, OutMax;	// 出力最小, 最大値
	float SumMin, SumMax;	// 積分最大値
	
};


/*********************
レートリミッタ
概要：
 [dif/cycle]
 ダウンレートも正数で指定する 
引数：

**********************/
class ctrl_ratelimiter
{
public:
	ctrl_ratelimiter(){};	// デフォルトコンストラクタ
	ctrl_ratelimiter(float CtrlPeriod, float Rate);					// 上下一緒
	ctrl_ratelimiter(float CtrlPeriod, float UpRate, float DownRate);	//
	ctrl_ratelimiter(float CtrlPeriod, float UpRate, float DownRate, float SigInit);	//	
	ctrl_ratelimiter(const ctrl_ratelimiter& rother);	// コピーコンストラクタ	
	virtual ~ctrl_ratelimiter(void){};					// デストラクタ
	
	void setCtrlPeriod(float CtrlPeriod);
	void setParam(float CtrlPeriod, float Rate);
	void setParam(float CtrlPeriod, float UpRate, float DownRate);
	void setParam(float CtrlPeriod, float UpRate, float DownRate, float SigInit);
	void setRate(float Rate);
	void setRate(float UpRate, float DownRate);
	void setValue(float Value);
	
	// レートリミット処理
	virtual float limitRate(float Sig);	// 値返し
	virtual void limitRate(float* Sig);	// 参照返し
protected:
	float CtrlPeriod;
	float UpRate;	// 1ステップ当たり
	float DownRate;	// 正数で指定
	float SigLast;
};


/*********************
レートリミッタ(絶対値)
概要：
 [dif/cycle]
 ダウンレートも正数で指定する。
 負の値の時のダウンレートは、正方向に対して効く
引数：

**********************/
class ctrl_ratelimiter_abs : public ctrl_ratelimiter
{
public:
	ctrl_ratelimiter_abs(float CtrlPeriod, float Rate) : ctrl_ratelimiter(CtrlPeriod, Rate){};					// 上下一緒
	ctrl_ratelimiter_abs(float CtrlPeriod, float UpRate, float DownRate) : ctrl_ratelimiter(CtrlPeriod, UpRate, DownRate, 0.0){};	//
	ctrl_ratelimiter_abs(float CtrlPeriod, float UpRate, float DownRate, float SigInit) : ctrl_ratelimiter(CtrlPeriod, UpRate, DownRate, SigInit){};	//	
	ctrl_ratelimiter_abs(){};	// デフォルトコンストラクタ
	ctrl_ratelimiter_abs(const ctrl_ratelimiter_abs& rother);	// コピーコンストラクタ
	
	float limitRate(float Sig);
	void limitRate(float* Sig);
};


#endif
