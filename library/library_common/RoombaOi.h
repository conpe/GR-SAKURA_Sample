/********************************************/
/*			Roomba OpenInterfase			*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/12/17		*/
/********************************************/
/*
【概要】
シリアル通信でルンバを制御します。
700シリーズで確認。

beginでシリアル関係の設定とモード遷移
シリアルは115200kbps

受信の処理は複数インスタンス対応してないよ

*/


#ifndef ROOMBA_OI_HPP_
#define ROOMBA_OI_HPP_

#include <string.h>

#include "CommonDataType.h"
#include "SCI.h"
#include "RingBuffer.h"


#define ROI_BAUDRATE_DEFAULT	115200
#define ROI_CONNECT_CNT		50

#define ROI_ENCODER_COUNT 490	// タイヤ1周あたりのエンコーダカウント (後で要調整

#define ROI_SONG_NUM_MIN 0
#define ROI_SONG_NUM_MAX 3	// roomba500のroiの説明書だと、4まで出来るって書いてあるけど、700だと3までっぽい。
//#define ROI_SONG_NUM_MAX 0
#define ROI_SONG_LENGTH_MAX 16

// 時間取得関数
extern uint32_t getTime_ms(void);

enum ROI_OPCODE{
	ROI_OPCODE_START		= 0,	// 1
	ROI_OPCODE_BAUD			= 1,	// 2
	ROI_OPCODE_CONTROL		= 2,	// 3
	ROI_OPCODE_SAFE			= 3,	// 4
	ROI_OPCODE_FULL			= 4,	// 5
	ROI_OPCODE_POWER		= 5,	// 6
	ROI_OPCODE_SPOT			= 6,	// 7
	ROI_OPCODE_CLEAN 		= 7,	// 8
	ROI_OPCODE_MAX			= 8,
	ROI_OPCODE_DRIVE		= 9,
	ROI_OPCODE_DRIVE_WHEELS		= 10,
	ROI_OPCODE_MOTORS		= 11,
	ROI_OPCODE_PWM_MOTORS		= 12,
	ROI_OPCODE_DRIVE_PWM		= 13,
	ROI_OPCODE_LEDS			= 14,
	ROI_OPCODE_SONG			= 15,
	ROI_OPCODE_PLAY			= 16,
	ROI_OPCODE_STREAM		= 17,
	ROI_OPCODE_QUERY_LIST		= 18,
	ROI_OPCODE_DO_STREAM		= 19,
	ROI_OPCODE_QUERY		= 20,
	ROI_OPCODE_FORCE_SEEKING_DOG	= 21,
	ROI_OPCODE_SCHEDULING_LEDS	= 22,
	ROI_OPCODE_DIGIT_LEDS_RAW	= 23,
	ROI_OPCODE_DIDIT_LEDS_ASCII	= 24,
	ROI_OPCODE_BUTTONS		= 25,
	ROI_OPCODE_SCHEDULE		= 26,
	ROI_OPCODE_SET_DAY_TIME		= 27,
};

const uint8_t ROI_OPCODE_ID[] = {
	128,129,130,131,132,133,134,135,136,137,145,138,144,146,139,140,141,148,149,150,142,143,162,163,164,165,167,168
};
const uint8_t ROI_OPCODE_BYTES[] = {
	0,1,0,0,0,0,0,0,0,4,4,1,3,4,3,2,1,1,1,1,1,0,2,4,4,1,5,3
};

