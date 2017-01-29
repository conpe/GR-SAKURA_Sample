/********************************************/
/*		るんば				*/
/*			ROI			*/
/*			for RX63n @ CS+		*/
/*			Wrote by conpe_		*/
/*			2016/03/09		*/
/********************************************/

//【更新履歴】
// 2016.03.09 書き始め

#include "RoombaOi.h"

//static
ROI_SENSPACKET_DATA RoombaOi::SensData;
ROI_SENSPACKET_DATA RoombaOi::SensDataTmp;
bool_t RoombaOi::IsRcvOnceAtLeast;
ROI_SENSPACKET_ID* RoombaOi::SensDataNeverRcv;
uint8_t RoombaOi::RcvPacketNum;
bool_t RoombaOi::IsConnect;
uint16_t RoombaOi::ConnectCnt;
bool_t RoombaOi::IsRcvSet;
uint16_t RoombaOi::RcvSetDataNum;
bool_t RoombaOi::SumErr;

RoombaOi::RoombaOi(void){
	IsRcvSet = false;
	RcvSetDataNum = 0;
	SumErr = false;
}
RoombaOi::~RoombaOi(void){
	delete[] SensDataNeverRcv;
}


// 初期化
int8_t RoombaOi::begin(Sci_t *SetSci){
	RoiSci = SetSci;
	SciBaud = ROI_BAUDRATE_DEFAULT;
	
	if(RoiSci->begin(SciBaud, true, true)){	// use tx, rx
		return -2;
	}
	
	setMode(ROI_MODE_PASSIVE);
	
	IsRcvOnceAtLeast = false;	// 要求パケット全部受信した
	IsConnect = true;		// 通信ok  受信要求でクリア。受信したらセット
	
	LedBits = 0x00;
	LedBattColor = 0x00;
	LedBattIntensity = 0x00;
	
	fSetMusic = false;
	
	TimePlaySong = 0;
	
	// センサデータ
	SensData.SongPlaying = false;
	
	
	return 0;
}

// タスク
//  通信okフラグ処理
//  音楽再生タスク
void RoombaOi::task(void){
	
	handleReceive();
	
	if(IsRcvSet){					// 受信設定してる
		if(ConnectCnt > ROI_CONNECT_CNT){	// 設定カウントより増えちゃったらルンバと通信できてない！
			if(IsConnect){
				outLedDispAscii("Err1");	// Err表示
				//Sci0.print("*******\r\nRmb Connect Disable\r\n**********\r\n");
			}
			IsConnect = false;
		}else{
			++ConnectCnt;
		}
	}
	// 音楽再生タスク
	playMusicTask();
}


// 【コマンド送信】
// モードセット
int8_t RoombaOi::setMode(ROI_MODE Mode){
	RoombaMode = Mode;
	switch(Mode){
	case ROI_MODE_PASSIVE:
		return sendCommand(ROI_OPCODE_START);
		break;
	case ROI_MODE_SAFE:
		return sendCommand(ROI_OPCODE_SAFE);
		break;
	case ROI_MODE_FULL:
		return sendCommand(ROI_OPCODE_FULL);
		break;
	case ROI_MODE_POWERDOWN:
		return sendCommand(ROI_OPCODE_POWER);
		break;
	}
	
	return 1;	// モード間違ってない？
}

// タイヤ駆動
int8_t RoombaOi::driveWheels(int16_t RightSpd, int16_t LeftSpd){
	uint16_t SendData[2];
	SendData[0] = RightSpd;
	SendData[1] = LeftSpd;
	return sendCommand(ROI_OPCODE_DRIVE_WHEELS, SendData);
}

// タイヤ駆動(PWM指定)
int8_t RoombaOi::drivePwm(int16_t RTireDuty, int16_t LTireDuty){
	int16_t SendData[2];
	SendData[0] = RTireDuty;
	SendData[1] = LTireDuty;
	return sendCommand(ROI_OPCODE_DRIVE_PWM, (uint16_t*)SendData);
}



