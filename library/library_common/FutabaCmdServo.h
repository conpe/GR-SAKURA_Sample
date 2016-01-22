/********************************************/
/*		フタバのコマンドサーボ				*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/07/19		*/
/********************************************/

//
// enableTorque()してsetGoalPosition()すれば取りあえず動く


// デフォルト設定
// ボーレート : 115200
// サーボID : 1


//更新履歴
// 2015/02/25 SH7125用から移植。C++化。　
// 2015/07/19 受信待ちをなくすため、割り込みで実行するようにした。


//更新したい
// 通信の管理をSCIモジュールごとに行う。できてないから困っちゃう。受信割り込みとかが無駄に増えてるよん(´・ω・｀)。通信内容のあれでsci覚えておくのもやだし
// ->SCIクラスに、パケット通信版をつくる。(通常通信とパケット通信の混合は…考えなくていっかな？

#ifndef __FUTABASERIALSERVO_H__
#define __FUTABASERIALSERVO_H__

#include <machine.h>	// マイコンコンパイラ固有関数

#include "SCI.h"
#include "CommonDataType.h"

// debug用
//#include "SCI.h"

//#define FTCMDSV_COMUSNUM_DEFAULT 32		// 覚えておける通信内容
#define FTCMDSV_COMUSNUM_DEFAULT 12		// 覚えておける通信内容

#define CMDSV_BAUDRATE_DEFAULT 115200

//↓ これいらない
#define CMDSV_ADRS_ServoID 0x04
#define CMDSV_ADRS_Reverse 0x05
#define CMDSV_ADRS_BaudRate 0x06
#define CMDSV_ADRS_ReturnDelay 0x07
#define CMDSV_ADRS_CWAngleLimit 0x08
#define CMDSV_ADRS_CCWAngleLimit 0x0A
#define CMDSV_ADRS_TemperatureLimit 0x0E
#define CMDSV_ADRS_TorqueInSilence 0x16
#define CMDSV_ADRS_WarmupTime 0x17
#define CMDSV_ADRS_CWComplianceMargin 0x18
#define CMDSV_ADRS_CCWComplianceMargin 0x19
#define CMDSV_ADRS_CWComplianceSlope 0x1A
#define CMDSV_ADRS_CCWComplianceSlope 0x1B
#define CMDSV_ADRS_Punch 0x64
#define CMDSV_ADRS_GoalPosition 0x1E
#define CMDSV_ADRS_GoalTime 0x20
#define CMDSV_ADRS_MaxTorque 0x23
#define CMDSV_ADRS_TorqueEnable 0x24
#define CMDSV_ADRS_PresentPosition 0x2A
#define CMDSV_ADRS_PresentTime 0x2C
#define CMDSV_ADRS_PresentSpeed 0x2E
#define CMDSV_ADRS_PresentCurrent 0x30
#define CMDSV_ADRS_PresentTemperature 0x32
#define CMDSV_ADRS_PresentVolts 0x34



#define CMDSV_BR_9600 0
#define CMDSV_BR_14400 1
#define CMDSV_BR_19200 2
#define CMDSV_BR_28800 3
#define CMDSV_BR_38400 4
#define CMDSV_BR_57600 5
#define CMDSV_BR_76800 6
#define CMDSV_BR_115200 7	// default
#define CMDSV_BR_153600 8
#define CMDSV_BR_230400 9

enum ftcmdsv_torque_enable{
	CMDSV_TORQUE_OFF	= 0,
	CMDSV_TORQUE_ON		= 1,
	CMDSV_TORQUE_BRAKE	= 2,
};