enum ROI_SENSPACKET_ID{
	ROI_SENSPACKET_GROUP0			= 0,
	ROI_SENSPACKET_GROUP1			= 1,
	ROI_SENSPACKET_GROUP2			= 2,
	ROI_SENSPACKET_GROUP3			= 3,
	ROI_SENSPACKET_GROUP4			= 4,
	ROI_SENSPACKET_GROUP5			= 5,
	ROI_SENSPACKET_GROUP6			= 6,
	ROI_SENSPACKET_GROUP100			= 100,
	ROI_SENSPACKET_GROUP102			= 101,
	ROI_SENSPACKET_GROUP106			= 106,
	ROI_SENSPACKET_GROUP107			= 107,
	ROI_SENSPACKET_BUMPS_WHEELDROPS		= 7,
	ROI_SENSPACKET_WALL			= 8,
	ROI_SENSPACKET_CLIFF_LEFT		= 9,
	ROI_SENSPACKET_CLIFF_FRONT_LEFT		= 10,
	ROI_SENSPACKET_CLIFF_FRONT_RIGHT	= 11,
	ROI_SENSPACKET_CLIFF_RIGHT		= 12,
	ROI_SENSPACKET_VIRTUAL_WALL		= 13,
	ROI_SENSPACKET_OVERCURRENTS		= 14,
	ROI_SENSPACKET_DIRT_DETECT		= 15,
	ROI_SENSPACKET_IR_OPCODE		= 17,
	ROI_SENSPACKET_BUTTONS			= 18,
	ROI_SENSPACKET_DISTANCE			= 19,
	ROI_SENSPACKET_ANGLE			= 20,
	ROI_SENSPACKET_CHARGING_STATE		= 21,
	ROI_SENSPACKET_VOLTAGE			= 22,
	ROI_SENSPACKET_CURRENT			= 23,
	ROI_SENSPACKET_TEMPERATURE		= 24,
	ROI_SENSPACKET_BATTERY_CHARGE		= 25,
	ROI_SENSPACKET_BATTERY_CAPACITY		= 26,
	ROI_SENSPACKET_WALL_SIGNAL		= 27,
	ROI_SENSPACKET_CLIFF_LEFT_SIGNAL	= 28,
	ROI_SENSPACKET_CLIFF_FRONT_LEFT_SIGNAL	= 29,
	ROI_SENSPACKET_CLIFF_FRONT_RIGHT_SIGNAL	= 30,
	ROI_SENSPACKET_CLIFF_RIGHT_SIGNAL	= 31,
	ROI_SENSPACKET_CHARGER_AVAILABLE	= 34,
	ROI_SENSPACKET_OPEN_INTERFCE_MODE	= 35,
	ROI_SENSPACKET_SONG_NUMBER		= 36,
	ROI_SENSPACKET_SONG_PLAYING		= 37,
	ROI_SENSPACKET_OI_STREAM_NUM_PACKETS	= 38,
	ROI_SENSPACKET_VELOCITY			= 39,
	ROI_SENSPACKET_RADIUS			= 40,
	ROI_SENSPACKET_VELOCITY_RIGHT		= 41,
	ROI_SENSPACKET_VELOCITY_LEFT		= 42,
	ROI_SENSPACKET_ENCODER_COUNTS_LEFT	= 43,
	ROI_SENSPACKET_ENCODER_COUNTS_RIGHT	= 44,
	ROI_SENSPACKET_LIGHT_BUMPER		= 45,
	ROI_SENSPACKET_LIGHT_BUMP_LEFT		= 46,
	ROI_SENSPACKET_LIGHT_BUMP_FRONT_LEFT	= 47,
	ROI_SENSPACKET_LIGHT_BUMP_CENTER_LEFT	= 48,
	ROI_SENSPACKET_LIGHT_BUMP_CENTER_RIGHT	= 49,
	ROI_SENSPACKET_LIGHT_BUMP_FRONT_RIGHT	= 50,
	ROI_SENSPACKET_LIGHT_BUMP_RIGHT		= 51,
	ROI_SENSPACKET_IR_OPCODE_LEFT		= 52,
	ROI_SENSPACKET_IR_OPCODE_RIGHT		= 53,
	ROI_SENSPACKET_LEFT_MOTOR_CURRENT	= 54,
	ROI_SENSPACKET_RIGHT_MOTOR_CURRENT	= 55,
	ROI_SENSPACKET_MAIN_BRUSH_CURRENT	= 56,
	ROI_SENSPACKET_SIDE_BRUSH_CURRENT	= 57,
	ROI_SENSPACKET_STASIS			= 58,
	ROI_SENSPACKET_NONE			= 0xFF
};


typedef struct{
	uint8_t NoteNum;
	uint8_t NoteDuration;
}roi_song_packet_t;


extern const roi_song_packet_t RmbSong[];