/* LED */
// 一括設定
int8_t RoombaOi::outLeds(uint8_t LedBits){
	uint8_t SendData[3];
	
	this->LedBits = LedBits;
	SendData[0] = this->LedBits;
	SendData[1] = this->LedBattColor;
	SendData[2] = this->LedBattIntensity;
	
	return sendCommand(ROI_OPCODE_LEDS, SendData);
}
// バッテリインジケータ
// Color 色(0:green - 255:red) Intensity 明るさ(0:off - 255:max)
//  Intensityは効果なしっぽい
int8_t RoombaOi::outLedBattColor(uint8_t Color, uint8_t Intensity){
	uint8_t SendData[3];
	
	this->LedBattColor = Color;
	this->LedBattIntensity = Intensity;
	
	SendData[0] = this->LedBits;
	SendData[1] = this->LedBattColor;
	SendData[2] = this->LedBattIntensity;
	
	return sendCommand(ROI_OPCODE_LEDS, SendData);
}

int8_t RoombaOi::outLedBit(bool_t LedOnOff, uint8_t BitNum){
	uint8_t SendData[3];
	
	if(LedOnOff){	// 光らす
		this->LedBits = this->LedBits|(0x01<<BitNum);
	}else{		// 消す
		this->LedBits = this->LedBits&~(0x01<<BitNum);
	}
	
	SendData[0] = this->LedBits;
	SendData[1] = this->LedBattColor;
	SendData[2] = this->LedBattIntensity;
	
	return sendCommand(ROI_OPCODE_LEDS, SendData);
}


// Scheduling LEDs
int8_t RoombaOi::outSchedulingLeds(uint8_t LedBits){
	uint8_t SendData[2];
	
	this->SchedulingLedBits = LedBits;
	
	SendData[0] = this->WeekdayLedBits;
	SendData[1] = this->SchedulingLedBits;
	
	return sendCommand(ROI_OPCODE_SCHEDULING_LEDS, SendData);
}
// Schedule
int8_t RoombaOi::outSchedulingLedBit(bool_t LedOnOff, uint8_t BitNum){
	uint8_t SendData[2];
	
	if(LedOnOff){	// 光らす
		this->SchedulingLedBits = this->SchedulingLedBits|(0x01<<BitNum);
	}else{		// 消す
		this->SchedulingLedBits = this->SchedulingLedBits&(~(0x01<<BitNum));
	}
	SendData[0] = this->WeekdayLedBits;
	SendData[1] = this->SchedulingLedBits;
	
	return sendCommand(ROI_OPCODE_SCHEDULING_LEDS, SendData);
}




// LED display
int8_t RoombaOi::outLedDispAscii(uint8_t *Digits){
	return sendCommand(ROI_OPCODE_DIDIT_LEDS_ASCII, Digits);
}
	
	
// 音
// 音登録して鳴らす
int8_t RoombaOi::outNote(uint8_t SongNum, uint8_t SongLength, roi_song_packet_t* SongPacket){
	setSong(SongNum, SongLength, SongPacket);
	return playSong(SongNum);
}
// 音登録して鳴らす
// 1音のみ
int8_t RoombaOi::outNoteSingle(uint8_t SongNum, uint8_t NoteNum, uint8_t NoteDuration){
	roi_song_packet_t SongPacket;
	SongPacket.NoteNum = NoteNum;
	SongPacket.NoteDuration = NoteDuration;
	
	setSong(SongNum, 1, &SongPacket);
	return playSong(SongNum);
}
// 音楽登録
int8_t RoombaOi::setSong(uint8_t SongNum, uint8_t SongLength, const roi_song_packet_t* SongPacket){
	int8_t ack = -1;
	uint8_t* SendData;
	uint8_t i;
	
	SendData = new uint8_t[2+SongLength*2];
		if(NULL==SendData) __heap_chk_fail();
	SendData[0] = (uint8_t)SongNum;
	SendData[1] = SongLength;
	for(i=0;i<SongLength;i++){
		if(i>=ROI_SONG_LENGTH_MAX){
			break;
		}
		SendData[i*2+2] = SongPacket[i].NoteNum;
		SendData[i*2+3] = SongPacket[i].NoteDuration;
	}
	
	// デバッグ情報
	/*
	Sci0.print("setSong %d %d [", SongNum,SongLength);
	for(i = 0;i<2+SongLength*2;i++){
		Sci0.print("%d ", SendData[i]);
	}
	Sci0.print("]\r\n");
	*/
	ack = sendCommand(ROI_OPCODE_SONG, SendData, 2+SongLength*2);
	
	delete[] SendData;
	return ack;
}

