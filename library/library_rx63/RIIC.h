/********************************************/
/*		RIICモジュール使用 I2C通信			*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/02/26		*/
/********************************************/
// RIICモジュールを使用したI2C通信

// I2Cアドレスは右づめ7bitで指定。 0b*xxx,xxxx
// (1bit左シフトして送信する)

// I2c_comu_t(を継承した)通信クラスを通して送受信する。
// 

//debug
#include "SCI.h"

// めっちゃタイミングが悪いと、1バイトのみ単発で送ると送られないかも！
// (「バッファすべての送信終了の瞬間に新しく1バイト追加」で発生する可能性あり)
// もう1バイト送れば流れだすのであまり気にしなくてもいいかも。


// 使ってる外の関数
// getPCLK(); @ GR_define.h

//【更新履歴】
// 2015.02.26 新規作成
// 2015.06.26 I2C通信内容クラス(I2c_comu_t)で通信を管理するように変更
// 2015.12.26 アービトレーションロスト検出機能追加(スレーブデバイスがSDAをlow固定にしている時にSCLクロック送って開放させる)
// 2016.05.06 受信のみ関数実装
// 2016.05.25 beginが複数回行われた際は、指定バッファサイズが増加した場合には更新する

//【更新予定】
// 送信バッファを持たないようにしたい(通信オブジェクトに持たせる)
// 通信相手によって違うバッファを使う仕組みほしいな？
// 実質自作基板専用になってるのを修正したい(ポート番号とか)
// 通信エラー内容を示すように。現状true, falseのみ。
// 通信中にbegin()呼ばれたらどうしよう。バッファ破棄？通信強制終了？

#ifndef __RIIC_H__
#define __RIIC_H__

#include "iodefine.h"
#include "CommonDataType.h"
#include "RingBuffer.h"
#include "portReg.h"


// ポート
#define PORT_SCL0 P12
#define PORT_SDA0 P13
#define PORT_SCL2 P16
#define PORT_SDA2 P17

// ボーレート
#define I2C_BAUDRATE_DEFAULT 400
// リングバッファのサイズ
#define I2C_BUFFER_SIZE_DEFAULT 15	// 送受信の場合、相手アドレス+送信データ+相手アドレス 分のバッファが必要
// 覚えておける通信処理数
#define I2C_COMUSNUM_DEFAULT 32
// 通信完了オブジェクトバッファ数
#define I2C_DONE_COMUSNUM 64

// エラーカウント
#define I2C_ERROR_CNT_MAX 15


// エラーコード
#define I2C_ERR_OK	0
#define I2C_ERR_NG	-1
#define I2C_ERR_TIMEOUT	-2


// どっかに実体がいる関数
extern uint32_t getPCLK(void);

enum i2c_module
{
	I2C_RIIC0 = 0,
	I2C_RIIC1 = 1,
	I2C_RIIC2 = 2,
	I2C_RIIC3 = 3
};


enum i2c_clock_div
{
	I2C_CLOCK_DIV1 = 0x00,  
	I2C_CLOCK_DIV2 = 0x01,  
	I2C_CLOCK_DIV4 = 0x02,  
	I2C_CLOCK_DIV8 = 0x03, 
	I2C_CLOCK_DIV16 = 0x04, 
	I2C_CLOCK_DIV32 = 0x05, 
	I2C_CLOCK_DIV64 = 0x06, 
	I2C_CLOCK_DIV128 = 0x07
};

enum i2c_status
{
	I2C_IDLE,
	I2C_TRANSMIT,
	I2C_RECEIVE,
	I2C_TRANSMIT_RECEIVE,
	I2C_RESET
};
enum i2c_statusin
{
	I2C_TRANSMIT_DSTADDRESS_T,
	I2C_RESTART,
	I2C_TRANSMIT_DSTADDRESS_R,
	I2C_TRANSMIT_DATA,
	I2C_READ_DATA,
	I2C_READ_END,
	I2C_NACK,
};


class I2c_comu_t;
class I2c_comu_general_t;

// I2C処理(RIICモジュール)
class I2c_t{
public:
	// Constructor
	I2c_t(i2c_module I2cModule);
	~I2c_t(void);
	
	// 準備
	int8_t begin(uint16_t Baud_kbps = I2C_BAUDRATE_DEFAULT, uint16_t TxBuffNum = I2C_BUFFER_SIZE_DEFAULT, uint16_t ComusNum = I2C_COMUSNUM_DEFAULT);
	
	// 通信完了タスク
	static void task(void);
	
	RingBuffer<I2c_comu_t*> *ComusBuff;	// 通信処理記憶バッファ
	I2c_comu_t* CurrentComu;		// 今処理中の通信
	
	// 割り込み送受信
	int8_t attach(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum = 0, void (*CallBackFunc)(I2c_comu_t* Comu) = NULL);		// 送受信
	int8_t attach(I2c_comu_t* AttachComu);	
	
	// 個別送受信(割り込み送受信開始)
	int8_t intstart_transmit(uint8_t DstAddress, uint8_t* TrData, uint8_t TrDataNum);
	int8_t intstart_receive(uint8_t DstAddress, uint8_t* RcData, uint8_t RcDataNum);
	int8_t intstart_transmit_receive(uint8_t DstAddress, uint8_t* TrData, uint8_t TrDataNum, uint8_t* RcData, uint8_t RcDataNum);
	