const uint8_t ROI_SENSPACKET_BYTES[] = 
{
	26,10,6,10,14,12,52,1,1,1,	// 0-9
	1,1,1,1,1,1,1,1,1,2,	// 10-19
	2,1,2,2,1,2,2,2,2,2,	// 20-29
	2,2,1,2,1,1,1,1,1,2,	// 30-39
	2,2,2,2,2,1,2,2,2,2,	// 40-49
	2,2,1,1,2,2,2,2,1,0	// 50-59
	// 100 : 80
	// 101 : 28
	// 106 : 12
	// 107 : 9	// この辺どう実装しようか
};

typedef struct ROI_SENSPACKET_DATA{
	uint8_t BumpsWheeldrops;
	uint8_t Wall;
	uint8_t CliffLeft;
	uint8_t CliffFrontLeft;
	uint8_t CliffFrontRight;
	uint8_t CliffRight;
	uint8_t VirtualWall;
	uint8_t Overcurrents;
	uint8_t DirtDetect;
	uint8_t IrOpcode;
	uint8_t Buttons;
	int16_t Distance;
	int16_t Angle;
	uint8_t ChargingState;
	uint8_t Voltage;
	int8_t Current;
	int8_t Temperature;
	uint16_t BatteryCharge;
	uint16_t BatteryCapacity;
	uint16_t WallSignal;
	uint16_t CliffLeftSignal;
	uint16_t CliffFrontLeftSignal;
	uint16_t CliffFrontRightSignal;
	uint16_t CliffRightSignal;
	uint8_t ChargerAvailable;
	uint8_t OpenInterfeceMode;
	uint8_t SongNumber;
	uint8_t SongPlaying;
	uint8_t OiStreamNumPackets;
	int16_t Velocity;
	int16_t Radius;
	int16_t VelocityRight;
	int16_t VelocityLeft;
	uint16_t EncoderCountsLeft;
	uint16_t EncoderCountsRight;
	uint8_t LightBumper;
	uint16_t LightBumpLeft;
	uint16_t LightBumpFrontLeft;
	uint16_t LightBumpCenterLeft;
	uint16_t LightBumpCenterRight;
	uint16_t LightBumpFrontRight;
	uint16_t LightBumpRight;
	uint8_t IrOpcodeLeft;
	uint8_t IrOpcodeRight;
	int16_t LeftMotorCurrent;
	int16_t RightMotorCurrent;
	int16_t MainBrushCurrent;
	int16_t SideBrushCurrent;
	uint8_t Stasis;
}ROI_SENSPACKET_DATA;



enum ROI_MODE{
	ROI_MODE_PASSIVE,
	ROI_MODE_SAFE,
	ROI_MODE_FULL,
	ROI_MODE_POWERDOWN,
};

class RoombaOi{
public:
	Sci_t *RoiSci;
	static ROI_SENSPACKET_DATA SensData;	// ルンバから受信したセンサデータ
	static ROI_SENSPACKET_DATA SensDataTmp;	// チェックサム確認までのtmp
	
	RoombaOi(void);
	~RoombaOi(void);

	// 初期化
	int8_t begin(Sci_t *SetSci);
	
	// 定期実行タスク
	void task(void);

	// 【コマンド送信】
	// モード
	int8_t setMode(ROI_MODE Mode);
	
	// 駆動
	int8_t drive(int16_t Spd, int16_t Radius);		// [mm/s], [mm]
	int8_t driveWheels(int16_t RightSpd, int16_t LeftSpd);	// [mm/s]
	int8_t drivePwm(int16_t RightPwm, int16_t LeftPwm);	// -255 - 255
	