// 音楽鳴らす
int8_t RoombaOi::playSong(uint8_t SongNum){
	TimePlaySong = getTime_ms();
	SensData.SongPlaying = true;	// 通信遅れを考慮して予めtrueセットしておく
	return sendCommand(ROI_OPCODE_PLAY, SongNum);
}


// 曲再生
// 再生開始
int8_t RoombaOi::playMusic(const roi_song_packet_t* SongPackets){
	uint8_t MusicLength;
	
	this->SongPackets = SongPackets;
	this->PlayMusicCnt = 0;
	this->PlayMusicSongNum = 0;
	
	// 初回データ登録
	
	// 16以上か確認
	for(MusicLength=0; MusicLength<ROI_SONG_LENGTH_MAX; MusicLength++){
		if((SongPackets[PlayMusicCnt+MusicLength].NoteNum==0x00)&&(SongPackets[PlayMusicCnt+MusicLength].NoteDuration==0x00)){
			// ROI_SONG_LENGTH_MAX個行く前にデータ終了
			//MusicLength++;
			break;
		}
	}
	if(MusicLength>0){
		// 次の1Num分登録
		setSong(this->PlayMusicSongNum, MusicLength, &SongPackets[PlayMusicCnt]);
		fSetMusic = true;
	}else{
		fSetMusic = false;
	}
	
	// 次登録用
	PlayMusicCnt += MusicLength;
	
		
	return 0;
}

// 再生処理(定期的に呼び出す)
void RoombaOi::playMusicTask(void){
	uint8_t MusicLength;
	
	// 再生終わったら次の音楽入れて再生する
	if(fSetMusic){	// 次のデータ登録されてる
		if(!isSongPlaying()){
			// 次の16バイト再生
			
			// 再生
			playSong(this->PlayMusicSongNum);
			// 実際に開始するのは送信が終わってから。
			// その後、isSongPlaying()がtrueになる。
			
			// 次再生用
			if(PlayMusicSongNum<ROI_SONG_NUM_MAX){
				PlayMusicSongNum++;
			}else{
				PlayMusicSongNum = ROI_SONG_NUM_MIN;
			}
			
			// データ数確認
			for(MusicLength=0; MusicLength<ROI_SONG_LENGTH_MAX; MusicLength++){
				if((SongPackets[PlayMusicCnt+MusicLength].NoteNum==0x00)&&(SongPackets[PlayMusicCnt+MusicLength].NoteDuration==0x00)){
					// ROI_SONG_LENGTH_MAX個行く前にデータ終了
					//MusicLength++;
					break;
				}
			}
			if(MusicLength>0){
				// 次の1Num分登録
				setSong(this->PlayMusicSongNum, MusicLength, &SongPackets[PlayMusicCnt]);
				//playSong(this->PlayMusicSongNum);
				fSetMusic = true;
			}else{
				fSetMusic = false;
			}
			// 次登録用
			PlayMusicCnt += MusicLength;
			
			
		}
	}else{
		/* do nothing. */
	}
	
}
// 停止
void RoombaOi::stopMusic(void){
	fSetMusic = false;
	//outNoteSingle(0, 127, 0);
}
// 停止
void RoombaOi::resumeMusic(void){
	fSetMusic = true;
}






//【受信要求】
int8_t RoombaOi::setRcvStream(uint8_t NumPacket, ROI_SENSPACKET_ID* SensDatas){
	int8_t ack;
	uint8_t *SendData;
	
	SendData = new uint8_t[NumPacket+1];
		if(NULL==SendData) __heap_chk_fail();
	SendData[0] = NumPacket;
	
	//memcpy(SendData+1, SensDatas, NumPacket);
	//memcpy(SensDataNeverRcv, SensDatas, NumPacket);
	if(SensDataNeverRcv){
		delete[] SensDataNeverRcv;
	}
	SensDataNeverRcv = new ROI_SENSPACKET_ID[NumPacket];
		if(NULL==SensDataNeverRcv) __heap_chk_fail();
	if(NULL==SensDataNeverRcv){
		return -1;
	}
	
	RcvSetDataNum = 0;
	
	for(uint8_t i=0; i<NumPacket; i++){
		if(sizeof(ROI_SENSPACKET_BYTES) > (uint8_t)(*SensDatas)){
			RcvSetDataNum += (1 + ROI_SENSPACKET_BYTES[(uint8_t)(*SensDatas)]);
			SendData[1+i] = (uint8_t)(*SensDatas);
			SensDataNeverRcv[i] = *SensDatas;
			SensDatas++;
		}
	}
	
	ack = sendCommand(ROI_OPCODE_STREAM, SendData, NumPacket+1);
	
	delete[] SendData;
	
	IsRcvOnceAtLeast = false;	// すべて一度は受信したフラグ消去
	IsConnect = false;		// 受信するまでクリアしておく
	
	RcvPacketNum = NumPacket;	// 受信するデータ数
	IsRcvSet = true;
	
	return ack;
}