enum ftcmdsv_comu_content{
	CMDSV_SERVOID			= 0x00,
	CMDSV_REVERS			= 0x01,
	CMDSV_BAUDRATE			= 0x02,
	CMDSV_RETURNDELAY		= 0x03,
	CMDSV_CWANGLELIMIT		= 0x04,
	CMDSV_CCWANGLELIMIT		= 0x05,
	CMDSV_TEMPERATURELIMIT	= 0x06,
	CMDSV_TORQUEINSILENCE	= 0x07,
	CMDSV_WARMUPTIME		= 0x08,
	CMDSV_CWCOMPLIANCEMARGIN	= 0x09,
	CMDSV_CCWCOMPLIANCEMARGIN	= 0x0A,
	CMDSV_CWCOMPLIANCESLOPE		= 0x0B,
	CMDSV_CCWCOMPLIANCESLOPE	= 0x0C,
	CMDSV_PUNCH					= 0x0D,
	CMDSV_GOALPOSITION			= 0x0E,
	CMDSV_GOALTIME				= 0x0F,
	CMDSV_MAXTORQUE				= 0x10,
	CMDSV_TORQUEENABLE			= 0x11,
	CMDSV_PRESENTPOSITION		= 0x12,
	CMDSV_PRESENTTIME			= 0x13,
	CMDSV_PRESENTSPEED			= 0x14,
	CMDSV_PRESENTCURRENT		= 0x15,
	CMDSV_PRESENTTEMPERATURE	= 0x16,
	CMDSV_PRESENTVOLTS			= 0x17,
	CMDSV_WRITEROM				=0x80,
	CMDSV_RESET				=0x81,
};



typedef struct ftcmdsv_data{
	uint8_t ServoID;	// ROM
	uint8_t Reverse;
	uint16_t BaudRate;
	uint8_t ReturnDelay;
	uint16_t CWAngleLimit;
	uint16_t CCWAngleLimit;
	uint16_t TemperatureLimit;
	uint8_t TorqueInSilence;
	uint8_t WarmupTime;
	uint8_t CWComplianceMargin;
	uint8_t CCWComplianceMargin;
	uint8_t CWComplianceSlope;
	uint8_t CCWComplianceSlope;
	uint16_t Punch;
	uint16_t GoalPosition;		// RAM
	uint16_t GoalTime;
	uint8_t MaxTorque;
	ftcmdsv_torque_enable TorqueEnable;
	int16_t PresentPosition;	// *10 deg
	uint16_t PresentTime;
	int16_t PresentSpeed;
	uint16_t PresentCurrent;
	int16_t PresentTemperature;
	uint16_t PresentVolts;
} ftcmdsv_data;


class futaba_cmd_servo_comu_t;

class futaba_cmd_servo{
	
public:
		
	/*********************
	コンストラクタ
	引数：
			SCI操作インスタンス
			ボーレート 4800～115200(default)
	**********************/
	futaba_cmd_servo(uint8_t ID, Sci_t* Sci, uint32_t BaudRate);
	futaba_cmd_servo(uint8_t ID, Sci_t* Sci);
	~futaba_cmd_servo(void);

	int8_t begin(void);

	/*********************
	サーボID設定
	ROM保存まで行う
	引数：
			newID	：新しいサーボID
	**********************/
	void setID(uint8_t newID);

	/*********************
	通信速度設定
	引数：	
		newBR	：新しい通信速度(CMDSV_BR_9600,...)
	**********************/
	void setBaudRate(uint8_t newBR);

	/*********************
	ROM領域を保存する
	引数：	
	**********************/
	void writeROM(void);

	/*********************
	サーボをリセット
	引数：	
	**********************/
	void reset(void);
	
	
	/*********************
	レジスタ読み込み要求
	引数：	
		ftcmdsv_comu_content : 読むレジスタ
	**********************/
	int8_t update(ftcmdsv_comu_content Reg){return reqReadMemory(Reg);};
	
	
	/*********************
	目標位置
	引数：
			angle	：目標角度(0.1度単位 90.2度→902 )
	**********************/
	void setGoalPosition(int16_t angle);
	int8_t updateGoalPosition(void){return reqReadMemory(CMDSV_GOALPOSITION);};		// レジスタ読む通信
	int16_t getGoalPosition(void){return ResisterData.GoalPosition;};				// 結果取得

	/*********************
	移動時間
	引数：
			TimeMs	：目標時間 [ms]
	**********************/
	void setGoalTime(uint16_t TimeMs);
	int8_t updateGoalTime(void){return reqReadMemory(CMDSV_GOALTIME);};		// レジスタ読む通信
	int16_t getGoalTime(void){return ResisterData.GoalTime;};				// 結果取得

	/*********************
	最大トルク
	引数：	
		Torque : 最大トルク [%]
	**********************/
	void setMaxTorque(uint8_t MaxTorque);