	// LED
	int8_t outLeds(uint8_t LedBits);
	int8_t outLedBattColor(uint8_t Color, uint8_t Intensity);	// Cleanボタン・電池マーク　Color 色(0:green - 255:red) Intensity 明るさ(0:off - 255:max)
	int8_t outLedBit(bool_t LedOnOff, uint8_t BitNum);
	int8_t outLedOk(bool_t LedOnOff){return outLedBit(LedOnOff, 7);};
	int8_t outLedDustFull(bool_t LedOnOff){return outLedBit(LedOnOff, 6);};
	int8_t outLedDirtDetect(bool_t LedOnOff){return outLedBit(LedOnOff, 5);};
	int8_t outLedClean(bool_t LedOnOff){return outLedBit(LedOnOff, 4);};
	int8_t outLedCheckRobot(bool_t LedOnOff){return outLedBit(LedOnOff, 3);};
	int8_t outLedDock(bool_t LedOnOff){return outLedBit(LedOnOff, 2);};
	int8_t outLedSpot(bool_t LedOnOff){return outLedBit(LedOnOff, 1);};
	int8_t outLedDebris(bool_t LedOnOff){return outLedBit(LedOnOff, 0);};
	// Scheduling LEDs
	int8_t outSchedulingLeds(uint8_t LedBits);
	int8_t outSchedulingLedBit(bool_t LedOnOff, uint8_t BitNum);
	int8_t outLedDay(bool_t LedOnOff){return outSchedulingLedBit(LedOnOff, 7);};
	int8_t outLedHour(bool_t LedOnOff){return outSchedulingLedBit(LedOnOff, 6);};
	int8_t outLedMinute(bool_t LedOnOff){return outSchedulingLedBit(LedOnOff, 5);};
	int8_t outLedSchedule(bool_t LedOnOff){return outSchedulingLedBit(LedOnOff, 4);};
	int8_t outLedClock(bool_t LedOnOff){return outSchedulingLedBit(LedOnOff, 3);};
	int8_t outLedAM(bool_t LedOnOff){return outSchedulingLedBit(LedOnOff, 2);};
	int8_t outLedPM(bool_t LedOnOff){return outSchedulingLedBit(LedOnOff, 1);};
	int8_t outLedColon(bool_t LedOnOff){return outSchedulingLedBit(LedOnOff, 0);};
	
	
	// LED display
	int8_t outLedDispAscii(uint8_t *Digits);
	
	// Song
	// 音登録して鳴らす
	int8_t outNote(uint8_t SongNum, uint8_t SongLength, roi_song_packet_t* SongPacket);
	// 1音のみ
	int8_t outNoteSingle(uint8_t SongNum, uint8_t NoteNum, uint8_t NoteDuration);
	// 音楽登録
	int8_t setSong(uint8_t SongNum, uint8_t SongLength, const roi_song_packet_t* SongPacket);
	// 音楽鳴らす
	int8_t playSong(uint8_t SongNum);
	
	// 音楽鳴らす(longVer)
	// 再生開始
	int8_t playMusic(const roi_song_packet_t* SongPackets);
	// 再生処理(定期的に呼び出す)
	void playMusicTask(void);
	// 停止
	void stopMusic(void);
	// 再開
	void resumeMusic(void);
	
	// 再生中？
	bool_t isSongPlaying(void){return ((bool_t)SensData.SongPlaying)||((getTime_ms()<(TimePlaySong+80)));};
	
	
	// 【受信要求】
	int8_t setRcvStream(uint8_t NumPacket, ROI_SENSPACKET_ID* SensDatas);	// 定期受信するセンサデータ登録
	bool_t isRcvOnceAtLeast(void){return IsRcvOnceAtLeast;};	// 受信要求出した信号たち、最低一回は受信した？
	bool_t isConnect(void){return IsConnect;};		// 通信出来てる？(受信できてる？)
	
	// 【センサ値取得】
	int8_t getLightBumpSignal(uint16_t *LightBumpSignal);	// Left, FrontLeft, CenterLeft, CenterRight, FrontRight, Right
	int8_t getCliffSignal(uint16_t *CliffSignal);		// Left, FrontLeft, FrontRight, Right
	// 物理バンパー当たってる？
	bool_t isBump(void);
	bool_t isBumpRight(void);
	bool_t isBumpLeft(void);
	bool_t isCliff(void){return SensData.CliffLeft||SensData.CliffFrontLeft||SensData.CliffRight||SensData.CliffFrontRight;};
	// タイヤ
	// エンコーダ値取得
	// 0～65535
	int8_t getEncoderCounts(uint16_t* EncR, uint16_t* EncL);
	//バッテリ
	// 残容量[mAh]
	uint16_t getBatteryCharge(void);
	// 最大容量[mAh]
	uint16_t getBatteryCapacity(void);
	// SOC[%]
	uint8_t getBatterySoc(void);
	// ボタン
	bool_t isPushClean(void){return 0!=(SensData.Buttons&0x01);};	// Ceanボタン押されてる？
	bool_t isPushSpot(void){return 0!=(SensData.Buttons&0x02);};	// Spotボタン押されてる？
	bool_t isPushDock(void){return 0!=(SensData.Buttons&0x04);};	// Dockボタン押されてる？
	bool_t isPushMinute(void){return 0!=(SensData.Buttons&0x08);};	// Minuteボタン押されてる？(only Roomba560,570)
	bool_t isPushHour(void){return 0!=(SensData.Buttons&0x10);};	// Hourボタン押されてる？(only Roomba560,570)
	bool_t isPushDay(void){return 0!=(SensData.Buttons&0x20);};	// Dayボタン押されてる？(only Roomba560,570)
	bool_t isPushSchedule(void){return 0!=(SensData.Buttons&0x40);};// Scheduleボタン押されてる？(only Roomba560,570,7xx)
	bool_t isPushClock(void){return 0!=(SensData.Buttons&0x80);};	// Clockボタン押されてる？(only Roomba560,570,7xx)
	