//【センサ値受信】

// 光バンパー
// Left, FrontLeft, CenterLeft, CenterRight, FrontRight, Right
int8_t RoombaOi::getLightBumpSignal(uint16_t *LightBumpSignal){
	*LightBumpSignal = SensData.LightBumpLeft;
	*(LightBumpSignal+1) = SensData.LightBumpFrontLeft;
	*(LightBumpSignal+2) = SensData.LightBumpCenterLeft;
	*(LightBumpSignal+3) = SensData.LightBumpCenterRight;
	*(LightBumpSignal+4) = SensData.LightBumpFrontRight;
	*(LightBumpSignal+5) = SensData.LightBumpRight;
	
	return 6;
}

// バンパー
bool_t RoombaOi::isBump(void){
	return 0!=(SensData.BumpsWheeldrops&0x03);
}
bool_t RoombaOi::isBumpRight(void){
	return 0!=(SensData.BumpsWheeldrops&0x01);
}
bool_t RoombaOi::isBumpLeft(void){
	return 0!=(SensData.BumpsWheeldrops&0x02);
}
//タイヤ
// エンコーダ値取得
int8_t RoombaOi::getEncoderCounts(uint16_t* EncR, uint16_t* EncL){
	*EncR = SensData.EncoderCountsRight;		// ROIのでーたしーと内でRLちぐはぐだけど
	*EncL = SensData.EncoderCountsLeft;		// これで合ってる。
	return 0;
}
//バッテリ
// 残容量[mAh]
uint16_t RoombaOi::getBatteryCharge(void){
	return SensData.BatteryCharge;
}
// 最大容量[mAh]
uint16_t RoombaOi::getBatteryCapacity(void){
	return SensData.BatteryCapacity;
}
// SOC[%]
uint8_t RoombaOi::getBatterySoc(void){
	return 100*SensData.BatteryCharge/SensData.BatteryCapacity;
}





// コマンド送信
int8_t RoombaOi::sendCommand(ROI_OPCODE OpCode){
	return RoiSci->transmit(ROI_OPCODE_ID[OpCode]);
}
// コマンド送信(1バイトデータ有り)
int8_t RoombaOi::sendCommand(ROI_OPCODE OpCode, uint8_t Data){
	int8_t ack;	// 0じゃなかったらなにかある
	ack = RoiSci->transmit(ROI_OPCODE_ID[OpCode]);
	ack |= RoiSci->transmit(Data);
	
	return ack;
}
// コマンド送信(2バイトデータ有り)
int8_t RoombaOi::sendCommand(ROI_OPCODE OpCode, uint16_t Data){
	int8_t ack;	// 0じゃなかったらなにかある
	ack = RoiSci->transmit(ROI_OPCODE_ID[OpCode]);
	ack |= RoiSci->transmit((uint8_t)(Data>>8));
	ack |= RoiSci->transmit((uint8_t)(Data&0x00FF));
	
	return ack;
}
//コマンド送信(データ有り)
// 送信するバイト数はROI_OPCODE_BYTESを使う
int8_t RoombaOi::sendCommand(ROI_OPCODE OpCode, uint8_t *Data){
	uint8_t DataNum = ROI_OPCODE_BYTES[OpCode];
	int8_t ack;	// 0じゃなかったらなにかある
	
	ack = RoiSci->transmit(ROI_OPCODE_ID[OpCode]);
	for(uint8_t i=0;i<DataNum;i++){
		ack |= RoiSci->transmit(*Data);
		Data++;
	}
	
	return ack;
}
//コマンド送信(データ有り)
// 送信するバイト数はROI_OPCODE_BYTESを使う
int8_t RoombaOi::sendCommand(ROI_OPCODE OpCode, uint16_t *Data){
	uint8_t DataNum = ROI_OPCODE_BYTES[OpCode]/2;
	int8_t ack;	// 0じゃなかったらなにかある
	
	ack = RoiSci->transmit(ROI_OPCODE_ID[OpCode]);
	for(uint8_t i=0;i<DataNum;i++){
		ack |= RoiSci->transmit((uint8_t)((*Data>>8)&0x00ff));
		ack |= RoiSci->transmit((uint8_t)((*Data)&0x00ff));
		Data++;
	}
	
	return ack;
}
//コマンド送信(データ, 送信バイト指定)
// 
int8_t RoombaOi::sendCommand(ROI_OPCODE OpCode, uint8_t *Data, uint8_t DataNum){
	int8_t ack;	// 0じゃなかったらなにかある
	
	ack = RoiSci->transmit(ROI_OPCODE_ID[OpCode]);
	for(uint8_t i=0;i<DataNum;i++){
		ack |= RoiSci->transmit(*Data);
		Data++;
	}
	
	return ack;
}