	/*********************
	トルクEnable
	引数：	ID		：サーボID
			enable	：CMDSV_TORQUE_ON, CMDSV_TORQUE_OFF, CMDSV_TORQUE_BRAKE
	**********************/
	int8_t enableTorque(ftcmdsv_torque_enable Enable);






	/*********************
	サーボ角度
	引数：	
	**********************/
	int8_t updatePresentPosition(void){return reqReadMemory(CMDSV_PRESENTPOSITION);};
	int16_t getPresentPosition(void){return ResisterData.PresentPosition;};


	/*********************
	現在時間 10ms単位
	**********************/
	int8_t updatePresentTime(void){return reqReadMemory(CMDSV_PRESENTTIME);};
	int16_t getPresentTime(void){return ResisterData.PresentTime;};
		
		
	/*********************
	現在速度
	引数：
	返値：
		signed int ： deg/sec
	**********************/
	int8_t updatePresentSpeed(void){return reqReadMemory(CMDSV_PRESENTSPEED);};
	int16_t getPresentSpeed(void){return ResisterData.PresentSpeed;};
	
	/*********************
	負荷電流
	引数：
	返値：
		uint16_t ： 電流 [mV]
	**********************/
	int8_t updatePresentCurrent(void){return reqReadMemory(CMDSV_PRESENTCURRENT);};
	uint16_t getPresentCurrent(void){return ResisterData.PresentCurrent;};
	
	/*********************
	電源電圧
	引数：
	返値：
		signed int ： mV
	**********************/
	int8_t updatePresentVolts(void){return reqReadMemory(CMDSV_PRESENTVOLTS);};
	uint16_t getPresentVolts(void){return ResisterData.PresentVolts;};
	
	/*********************
	基板温度
	引数：
	返値：
		signed int ： 温度 [度]
	**********************/
	int8_t updatePresentTemperature(void){return reqReadMemory(CMDSV_PRESENTTEMPERATURE);};
	int16_t getPresentTemperature(void){return ResisterData.PresentTemperature;};
	
	
	
	
	//debuf
	static bool_t fAttaching;
private:
	Sci_t *ServoSci;
	uint32_t BaudRate;
	uint8_t ID;
	ftcmdsv_data ResisterData;
	static const uint8_t ResisterAdrsLength[24][2];	// 各レジスタのアドレスとデータ長
	
	//static bool_t fAttaching;
	static bool_t fManaging;
	
	/*********************
	ショートパケット生成
	引数：	
			Flag	：フラグたち
			Address	：メモリアドレス
			Length	：1サーボあたりのデータの長さ
			Cnt		：サーボの数
			*Data	：データ sizeof(Data) = Length*Cnt
	**********************/
	int8_t genShortPacket(uint8_t** TxData, uint8_t Flag, uint8_t Address, uint8_t Length, uint8_t Cnt, uint8_t *TrData);
	int8_t genShortPacket(uint8_t* TxData, uint8_t Flag, uint8_t Address, uint8_t Length, uint8_t Cnt, uint8_t *TrData);
	
	/*********************
	ショートパケット送信
	　一つのサーボにデータを送る
	　サーボ通信クラスをnewしてattachSciComuに投げる
	引数：	
			Flag	：フラグたち
			Address	：メモリアドレス
			Length	：1サーボあたりのデータの長さ
			Cnt		：サーボの数
			*Data	：データ sizeof(Data) = Length*Cnt
	**********************/
	int8_t sendShortPacket(ftcmdsv_comu_content ComuType, uint8_t Flag, uint8_t Address, uint8_t Length, uint8_t Cnt, uint8_t *TrData);
	
	
	/*********************
	データ書く
	　一つのサーボにデータを送る
	　サーボ通信クラスをnewしてattachSciComuに投げる
	引数：	
			Flag	：フラグたち
			Address	：メモリアドレス
			Length	：1サーボあたりのデータの長さ
			Cnt		：サーボの数
			*Data	：データ sizeof(Data) = Length*Cnt
	**********************/
	int8_t writeMemory(ftcmdsv_comu_content ComuType, uint8_t *TrData);
	
	/*********************
	データ読む要求
	引数：	ID		：サーボID
			Address	：メモリアドレス
			Length	：1サーボあたりのデータの長さ
			*Data	：データ 
	**********************/
	void readMemory(uint8_t Address, uint8_t Length, uint8_t *RcData);	// 受信するまで待つ版
	int8_t reqReadMemory(ftcmdsv_comu_content ComuType);				// パケット通信版