	// コマンド送信
	int8_t sendCommand(ROI_OPCODE OpCode);				// コマンドのみ(データ無し)
	int8_t sendCommand(ROI_OPCODE OpCode, uint8_t Data);		// 1byteデータ有り
	int8_t sendCommand(ROI_OPCODE OpCode, uint16_t Data);		// 2byteデータ有り
	int8_t sendCommand(ROI_OPCODE OpCode, uint8_t *Data);		// データ有り(送信数はROI_OPCODE_BYTESを参照する)
	int8_t sendCommand(ROI_OPCODE OpCode, uint16_t *Data);
	int8_t sendCommand(ROI_OPCODE OpCode, uint8_t *Data, uint8_t DataNum);	// データ有り(送信数指定)
	
	
private:
	uint32_t SciBaud;
	ROI_MODE RoombaMode;	// 今のモード
	uint8_t LedBits;	// LED(bit3:CheckRobot, bit2:Dock, bit1:Spot, bit0:Debris)
	uint8_t LedBattColor;	// LED Cleanボタンの色(0:green - 255:red)
	uint8_t LedBattIntensity;	// LED Cleanボタンの明るさ(0:off - 255:max)
	uint8_t SchedulingLedBits;
	uint8_t WeekdayLedBits;
	
	//音
	uint32_t TimePlaySong;		// 再生要求だした時間
	const roi_song_packet_t* SongPackets;	// 再生中曲データ
	uint16_t PlayMusicCnt;		// 再生中曲データ位置
	uint8_t PlayMusicSongNum;	// 再生中SongNum
	uint8_t fSetMusic;		// 曲データセット済み
	
	static bool_t IsRcvOnceAtLeast;	// 全部一度は受信したフラグ(setRcvStreamでリセット)
	static ROI_SENSPACKET_ID* SensDataNeverRcv;	// 受信要求出してまた受信してないやつ。受信したら0xFFにする
	static uint8_t RcvPacketNum;	// 受信パケット数
	static bool_t IsConnect;	// 一度は何かしら受信したフラグ
	static uint16_t ConnectCnt;	// 受信有効カウント
	static bool_t IsRcvSet;		// 受信設定されてるフラグ
	static uint16_t RcvSetDataNum;	// 受信設定されてるデータの受信バイト数
	
	
	uint8_t RcvState;		// 受信処理状態 0:Idle, 1:ヘッダ来た, 2:データ受信中, 3:データ受信完了
	uint8_t DataNumForRcv;		// 受信する数	
	uint8_t RcvDataNum;		// 受信した数	
	uint8_t RcvDataTmp[2];		// 受信データ(各パケット)
	uint8_t CntRcvDataTmp;		//
	ROI_SENSPACKET_ID PacketId;	// 受信中のパケットID
	uint16_t Sum;			// チェックサムチェック用
	
	//static void handleReceive(RingBuffer<uint8_t>* RcvBuff);			// チェックサムとか見て受信データに突っ込む
	void handleReceive(void);			// チェックサムとか見て受信データに突っ込む
	static void setSensData(ROI_SENSPACKET_ID PacketId, uint8_t* RcvData);	// SensDataにデータを力技でセットする
	static bool_t SumErr;	// チェックサム違う
	static bool_t checkRcvOnceAtLeast(ROI_SENSPACKET_ID PacketId);	// 一度は受信したのをチェックする。受信したら0xffに置き換える
	
	
};

#endif