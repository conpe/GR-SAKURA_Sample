/**************************************************
GR_Ex15.h
	GR-SAKURA用の拡張基板GR_Ex15用のライブラリだよ。
	いろいろ初期化したりするよ。
	
	グローバルのインスタンス用意しておくよ。
	gr_ex GrEx;
	
	モータードライバ
	//ジャイロ
	スピーカ	gr_ex_sp SP;
	CAN通信
	シリアル通信
	I2C
	エンコーダ
	サーボモータ
**************************************************/


#ifndef __GR_EX15_H__
#define __GR_EX15_H__


#include "portReg.h"
#include "RingBuffer.h"

#include "GR_define.h"

enum grex_ad_ch{
	GREX_AD0,
	GREX_AD1,
	GREX_AD2,
	GREX_AD3,
	GREX_AD4,
	GREX_AD5,
	GREX_AD6,
	GREX_AD7
};



#define GREX_IO0 PD7
#define GREX_IO1 PD6
#define GREX_IO2 PD5
#define GREX_IO3 PD4
#define GREX_IO4 PD3
#define GREX_IO5 PD2
#define GREX_IO6 PD1
#define GREX_IO7 PD0
#define GREX_IO20 PE3
#define GREX_IO21 PE1
#define GREX_IO22 P53
#define GREX_IO23 P35

// スピーカ
#define GREX_SPOUT PE2
#define EXSP_BUFFSIZE 512

// モーター
#define GREX_MOTOR_PWMCNT_1KHz 47999

#define GREX_PIN_MOTOR0P P20
#define GREX_PIN_MOTOR0A P05
#define GREX_PIN_MOTOR0B P07
#define GREX_PIN_MOTOR1P P21
#define GREX_PIN_MOTOR1A P17
#define GREX_PIN_MOTOR1B P51
#define GREX_PIN_MOTOR2P PC1
#define GREX_PIN_MOTOR2A P54
#define GREX_PIN_MOTOR2B P55
#define GREX_PIN_MOTOR3P PJ3
#define GREX_PIN_MOTOR3A PC4
#define GREX_PIN_MOTOR3B PE0

enum gr_ex_motor_num{
	MOTOR0,	
	MOTOR1,	
	MOTOR2,	
	MOTOR3,
};
enum gr_ex_motor_dir{
	CW,
	CCW,
	BRAKE,
	FREE,
};



// エンコーダ
#define GREX_PIN_ENC0A P24
#define GREX_PIN_ENC0B P25
#define GREX_PIN_ENC1A P22
#define GREX_PIN_ENC1B P23

enum gr_ex_encoder_num{
	ENC0,
	ENC1
};




/*********************
スピーカ
概要：
 バッファに用意した波形データを順次出力する。
**********************/
class gr_ex_sp{
public:
	RingBuffer<uint8_t> *SpBuff;
	
	gr_ex_sp(void);
	~gr_ex_sp(void);
	void setPeriodCnt(uint16_t PeriodCnt);	// カウント数
	void setDutyCnt(uint16_t DutyCnt);			// カウント数
	
	void setupPlay(uint16_t SamplingFreq);	// 再生設定
	void stopPlay(void);					// 再生停止
	void isrSpPlay(void);					// 割り込み(SamplingFreq周期)
	
	// バッファ操作
	bool_t isBuffEmpty(void){return SpBuff->isEmpty();};
	bool_t isBuffFull(void){return SpBuff->isFull();};
	int8_t addBuff(uint8_t data){return SpBuff->add(data);};
	void clearBuff(void){SpBuff->clear();};

	int16_t getNumElements(void){return SpBuff->getNumElements();};
private:
	uint16_t _Freq;
	void setupRegisterPwm(void);
};


class gr_ex_motor{
public:
	gr_ex_motor(gr_ex_motor_num MotorNum);
	
	void setPeriodCnt(uint16_t PeriodCnt);
	void setDutyCnt(uint16_t DutyCnt);
	void setDir(gr_ex_motor_dir);
	
	uint16_t getPeriodCnt(void){return PeriodCnt_;};
	
	//void setSpd(uint16_t Spd);	//速度制御したいなーって
	
private:
	gr_ex_motor_num _Motor;
	//bool_t _Enable;
	gr_ex_motor_dir _Dir;
	uint16_t PeriodCnt_;
	
	void setRegister(void);
};



class gr_ex_encoder{
public:
	bool_t fOverflow_;
	bool_t fUnderflow_;
	
	gr_ex_encoder(gr_ex_encoder_num EncNum);
	void startCnt(void);
	void stopCnt(void);
	void setCnt(uint16_t Cnt);
	uint16_t getCnt(void);		// カウント値
	int16_t getCntDiff(void);	// 前回値との差
private:
	uint16_t CntLast_;
	gr_ex_encoder_num Enc_;
	void setRegister(void);

};




class gr_ex{
public:
	gr_ex_sp *Sp;
	gr_ex_motor *Motor0;
	gr_ex_motor *Motor1;
	gr_ex_motor *Motor2;
	gr_ex_motor *Motor3;
	gr_ex_encoder *Enc0;
	gr_ex_encoder *Enc1;
	
	gr_ex(void);
	~gr_ex(void);
	void begin(void);
	
	void startAD(void);
	void stopAD(void){S12AD.ADCSR.BIT.ADST = 0;};
	uint16_t getAD(grex_ad_ch AdCh);
	
private:
	// privatemember

	// private function
	
	
	
};


extern gr_ex GrEx;





#endif