	/*********************
	サーボ通信クラスをバッファに登録
	引数：	
		NewComu	：登録するサーボ通信クラス
	戻値：
		attach成功なら0
	**********************/
	int8_t attachComu(futaba_cmd_servo_comu_t* NewComu);
	
	
	/*********************
	コマンド送信後に待つ
	引数：	
	**********************/
	void waitEndCommand(void);
	
	
/********* 通信処理 **********/
	// 受信待ち中フラグ (この辺ホントはSCIモジュールごとに用意したい)
	static bool_t fRcv;
	// 現在通信中の通信
	static futaba_cmd_servo_comu_t* CurrentComu;
	// 通信内容記憶バッファ
	static RingBuffer<futaba_cmd_servo_comu_t*> *ComusBuff;
	
	/*********************
	送信マネジメント関数
	 通信を開始する
	 通信登録時、SCI送信完了時、データ受信完了時に呼ばれる
	引数：
		なし
	返値：
		int8_t エラー情報
	**********************/
	int8_t manageComuStart(void);
	
	/*********************
	受信マネジメント関数
	引数：
		なし
	返値：
		なし
	**********************/
	void manageComuReceive(RingBuffer<uint8_t> * RxBuff);
	
	
	/*********************
	送信完了割り込み
	 受信待ちなら受信開始関数を呼ぶ
	 でなければmanageComuStartで次の送信をする
	引数：
		なし
	返値：
		なし
	**********************/
	static void intTxEnd(void);
	
	/*********************
	受信開始
	 受信コールバック関数を登録する
	引数：
		なし
	返値：
		なし
	**********************/
	void startRcv(void);
	
	/*********************
	受信終わり
	 受信コールバック関数を外す
	引数：
		なし
	返値：
		なし
	**********************/
	void endRcv(void);
	
	/*********************
	受信処理
	 受信完了割り込みで呼ばれる
	引数：
		なし
	返値：
		なし
	**********************/
	static void intRx(RingBuffer<uint8_t> * RxBuff);
	
	/*********************
	受信処理
	 受信割り込みで呼ばれる
	 各通信内容に応じてResisterDataを更新
	引数：
		RingBuffer<uint8_t> * RxBuff
	返値：
		なし
	**********************/
	void fetchRcvData(futaba_cmd_servo_comu_t* Comu);
	
	
	
	
};


/********
サーボ通信クラス
********/
class futaba_cmd_servo_comu_t{
public:
	futaba_cmd_servo* FTCMDSV;		// サーボオブジェクト
	ftcmdsv_comu_content ComuType;	// 通信内容
	uint8_t* TxData;		// 送信データ
	uint16_t TxNum;			// 送信する数
	uint8_t* RxData;		// 送信データ
	uint16_t RxNum;			// 受信する数
	uint16_t ReceivedNum;	// 受信した数
	uint8_t Sum;			// チェックサム受信時確認用
	
	// コンストラクタ
	futaba_cmd_servo_comu_t(futaba_cmd_servo* FTCMDSV, ftcmdsv_comu_content ComuType, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum){
		this->FTCMDSV = FTCMDSV;
		this->ComuType = ComuType;
		this->TxNum = TxNum;
		this->RxNum = RxNum;
		this->ReceivedNum = 0;
		if(TxNum>0){
			// 送信データコピー
			if(TxNum<0x10){	// 最低確保バイト数だけ確保 (確保するメモリを揃えるため)
				TxNum = 0x10;
			}
			this->TxData = new uint8_t[TxNum];
			if(NULL!=this->TxData){
				memcpy(this->TxData, TxData, this->TxNum);
			}
		}else{
			this->TxData = NULL;
		}
		// 受信データ領域
		if(RxNum>0){
			if(RxNum<0x10){
				RxNum = 0x10;
			}
			this->RxData = new uint8_t[RxNum];
		}else{
			this->RxData = NULL;
		}
	};
	// デストラクタ
	~futaba_cmd_servo_comu_t(void){
		if(NULL!=TxData){
			delete[] TxData;
		}
		if(NULL!=RxData){
			delete[] RxData;
		}
	};
	
};

#endif