//static
// SCI受信処理
void RoombaOi::handleReceive(void){
	uint8_t RcvData;		// 受信データひとつ
	
	if(!IsRcvSet){	// 受信設定前だったら処理しない。
		return;
	}
	
	static RingBuffer<uint8_t> debugBuf(128);
	static RingBuffer<uint8_t> debugBufState(128);
	
	static uint32_t LastTime;
	bool_t prin = false;
	static uint16_t debugDataNumTmp;
	
	// 200msecごと
	if(((LastTime+200) < getTime_ms()) || (false == IsConnect)){
		//prin = true;
		LastTime = getTime_ms();
	}
	if(false == IsConnect){
		prin = true;
		//LastTime = getTime_ms();
	}
	if(prin){
		//Sci0.print("%08d Rmb RcvBuffNum = %d ", getTime_ms(), RoiSci->available());
	}
	
	while(RoiSci->isAvailable()){		// 受信データある限り
		RcvData = RoiSci->read();
		Sum += RcvData;
		debugBuf.add(RcvData);
		debugBufState.add(RcvState);
		/*
		if(prin){
			Sci0.print("0x%02X ", RcvData);
		}
		*/
		switch(RcvState){
		case 0:	// idle(ヘッダ待ち)
			if(19==RcvData){	// ヘッダ来た
				//if(!SumErr){
					RcvState = 1;
				//}else{
				//	SumErr = false;	// 一回見送る(前回チェックサム合ってなかったので受信周期ずらすため)
				//}
				Sum = RcvData;
			}
			break;
		case 1:	// 受信バイト数待ち
			DataNumForRcv = RcvData;
			RcvDataNum = 0;
			debugDataNumTmp = DataNumForRcv;
			RcvState = 2;
			CntRcvDataTmp = 0;
			
			if(DataNumForRcv != RcvSetDataNum){	// 予定していた受信数と異なる
				RcvState = 0;
			}
			break;
		case 2:	// データ受信
			// パケットID
			if(0==CntRcvDataTmp){
				PacketId = (ROI_SENSPACKET_ID)RcvData;	// パケットID来た
				if(sizeof(ROI_SENSPACKET_BYTES)<=PacketId){	// パケットIDおかしい
					RcvState = 0;
					break;
				}
				CntRcvDataTmp ++;
			}else{
			// データ
				if(sizeof(RcvDataTmp) < CntRcvDataTmp){
					RcvState = 0;
					break;
				}
				RcvDataTmp[CntRcvDataTmp-1] = RcvData;
				if(ROI_SENSPACKET_BYTES[PacketId]<=CntRcvDataTmp){	// パケットIDごとのバイト数受信した
					//そのセンサデータ受信終わり
					setSensData(PacketId, RcvDataTmp);	// データセット
					CntRcvDataTmp = 0;	// 次のパケットid受信用
					if(!IsRcvOnceAtLeast){	// まだ一度も受信してないパケットがいる
						checkRcvOnceAtLeast(PacketId);	// 受信済みであることをセット
					}
				}else{
					CntRcvDataTmp ++;
				}
			}
			RcvDataNum++;
			if(DataNumForRcv <= RcvDataNum){	// 必要数受信したら受信完了
				RcvState = 3;
			}
			break;
		case 3:	// チェックサム
			// チェックサムまで足して下2桁が0x00ならok
			
			if(0x00 == (Sum&0x00FF)){	// チェックサム計算
				IsConnect = true;	// 受信したので受信フラグ立てる
				ConnectCnt = 0;		// 受信カウンタクリア
				SumErr = false;
				
				// サム合ってたのでデータをコピー
				SensData = SensDataTmp;
				
				
				if(prin){
					//Sci0.print("%08d sum OK 0x%02X 0x%02X\r\n", getTime_ms(), RcvData, Sum-RcvData);
				}
				
			}else{
				//Debug
				Sci0.print("%08d sum NG 0x%02X\r\n", getTime_ms(), RcvData);
					
				Sci0.print("BufOut = 0x ");
				while(false == debugBuf.isEmpty()){
					Sci0.print("%02X ", debugBuf.read());
				}
				Sci0.print("\r\nStaOut = 0x ");
				while(false == debugBufState.isEmpty()){
					Sci0.print("%02X ", debugBufState.read());
				}
				Sci0.print("\r\niscon=%d RcvDataNum=%d/%d state=%d, sum=0x%04x\r\n", isConnect(), RcvDataNum, debugDataNumTmp, RcvState, Sum);
				
				
				SumErr = true;
			}
			RcvState = 0;
			break;
		default:
			Sci0.print("Rmb RcvState Err\r\n");
			RcvState = 0;
		}
		
		
		if(RcvState == 0){
			debugBuf.clear();
			debugBufState.clear();
		}
		
		
		
		
	}
		
}

