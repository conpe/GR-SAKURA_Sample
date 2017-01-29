/********************************************/
/*		����				*/
/*			ROI			*/
/*			for RX63n @ CS+		*/
/*			Wrote by conpe_		*/
/*			2016/03/09		*/
/********************************************/

//�y�X�V�����z
// 2016.03.09 �����n��

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


// ������
int8_t RoombaOi::begin(Sci_t *SetSci){
	RoiSci = SetSci;
	SciBaud = ROI_BAUDRATE_DEFAULT;
	
	if(RoiSci->begin(SciBaud, true, true)){	// use tx, rx
		return -2;
	}
	
	setMode(ROI_MODE_PASSIVE);
	
	IsRcvOnceAtLeast = false;	// �v���p�P�b�g�S����M����
	IsConnect = true;		// �ʐMok  ��M�v���ŃN���A�B��M������Z�b�g
	
	LedBits = 0x00;
	LedBattColor = 0x00;
	LedBattIntensity = 0x00;
	
	fSetMusic = false;
	
	TimePlaySong = 0;
	
	// �Z���T�f�[�^
	SensData.SongPlaying = false;
	
	
	return 0;
}

// �^�X�N
//  �ʐMok�t���O����
//  ���y�Đ��^�X�N
void RoombaOi::task(void){
	
	handleReceive();
	
	if(IsRcvSet){					// ��M�ݒ肵�Ă�
		if(ConnectCnt > ROI_CONNECT_CNT){	// �ݒ�J�E���g��葝����������烋���o�ƒʐM�ł��ĂȂ��I
			if(IsConnect){
				outLedDispAscii("Err1");	// Err�\��
				//Sci0.print("*******\r\nRmb Connect Disable\r\n**********\r\n");
			}
			IsConnect = false;
		}else{
			++ConnectCnt;
		}
	}
	// ���y�Đ��^�X�N
	playMusicTask();
}


// �y�R�}���h���M�z
// ���[�h�Z�b�g
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
	
	return 1;	// ���[�h�Ԉ���ĂȂ��H
}

// �^�C���쓮
int8_t RoombaOi::driveWheels(int16_t RightSpd, int16_t LeftSpd){
	uint16_t SendData[2];
	SendData[0] = RightSpd;
	SendData[1] = LeftSpd;
	return sendCommand(ROI_OPCODE_DRIVE_WHEELS, SendData);
}

// �^�C���쓮(PWM�w��)
int8_t RoombaOi::drivePwm(int16_t RTireDuty, int16_t LTireDuty){
	int16_t SendData[2];
	SendData[0] = RTireDuty;
	SendData[1] = LTireDuty;
	return sendCommand(ROI_OPCODE_DRIVE_PWM, (uint16_t*)SendData);
}



