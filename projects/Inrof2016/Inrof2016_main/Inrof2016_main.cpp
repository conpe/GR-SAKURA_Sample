/***********************************************************************/
/*                                                                     */
/*  FILE        :Main.c or Main.cpp                                    */
/*  DATE        :Tue, Oct 31, 2006                                     */
/*  DESCRIPTION :Main Program                                          */
/*  CPU TYPE    :                                                      */
/*                                                                     */
/*  NOTE:THIS IS A TYPICAL EXAMPLE.                                    */
/*                                                                     */
/***********************************************************************/
//#include "typedefine.h"
#ifdef __cplusplus
//#include <ios>                        // Remove the comment when you use ios
//_SINT ios_base::Init::init_cnt;       // Remove the comment when you use ios
#endif

void main(void);
#ifdef __cplusplus
extern "C" {
void abort(void);
}
#endif

#include <machine.h>
#include "Inrof2016_lib.h"
#include "Inrof2016_upper_ctrl.h"

uint8_t motorset;

	

void clearWDT(void);		// �E�H�b�`�h�b�O�E�^�C�}�N���A
void softwareReset(void);	// �\�t�g�E�F�A���Z�b�g


#ifdef __cplusplus
extern "C" {
#endif
void __heap_chk_fail(void){
	Sci0.print("h\r\n/****************/\r\nheap_chk_fail\r\n/****************/\r\n");
	
	Inrof.disableOutTyre();			// �^�C���o��off
	Inrof.Rmb->driveWheels(0,0);
	
	abort();
}
#ifdef __cplusplus
}
#endif


void setup()
{
	uint8_t EepTest;
	
	roi_song_packet_t SongPacket[2];
	
	for(uint16_t i=0;i<65534;i++);	// ���傢�҂�
	for(uint16_t i=0;i<65534;i++);	// ���傢�҂�
	for(uint16_t i=0;i<65534;i++);	// ���傢�҂�
	for(uint16_t i=0;i<65534;i++);	// ���傢�҂�
	for(uint16_t i=0;i<65534;i++);	// ���傢�҂�
	for(uint16_t i=0;i<65534;i++);	// ���傢�҂�
	
	// GR-SAKURA�֌W������
	GR_begin();
	
	// ���Ӌ@�평����
	Inrof.begin();
	
	// �N�����b�Z�[�W
	Sci0.print("Hello Rx63n\r\n");
	Sci0.print(" ...ServoTest\r\n");
	
	// �N�����f�f
	// �d�������ł̃��Z�b�g�H
	if(1==SYSTEM.RSTSR1.BIT.CWSF){
		Sci0.print("Warm Start\r\n");	// �d�������ȊO�ł̃��Z�b�g
	}else{
		Sci0.print("Cold Start\r\n");	// �d�������ł̃��Z�b�g
	}
	// ���Z�b�g�v���o��
	
	if(1==SYSTEM.RSTSR0.BIT.LVD0RF){	// �d���Ď�0
		Sci0.print("LVD0RF Reset\r\n");
	}
	if(1==SYSTEM.RSTSR0.BIT.LVD1RF){	// �d���Ď�1
		Sci0.print("LVD0RF Reset\r\n");
	}
	if(1==SYSTEM.RSTSR0.BIT.LVD2RF){	// �d���Ď�2
		Sci0.print("LVD0RF Reset\r\n");
	}
	if(1==SYSTEM.RSTSR2.BIT.IWDTRF){	// �Ɨ��E�H�b�`�h�b�O�^�C�}
		Sci0.print("IWDT Reset\r\n");
	}
	if(1==SYSTEM.RSTSR2.BIT.WDTRF){		// �E�H�b�`�h�b�O�^�C�}
		Sci0.print("WDT Reset\r\n");
	}
	if(1==SYSTEM.RSTSR2.BIT.SWRF){		// �\�t�g�E�F�A���Z�b�g
		Sci0.print("Software Reset\r\n");
	}
	
	
	// �^�C���쓮 �񋖉�
	//Inrof.enableOutTyre();
	Inrof.disableOutTyre();
	Inrof.Rmb->driveWheels(0,0);
	// �T�[�{�쓮 �񋖉�
	InrofUpper.ServoGrab->enableTorque(CMDSV_TORQUE_OFF);
	InrofUpper.ServoPitch->enableTorque(CMDSV_TORQUE_OFF);
	
	Inrof.Rmb->outLedDispAscii("AUTO");		// �����o�� AUTO�\��
	ExtSci.setMode(inrof2016_aplSci_t::PC_THROUGH);	// �V���A���ʐM���[�h���X���[���[�h��
	Inrof.Rmb->outLeds(0xFF);		// �S�_��
	Inrof.Rmb->outSchedulingLeds(0xFF);	// �S�_��
	
	Inrof.Rmb->outLedSpot(true);			// SPOT�{�^����L���� // PS3�R�����[�h
	Inrof.Rmb->outLedClock(true);			// Clock�{�^����L���� // �L�����u���[�V����
	
	// �N����
	SongPacket[0].NoteNum = 69;
	SongPacket[0].NoteDuration = 4;
	SongPacket[1].NoteNum = 72;
	SongPacket[1].NoteDuration = 4;
	Inrof.Rmb->outNote(0, 2, SongPacket);
	//debug
	
	// EepTest
	readMem(MEM_LAST_SHUTDOWN, &EepTest);
	Sci0.print("MEM_LAST_SHUTDOWN %d\r\n", EepTest);
	EepTest ++;
	writeMem(MEM_LAST_SHUTDOWN, &EepTest);
	
	//�ȍĐ�
	//Inrof.Rmb->playMusic(RmbSong);
}