void RoombaOi::setSensData(ROI_SENSPACKET_ID PacketId, uint8_t* RcvData){
	switch(PacketId){
	case ROI_SENSPACKET_BUMPS_WHEELDROPS:
		SensDataTmp.BumpsWheeldrops = *RcvData;
		break;
	case ROI_SENSPACKET_WALL:
		SensDataTmp.Wall = *RcvData;
		break;
	case ROI_SENSPACKET_CLIFF_LEFT:
		SensDataTmp.CliffLeft = *RcvData;
		break;
	case ROI_SENSPACKET_CLIFF_FRONT_LEFT:
		SensDataTmp.CliffFrontLeft = *RcvData;
		break;
	case ROI_SENSPACKET_CLIFF_FRONT_RIGHT:
		SensDataTmp.CliffFrontRight = *RcvData;
		break;
	case ROI_SENSPACKET_CLIFF_RIGHT:
		SensDataTmp.CliffRight = *RcvData;
		break;
	case ROI_SENSPACKET_VIRTUAL_WALL:
		SensDataTmp.VirtualWall = *RcvData;
		break;
	case ROI_SENSPACKET_OVERCURRENTS:
		SensDataTmp.Overcurrents = *RcvData;
		break;
	case ROI_SENSPACKET_DIRT_DETECT:
		SensDataTmp.DirtDetect = *RcvData;
		break;
	case ROI_SENSPACKET_IR_OPCODE:
		SensDataTmp.IrOpcode = *RcvData;
		break;
	case ROI_SENSPACKET_BUTTONS:
		SensDataTmp.Buttons = *RcvData;
		break;
	case ROI_SENSPACKET_DISTANCE:
		SensDataTmp.Distance = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_ANGLE:
		SensDataTmp.Angle = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_CHARGING_STATE:
		SensDataTmp.ChargingState = *RcvData;
		break;
	case ROI_SENSPACKET_VOLTAGE:
		SensDataTmp.Voltage = *RcvData;
		break;
	case ROI_SENSPACKET_CURRENT:
		SensDataTmp.Current = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_TEMPERATURE:
		SensDataTmp.Temperature = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_BATTERY_CHARGE:
		SensDataTmp.BatteryCharge = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_BATTERY_CAPACITY:
		SensDataTmp.BatteryCapacity = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_WALL_SIGNAL:
		SensDataTmp.WallSignal = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_CLIFF_LEFT_SIGNAL:
		SensDataTmp.CliffLeftSignal = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_CLIFF_FRONT_LEFT_SIGNAL:
		SensDataTmp.CliffFrontLeftSignal = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_CLIFF_FRONT_RIGHT_SIGNAL:
		SensDataTmp.CliffFrontRightSignal = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_CLIFF_RIGHT_SIGNAL:
		SensDataTmp.CliffRightSignal = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_CHARGER_AVAILABLE:
		SensDataTmp.ChargerAvailable = *RcvData;
		break;
	case ROI_SENSPACKET_OPEN_INTERFCE_MODE:
		SensDataTmp.OpenInterfeceMode = *RcvData;
		break;
	case ROI_SENSPACKET_SONG_NUMBER:
		SensDataTmp.SongNumber = *RcvData;
		break;
	case ROI_SENSPACKET_SONG_PLAYING:
		SensDataTmp.SongPlaying = *RcvData;
		break;
	case ROI_SENSPACKET_OI_STREAM_NUM_PACKETS:
		SensDataTmp.OiStreamNumPackets = *RcvData;
		break;
	case ROI_SENSPACKET_VELOCITY:
		SensDataTmp.Velocity = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_RADIUS:
		SensDataTmp.Radius = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_VELOCITY_RIGHT:
		SensDataTmp.VelocityRight = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_VELOCITY_LEFT:
		SensDataTmp.VelocityLeft = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_ENCODER_COUNTS_LEFT:
		SensDataTmp.EncoderCountsLeft = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_ENCODER_COUNTS_RIGHT:
		SensDataTmp.EncoderCountsRight = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_LIGHT_BUMPER:
		SensDataTmp.LightBumper = *RcvData;
		break;
	case ROI_SENSPACKET_LIGHT_BUMP_LEFT:
		SensDataTmp.LightBumpLeft = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_LIGHT_BUMP_FRONT_LEFT:
		SensDataTmp.LightBumpFrontLeft = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_LIGHT_BUMP_CENTER_LEFT:
		SensDataTmp.LightBumpCenterLeft = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_LIGHT_BUMP_CENTER_RIGHT:
		SensDataTmp.LightBumpCenterRight = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_LIGHT_BUMP_FRONT_RIGHT:
		SensDataTmp.LightBumpFrontRight = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_LIGHT_BUMP_RIGHT:
		SensDataTmp.LightBumpRight = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_IR_OPCODE_LEFT:
		SensDataTmp.IrOpcodeLeft = *RcvData;
		break;
	case ROI_SENSPACKET_IR_OPCODE_RIGHT:
		SensDataTmp.IrOpcodeRight = *RcvData;
		break;
	case ROI_SENSPACKET_LEFT_MOTOR_CURRENT:
		SensDataTmp.LeftMotorCurrent = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_RIGHT_MOTOR_CURRENT:
		SensDataTmp.RightMotorCurrent = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_MAIN_BRUSH_CURRENT:
		SensDataTmp.MainBrushCurrent = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_SIDE_BRUSH_CURRENT:
		SensDataTmp.SideBrushCurrent = ((*RcvData)<<8) | (*(RcvData+1));
		break;
	case ROI_SENSPACKET_STASIS:
		SensDataTmp.Stasis = *RcvData;
		break;
	
	}
}

bool_t RoombaOi::checkRcvOnceAtLeast(ROI_SENSPACKET_ID PacketId){
	uint8_t i;
	// 来たパケットIDが存在する奴だったら受信済みとしてROI_SENSPACKET_NONEに置き換える
	for(i=0;i<RcvPacketNum;i++){
		if(SensDataNeverRcv[i] == PacketId){
			SensDataNeverRcv[i] = ROI_SENSPACKET_NONE;
			break;
		}
	}
	
	// 全部受信済みならフラグ立てる
	bool_t FlagTmp = true;;
	for(i=0;i<RcvPacketNum;i++){	// 受信パケット数だけ繰り返す。
		FlagTmp = FlagTmp&&(ROI_SENSPACKET_NONE==SensDataNeverRcv[i]);
		if(!FlagTmp){	// Flagがfalseになったらそこでやめる
			break;	
		}
	}
	
	// 判定結果
	IsRcvOnceAtLeast = FlagTmp;
	
	return FlagTmp;
}