/* LED */
// �ꊇ�ݒ�
int8_t RoombaOi::outLeds(uint8_t LedBits){
	uint8_t SendData[3];
	
	this->LedBits = LedBits;
	SendData[0] = this->LedBits;
	SendData[1] = this->LedBattColor;
	SendData[2] = this->LedBattIntensity;
	
	return sendCommand(ROI_OPCODE_LEDS, SendData);
}
// �o�b�e���C���W�P�[�^
// Color �F(0:green - 255:red) Intensity ���邳(0:off - 255:max)
//  Intensity�͌��ʂȂ����ۂ�
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
	
	if(LedOnOff){	// ���炷
		this->LedBits = this->LedBits|(0x01<<BitNum);
	}else{		// ����
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
	
	if(LedOnOff){	// ���炷
		this->SchedulingLedBits = this->SchedulingLedBits|(0x01<<BitNum);
	}else{		// ����
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
	
	
// ��
// ���o�^���Ė炷
int8_t RoombaOi::outNote(uint8_t SongNum, uint8_t SongLength, roi_song_packet_t* SongPacket){
	setSong(SongNum, SongLength, SongPacket);
	return playSong(SongNum);
}
// ���o�^���Ė炷
// 1���̂�
int8_t RoombaOi::outNoteSingle(uint8_t SongNum, uint8_t NoteNum, uint8_t NoteDuration){
	roi_song_packet_t SongPacket;
	SongPacket.NoteNum = NoteNum;
	SongPacket.NoteDuration = NoteDuration;
	
	setSong(SongNum, 1, &SongPacket);
	return playSong(SongNum);
}
// ���y�o�^
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
	
	// �f�o�b�O���
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

// ���y�炷
int8_t RoombaOi::playSong(uint8_t SongNum){
	TimePlaySong = getTime_ms();
	SensData.SongPlaying = true;	// �ʐM�x����l�����ė\��true�Z�b�g���Ă���
	return sendCommand(ROI_OPCODE_PLAY, SongNum);
}


// �ȍĐ�
// �Đ��J�n
int8_t RoombaOi::playMusic(const roi_song_packet_t* SongPackets){
	uint8_t MusicLength;
	
	this->SongPackets = SongPackets;
	this->PlayMusicCnt = 0;
	this->PlayMusicSongNum = 0;
	
	// ����f�[�^�o�^
	
	// 16�ȏォ�m�F
	for(MusicLength=0; MusicLength<ROI_SONG_LENGTH_MAX; MusicLength++){
		if((SongPackets[PlayMusicCnt+MusicLength].NoteNum==0x00)&&(SongPackets[PlayMusicCnt+MusicLength].NoteDuration==0x00)){
			// ROI_SONG_LENGTH_MAX�s���O�Ƀf�[�^�I��
			//MusicLength++;
			break;
		}
	}
	if(MusicLength>0){
		// ����1Num���o�^
		setSong(this->PlayMusicSongNum, MusicLength, &SongPackets[PlayMusicCnt]);
		fSetMusic = true;
	}else{
		fSetMusic = false;
	}
	
	// ���o�^�p
	PlayMusicCnt += MusicLength;
	
		
	return 0;
}

// �Đ�����(����I�ɌĂяo��)
void RoombaOi::playMusicTask(void){
	uint8_t MusicLength;
	
	// �Đ��I������玟�̉��y����čĐ�����
	if(fSetMusic){	// ���̃f�[�^�o�^����Ă�
		if(!isSongPlaying()){
			// ����16�o�C�g�Đ�
			
			// �Đ�
			playSong(this->PlayMusicSongNum);
			// ���ۂɊJ�n����̂͑��M���I����Ă���B
			// ���̌�AisSongPlaying()��true�ɂȂ�B
			
			// ���Đ��p
			if(PlayMusicSongNum<ROI_SONG_NUM_MAX){
				PlayMusicSongNum++;
			}else{
				PlayMusicSongNum = ROI_SONG_NUM_MIN;
			}
			
			// �f�[�^���m�F
			for(MusicLength=0; MusicLength<ROI_SONG_LENGTH_MAX; MusicLength++){
				if((SongPackets[PlayMusicCnt+MusicLength].NoteNum==0x00)&&(SongPackets[PlayMusicCnt+MusicLength].NoteDuration==0x00)){
					// ROI_SONG_LENGTH_MAX�s���O�Ƀf�[�^�I��
					//MusicLength++;
					break;
				}
			}
			if(MusicLength>0){
				// ����1Num���o�^
				setSong(this->PlayMusicSongNum, MusicLength, &SongPackets[PlayMusicCnt]);
				//playSong(this->PlayMusicSongNum);
				fSetMusic = true;
			}else{
				fSetMusic = false;
			}
			// ���o�^�p
			PlayMusicCnt += MusicLength;
			
			
		}
	}else{
		/* do nothing. */
	}
	
}
// ��~
void RoombaOi::stopMusic(void){
	fSetMusic = false;
	//outNoteSingle(0, 127, 0);
}
// ��~
void RoombaOi::resumeMusic(void){
	fSetMusic = true;
}






//�y��M�v���z
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
	
	IsRcvOnceAtLeast = false;	// ���ׂĈ�x�͎�M�����t���O����
	IsConnect = false;		// ��M����܂ŃN���A���Ă���
	
	RcvPacketNum = NumPacket;	// ��M����f�[�^��
	IsRcvSet = true;
	
	return ack;
}

//�y�Z���T�l��M�z

// ���o���p�[
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

// �o���p�[
bool_t RoombaOi::isBump(void){
	return 0!=(SensData.BumpsWheeldrops&0x03);
}
bool_t RoombaOi::isBumpRight(void){
	return 0!=(SensData.BumpsWheeldrops&0x01);
}
bool_t RoombaOi::isBumpLeft(void){
	return 0!=(SensData.BumpsWheeldrops&0x02);
}
//�^�C��
// �G���R�[�_�l�擾
int8_t RoombaOi::getEncoderCounts(uint16_t* EncR, uint16_t* EncL){
	*EncR = SensData.EncoderCountsRight;		// ROI�̂Ł[�����[�Ɠ���RL�����͂�������
	*EncL = SensData.EncoderCountsLeft;		// ����ō����Ă�B
	return 0;
}
//�o�b�e��
// �c�e��[mAh]
uint16_t RoombaOi::getBatteryCharge(void){
	return SensData.BatteryCharge;
}
// �ő�e��[mAh]
uint16_t RoombaOi::getBatteryCapacity(void){
	return SensData.BatteryCapacity;
}
// SOC[%]
uint8_t RoombaOi::getBatterySoc(void){
	return 100*SensData.BatteryCharge/SensData.BatteryCapacity;
}





// �R�}���h���M
int8_t RoombaOi::sendCommand(ROI_OPCODE OpCode){
	return RoiSci->transmit(ROI_OPCODE_ID[OpCode]);
}
// �R�}���h���M(1�o�C�g�f�[�^�L��)
int8_t RoombaOi::sendCommand(ROI_OPCODE OpCode, uint8_t Data){
	int8_t ack;	// 0����Ȃ�������Ȃɂ�����
	ack = RoiSci->transmit(ROI_OPCODE_ID[OpCode]);
	ack |= RoiSci->transmit(Data);
	
	return ack;
}
// �R�}���h���M(2�o�C�g�f�[�^�L��)
int8_t RoombaOi::sendCommand(ROI_OPCODE OpCode, uint16_t Data){
	int8_t ack;	// 0����Ȃ�������Ȃɂ�����
	ack = RoiSci->transmit(ROI_OPCODE_ID[OpCode]);
	ack |= RoiSci->transmit((uint8_t)(Data>>8));
	ack |= RoiSci->transmit((uint8_t)(Data&0x00FF));
	
	return ack;
}
//�R�}���h���M(�f�[�^�L��)
// ���M����o�C�g����ROI_OPCODE_BYTES���g��
int8_t RoombaOi::sendCommand(ROI_OPCODE OpCode, uint8_t *Data){
	uint8_t DataNum = ROI_OPCODE_BYTES[OpCode];
	int8_t ack;	// 0����Ȃ�������Ȃɂ�����
	
	ack = RoiSci->transmit(ROI_OPCODE_ID[OpCode]);
	for(uint8_t i=0;i<DataNum;i++){
		ack |= RoiSci->transmit(*Data);
		Data++;
	}
	
	return ack;
}
//�R�}���h���M(�f�[�^�L��)
// ���M����o�C�g����ROI_OPCODE_BYTES���g��
int8_t RoombaOi::sendCommand(ROI_OPCODE OpCode, uint16_t *Data){
	uint8_t DataNum = ROI_OPCODE_BYTES[OpCode]/2;
	int8_t ack;	// 0����Ȃ�������Ȃɂ�����
	
	ack = RoiSci->transmit(ROI_OPCODE_ID[OpCode]);
	for(uint8_t i=0;i<DataNum;i++){
		ack |= RoiSci->transmit((uint8_t)((*Data>>8)&0x00ff));
		ack |= RoiSci->transmit((uint8_t)((*Data)&0x00ff));
		Data++;
	}
	
	return ack;
}
//�R�}���h���M(�f�[�^, ���M�o�C�g�w��)
// 
int8_t RoombaOi::sendCommand(ROI_OPCODE OpCode, uint8_t *Data, uint8_t DataNum){
	int8_t ack;	// 0����Ȃ�������Ȃɂ�����
	
	ack = RoiSci->transmit(ROI_OPCODE_ID[OpCode]);
	for(uint8_t i=0;i<DataNum;i++){
		ack |= RoiSci->transmit(*Data);
		Data++;
	}
	
	return ack;
}

//static
// SCI��M����
void RoombaOi::handleReceive(void){
	uint8_t RcvData;		// ��M�f�[�^�ЂƂ�
	
	if(!IsRcvSet){	// ��M�ݒ�O�������珈�����Ȃ��B
		return;
	}
	
	static RingBuffer<uint8_t> debugBuf(128);
	static RingBuffer<uint8_t> debugBufState(128);
	
	static uint32_t LastTime;
	bool_t prin = false;
	static uint16_t debugDataNumTmp;
	
	// 200msec����
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
	
	while(RoiSci->isAvailable()){		// ��M�f�[�^�������
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
		case 0:	// idle(�w�b�_�҂�)
			if(19==RcvData){	// �w�b�_����
				//if(!SumErr){
					RcvState = 1;
				//}else{
				//	SumErr = false;	// ��񌩑���(�O��`�F�b�N�T�������ĂȂ������̂Ŏ�M�������炷����)
				//}
				Sum = RcvData;
			}
			break;
		case 1:	// ��M�o�C�g���҂�
			DataNumForRcv = RcvData;
			RcvDataNum = 0;
			debugDataNumTmp = DataNumForRcv;
			RcvState = 2;
			CntRcvDataTmp = 0;
			
			if(DataNumForRcv != RcvSetDataNum){	// �\�肵�Ă�����M���ƈقȂ�
				RcvState = 0;
			}
			break;
		case 2:	// �f�[�^��M
			// �p�P�b�gID
			if(0==CntRcvDataTmp){
				PacketId = (ROI_SENSPACKET_ID)RcvData;	// �p�P�b�gID����
				if(sizeof(ROI_SENSPACKET_BYTES)<=PacketId){	// �p�P�b�gID��������
					RcvState = 0;
					break;
				}
				CntRcvDataTmp ++;
			}else{
			// �f�[�^
				if(sizeof(RcvDataTmp) < CntRcvDataTmp){
					RcvState = 0;
					break;
				}
				RcvDataTmp[CntRcvDataTmp-1] = RcvData;
				if(ROI_SENSPACKET_BYTES[PacketId]<=CntRcvDataTmp){	// �p�P�b�gID���Ƃ̃o�C�g����M����
					//���̃Z���T�f�[�^��M�I���
					setSensData(PacketId, RcvDataTmp);	// �f�[�^�Z�b�g
					CntRcvDataTmp = 0;	// ���̃p�P�b�gid��M�p
					if(!IsRcvOnceAtLeast){	// �܂���x����M���ĂȂ��p�P�b�g������
						checkRcvOnceAtLeast(PacketId);	// ��M�ς݂ł��邱�Ƃ��Z�b�g
					}
				}else{
					CntRcvDataTmp ++;
				}
			}
			RcvDataNum++;
			if(DataNumForRcv <= RcvDataNum){	// �K�v����M�������M����
				RcvState = 3;
			}
			break;
		case 3:	// �`�F�b�N�T��
			// �`�F�b�N�T���܂ő����ĉ�2����0x00�Ȃ�ok
			
			if(0x00 == (Sum&0x00FF)){	// �`�F�b�N�T���v�Z
				IsConnect = true;	// ��M�����̂Ŏ�M�t���O���Ă�
				ConnectCnt = 0;		// ��M�J�E���^�N���A
				SumErr = false;
				
				// �T�������Ă��̂Ńf�[�^���R�s�[
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
	// �����p�P�b�gID�����݂���z���������M�ς݂Ƃ���ROI_SENSPACKET_NONE�ɒu��������
	for(i=0;i<RcvPacketNum;i++){
		if(SensDataNeverRcv[i] == PacketId){
			SensDataNeverRcv[i] = ROI_SENSPACKET_NONE;
			break;
		}
	}
	
	// �S����M�ς݂Ȃ�t���O���Ă�
	bool_t FlagTmp = true;;
	for(i=0;i<RcvPacketNum;i++){	// ��M�p�P�b�g�������J��Ԃ��B
		FlagTmp = FlagTmp&&(ROI_SENSPACKET_NONE==SensDataNeverRcv[i]);
		if(!FlagTmp){	// Flag��false�ɂȂ����炻���ł�߂�
			break;	
		}
	}
	
	// ���茋��
	IsRcvOnceAtLeast = FlagTmp;
	
	return FlagTmp;
}