#define DISP_MENU_NUM 2
enum disp_menu_mode{
	DISP_MENU_NORMAL 		= 0,
	DISP_MENU_INITDATAFLASH		= 1
};
const uint8_t DispMenuCurMax[DISP_MENU_NUM] = 
{
	2,
	2
};


void loop(void){
	
	static bool_t sw[2];
	
	static bool_t sw0[2];
	static bool_t sw1[2];
	static bool_t sw2[2];
	static bool_t sw3[2];
	static bool_t SwRmbClean[2];
	static bool_t SwRmbSpot[2];
	static bool_t SwRmbDock[2];
	static bool_t SwRmbClock[2];
	static bool_t SwRmbSchedule[2];
	static bool_t SwRmbDay[2];
	
	static bool_t IsBump[2];
	
	static uint8_t LoopCnt;
	
	
	// OLED���j���[
	// 0:�ʏ� [Inrof2016] [RmbOff][RmbOn]
	// 1:�f�[�^�t���b�V�������� [clearDataFlash][yes / no]
	static disp_menu_mode DispMenuMode = DISP_MENU_NORMAL;	// ���j���[
	static uint8_t DispMenuCursol = 0xFF;	// �J�[�\��	0xFF�Ń��j���[�؂�ւ�
	static uint8_t DispMenuEnter = 0;	// ���� 0:�����ĂȂ�, 1:����, 2:�L�����Z��
	
	
	
	roi_song_packet_t SongPacket[5];
	
	//position* MachinePos;
	
	
	// �X�C�b�`�ǂݎ��
	sw[1] = sw[0];
	sw[0] = (readPin(GR_SW)==0);
	sw0[1] = sw0[0];
	sw0[0] = (readPin(OLED_SW0)==0);
	sw1[1] = sw1[0];
	sw1[0] = (readPin(OLED_SW1)==0);
	sw2[1] = sw2[0];
	sw2[0] = (readPin(OLED_SW2)==0);
	sw3[1] = sw3[0];
	sw3[0] = (readPin(OLED_SW3)==0);
	
	SwRmbClean[1] = SwRmbClean[0];
	SwRmbClean[0] = Inrof.Rmb->isPushClean();
	SwRmbSpot[1] = SwRmbSpot[0];
	SwRmbSpot[0] = Inrof.Rmb->isPushSpot();
	SwRmbDock[1] = SwRmbDock[0];
	SwRmbDock[0] = Inrof.Rmb->isPushDock();
	SwRmbClock[1] = SwRmbClock[0];
	SwRmbClock[0] = Inrof.Rmb->isPushClock();
	SwRmbSchedule[1] = SwRmbSchedule[0];
	SwRmbSchedule[0] = Inrof.Rmb->isPushSchedule();
	SwRmbDay[1] = SwRmbDay[0];
	SwRmbDay[0] = Inrof.Rmb->isPushDay();
	
	IsBump[1] = IsBump[0];
	IsBump[0] = Inrof.Rmb->isBump();
//	Inrof.updateEncoder();
	
	
	//outPin(GR_LED1, sw0[0]);
	
	if(Inrof.fCnt200){
	//if(0){
		//Inrof.resetBallSensors();
					
		Inrof.fCnt200 = false;
		
		Oled.initRegister();	// �\�����ꂿ������肷��̂ŁA���W�X�^�Z�b�g������
		
		// ��2�s�̐���
		if(DispMenuEnter == 2){	// �L�����Z���{�^��
			DispMenuCursol = 0xFF;	// ���j���[��������
		}
		// ���j���[��
		Oled.setCurNewLine(0,0);
		if(DispMenuCursol == 0xFF){
			Oled.print(">");
			if(DispMenuEnter == 1){
				DispMenuCursol = 0;	// ���e�I���֑J��
				DispMenuEnter = 0;
			}
		}else{
			Oled.print(" ");
		}
		switch(DispMenuMode){
		case DISP_MENU_NORMAL:
			Oled.print("Inrof2016");
			break;
		case DISP_MENU_INITDATAFLASH:
			Oled.print("InitDataFlash");
			break;
		default:
			break;
		}
		
		
		// ���e
		Oled.setCurNewLine(1,1);
		switch(DispMenuMode){
		case DISP_MENU_NORMAL:
			if(DispMenuCursol == 0){
				Oled.print(">RmbOff /  RmbOn");
				if(DispMenuEnter == 1){	// ����
					Inrof.Rmb->setMode(ROI_MODE_POWERDOWN);	// off�ɂ���
					DispMenuCursol = 0xFF;
				}
			}else if(DispMenuCursol == 1){
				Oled.print(" RmbOff / >RmbOn");
				if(DispMenuEnter == 1){	// ����
					Inrof.Rmb->setMode(ROI_MODE_FULL);	// �t�����[�h�ɂ���
					DispMenuCursol = 0xFF;
				}
			}else{	
				Oled.print(" RmbOff /  RmbOn");
			}
			break;
		case DISP_MENU_INITDATAFLASH:
			if(DispMenuCursol == 0){
				Oled.print(">Yes /  No");
				if(DispMenuEnter == 1){	// ����
					clearAllMem();		// �f�[�^�t���b�V��������
					DispMenuCursol = 0xFF;
				}
			}else if(DispMenuCursol == 1){
				Oled.print(" Yes / >No");
				if(DispMenuEnter == 1){	// ����
					// ���������߂�
					DispMenuCursol = 0xFF;
				}
			}else{
				Oled.print(" Yes /  No");
			}
			break;
		default:
			
			break;
		}
		
		
		DispMenuEnter = 0;	// �G���^�[���N���A
		
		
		
		Oled.setCurNewLine(2,0);
		switch(ExtSci.getMode()){
		case inrof2016_aplSci_t::PC:
			Oled.print("ExtSciMode = PC");
			break;
		case inrof2016_aplSci_t::PC_THROUGH:
			Oled.print("ExtSciMode = PC_THROU");
			break;
		case inrof2016_aplSci_t::PS3:
			Oled.print("ExtSciMode = PS3");
			if(ExtSci.isPs3ConAvailable()){
				Oled.print("(OK)");
			}else{
				Oled.print("(NG)");
			}
			break;
		default:
			Oled.print("ExtSciMode = none");
			break;
		}
			
		
		Oled.setCurNewLine(3,0);
		Oled.print("Batt=%04d/%04d (%03d%%)", Inrof.Rmb->getBatteryCharge(),
							Inrof.Rmb->getBatteryCapacity(),
							Inrof.Rmb->getBatterySoc()
							);
		
		Oled.setCurNewLine(4,0);
		Oled.print("Line2=%05d,3=%05d", Inrof.getLineSensAd(2), Inrof.getLineSensAd(3));
		
		Oled.setCurNewLine(5,0);
		Oled.print("ArmEnc=0x%02X,raw%02X,%d\n", InrofUpper.ArmEnc, InrofUpper.ArmEncRaw, LoopCnt++);
		
		Oled.setCurNewLine(6,0);
		Oled.print("PR=%03d_C=%03d_L=%03d\r\n", Inrof.PsdRight->getDistance(), Inrof.PsdCenter->getDistance(), Inrof.PsdLeft->getDistance());
		
		
		// SCI0
		//uint16_t EncR,EncL;
		//Inrof.Rmb->getEncoderCounts(&EncR, &EncL);
		//Sci0.print("EncR=%05d, EncL=%05d, ReqSpdR=%05d, ReqSpdL=%05d\r\n", EncR, EncL, Inrof.Rmb->SensData.VelocityRight, Inrof.Rmb->SensData.VelocityLeft);
		
		//Sci0.print("Line0=%05d,1=%05d,2=%05d,3=%05d\r\n", Inrof.getLineSensAd(0), Inrof.getLineSensAd(1), Inrof.getLineSensAd(2), Inrof.getLineSensAd(3));
		
		//MachinePos = Inrof.getMachinePos();
		//Sci0.print("Pos.X=%04.0f, Y=%04.0f, Th=%02.2f\r\n", MachinePos->X, MachinePos->Y, MachinePos->Th);
		
	//	Sci0.print("isRmbConnect?%01d, isRcvOnceAtLeast?%01d, RmbSOC=%03d\r\n", Inrof.Rmb->isConnect(), Inrof.Rmb->isRcvOnceAtLeast(), Inrof.Rmb->getBatterySoc());
		
	//	Sci0.print("ArmEnc = 0x%02X\r\n", Inrof.ArmEnc);
	
	/*
		Sci0.print("PSD_R(%x) = %03dmm C(%x)= %03dmm L(%x) = %03dmm, ArmEnc=%d(0x%X) iscliff %d spd%f, col%d(hsv%3d)\r\n", 
				Inrof.PsdRight->isAlive(), Inrof.PsdRight->getDistance(), 
				Inrof.PsdCenter->isAlive(), Inrof.PsdCenter->getDistance(), 
				Inrof.PsdLeft->isAlive(), Inrof.PsdLeft->getDistance(), 
				InrofUpper.ArmEnc,InrofUpper.ArmEncRaw, 
				Inrof.Rmb->isCliff(), 
				Inrof.SpdTgt,
				(uint8_t)InrofUpper.ColorSens->getBallColor(),
				InrofUpper.ColorSens->getHsvH()
				);
				
	*/
		//Sci0.print("ComuStatus Rmb=0x%02x, I2c=0x%02x, Sv=0x%02x\r\n", Inrof.Rmb->isConnect(), Inrof.ComuStatusI2C, Inrof.ComuStatusSv);
	//	Sci0.print("SearvoPos G=%03d, P=%03d\r\n", Inrof.ServoGrab->getPresentPosition(), Inrof.ServoPitch->getPresentPosition());
	//	Sci0.print("SearvoCurrent G=%04d, P=%04d\r\n", Inrof.ServoGrab->getPresentCurrent(), Inrof.ServoPitch->getPresentCurrent());
		
		
	//	Sci0.print("SCI0 Line0=%05d, 1=%05d, 2=%05d, 3=%05d SongPlay %d\r\n", Inrof.getLineSensAd(0), Inrof.getLineSensAd(1), Inrof.getLineSensAd(2), Inrof.getLineSensAd(3), Rmb.isSongPlaying());
	//	Sci1.print("SCI1 Line0=%05d, 1=%05d, 2=%05d, 3=%05d\r\n", Inrof.getLineSensAd(0), Inrof.getLineSensAd(1), Inrof.getLineSensAd(2), Inrof.getLineSensAd(3));
		
		if(ExtSci.getMode() == inrof2016_aplSci_t::PS3){
			Sci0.print("SV Pitch%d, Grab%d\r\n", InrofUpper.ServoPitch->getPresentPosition(), InrofUpper.ServoGrab->getPresentPosition());
		}
	}
	
	if(sw0[0] != sw0[1]){
		if(sw0[0]){
		// �X�C�b�`0������
			
			DispMenuEnter = 1;
			
		}
	}
	if(sw1[0] != sw1[1]){
		if(sw1[0]){
			
			// ���j���[�J�[�\���ړ�
			if(DispMenuCursol==0xFF){	// ���j���[�ړ�
				if(DispMenuMode>0){
					DispMenuMode = (disp_menu_mode)((int)DispMenuMode-1);
				}else{
					DispMenuMode = (disp_menu_mode)(DISP_MENU_NUM-1);
				}
			}else if(DispMenuCursol>0){
				DispMenuCursol--;
			}else{
				DispMenuCursol = DispMenuCurMax[DispMenuMode]-1;
			}
			
		}else{
			
		}
	}
		
	if(sw2[0] != sw2[1]){
		if(sw2[0]){
			
			// ���j���[�J�[�\���ړ�
			if(DispMenuCursol==0xFF){	// ���j���[�ړ�
				if(DispMenuMode<(DISP_MENU_NUM-1)){
					DispMenuMode = (disp_menu_mode)((int)DispMenuMode+1);
				}else{
					DispMenuMode = (disp_menu_mode)0;
				}
			}else if(DispMenuCursol<DispMenuCurMax[DispMenuMode]-1){
				DispMenuCursol++;
			}else{
				DispMenuCursol = 0;
			}
			
		}else{
			
		}
		
	}
	
	if(sw3[0] != sw3[1]){
		if(sw3[0]){
			Sci0.print("SW3 pushed.\r\n");
			
			DispMenuEnter = 2;
			
		}else{
			
		}
	}
	
	
	// �����o�̃{�^��
	// Clean�{�^��
	// ����on/off
	if(SwRmbClean[0] && (!SwRmbClean[1])){	
		
		if(Inrof.isEnableOutTyre()){
			// ���싖������
			Inrof.disableOutTyre();			// �^�C���o��off
			Inrof.Rmb->driveWheels(0,0);
			
			InrofUpper.ServoGrab->enableTorque(CMDSV_TORQUE_OFF);
			InrofUpper.ServoPitch->enableTorque(CMDSV_TORQUE_OFF);
			
			// ���炷
			Inrof.Rmb->outNoteSingle(0, 69, 8);
			
			// �����o�N���Ă邩�킩��Ȃ����߁A�S������
			Inrof.Rmb->outLeds(0xFF);	// �S�_��
			Inrof.Rmb->outSchedulingLeds(0xFF);	// �S�_��
			// Clean�{�^������
			Inrof.Rmb->outLedClean(false);
			
		}else{
			// ���싖��
			Inrof.enableOutTyre();			// �^�C���o��on
			Inrof.Rmb->driveWheels(0,0);
			
			// �N���[���{�^���݂̂���
			Inrof.Rmb->outLeds(0x00);		// �S����
			Inrof.Rmb->outSchedulingLeds(0x00);	// �S����
			Inrof.Rmb->outLedClean(true);	// green
				
			InrofUpper.ServoGrab->enableTorque(CMDSV_TORQUE_ON);
			InrofUpper.ServoPitch->enableTorque(CMDSV_TORQUE_ON);
			
			if(Inrof.getMode() == INROF_DRIVE_PS3){
				
				InrofUpper.ServoGrab->setMaxTorque(80);
				InrofUpper.ServoPitch->setMaxTorque(80);
				InrofUpper.ServoGrab->setGoalTime(200);
				InrofUpper.ServoPitch->setGoalTime(200);
				
				SongPacket[0].NoteNum = 69;
				SongPacket[0].NoteDuration = 4;
				SongPacket[1].NoteNum = 72;
				SongPacket[1].NoteDuration = 4;
				Inrof.Rmb->outNote(0, 2, SongPacket);
					
			}else{
				
			InrofUpper.ServoGrab->setGoalPosition(SERVOPOS_RUN_G);
			InrofUpper.ServoPitch->setGoalPosition(SERVOPOS_RUN_P);
				switch(Inrof.getState()){
				case INROF_INITIALIZE:	// �܂����������B�ʐM�n���슮���҂�
					// ���炷
					SongPacket[0].NoteNum = 72;
					SongPacket[0].NoteDuration = 4;
					SongPacket[1].NoteNum = 72;
					SongPacket[1].NoteDuration = 4;
					SongPacket[2].NoteNum = 72;
					SongPacket[2].NoteDuration = 4;
					Inrof.Rmb->outNote(0, 3, SongPacket);
					break;
				case INROF_IDLE:	// ���������B�������ĂȂ�
							// ���Z�J�n
					SongPacket[0].NoteNum = 69;
					SongPacket[0].NoteDuration = 4;
					SongPacket[1].NoteNum = 72;
					SongPacket[1].NoteDuration = 4;
					Inrof.Rmb->outNote(0, 2, SongPacket);
					
					
					// ����J�n
					Inrof.setState(INROF_TOGOAL);
					break;
				default:
					/* no statement */
					break;
				}
			}
			
			//Inrof.Rmb->setMode(ROI_MODE_FULL);	// �t�����[�h��
		}
	}
	
	// Dock�{�^��
	if(SwRmbDock[0] && (!SwRmbDock[1])){	// �����オ��
		Sci0.print("push : Dock");
		// ���炷
		//Inrof.Rmb->outNoteSingle(0, 69, 8);
		if(Inrof.Rmb->isSongPlaying()){
			Inrof.Rmb->stopMusic();
		}else{
			//Inrof.Rmb->resumeMusic();
			Inrof.Rmb->playMusic(RmbSong);
		}	
	}
	
	
	// Clock
	if(SwRmbClock[0] && (!SwRmbClock[1])){	// �����オ��
		// ���炷
		Inrof.Rmb->outNoteSingle(0, 69, 8);
		Sci0.print("Button : Clock\r\n");
		
		// Clock���[�h�ŗL����LED��_��
		Inrof.Rmb->outLedOk(true);
		Inrof.Rmb->outLedDay(true);
		Inrof.Rmb->outLedSpot(false);
		Inrof.Rmb->outLedClock(false);
		
		//Day�{�^��
		if(SwRmbDay[0] && (!SwRmbDay[1])){
			// ���炷
			Inrof.Rmb->outNoteSingle(0, 69, 8);
			Sci0.print("Button : Day\r\n");
			//Inrof.setState(INROF_LINECALIB);	// �Z���T�L�����u���[�V����
			
			
			Inrof.FieldBall = 2;
		Inrof.Rmb->outLedDispAscii("bal2");
			
			
		}
	}else if(!SwRmbClock[0] && SwRmbClock[1]){	// ����������(Clock�������āAok�������ꂽ�H)
		// ���炷
		Inrof.Rmb->outNoteSingle(0, 69, 8);
		Sci0.print("Button : Clock->OK\r\n");
		Inrof.Rmb->outLedOk(false);
		Inrof.Rmb->outLedDay(false);
		Inrof.Rmb->outLedSpot(true);
		Inrof.Rmb->outLedClock(true);
		
		
		
		//softwareReset();
			Inrof.FieldBall = 3;
		Inrof.Rmb->outLedDispAscii("bal3");
		
	}
	
	
	// Schedule�{�^��
	if(SwRmbSchedule[0] && (!SwRmbSchedule[1])){
		
		// ���炷
		roi_song_packet_t SongPacket;
		SongPacket.NoteNum = 69;
		SongPacket.NoteDuration = 8;
		Inrof.Rmb->outNote(0, 1, &SongPacket);	
	}
	
	// SPOT�{�^��
	// ���s���[�h�؂�ւ� 
	if(SwRmbSpot[0] && (!SwRmbSpot[1])){
		
		if(INROF_DRIVE_AUTO==Inrof.getMode()){
			// PS3�R���ő��샂�[�h�ɂ���
			Inrof.setMode(INROF_DRIVE_PS3);
			Inrof.Rmb->driveWheels(0,0);
			Inrof.Rmb->outLedDispAscii("PS3");
			
			ExtSci.setMode(inrof2016_aplSci_t::PS3);
			Sci0.print("Push Roomba Spot(toPS3mode)\r\n");
			
			// ���炷
			Inrof.Rmb->outNoteSingle(0, 69, 8);
			
			//
		}else{
			Inrof.setMode(INROF_DRIVE_AUTO);
			Inrof.Rmb->outLedDispAscii("AUTO");	// �������[�h�ɂ���
			
			ExtSci.setMode(inrof2016_aplSci_t::PC_THROUGH);
			Sci0.print("Push Roomba Spot(toAutomode)\r\n");
			
			// ���炷
			SongPacket[0].NoteNum = 69;
			SongPacket[0].NoteDuration = 4;
			SongPacket[1].NoteNum = 72;
			SongPacket[1].NoteDuration = 4;
			Inrof.Rmb->outNote(0, 2, SongPacket);
			
			// ��ԃ��Z�b�g
			
			Inrof.initialize();
			InrofUpper.initialize();
			Inrof.setState(INROF_IDLE);
			
		}
	}
	
	
	
	
	
	
	// ������s
	Inrof.step();
	
	// �E�H�b�`�h�b�O�E�^�C�}�N���A
	clearWDT();
}