	// 個別送受信
	// その場で送受信
	// RIICの割り込みより優先度の高い割り込みから実行しないこと
	int8_t transmit(uint8_t DstAddress, uint8_t* TrData, uint8_t TrDataNum);
	int8_t receive(uint8_t DstAddress, uint8_t* RcData, uint8_t RcDataNum);
	int8_t transmit_receive(uint8_t DstAddress, uint8_t* TrData, uint8_t TrDataNum, uint8_t* RcData, uint8_t RcDataNum);
	
	
	// ステータス
	uint16_t getComusNum(void){return ComusNumMax;};
	bool_t isIdle(void){return Status==I2C_IDLE;};
	i2c_status getStatus(void){return Status;};
	i2c_statusin getStatusIn(void){return StatusIn;};
	
	// エラーチェック
	bool_t isError(void){return (ErrCnt>=I2C_ERROR_CNT_MAX);};
	
	// isr
	void isrTx(void);
	void isrTxEnd(void);
	void isrRx(void);
	void isrIdle(void);
	void isrStop(void);
	void isrNack(void);
	void isrArbitrationLost(void);
	
	// I2Cスレーブさんたちをリセット
	//void resetI2C(void);
	void resetI2C(I2c_comu_t* attachcomu = NULL);
	
private:
	
	volatile __evenaccess struct st_riic *I2Creg;	// レジスタ
	i2c_module _ModuleNum;				// RIICモジュール選択
	bool_t fInit;					// I2C初期化済みフラグ
	uint16_t ComusNumMax;				// 通信内容保持数
	uint16_t TxBuffNumMax;
	uint16_t Baud_kbps;				// 通信速度 100or400
	
	RingBuffer<uint8_t> *TxBuff;		// 送信バッファ //Comusの中で持ってる奴使ったほうがよいのでは
	//RingBuffer<uint8_t> *RxBuff;		// 受信バッファ
	uint8_t *RcvBuffer;		// 受信バッファポインタ
	i2c_status Status;		// I2Cステータス (送信中とか)
	i2c_statusin StatusIn;		// I2Cステータス詳細 (送受信のどの段階か)
	uint8_t DstAddress;		// 通信相手
	int16_t CntTransmit;		// のこり送信数
	int16_t CntReceive;		// のこり受信数
	int16_t RcDataNum;		// 受信するデータ数		
	
	uint16_t ErrCnt;		// エラーカウント
	
	int8_t NextAttachIndex;		// 次にアタッチする場所
	int8_t NextComuIndex;		// 次に処理する通信
	int8_t CurrentComuIndex;	// 今処理してる通信
	
	bool_t Starting;		// 送信開始処理中
	
	static RingBuffer<I2c_comu_t*> *FinComus;	// 通信完了オブジェクトバッファ
	
	void initRegister(uint16_t Baud_kbps);
	void setPinModeI2C(void);
	
	// 複数送信モードでの送信開始
	int8_t startComu(void);
	
	// 
	void reqStartCondition(void){I2Creg->ICCR2.BIT.ST=1;};		// スタートコンディション発行要求
	void reqRestartCondition(void){I2Creg->ICCR2.BIT.RS=1;};	// リスタートコンディション発行要求
	void reqStopCondition(void){I2Creg->ICCR2.BIT.SP=1;};		// ストップコンディション発行要求
	bool_t isBusyBus(void){return I2Creg->ICCR2.BIT.BBSY;};		// バスがビジーかチェック(スタートコンディションで1, ストップコンディションで0)
	
	
};


// I2C通信内容クラス
class I2c_comu_t{
public:
	//I2c_comu_t(void);
	I2c_comu_t(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum = 0);	// 送受信 or 送信のみ
	I2c_comu_t(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum);	// 送受信
	virtual ~I2c_comu_t(void);	// 仮想関数にするとサブクラスのデストラクタが呼ばれてからスーパークラスのデストラクタが呼ばれる
	
	uint8_t DevId;		// デバイスID
	int8_t AttachIndex;
	uint8_t DestAddress;
	uint8_t* TxData;
	uint16_t TxNum;
	uint8_t* RxData;
	uint16_t RxNum;
	bool_t Err;		//何入れる？
	
	// コールバック関数
	virtual void callBack(void){};
	
protected:
	// 通信内容設定(コンストラクタから呼ばれる)
	void setI2c(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum);	// 
};


// I2Cとりあえず使いたい時用クラス
// コールバック関数を登録する形式(しなくてもいいよっ！)
class I2c_comu_general_t : public I2c_comu_t{
public:
	//I2c_comu_general_t(void): I2c_comu_t(){};
	I2c_comu_general_t(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum, void (*CallBackFunc)(I2c_comu_t*))	: I2c_comu_t(AttachIndex, DestAddress, TxData, TxNum, RxNum){this->CallBackFunc = CallBackFunc;};	// 送受信
	I2c_comu_general_t(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, void (*CallBackFunc)(I2c_comu_t*)) 				: I2c_comu_t(AttachIndex, DestAddress, TxData, TxNum){this->CallBackFunc = CallBackFunc;};			// 送信のみ
	I2c_comu_general_t(int8_t AttachIndex, uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum)													: I2c_comu_t(AttachIndex, DestAddress, TxData, TxNum){this->CallBackFunc = NULL;};					// 送信のみ(コールバックなし)
	
	void callBack(void){ 
		if(NULL!=CallBackFunc){
			(*CallBackFunc)(this);
		}
	};	//コールバック登録されてたらそれ実行
	
	static uint8_t DevId;		// デバイスID
private:
	void (*CallBackFunc)(I2c_comu_t* Comu);	//コールバック関数へのポインタ
};



extern I2c_t I2C0;
extern I2c_t I2C1;
extern I2c_t I2C2;
extern I2c_t I2C3;


#endif