// �E�H�b�`�h�b�O�E�^�C�}�N���A
// WDT�͂͂��߂ɃN���A�����s�����Ƃ����瓮���n�߂�
void clearWDT(void){
	IWDT.IWDTRR = 0x00;
	IWDT.IWDTRR = 0xFF;
	WDT.WDTRR = 0x00;
	WDT.WDTRR = 0xFF;
}
	
void softwareReset(void){
	SYSTEM.PRCR.WORD = 0xA502;  // PRKEY=0xA5, PRC1=1
	SYSTEM.SWRR = 0xA501;
}
	
void setupRx(void);
void main(void)
{
	setupRx();
	setup();
	
	while(1){
		loop();
	}
}

void setupRx(void){
	volatile int i;
	
    // Protection off
    SYSTEM.PRCR.WORD = 0xA503u;

    // Stop sub-clock
    SYSTEM.SOSCCR.BYTE = 0x01u;

    // Set main oscillator settling time to 10ms (131072 cycles @ 12MHz)
    SYSTEM.MOSCWTCR.BYTE = 0x0Du;

    // Set PLL circuit settling time to 10ms (2097152 cycles @ 192MHz)
    SYSTEM.PLLWTCR.BYTE = 0x0Eu;
	for( i=0 ; i<636  ; i++ )					// 20.1053ms/143.75kHz/5cyc=635.527
		nop( );

    // Set PLL circuit to x16
    SYSTEM.PLLCR.WORD = 0x0F00u;

    // Start the external 12Mhz oscillator
    SYSTEM.MOSCCR.BYTE = 0x00u;

    // Turn on the PLL
    SYSTEM.PLLCR2.BYTE = 0x00u;

    // Wait over 12ms (~2075op/s @ 125KHz)
    for(volatile uint16_t i = 0; i < 2075u; i++)
    {
        nop();
    }

    // Configure the clocks as follows -
    //Clock Description              Frequency
    //----------------------------------------
    //PLL Clock frequency...............192MHz
    //System Clock Frequency.............96MHz
    //Peripheral Module Clock B..........48MHz
    //FlashIF Clock......................48MHz
    //External Bus Clock.................48MHz
    SYSTEM.SCKCR.LONG = 0x21021211u;
	while( SYSTEM.SCKCR.LONG != 0x21021211 )  ;	// Wait Finish

    // Configure the clocks as follows -
    //Clock Description              Frequency
    //----------------------------------------
    //USB Clock..........................48MHz
    //IEBus Clock........................24MHz
    SYSTEM.SCKCR2.WORD = 0x0033u;
	while( SYSTEM.SCKCR2.WORD != 0x0033 )  ;	// Wait Finish

    // Set the clock source to PLL
    SYSTEM.SCKCR3.WORD = 0x0400u;
	while( SYSTEM.SCKCR3.WORD != 0x0400 )  ;	// Wait Finish

    // Stop external bus
    SYSTEM.SYSCR0.WORD  = 0x5A01;

    // Protection on
    SYSTEM.PRCR.WORD = 0xA500u;

}


#ifdef __cplusplus
void abort(void)
{
	Sci0.print("abort\r\n");
	while(1){
		clearWDT();	
	}
}
#endif
