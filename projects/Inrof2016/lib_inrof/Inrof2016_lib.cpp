
#include "Inrof2016_lib.h"

inrof2016 Inrof;
inrof2016_aplSci_t ExtSci(&Sci0);	// SBDBT��Ƃ̒ʐM(PC, PS3�R��)


// �����o
// SCI2
RoombaOi Rmb = RoombaOi();

inrof2016::inrof2016(void){
	
}


void inrof2016::begin(void){
	uint16_t i;
	
	GrEx.begin();		// �g���������
	GrExOled.begin();	// �g�����OLED������

// �O���ʐM
	Sci0.begin(115200, true, true, 1024, 128);	// PC use Tx, Rx
	//Sci1.begin(115200, true, true, 255, 128);	// �f�o�b�K use Tx, Rx
	
// �I�u�W�F�N�g�p��
	// �����o
	Rmb = &::Rmb;
	Rmb->begin(&Sci2);
		
	// �G���R�[�_
	EncR = GrEx.Enc1;
	EncL = GrEx.Enc0;
	
	// MPU6050
	Mpu = new invensense_MPU(&I2C0, IMU_I2C_ADDRESS);
	
	// �����Z���T
	beginBallSensor();
	
//������	
	
	// IMU
	ComuStatusI2C |= Mpu->setup();
	
	// ���C���Z���T
	setPinMode(LINESENS_PORT_LED, PIN_OUTPUT);	// LED���点��|�[�g
	// AD�ϊ��J�n
	GrEx.startAD();
		
	
	// �����o������
	Inrof.Rmb->setMode(ROI_MODE_FULL);	// �t�����[�h�ɂ���
	// �����o���[�h�ύX�҂�
	for(i=0;i<65534;i++);	// ���傢�҂�
	for(i=0;i<65534;i++);	// ���傢�҂�
	// ��M�������f�[�^�ݒ�
	ROI_SENSPACKET_ID SensDataReq[] = {
				ROI_SENSPACKET_BUMPS_WHEELDROPS,
				ROI_SENSPACKET_BATTERY_CHARGE,
				ROI_SENSPACKET_BATTERY_CAPACITY,
				ROI_SENSPACKET_ENCODER_COUNTS_LEFT,
				ROI_SENSPACKET_ENCODER_COUNTS_RIGHT,
				ROI_SENSPACKET_BUTTONS,
				ROI_SENSPACKET_CLIFF_LEFT,
				ROI_SENSPACKET_CLIFF_FRONT_LEFT,
				ROI_SENSPACKET_CLIFF_FRONT_RIGHT,
				ROI_SENSPACKET_CLIFF_RIGHT,
				ROI_SENSPACKET_CLIFF_LEFT_SIGNAL,
				ROI_SENSPACKET_CLIFF_FRONT_LEFT_SIGNAL,
				ROI_SENSPACKET_CLIFF_FRONT_RIGHT_SIGNAL,
				ROI_SENSPACKET_CLIFF_RIGHT_SIGNAL,
				ROI_SENSPACKET_SONG_PLAYING
	};
	Inrof.Rmb->setRcvStream(sizeof(SensDataReq) / sizeof(SensDataReq[0]), SensDataReq);
	
	
	
	// �㕔������
	InrofUpper.begin();
	
	// �t���O�n���Z�b�g
	initialize();
	
	
	setup1msInterrupt();	//1ms���Ɗ��荞�ݐݒ�
}

void inrof2016::initialize(void){
	float tmp[3];
	
	// �f�[�^�t���b�V��������
	initMem();
	
	// ����p
	// �^�C��
	//PidCtrlRTire = new ctrl_pid(SAMPLETIME_CTRL_S, 10.0F, 0.0F, 0.1F, -255,255, -1000, 1000);	// ���x->�f���[�e�B�[
	//PidCtrlLTire = new ctrl_pid(SAMPLETIME_CTRL_S, 10.0F, 0.0F, 0.1F, -255,255, -1000, 1000);
	if(PidCtrlRTire){	delete PidCtrlRTire; }
	PidCtrlRTire = new ctrl_pid(SAMPLETIME_CTRL_S, 20.0F, 0.0F, 1.0F, -255,255, -500, 500);	// ���x->�f���[�e�B�[
	if(PidCtrlLTire){	delete PidCtrlLTire; }
	PidCtrlLTire = new ctrl_pid(SAMPLETIME_CTRL_S, 20.0F, 0.0F, 1.0F, -255,255, -500, 500);
	// ���x���[�g���~�b�^
	if(RateLimitSpdTgt){	delete RateLimitSpdTgt; }
	RateLimitSpdTgt = new ctrl_ratelimiter_abs(SAMPLETIME_CTRL_S, 400.0F, 1000.0F);	// 500 mm/s/s
	
	if(RateLimitAngVelTgt){ delete RateLimitAngVelTgt; }
	RateLimitAngVelTgt = new ctrl_ratelimiter_abs(SAMPLETIME_CTRL_S, 3.0F, 3.0F);		// 0.3 rad/s/s	
	
	// ���C���g���[�X
	readMem(MEM_LINETRACE_PID_KP, &tmp[0]);
	readMem(MEM_LINETRACE_PID_KI, &tmp[1]);
	readMem(MEM_LINETRACE_PID_KD, &tmp[2]);
	
	if(PidCtrlLineTrace){ delete PidCtrlLineTrace; }
	PidCtrlLineTrace = new ctrl_pid(SAMPLETIME_CTRL_S, 
					tmp[0], 
					tmp[1], 
					tmp[2], 
					-RMB_ANGVEL_MAX,RMB_ANGVEL_MAX, -RMB_ANGVEL_MAX/10.0F, RMB_ANGVEL_MAX/10.0F);
	
			
	// �{�[���Z���T���W���[��������
	initBallSensor();
	
	// �㕔������
	InrofUpper.initialize();
	
	// �t���O�n���Z�b�g
	// ����͑S�^�X�N���s����
	fExeReqTask0 = true;
	fExeReqTask1 = true;
	fExeReqTask2 = true;
	
	fBattEmpty = false;
	fCnt200 = false;
	
	fResetEncoder = true;
	
	SpdTgt = 0;
	AngVelTgt = 0;
	RTireDutyTgt = 0;
	LTireDutyTgt = 0;
	
	
	// �ԑ̏����ʒu
	MachinePos.X = 0.0;
	MachinePos.Y = -200.0;
	MachinePos.dX = 0.0;
	MachinePos.dY = 0.0;
	MachinePos.Th = PI/2.0;
	MachinePos.dTh = 0.0;
	
	// �O�Ɍ����_���������ꏊ
	LastFindCrossPos.X = 0.0;
	LastFindCrossPos.Y = -200.0;
	Cross = INROF_CROSS_NONE;
	
	
	CalcOdmetry.REncCntDiff[0] = 0.0;
	CalcOdmetry.LEncCntDiff[0] = 0.0;
	CalcOdmetry.CosTh[0] = 0.0;
	CalcOdmetry.SinTh[0] = 1.0;
	CalcOdmetry.dth2[0] = 0.0;
	CalcOdmetry.REncCntDiff[1] = 0.0;
	CalcOdmetry.LEncCntDiff[1] = 0.0;
	CalcOdmetry.CosTh[1] = 0.0;
	CalcOdmetry.SinTh[1] = 1.0;
	CalcOdmetry.dth2[1] = 0.0;
	
	// �t�B�[���h
	PosCrossStart.X = FIELD_INROF_CROSS_START_X;
	PosCrossStart.Y = FIELD_INROF_CROSS_START_Y;
	PosCrossRed.X = FIELD_INROF_CROSS_RED_X;
	PosCrossRed.Y = FIELD_INROF_CROSS_RED_Y;
	PosCrossYellow.X = FIELD_INROF_CROSS_YELLOW_X;
	PosCrossYellow.Y = FIELD_INROF_CROSS_YELLOW_Y;
	PosCrossBlue.X = FIELD_INROF_CROSS_BLUE_X;
	PosCrossBlue.Y = FIELD_INROF_CROSS_BLUE_Y;
	PosCrossBallFieldBegin.X = FIELD_INROF_CROSS_BALLFIELD_BEGIN_X;
	PosCrossBallFieldBegin.Y = FIELD_INROF_CROSS_BALLFIELD_BEGIN_Y;
	PosCrossBallFieldEnd.X = FIELD_INROF_CROSS_BALLFIELD_END_X;
	PosCrossBallFieldEnd.Y = FIELD_INROF_CROSS_BALLFIELD_END_Y;
	PosCrossGoalRed.X = FIELD_INROF_CROSS_GOAL_RED_X;
	PosCrossGoalRed.Y = FIELD_INROF_CROSS_GOAL_RED_Y;
	PosCrossGoalYellow.X = FIELD_INROF_CROSS_GOAL_YELLOW_X;
	PosCrossGoalYellow.Y = FIELD_INROF_CROSS_GOAL_YELLOW_Y;
	PosCrossGoalBlue.X = FIELD_INROF_CROSS_GOAL_BLUE_X;
	PosCrossGoalBlue.Y = FIELD_INROF_CROSS_GOAL_BLUE_Y;
	
	// �̂���{�[����
	FieldBall = FIELD_BALL_NUM_INIT;
	
	
	State = INROF_INITIALIZE;
	ActionState = INROF_ACTION_IDLE;
	
	DriveMode = INROF_DRIVE_AUTO;
	
	fFinishManageAction = false;
	fFinishAction = false;
	
	fAdjustMachinePos = true;
	
	
	Goal = INROF_GOAL_YELLOW;
	
	Time_msec = 0;
}

void inrof2016::beginBallSensor(void){
	//uint16_t cnt;
	
	PsdCenter = new inrof2016_st_vl_t(	&I2C0,		PSDSENS_FRONT_ADRS,	PSDSENS_FRONT_ENABLEPIN);
	PsdRight = new inrof2016_st_vl_t(	&I2C0,		PSDSENS_RIGHT_ADRS,	PSDSENS_RIGHT_ENABLEPIN);
	PsdLeft = new inrof2016_st_vl_t(	&I2C0,		PSDSENS_LEFT_ADRS,	PSDSENS_LEFT_ENABLEPIN);
	
	//for(cnt=0;cnt<65500;cnt++);	// ������Ƃ܂�
	// I2C�A�h���X�̐ݒ肪����̂ŁA�ЂƂ܂�������
	PsdLeft->disableModule();
	PsdCenter->disableModule();
	PsdRight->disableModule();
	
}

void inrof2016::initBallSensor(void){
	uint16_t cnt;
	
	// I2C�A�h���X�̐ݒ肪����̂ŁA�ЂƂ܂�������
	PsdLeft->disableModule();
	PsdCenter->disableModule();
	PsdRight->disableModule();
	
	
	// �Z���T�ʒu�Z�b�g
	PsdLeft->setSensPos(	PSDSENS_LEFT_POS_X,	PSDSENS_LEFT_POS_Y,	PSDSENS_LEFT_POS_TH);
	PsdCenter->setSensPos(	PSDSENS_FRONT_POS_X,	PSDSENS_FRONT_POS_Y,	PSDSENS_FRONT_POS_TH);
	PsdRight->setSensPos(	PSDSENS_RIGHT_POS_X,	PSDSENS_RIGHT_POS_Y,	PSDSENS_RIGHT_POS_TH);
	
	// ���ɋN������
	// �Z���T����J�n(������I2C�A�h���X�ݒ�)
	PsdLeft->enableModule();		// ���W���[�����Z�b�g����
	for(cnt=0;cnt<65500;cnt++);		// �N����̂�҂�
	PsdLeft->begin();			// �V����I2C�A�h���X���Z�b�g���ē���J�n
	for(cnt=0;cnt<65500;cnt++);		// �ݒ芮����҂�
	//while(!PsdLeft->isIdleComu());	// �ݒ芮����҂�
	PsdCenter->enableModule();
	for(cnt=0;cnt<65500;cnt++);		// �N����̂�҂�
	PsdCenter->begin();
	for(cnt=0;cnt<65500;cnt++);		// �ݒ芮����҂�
	//while(!PsdCenter->isIdleComu());	// �ݒ芮����҂�
	PsdRight->enableModule();		// �N����̂�҂�
	for(cnt=0;cnt<65500;cnt++);
	PsdRight->begin();
	for(cnt=0;cnt<65500;cnt++);		// �ݒ芮����҂�
	//while(!PsdRight->isIdleComu());	// �ݒ芮����҂�
	
	
}





void inrof2016::step(void){
	if(fExeReqTask0){
		stepTask0();
		fExeReqTask0 = false;
	}
	if(fExeReqTask1){
		stepTask1();
		fExeReqTask1 = false;
	}
	if(fExeReqTask2){	// 10ms�̂���ɂ��Ȃ��ƁA�J���[�Z���T��I2C�ł��Ȃ�(new���s����)�H�H�H
		stepTask2();
		fExeReqTask2 = false;
	}
}

// 1ms���ƂɎ��s����֐�
void inrof2016::isr1ms(void){
	GR_cnt();
	Time_msec++;
	
	fExeReqTask0 = true;	// 1ms����
	
	if(CntTask1<19){		// 20ms����
		CntTask1 ++;
	}else{
		CntTask1 = 0;
		fExeReqTask1 = true;
	}
	
	if(CntTask2<199){	// 200ms����
		CntTask2++;
	}else{
		CntTask2 = 0;
		fExeReqTask2 = true;
	}
	
}

// 1ms����
void inrof2016::stepTask0(void){
	
	// test
	// 1s���ƂɃ`�J�`�J
	static uint16_t cnt;
	static bool_t f;
	if(1000 < cnt++){
		if(f){
			outPin(GR_LED0, 0);
			f = 0;
		}else{
			outPin(GR_LED0, 1);
			f = 1;
		}
		cnt = 0;
	}
	
	
	static uint8_t Col=0;
	if(!fBattEmpty){
		if(0==cnt%300){
			Col+=10;
			Inrof.Rmb->outLedBattColor(Col,0xFF);
		}
	}
}

//20ms����
void inrof2016::stepTask1(void){
	
	// EEPROM��������
	drvDataFlash_WriteTask();
	
	// �����omusic
	Inrof.Rmb->playMusicTask();
	
	InrofUpper.task0();	// �㕔����^�X�N
	
	
	// PC�EPS3�R���ʐM����
	ExtSci.task();
		
	/***********
	 ���͏���
	************/
	// ���C���Z���T�X�V
	updateLineSense();

	// ���擾�v��
	//Mpu->measure();
	
	if(checkInitialize()){	// �������`�F�b�N(�����o�ʐM�Ȃ�))// �������I������� State��INITIALIZE����ɂ���B
	//if(1){	// �������`�F�b�N// �������I������� State��INITIALIZE����ɂ���B
		
		
		// �����o���p�[����������~�܂�
		//Rmb->driveWheels(250*(!Rmb->isBumpRight()),250*(!Rmb->isBumpLeft()));
		
		/***********
		 ���͏���
		************/
		// �G���R�[�_�X�V
		updateEncoder();
		
		/***********
		 ����
		************/
		// �^�C����ԍX�V
		updateTyre();
		
		// �I�h���g���X�V
		//adjustMachinePos();	// �␳ <- ���C���g���[�X���Ŏ��{
		updateOdmetry();
		
		
		// ���C���Z���T��_����
		IsCrossing = judgeCross();
		// ���s��������
		judgeRunOrient();
		// �{�[������������
		IsBallFinding = judgeBallFind();
		
		
		// ��ԊǗ�
		// ����Ǘ�
		// ���� (SpdTgt��AngVelTgt�����肷��)
		manageState();
		
		/***********
		 �o��
		************/
		
		// ���C���Z���T�f��
	//	SpdTgt = 100;
	//	AngVelTgt = 0.5*(min(getLineSensAd(2), getLineSensAd(3)) - min(getLineSensAd(0),getLineSensAd(1)));
		
		
		SpdTgtRated = RateLimitSpdTgt->limitRate(SpdTgt);
		AngVelTgtRated = RateLimitAngVelTgt->limitRate(AngVelTgt);
		
		// ���x����ŖڕW���x�𖞂����f���[�e�B�[��v�Z
		ctrlRun(SpdTgtRated, AngVelTgtRated, &RTireDutyTgt, &LTireDutyTgt);	// ���x����
		
		// �f���[�e�B�[�o��
		// �o�b�e���Ȃ��Ƃ��͏o�͂��Ȃ�
		if(!fBattEmpty && !I2C0.isError()){	// �o�b�e���オ���ĂȂ����
							// ���AI2C����Ȃ��
			// �^�C��
			if(INROF_DRIVE_PS3 == DriveMode){	// PS3���[�h
				drivePs3Con();
			}else{	
				outTyres(RTireDutyTgt, LTireDutyTgt, fMotorOut);		// �f���[�e�B�[�o��
				//outTyres(SpdTgt+AngVelTgt, SpdTgt-AngVelTgt, fMotorOut);	// ���x�o��
			}
			
		}else{
			//fMotorOut = false;
			disableOutTyre();
			outTyres(0, 0, fMotorOut);	//���[�^�[off
		}
	}else{
	}
	
	
	
	// �Z���T�X�V�v��
	ComuStatusI2C |= PsdLeft->measure();
	ComuStatusI2C |= PsdCenter->measure();
	ComuStatusI2C |= PsdRight->measure();
	PsdLeft->checkAlive();
	PsdCenter->checkAlive();
	PsdRight->checkAlive();
	
	
	//Sci0.print("ToFAt = %d\r\n", PsdCenter->measure());
	
}
	
// 200ms����
void inrof2016::stepTask2(void){
	
	fCnt200 = true;
	
	InrofUpper.task1();	// �㕔
		
	
	
	// �o�b�e�����X�V
//	getBattVoltage();

// �o�b�e���[�`�F�b�N
	bool_t fBattEmptyLast = fBattEmpty;
	fBattEmpty = isBattLow();
	if(!fBattEmptyLast){	// ����܂ŋ󂶂�Ȃ�����
		if(fBattEmpty){
			// �����ɂȂ������肳�ꂽ
			Rmb->outLedCheckRobot(true);	// �`�F�b�N�����v�_��
			Rmb->outLedBattColor(0xFF, 0xFF);	// �ԁA���邳max
		}
	}else{	// ����܂ŋ󂾂���
		if(!fBattEmpty){	// ����͌��C����
			// ���񌳋C�ɂȂ������肳�ꂽ
			Rmb->outLedCheckRobot(false);	// �`�F�b�N�����v����
			Rmb->outLedBattColor(0x00, 0x00);	// �΁A���邳min
		}
	}
}


/*********************
���[�h�ύX
�T�v�F
 
�����F

**********************/
void inrof2016::setMode(inrof_drive_mode NewMode){
	
	if(NewMode != DriveMode){	// �قȂ郂�[�h��������
		switch(NewMode){
		case INROF_DRIVE_AUTO:
			
			break;
		case INROF_DRIVE_PS3:
			// ���̈ʒu��ڕW�ʒu�ɂ���
			GrabAngleTgt = InrofUpper.ServoGrab->getPresentPosition();
			PitchAngleTgt = InrofUpper.ServoPitch->getPresentPosition();
			break;
		}
	}
	
	DriveMode = NewMode;
	
}
	
/*********************
���C���Z���T�A�b�v�f�[�g
�T�v�F
 �G���R�[�_�l���A�b�v�f�[�g
�����F

**********************/
void inrof2016::updateLineSense(void){
	static bool_t fLineSensOn;				// LED���点�Ă�t���O
	static uint16_t LineSensAdOn[LINESENS_NUM_SENSE];	// ���C���Z���TAD�l LED on�� 10bit
	static uint16_t LineSensAdOff[LINESENS_NUM_SENSE];	// ���C���Z���TAD�l LED off�� 10bit
	
	
	if(fLineSensOn){	// on������
		for(uint8_t i=0; i<LINESENS_NUM_SENSE; i++){
			LineSensAdOn[i] = GrEx.getAD10((grex_ad_ch)i);	// 10bit
		}
		// LED����
		outPin(LINESENS_PORT_LED, 0); 
		fLineSensOn = 0;
	}else{
		for(uint8_t i=0; i<LINESENS_NUM_SENSE; i++){
			LineSensAdOff[i] = GrEx.getAD10((grex_ad_ch)i);	// 10bit
		}
		
		// LED���炷
		outPin(LINESENS_PORT_LED, 1);
		fLineSensOn = 1;
		
	}
	
	// off��-on���ŃZ���T�l�Ƃ���
	for(uint8_t i=0; i<LINESENS_NUM_SENSE; i++){
		LineSensAd[(grex_ad_ch)i] = LineSensAdOff[(grex_ad_ch)i] - LineSensAdOn[(grex_ad_ch)i];
	}
	
}
/*********************
�G���R�[�_�A�b�v�f�[�g
�T�v�F
 �G���R�[�_�J�E���g�̑O��Ƃ̍����v�Z
�����F

**********************/
void inrof2016::updateEncoder(void){
	uint16_t REncCntLast, LEncCntLast;
	
	REncCntLast = this->REncCnt;
	LEncCntLast = this->LEncCnt;
	
	// �����o����G���R�[�_�l�擾
	Rmb->getEncoderCounts(&this->LEncCnt, &this->REncCnt);	// �����o���猩����t����
	
	if(fResetEncoder){	// ����͓����̓����B�����[���ɂ��邽�߁B
		REncCntLast = this->REncCnt;
		LEncCntLast = this->LEncCnt;
		fResetEncoder = false;
	}
	
	// �����v�Z
	REncCntDiff = REncCntLast - REncCnt;	// �����o���猩����t����
	LEncCntDiff = LEncCntLast - LEncCnt;	
	
	// ���I�[�o�[�t���[�Ƃ��ł��������Ɍv�Z�����
	// int16_t = uint16_t - uint16_t
	// int16_t�Ōv�Z�ł���l���傫�������ł���t�����ɉ�]�����Ɣ��f�������ʂɂȂ�B
}

/*********************
�^�C����ԃA�b�v�f�[�g
�T�v�F
 �G���R�[�_�l����p�x�Ɗp���x���A�b�v�f�[�g
�����F

**********************/
void inrof2016::updateTyre(void){
	
	// �^�C���p���x[rps]
	RTireSpd_rps = (float)REncCntDiff/ENC_CPR/GEAR_RATIO/SAMPLETIME_CTRL_S;
	LTireSpd_rps = (float)LEncCntDiff/ENC_CPR/GEAR_RATIO/SAMPLETIME_CTRL_S;
	
	// �^�C���p�x[rev]
	RTirePos_r = RTirePos_r + (float)REncCntDiff/ENC_CPR/GEAR_RATIO;
	LTirePos_r = LTirePos_r + (float)LEncCntDiff/ENC_CPR/GEAR_RATIO;
	
}
	
/*********************
���Ȉʒu�X�V
**********************/
void inrof2016::updateOdmetry(void){
	CalcOdmetry.REncCntDiff[1] = CalcOdmetry.REncCntDiff[0];
	CalcOdmetry.REncCntDiff[0] = REncCntDiff;
	CalcOdmetry.LEncCntDiff[1] = CalcOdmetry.LEncCntDiff[0];
	CalcOdmetry.LEncCntDiff[0] = LEncCntDiff;
	
	// �}�V���p�x
	float ThDiff = (CalcOdmetry.REncCntDiff[0] - CalcOdmetry.LEncCntDiff[0])*TIRE_DIA_MM*PI/TIRE_TREAD_MM/ENC_CPR/GEAR_RATIO;		//[rad]
	MachinePos.dTh = ThDiff / SAMPLETIME_CTRL_S;	// [rad/s]
	MachinePos.Th = MachinePos.Th + ThDiff;			// [rad]
	
	CalcOdmetry.CosTh[1] = CalcOdmetry.CosTh[0];
	CalcOdmetry.SinTh[1] = CalcOdmetry.SinTh[0];
	CalcOdmetry.CosTh[0] = cosf(MachinePos.Th);
	CalcOdmetry.SinTh[0] = sinf(MachinePos.Th);
	
	CalcOdmetry.dth2[1] = CalcOdmetry.dth2[0];
	CalcOdmetry.dth2[0] = (CalcOdmetry.REncCntDiff[0]+CalcOdmetry.REncCntDiff[1]+CalcOdmetry.LEncCntDiff[0]+CalcOdmetry.LEncCntDiff[1])*PI/ENC_CPR/GEAR_RATIO;
	
	// �}�V�����x(�t�B�[���h���W)
	float XDiff = (CalcOdmetry.dth2[0] * CalcOdmetry.CosTh[0] + CalcOdmetry.dth2[1] * CalcOdmetry.CosTh[1]) * TIRE_DIA_MM / 8;
	float YDiff = (CalcOdmetry.dth2[0] * CalcOdmetry.SinTh[0] + CalcOdmetry.dth2[1] * CalcOdmetry.SinTh[1]) * TIRE_DIA_MM / 8;		// [mm]
	MachinePos.dX = XDiff / SAMPLETIME_CTRL_S;	
	MachinePos.dY = YDiff / SAMPLETIME_CTRL_S;	// [mm/s]
	// �}�V���ʒu
	MachinePos.X = MachinePos.X + XDiff;
	MachinePos.Y = MachinePos.Y + YDiff;		// [mm]
	
	// �}�V�����x
	this->Spd = (CalcOdmetry.REncCntDiff[0] + CalcOdmetry.LEncCntDiff[0] + CalcOdmetry.REncCntDiff[1] + CalcOdmetry.LEncCntDiff[1])/2/2/ENC_CPR/GEAR_RATIO*(TIRE_DIA_MM*PI)/SAMPLETIME_CTRL_S;
	// �}�V������
	float NormalizedTh = MachinePos.getNormalizeTh();
	if((NormalizedTh < PI/4)&&(NormalizedTh >= -PI/4))	this->MachineOrientation = ORIENT_EAST;
	else if((NormalizedTh < PI*3/4)&&(NormalizedTh >= PI/4))	this->MachineOrientation = ORIENT_NORTH;
	else if((NormalizedTh >= PI*3/4)||(NormalizedTh < -PI*3/4))	this->MachineOrientation = ORIENT_WEST;
	else if((NormalizedTh < -PI/4)&&(NormalizedTh >= -PI*3/4))	this->MachineOrientation = ORIENT_SOUTH;
}


/*********************
��_����
�T�v�F��_�ɂ��邱�Ƃ𔻒肷��
�����F
�ߒl�F��_�ɂ��邩
**********************/
bool_t inrof2016::onCross(void){
	return ((getLineSensAd(0)<LINESENS_THRESHOLD_SIDE) && (getLineSensAd(3)<LINESENS_THRESHOLD_SIDE));
}


/*********************
��_����
�T�v�F��_�ɗ������Ƃ𔻒肷��
		�V������_�ɗ������Ɍ�_�C���f�b�N�X���X�V���Atrue��Ԃ�
		����ȊO�ł�false��Ԃ��B
�����F
�ߒl�F�V���������_�ɗ���
**********************/
bool_t inrof2016::judgeCross(void){
	bool_t IsNewCross = false;
	position LineSensPos;
	
	// �����_����
	if(onCross()){	// ���[���C���G���Ă�
		
		if(100.0<(MachinePos - LastFindCrossPos)){	// �O���������100mm�ȏ㉓���Ƃ��낾������(�A������h�~)
			LastFindCrossPos = MachinePos;	// �����������ꏊ�o���Ă���
			IsNewCross = true;
			//�ǂ��̌����_�����H
			getLineSenseGlobalPos(&LineSensPos);	// ���C���Z���T�̃O���[�o���Ȉʒu���擾
			if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossStart)){		// ���C���Z���T�ƁA�t�B�[���h��̌����_�̈ʒu���߂����
				Cross = INROF_CROSS_START;

#ifdef DEBUGSCI0
Sci0.print("judgeCross start\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossRed)){
				Cross = INROF_CROSS_RED;

#ifdef DEBUGSCI0
Sci0.print("judgeCross red\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossYellow)){
				Cross = INROF_CROSS_YELLOW;
				
#ifdef DEBUGSCI0
Sci0.print("judgeCross yellow\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR>(LineSensPos - PosCrossBlue)){
				Cross = INROF_CROSS_BLUE;

#ifdef DEBUGSCI0
Sci0.print("judgeCross blue\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR>(LineSensPos - PosCrossBallFieldBegin)){
				Cross = INROF_CROSS_BALLFIELD_BEGIN;
#ifdef DEBUGSCI0
Sci0.print("judgeCross ballbegin\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR>(LineSensPos - PosCrossBallFieldEnd)){
				Cross = INROF_CROSS_BALLFIELD_END;
#ifdef DEBUGSCI0
Sci0.print("judgeCross ballend\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossGoalRed)){
				Cross = INROF_CROSS_GOAL_RED;
#ifdef DEBUGSCI0
Sci0.print("judgeCross goalred\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossGoalYellow)){
				Cross = INROF_CROSS_GOAL_YELLOW;
#ifdef DEBUGSCI0
Sci0.print("judgeCross goalyellow\r\n");
#endif
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossGoalBlue)){
				Cross = INROF_CROSS_GOAL_BLUE;
#ifdef DEBUGSCI0
Sci0.print("judgeCross goalblue\r\n");
#endif
			}else{
				// �ǂ��Ƃ��߂��Ȃ��̂Ƀ��C����H�H�H�H
				Cross = INROF_CROSS_NONE;
#ifdef DEBUGSCI0
Sci0.print("st mX%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, PosCrossYellow.X, PosCrossYellow.Y, PosCrossYellow.Th);							
Sci0.print("judgeCross dokodoko??\r\n");
#endif
			}
		}
			
		
	}else{
		
	}
	
	return IsNewCross;
}


/*********************
�{�[������������
�T�v�F	�{�[��������������1�ɂȂ�B
		�����F���C���g���[�X���A�{�[���T���t�B�[���h��A���E�Z���T���ǂ��߂��l�A���E�΂߃Z���T���}�V���Ԃ��邭�炢�߂��l
�����F
�ߒl�F�V�����{�[����������
**********************/
bool_t inrof2016::judgeBallFind(void){
	bool_t BallExist_last = this->BallExist;
	
	this->BallExist = false;	// ���Z�b�g
	
	//if((INROF_CROSS_BALLFIELD_BEGIN == Cross) || (INROF_CROSS_BALLFIELD_END == Cross)){	// �{�[���]�[����
	//if( (1500.0 > MachinePos.Y) && ( (PI/6 > fabs(MachinePos.getNormalizeTh()-PI/2))||(PI/6 > fabs(MachinePos.getNormalizeTh()+PI/2) ) ) ){		// �{�[���]�[����, �قڏ㉺�����Ă���
	//if( 600.0 < MachinePos.X ){		// �{�[���]�[����
						// �ςȂƂ��ɓ]�����Ă����p�ɑS��ɂ������B
		
		// ���N���X�Z���T
		
		if( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdLeft->getDistance())		// �w�肵���������߂�
			&& (PSD_MIN < PsdLeft->getDistance())				// �L���͈�
			&& PsdLeft->isObjectBallArea(&MachinePos)			// �{�[���̓t�B�[���h��(�Z���T�l�擾�ł��ĂȂ����false�Ԃ�)
		){
			this->BallExist = true;		
		}
		// �E�N���X�Z���T
		if( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdRight->getDistance())		// �w�肵���������߂�
			&& (PSD_MIN < PsdRight->getDistance())				// �L���͈�
			&& PsdRight->isObjectBallArea(&MachinePos)			// �{�[���̓t�B�[���h��(�Z���T�l�擾�ł��ĂȂ����false�Ԃ�)
		){
			this->BallExist = true;		
		}
		
		// ���N�͒����͎g���ĂȂ������B
		// �����Z���T
		if( (BALLFIND_MAXDISTANCE_FRONT > PsdCenter->getDistance())		// �w�肵���������߂�
			&& (PSD_MIN < PsdCenter->getDistance())				// �L���͈�
			&& PsdCenter->isObjectBallArea(&MachinePos)			// �{�[���̓t�B�[���h��(�Z���T�l�擾�ł��ĂȂ����false�Ԃ�) // �X�^�[�g�]�[���œ���m�F���������ߖ���
		){
			this->BallExist = true;		
		}
		
	//}
		//}
		
	if(BallExist_last != this->BallExist){	// ���ʕω���
		this->Rmb->outLedDustFull(this->BallExist);	// �{�[������Ƃ��̓����o�̃S�~�}�[�N����
	}
	
	this->Rmb->outLedDebris(this->BallFindEnable);	// �T���L����
		
	if(BallFindEnable){	// enable�̂Ƃ��͌��ʂ��̂܂ܕԂ�
		return BallExist;
	}
	
	return false;
}
	
	
/*********************
�{�[���Z���T�����Z�b�g
�T�v�F	ToF�̃Z���T���ł܂����Ⴄ��Ń��Z�b�g�|����
�����F
�ߒl�F
**********************/
void inrof2016::resetBallSensors(void){
//	Inrof.PsdLeft->resetModule();
//	Inrof.PsdCenter->resetModule();
//	Inrof.PsdRight->resetModule();
	// �Ȃ���2�񑗂�Ȃ��ƍĊJ���Ȃ�
//	Inrof.PsdLeft->resetModule();
//	Inrof.PsdCenter->resetModule();
//	Inrof.PsdRight->resetModule();

#ifdef DEBUGSCI0
	Sci0.print("Reset Ball Sensors\r\n");
#endif
	initBallSensor();
}
	
void inrof2016::judgeRunOrient(void){
	
	// �قڊm���ȂƂ������X�V����
	if(MachinePos.Y < 1450){
		if((MachinePos.X < 100.0F) && (MachinePos.X > -100.0F)){	// ���̃��C����ɂ���
			if((MachinePos.getNormalizeTh() > PI/4)&&(MachinePos.getNormalizeTh() < PI/4*3)){
				LineTraceDir = INROF_LINEDIR_FORWARD;
			}else if((MachinePos.getNormalizeTh() < -PI/4)&&(MachinePos.getNormalizeTh() > -PI/4*3)){
				LineTraceDir = INROF_LINEDIR_BACKWARD;
			}
		}else if((MachinePos.X < 1240.0F) && (MachinePos.X > 940.0F)){	// �E�̃��C����ɂ���
			if((MachinePos.getNormalizeTh() > 0)&&(MachinePos.getNormalizeTh() < PI)){			// �k�����Ă�
				LineTraceDir = INROF_LINEDIR_BACKWARD;
			//}else if((MachinePos.getNormalizeTh() < -PI/4)&&(MachinePos.getNormalizeTh() > -PI/4*3)){	// ������Ă�
			}else{	// ������Ă�
				LineTraceDir = INROF_LINEDIR_FORWARD;
			}
		}
	}else{	// ��̃��C����ɂ���
		
		if((MachinePos.getNormalizeTh() < PI/2)&&(MachinePos.getNormalizeTh() > -PI/2)){			// �������Ă�
			LineTraceDir = INROF_LINEDIR_FORWARD;
		//}else if((MachinePos.getNormalizeTh() > PI/4*3)||(MachinePos.getNormalizeTh() < -PI/4*3)){	// �������Ă�
		}else{	// �������Ă�
			LineTraceDir = INROF_LINEDIR_BACKWARD;
		}
	}
	
}


// �}�V���ʒu�C��
// �����_���W���g��

void inrof2016::adjustMachinePos(void){
	
	if(IsCrossing){	// �����_�������Ȃ�
		switch(Cross){
		case INROF_CROSS_YELLOW:	// ���F�]�[���̌����_�Ȃ�
			if(ORIENT_NORTH==MachineOrientation){			// �k�����Ă�
#ifdef DEBUGSCI0
				outPin(GR_LED2, 1);
				Sci0.print("adjust Yel North mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossYellow.X;	
				MachinePos.Y = PosCrossYellow.Y - LINESENS_POS_X - 9;	// �ʒu�ƃ��C���Z���T�̈ʒu�ƃ��C����
				MachinePos.Th = PI/2;
				
#ifdef DEBUGSCI0
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}else if(ORIENT_SOUTH==MachineOrientation){	// ������Ă�
				outPin(GR_LED2, 1);
				
#ifdef DEBUGSCI0
				Sci0.print("adjust Yel South mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossYellow.X;
				MachinePos.Y = PosCrossYellow.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;
				
#ifdef DEBUGSCI0
			Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif			
			}
			break;
		case INROF_CROSS_BLUE:	// �]�[���̌����_�Ȃ�
			if(ORIENT_WEST==MachineOrientation){		// �������Ă�
				outPin(GR_LED2, 1);
#ifdef DEBUGSCI0
				Sci0.print("adjust Bul West mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossBlue.X + LINESENS_POS_X + 9;	
				MachinePos.Y = PosCrossBlue.Y;
				MachinePos.Th = PI;

#ifdef DEBUGSCI0	
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}else if(ORIENT_EAST==MachineOrientation){	// �������Ă�	// �Ȃ����Ē���Ȃ̂Œ���
				outPin(GR_LED2, 1);
#ifdef DEBUGSCI0
				Sci0.print("adjust Bul East mX%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossBlue.X - LINESENS_POS_X - 9;	
				MachinePos.Y = PosCrossBlue.Y;
				MachinePos.Th = 0;

#ifdef DEBUGSCI0		
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}
			break;
		case INROF_CROSS_RED:	// �ԃ]�[���̌����_�Ȃ�
			if(ORIENT_SOUTH==MachineOrientation){			// ������Ă�

#ifdef DEBUGSCI0
				outPin(GR_LED2, 1);
				Sci0.print("adjust Red South mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossRed.X;	
				MachinePos.Y = PosCrossRed.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;
				
#ifdef DEBUGSCI0
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif			
			}else if(ORIENT_NORTH==MachineOrientation){			// �k�����Ă�
				outPin(GR_LED2, 1);
#ifdef DEBUGSCI0
				Sci0.print("adjust Red North mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossRed.X;	
				MachinePos.Y = PosCrossRed.Y - LINESENS_POS_X - 9;
				MachinePos.Th = PI/2;

#ifdef DEBUGSCI0		
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}
			break;
		case INROF_CROSS_BALLFIELD_BEGIN:	// �{�[���]�[���J�n�̌����_�Ȃ�
			if(ORIENT_NORTH==MachineOrientation){			// �k�����Ă�
				outPin(GR_LED2, 1);

#ifdef DEBUGSCI0
				Sci0.print("adjust BalSt North mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
	#endif
				MachinePos.X = PosCrossBallFieldBegin.X;	
				MachinePos.Y = PosCrossBallFieldBegin.Y - LINESENS_POS_X - 9;
				MachinePos.Th = PI/2;

#ifdef DEBUGSCI0
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}else if(ORIENT_SOUTH==MachineOrientation){	// ������Ă�	// �Ȃ����Ē���Ȃ̂Œ���

#ifdef DEBUGSCI0
outPin(GR_LED2, 1);
				Sci0.print("adjust BalSt South mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossBallFieldBegin.X;	
				MachinePos.Y = PosCrossBallFieldBegin.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;

#ifdef DEBUGSCI0
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}
			break;
		case INROF_CROSS_BALLFIELD_END:	// �{�[���]�[���I���̌����_�Ȃ�
			if(ORIENT_SOUTH==MachineOrientation){	// ������Ă�
#ifdef DEBUGSCI0
				outPin(GR_LED2, 1);
				Sci0.print("adjust BalEnd South mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.X = PosCrossBallFieldEnd.X;
				MachinePos.Y = PosCrossBallFieldEnd.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;

#ifdef DEBUGSCI0		
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}
			break;
		case INROF_CROSS_START:	// �{�[���]�[���I���̌����_�Ȃ�
			if(ORIENT_SOUTH==MachineOrientation){	// ������Ă�

#ifdef DEBUGSCI0
				outPin(GR_LED2, 1);
				Sci0.print("adjust START South mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.Y = PosCrossStart.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;

#ifdef DEBUGSCI0				
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}else if(ORIENT_NORTH==MachineOrientation){	// �k�����Ă�

#ifdef DEBUGSCI0
			outPin(GR_LED2, 1);
				Sci0.print("adjust START South mX%f Y%f T%f", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
				MachinePos.Y = PosCrossStart.Y + LINESENS_POS_X - 9;
				MachinePos.Th = PI/2;

#ifdef DEBUGSCI0			
				Sci0.print(" -> X%f Y%f T%f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
#endif
			}
			break;
		}
	}else{
	//	outPin(GR_LED2, 0);
	}
	
}




//��ԊǗ�
int8_t inrof2016::manageState(void){
	
	// �Ђ�����{�[���Ƃ��ăS�[���������������獡�̂Ƃ�����Ȃ��Ȃ��B
	
	fFinishManageAction = manageAction(State);
	return 0;
}
	
// ����Ǘ�
// ����
//  �ړI�n�Ƃ��T�����[�h�Ƃ�
bool_t inrof2016::manageAction(inrof_state State){
	
	
	switch(State){
	case INROF_IDLE:
		SpdTgt = 0;
		AngVelTgt = 0;
		break;
	case INROF_TANSAKU:
		return manageAction_tansaku();
		break;
	case INROF_TOGOAL:
		return manageAction_goToGoal();
		break;
	case INROF_TOSTART:
		return manageAction_goToStart();
		break;
	case INROF_LINECALIB:
		return manageAction_calibLine();
		break;
	}
	
	return false;
}



/*********************
�T�����[�h����
�T�v�F
 �ǂ��ɂ��Ă��T����֍s���A�T������B
 �I������
  �{�[��������ăS�[���̕�����
 �ߒl�F bool_t ���슮��
 
��{��INROF_ACTION_LINE�̏�ԂŒT���B

**********************/
bool_t inrof2016::manageAction_tansaku(void){
	
	bool_t SeqInitSub = false;
	bool_t SeqInit = false;
	
	
	// ��ԑJ��(��_)
	if(IsCrossing){
		if(Cross==INROF_CROSS_BALLFIELD_END){	// �t�B�[���h�̒[�ɂ����炿����Ɖ�������90����]����
			// 180����]����
			ActionState = INROF_ACTION_TURN;
			ManageActionTansakuState_BallFind_turn = 0;	// ������Ɖ�����
			ManageActionTansakuState_BallFind_turn_Last = 0xFF;
			MachinePosActionManageSub = MachinePos;
		}else if(Cross==INROF_CROSS_BALLFIELD_BEGIN){
			if((INROF_ACTION_LINE == ActionState) && (fabs(MachinePos.getNormalizeTh() - PI/2)) < 0.5 ){	// ���C���g���[�X���Ăď�ނ��Ă鎞�Ƀ{�[���t�B�[���h�̏�̐����z����
				ActionState = INROF_ACTION_TURN;
				ManageActionTansakuState_BallFind_turn = 1;	// ������Ȃ��ėǂ��̂�1����
				ManageActionTansakuState_BallFind_turn_Last = 0xFF;
				MachinePosActionManageSub = MachinePos;
			}
		}else if(Cross==INROF_CROSS_START){
			
		}
		if((INROF_ACTION_LINE == ActionState) && ((Cross==INROF_CROSS_START) || (Cross==INROF_CROSS_YELLOW) || (Cross==INROF_CROSS_RED))){
			if(fabs(MachinePos.getNormalizeTh() + PI/2) < 0.5 ){	// ���C���g���[�X���Ăĉ��ނ��Ă鎞�ɃX�^�[�g�]�[��	// ���̂�����������I��
				ActionState = INROF_ACTION_TURN;
				ManageActionTansakuState_BallFind_turn = 1;	// ������Ȃ��ėǂ��̂�1����
				MachinePosActionManageSub = MachinePos;
			}
		}
	}
	
	
	
	// �t�B�[���h�[�����ŕǂɂԂ��肻��
	// �{�[���Z���T�Ō��m
	
	if((INROF_ACTION_LINE == ActionState) && (Cross==INROF_CROSS_BALLFIELD_BEGIN)){
		if(MachinePos.Y < 230.0F){
			float PosXw,PosYw;
			bool_t fFieldEnd = false;
			bool_t valid;
			
			valid = !PsdCenter->getGlobalObjPos(&MachinePos, &PosXw, &PosYw);
			if(valid){
				fFieldEnd |= (PosYw<(FIELD_INROF_CROSS_BALLFIELD_END_Y-10.0F));	// �{�[���]�[���O�ɕ��̗L��
			}
			valid = !PsdRight->getGlobalObjPos(&MachinePos, &PosXw, &PosYw);
			if(valid){
				fFieldEnd |= (PosYw<(FIELD_INROF_CROSS_BALLFIELD_END_Y-10.0F));
			}
			valid = !PsdLeft->getGlobalObjPos(&MachinePos, &PosXw, &PosYw);
			if(valid){
				fFieldEnd |= (PosYw<(FIELD_INROF_CROSS_BALLFIELD_END_Y-10.0F));
			}
			
			if( fFieldEnd ){
				// 180����]����
				ActionState = INROF_ACTION_TURN;
				ManageActionTansakuState_BallFind_turn = 0;	// ������Ɖ�����
				ManageActionTansakuState_BallFind_turn_Last = 0xFF;
				MachinePosActionManageSub = MachinePos;
			}
		}
	}
	
	// ��ԑJ��(�{�[�������鏈��)
	if(IsBallFinding){	// �{�[��������
#ifdef DEBUGSCI0
	Sci0.print("ball find.\r\n");
#endif
		ActionState = INROF_ACTION_BALLGET;		// �{�[���擾�X�e�[�g�ֈڍs
		BallFindEnable = false;					// �{�[���T����߂�
		MachinePosActionManage = MachinePos;	// 
#ifdef DEBUGSCI0
	Sci0.print("nowpos %f %f %f\r\n",MachinePosActionManage.X, MachinePosActionManage.Y, MachinePosActionManage.Th);
	#endif
		MachinePosActionManageSub = MachinePos;
		ManageActionTansakuState_BallFind = 0;	// �{�[���T���p�X�e�[�g������
		ManageActionTansakuState_BallFind_Last = 0xFF;
		// �ǂ̃Z���T�H
		BallFindDistance = 200;
		/*
		if( (BALLFIND_MAXDISTANCE_SIDE > PsdLeftSide->getDistance()) && PsdLeftSide->isObjectBallArea(&MachinePos) ){
			if(BallFindDistance > PsdLeftSide->getDistance()){	// ���̃Z���T���߂���΂����K�p
				BallDir = 1;	// �����
				BallFindDistance = PsdLeftSide->getDistance();
			}
		}
		*/
		if( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdRight->getDistance()) && PsdRight->isObjectBallArea(&MachinePos) ){
		// �������A�E�N���X�������獶��肷��

#ifdef DEBUGSCI0
Sci0.print("Ball find RightSensor\r\n");
#endif
			if(BallFindDistance > PsdRight->getDistance()){	// ���̃Z���T���߂���΂����K�p
				BallDir = 1;	// �E
				BallFindDistance = PsdRight->getDistance() * 0.5;
			}
		}
		if( (BALLFIND_MAXDISTANCE_FRONT > PsdCenter->getDistance()) && PsdCenter->isObjectBallArea(&MachinePos) ){
		// �Z���^�[�͂Ƃ肠��������肷��
			
#ifdef DEBUGSCI0
Sci0.print("Ball find CenterSensor\r\n");
#endif
			if(BallFindDistance > PsdCenter->getDistance()){	// ���̃Z���T���߂���΂����K�p
				BallDir = 1;
				BallFindDistance = PsdCenter->getDistance();
			}
		}
		if ( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdLeft->getDistance()) && PsdLeft->isObjectBallArea(&MachinePos) ){
		// �E�����A���N���X��������E��肷��

#ifdef DEBUGSCI0
			Sci0.print("Ball find LeftSensor\r\n");
#endif
if(BallFindDistance > PsdLeft->getDistance()){	
				BallDir = 0;	// �E
				BallFindDistance = PsdLeft->getDistance() * 0.5;
			}
		}
	}
	
	if(ActionStateLast != ActionState){
		SeqInit = true;
	}
	ActionStateLast = ActionState;
	
	
	if(SeqInit){
		switch(ActionState){
		case INROF_ACTION_LINE:	// �T�[�`�I
			Rmb->outLedDispAscii("Srch");
			break;
		case INROF_ACTION_BALLGET:	// �{�[���������B���
			Rmb->outLedDispAscii("Find");
			break;
		
		}
#ifdef DEBUGSCI0
		Sci0.print("%08d INROF_manageAction_tansaku:newSeq %d\r\n", getTime_ms(), ActionState);
		#endif
	}
	
	
	// ����
	switch(ActionState){
	case INROF_ACTION_IDLE:		// �܂��͂����ɓ���
		ActionState = INROF_ACTION_LINE;
		setTgtVel_LineTrace();
		ManageActionTansakuState_BallFind = 0;
		
		// �{�[���T�����̎p����
		InrofUpper.reqArmToCenter();
		
		break;
	case INROF_ACTION_TURN:		// 180�����
		
		if(ManageActionTansakuState_BallFind_turn!=ManageActionTansakuState_BallFind_turn_Last){	// ���񏈗�
			SeqInitSub = true;
			resetBallSensors();
#ifdef DEBUGSCI0
			Sci0.print("%08d INROF_ACTION_TURN:newSeq %d\r\n", getTime_ms(), ManageActionTansakuState_BallFind);
		#endif
		}
		ManageActionTansakuState_BallFind_turn_Last = ManageActionTansakuState_BallFind_turn;
		
		//adjustMachinePos();	// ���C���g���[�X�ł͂Ȃ����ǌ����_�Ŕ��肷�邱�Ƃ������̂�
		switch(ManageActionTansakuState_BallFind_turn){
		case 0:	// ������Ɖ�����
			if(SeqInitSub){
#ifdef DEBUGSCI0
				Sci0.print("Turn\r\n");
				#endif
			}
			if(80.0 < (MachinePos-MachinePosActionManageSub)){	// 80mm�ȏ㗣�ꂽ
				ManageActionTansakuState_BallFind_turn ++;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{		// �����܂ŉ�����
				this->SpdTgt = -60;		// [mmps]
				this->AngVelTgt = 0;	// [rps]
				BallFindEnable = false;
			}
			break;
		case 1:	//180�x����
			if(fabs(MachinePos.Th - (MachinePosActionManageSub.Th + PI)) < 0.1){
				//fFinishAction = 1;				// 180��������̂Ŏ��̃X�e�[�g��
				ManageActionTansakuState_BallFind_turn = 0;
				ActionState = INROF_ACTION_LINE;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [radps]
				BallFindEnable = true;
			}else{		// 180�����܂ŉ�]
				this->SpdTgt = 0;		// [mmps]
				this->AngVelTgt = 0.10F * ((MachinePosActionManageSub.Th + PI) - MachinePos.Th);	// [rad/s]
				
				// �������Ȃ��悤�ɂ���
				if(0.05F<this->AngVelTgt){
					this->AngVelTgt = 0.05;
				}else if(-0.05F>AngVelTgt){
					this->AngVelTgt = -0.05;
				}
			
				BallFindEnable = false;
			}
			break;
		}
		break;
	case INROF_ACTION_BALLGET:	// �{�[���������̂Œ͂܂���
		if(ManageActionTansakuState_BallFind!=ManageActionTansakuState_BallFind_Last){	// ���񏈗�
			SeqInitSub = true;
			//resetBallSensors();
			#ifdef DEBUGSCI0
			Sci0.print("%08dms INROF_ACTION_BALLGET:newSeq %d\r\n", getTime_ms(), ManageActionTansakuState_BallFind);
			#endif
		}
		ManageActionTansakuState_BallFind_Last = ManageActionTansakuState_BallFind;
		
		switch(ManageActionTansakuState_BallFind){
		case 0:		// �{�[���Ȃ��Ȃ�܂Ŕ��Ό���(���Ƃ��Ɩڂ̑O�Ƀ{�[�������������p)
			if(SeqInitSub){
				MachinePosActionManageSub = MachinePos;
			}
			
			if(fabs(MachinePos.Th - MachinePosActionManageSub.Th) > PI/2){		// 90�x�ȏ��������߂ĒT���ĊJ
				ManageActionTansakuState_BallFind = 40;
			}else{
			//if(UPPER_STATE_IDLE != InrofUpper.getState()){	// �A�[�������S�ȏ�Ԃ��m�F			
			//	InrofUpper.reqArmToCenter();		// ����Ȃ���ΒT����Ԃ̈ʒu�ɂ���
			//}else{
				if((BallFindDistance+30 < PsdCenter->getDistance()) || (0>PsdCenter->getDistance())){ 	// �w��l���傫�� or �����l
					ManageActionTansakuState_BallFind = 1;				// 20��������̂Ŏ��̃X�e�[�g��
					this->SpdTgt = 0;		// [mmps]
					this->AngVelTgt = 0.0;	// [rps]
				}else{
					this->SpdTgt = 0;		// [mmps]
					if(BallDir){	// �{�[���͍�����
						this->AngVelTgt = -0.02;	// �E��][rps]
					}else{
						this->AngVelTgt = 0.02;	// [rps]
					}
				}
			//}
			}
			break;
			
		case 1:		// �{�[��������܂ŉ��
		
			if(fabs(MachinePos.Th - MachinePosActionManageSub.Th) > PI/2){		// 90�x�ȏ��������߂ĒT���ĊJ
				ManageActionTansakuState_BallFind = 40;
				
			//}else if(( BallFindDistance+30 > PsdCenter->getDistance()) && PsdCenter->isObjectBallArea()){		//�w��l��菬���� and �L���͈�
			}else if(( (BallFindDistance+30) > PsdCenter->getDistance()) && (0 < PsdCenter->getDistance())){		// �{�[��������(�w��l��菬���� and �L���͈�)
			//}else if(( BallFindDistance+30 > PsdCenter->getDistance())){		//�w��l��菬���� -> �����l�������甽�������Ⴄ�̂Ń{�c
					// �Ώۂ̃{�[���ʒu��菬������Ό������Ɣ���B������Ɨ]�T������+30mm���Ă�
				ManageActionTansakuState_BallFind = 2;				// �{�[���������̂Ŏ��̃X�e�[�g��
				BallFindTh = MachinePos.Th;
				
				
			}else{
				if(BallDir){	// �{�[���͍�����
					//����
					this->SpdTgt = 0;		// [mmps]
					this->AngVelTgt = 0.02;	// [rps]
				}else{			// �{�[���͉E����
				//-����
					this->SpdTgt = 0;		// [mmps]
					this->AngVelTgt = -0.02;	// [rps]
				}
			}
			break;
			
			
		case 2:		// �t�����g�Z���T�Ń{�[���������B�{�[���Ȃ��Ȃ�܂ōX�ɉ���
			if(fabs(MachinePos.Th - MachinePosActionManageSub.Th) > (PI*1/2)){		// 90�x�ȏ��������߂ĒT���ĊJ
				ManageActionTansakuState_BallFind = 40;
				
			}else if((BallFindDistance+30 < PsdCenter->getDistance()) || (0>PsdCenter->getDistance())){ 	// �w��l��艓��(�Ⴄ�{�[�������Ȃ�����) or �����l(�����Ȃ����炢����)
				ManageActionTansakuState_BallFind = 3;				// �{�[���Ȃ��Ȃ����̂Ŏ��̃X�e�[�g��
				BallFindTh = (MachinePos.Th + BallFindTh)/2;
				
				// ToF�����Z���T�p����
				if(BallDir){	// ���� �}�V���̍��ɂ���
				//	BallFindTh = BallFindTh + 0.05F;	// ���傢��
				}else{		// �}�V���̉E�ɂ���
				//	BallFindTh = BallFindTh - 0.10F;	// ���傢�E
				}
				
				this->SpdTgt = 0;		// [mmps]
				this->AngVelTgt = (BallFindTh - MachinePos.Th)/12;	// [rps]
				
			}else{
				if(BallDir){	// �{�[���͍�����
					//����
					this->SpdTgt = 0;		// [mmps]
					this->AngVelTgt = 0.02;	// [rps]
				}else{			// �{�[���͉E����
					this->SpdTgt = 0;		// [mmps]
					this->AngVelTgt = -0.02;	// [rps]
				}
			}
			break;
			
			
		case 3:		// �{�[���Ȃ��Ȃ�܂ŉ�����BBallFindTh�����傤�ǃ{�[���̐^�񒆂���B���Ԃ�ˁB�Ƃ肠����������������B
			//����
			if(fabs(BallFindTh - MachinePos.Th) < 0.02){ 	// �{�[���̂ق�������
				ManageActionTansakuState_BallFind = 4;				// �{�[���̂ق��������̂Ŏ��̃X�e�[�g��
				this->SpdTgt = 0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{
				this->SpdTgt = 0;		// [mmps]
				//if(BallDir){
				//	this->AngVelTgt = -0.1 * (BallFindTh - MachinePos.Th);
				//}else{
					this->AngVelTgt = 0.2 * (BallFindTh - MachinePos.Th);	// [rps]
				//}
			}
			
			if(this->AngVelTgt > 0.02){
				this->AngVelTgt = 0.02;
			}else if(this->AngVelTgt< -0.02){
				this->AngVelTgt = -0.02;
			}
			break;
			
			
		case 4:		// �O�㋗�����킹���
			if(SeqInitSub){
				MachinePosActionManageSub = MachinePos;
			}
			//����
			if((BALLGET_MAXDISTANCE_FRONT<PsdCenter->getDistance()) || (0>PsdCenter->getDistance() ) || (MachinePosActionManageSub-MachinePos)>300){
				// �{�[���������� or 200mm�ȏ㉺������(�Z���T�ł܂����Ƃ���)
				ManageActionTansakuState_BallFind = 40;
			}else if((fabs(PsdCenter->getDistance() - BALLGET_DISTANCE_FRONT_ARMDOWN) < 5.0F) && (InrofUpper.getState() == UPPER_STATE_IDLE)){ 
				// �{�[���Ƃ̋������킹�� && �㔼�g�{�[���擾�\���
				
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
				// �A�[�����낷
				InrofUpper.reqBallGetPre(PI/2);	// �O��
				ManageActionTansakuState_BallFind ++;				// �������킹���̂Ŏ��̃X�e�[�g��
				StartTimeActionManage = getTime_ms();	// ����
								
			}else{
				// �t�����g�����Z���T�Ń{�[���Ƃ̋��������킹��
				this->SpdTgt = 5 * (PsdCenter->getDistance() - BALLGET_DISTANCE_FRONT_ARMDOWN);		// [mmps]
				// �Œᑬ�x�K���H
				if(100<SpdTgt){
					this->SpdTgt = 100;
				}else if(-100>SpdTgt){
					this->SpdTgt = -100;
				}
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
			
			
		case 5:		// �A�[�����낷�̑҂�
			if((UPPER_STATE_IDLE == InrofUpper.getState()) || (StartTimeActionManage + 12000 < getTime_ms()) ){ 	// �T�[�{������
				ManageActionTansakuState_BallFind ++;
				MachinePosActionManageSub = MachinePos;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
			}
			break;
			
			
		case 6:		// �O�i���āA�{�[���͂�
			if((BALLGET_DISTANCE_FRONT_ARMDOWN-BALLGET_DISTANCE_FRONT_GRAB) < (MachinePosActionManageSub - MachinePos)){ 	// �A�[���͂ވʒu�܂Ői�ޑO�i��
				ManageActionTansakuState_BallFind = 9;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				StartTimeActionManage = getTime_ms();	// ����
				
				// �A�[������
				InrofUpper.reqBallGet();
				
			}else{
				this->SpdTgt = 1.0*((BALLGET_DISTANCE_FRONT_ARMDOWN) - (MachinePosActionManageSub - MachinePos));		// [mmps]
				//SpdTgt = 30;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
			
		case 9:		// �{�[�������グ�Ĉړ��\�ȏ�ԂɂȂ�̂�҂�
			if((UPPER_STATE_BALL_GET != InrofUpper.getState()) || ((StartTimeActionManage + 5000) < getTime_ms())){ 	// �A�[�������グ��
				ManageActionTansakuState_BallFind ++;
#ifdef DEBUGSCI0
	Sci0.print("base: ballget and back\r\n");
	#endif
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
				// �t���O���Z�b�g
				fNextAction = false;
				
			}else{
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
			}
			break;
			
			
		case 10:	// �{�[���Ƃ����I�I�I�I���C���֖߂��
			if(SeqInitSub){
#ifdef DEBUGSCI0
				Sci0.print("TgtPos %f, %f, %f\r\n", MachinePosActionManage.X, MachinePosActionManage.Y, MachinePosActionManage.Th);
				Sci0.print("NowPos %f, %f, %f\r\n", MachinePos.X, MachinePos.Y, MachinePos.Th);
	#endif
			}
			if( fNextAction ){	// ���C����֖߂���
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
				// �{�[���Ƃ������Ƃɂ���B
				FieldBall--;
				
				
				// �����ŁA�{�[�����\���m�ۂ���
				// �������́A�t�B�[���h�Ƀ{�[�����Ȃ�
				// �ꍇ�́A�{�[����u���ɋA��
				if(InrofUpper.isBallFull() || (FieldBall<=0)){
#ifdef DEBUGSCI0
	Sci0.print("base: to goal\r\n");
	Sci0.print("base: turn North\r\n");
	#endif
					ManageActionTansakuState_BallFind = 11;
				}else{
#ifdef DEBUGSCI0
	Sci0.print("base: re search\r\n");
	#endif
					// �����łȂ��ꍇ�́A
					// ���̌����������čĂђT������B
					ManageActionTansakuState_BallFind = 41;
				}
				
				fNextAction = false; // �t���O���Z�b�g
			}else{
				// �܂������o�b�N
				/*
				if((MachinePos.getNormalizeTh() > PI/2) || (MachinePos.getNormalizeTh() < -PI/2)){	// �������Ă�
					SpdTgt = -3 * (MachinePosActionManage.X - MachinePos.X);		// [mmps]
				}else{																				// �E�����Ă�
					SpdTgt = 3 * (MachinePosActionManage.X - MachinePos.X);
				}*/
				if(ORIENT_WEST == MachineOrientation){	// �������Ă�
					this->SpdTgt = -3.0F * (MachinePosActionManage.X - MachinePos.X);		// [mmps]
					if(fabs(MachinePosActionManage.X - MachinePos.X) < 5.0){
					//if(MachinePosActionManage.X < MachinePos.X){
						fNextAction = true;
					}
				}else if(ORIENT_EAST == MachineOrientation){	// ��																		// �E�����Ă�
					this->SpdTgt = 3.0F * (MachinePosActionManage.X - MachinePos.X);
					if(fabs(MachinePosActionManage.X - MachinePos.X) < 5.0){
					//if(fabs(MachinePosActionManage.X - MachinePos.X) < 5.0){
						fNextAction = true;
					}
				}else if(ORIENT_SOUTH == MachineOrientation){	// ������Ă�
					this->SpdTgt = -3.0F * (MachinePosActionManage.Y - MachinePos.Y);		// [mmps]
					if(fabs(MachinePosActionManage.Y - MachinePos.Y) < 5.0){
					//if(MachinePosActionManage.Y < MachinePos.Y){
						fNextAction = true;
					}
				}else{											// �k�����Ă�
					this->SpdTgt = 3.0F * (MachinePosActionManage.Y - MachinePos.Y);
					if(fabs(MachinePosActionManage.Y - MachinePos.Y) < 5.0){
					//if(MachinePosActionManage.Y > MachinePos.Y){
						fNextAction = true;
					}
				}
				
				if(this->SpdTgt>100.0F){
					this->SpdTgt = 100.0F;
				}else if(this->SpdTgt< -100.0F){
					this->SpdTgt = -100.0F;
				}
				
				
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
		case 11:		// �k����
			if( fabs(MachinePos.getNormalizeTh()-PI/2) < 0.2F){	// ���������k������[rad]
				ManageActionTansakuState_BallFind ++;
#ifdef DEBUGSCI0
	Sci0.print("base: north\r\n");
#endif
			}else{
				// �k������(P����)
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.20 * (PI/2 - ((MachinePos.getNormalizeTh()< -PI/2)?MachinePos.getNormalizeTh()+2*PI:MachinePos.getNormalizeTh()));	// [rps]
				//��]���x�ɐ���
				if(0.04<AngVelTgt){
					this->AngVelTgt = 0.04;
				}else if(-0.04>AngVelTgt){
					this->AngVelTgt = -0.04;
				}
				
				if(0<AngVelTgt){	// ���̃V�[�P���X�ł̉�]�����ݒ�
					BallDir = 1;
				}else{
					BallDir = 0;
				}
			}
			
			break;
		
		case 12:	// ���C�����܂ł���ɉ��
			if((getLineSensAd(1)<LINESENS_THRESHOLD_CENTER)||(getLineSensAd(2)<LINESENS_THRESHOLD_CENTER)){	// �����Q�̃Z���T�����ꂩ�������Ȃ�܂�
				
				ManageActionTansakuState_BallFind = 13;
				fNextAction = false; // �t���O���Z�b�g
			}else{
				//���C���Ȃ���΂܂��
				//  ��肷�����������ǂ����悤
				this->SpdTgt = 0.0;		// [mmps]
				if(BallDir){	// ���̃{�[�����Ƃ���
					// �����Ɖ��(�����)
					this->AngVelTgt = 0.02;
				}else{
					// �����Ɖ��(�E���)
					this->AngVelTgt = -0.02;
				}
			}
			break;
		
		case 13:	// �{�[�����芮���҂��B�S�[���ֈړ�����
			//if((StartTimeActionManage + 2000) < getTime_ms()){	// �{�[���F�Z���T�N������2000ms�ȏ�o���Ă����炿������
			if(UPPER_STATE_IDLE==InrofUpper.getState()){	// �{�[���F�Z���T�N������2000ms�ȏ�o���Ă����炿������
			//�{�[���F�`�F�b�N
				//BallColorFixed = BallColor;
				
				// �{�[�������ĂȂ�������I�I�I
				// �܂��T���I�I�I
				if(InrofUpper.numBall()){
					// �{�[�������Ă邩��u���ɍs��
					this->SpdTgt = 0.0;		// [mmps]
					this->AngVelTgt = 0.0;	// [rps]
					
					ManageActionLineTraceDir = LineTraceDir;
					setState(INROF_TOGOAL);	// ���̏�ԂցB�{�[����u���ɍs���B
					ManageActionTansakuState_BallFind = 0;
				}else{
					// �����������Ă��Ƃ�����to start����
					setState(INROF_TOSTART);	// ���̏�ԂցB�{�[����u���ɍs���B
					
					ManageActionTansakuState_BallFind = 0x00;
				}
				
				//fJudgeColor = false;	// �F����I��
			}else{
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;
			}
			break;
// �ĒT���p�X�e�[�g	
//  �܂��{�[���������
//  �{�[�����������ǌ�������
		case 40:	//�{�[���������Ǝv�������ǌ�������
			// ���C����ɖ߂�
			if( fNextAction ){
			
				ManageActionTansakuState_BallFind ++;	// ��]����
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
				fNextAction = false; // �t���O���Z�b�g
			}else{
				if(ORIENT_WEST == MachineOrientation){	// �������Ă�
					this->SpdTgt = -3 * (MachinePosActionManage.X - MachinePos.X);		// [mmps]
					if(fabs(MachinePosActionManage.X - MachinePos.X) < 5.0){
						fNextAction = true;
					}
				}else if(ORIENT_EAST == MachineOrientation){	// ��																		// �E�����Ă�
					this->SpdTgt = 3 * (MachinePosActionManage.X - MachinePos.X);
					if(fabs(MachinePosActionManage.X - MachinePos.X) < 5.0){
						fNextAction = true;
					}
				}else if(ORIENT_SOUTH == MachineOrientation){	// ������Ă�
					this->SpdTgt = -3 * (MachinePosActionManage.Y - MachinePos.Y);		// [mmps]
					if(fabs(MachinePosActionManage.Y - MachinePos.Y) < 5.0){
						fNextAction = true;
					}
				}else{											// �k�����Ă�
					this->SpdTgt = 3 * (MachinePosActionManage.Y - MachinePos.Y);
					if(fabs(MachinePosActionManage.Y - MachinePos.Y) < 5.0){
						fNextAction = true;
					}
				}
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
		case 41:	//���E�{�[���������Ǝv�������ǈ����
			// ���������ɖ߂��ĒT���ĊJ
			if(0.5 > fabs(MachinePosActionManage.Th - MachinePos.Th)){
				ManageActionTansakuState_BallFind ++;
			}else{
				SpdTgt = 0.0;		// [mmps]
				AngVelTgt = 0.15*(MachinePosActionManage.Th - MachinePos.Th);	// [rps]
				
				// �������Ȃ��悤�ɂ���
				if(0.04F<this->AngVelTgt){
					this->AngVelTgt = 0.04;
				}else if(-0.04F>AngVelTgt){
					this->AngVelTgt = -0.04;
				}
			}
			break;
		case 42:	// ���C�����܂ł���ɉ��
			if((getLineSensAd(1)<LINESENS_THRESHOLD_CENTER)||(getLineSensAd(2)<LINESENS_THRESHOLD_CENTER)){	// �����Q�̃Z���T�����ꂩ�������Ȃ�܂�
				
				ActionState = INROF_ACTION_IDLE;	// ���������T�����܂�(IDLE�s���ď��������Ă���)
				ManageActionTansakuState_BallFind = 0;
				ManageActionTansakuState_BallFind_Last = 0xFF;
			}else{
				//���C���Ȃ���΂܂��
				//  ��肷�����������ǂ����悤
				this->SpdTgt = 0.0;		// [mmps]
				if(BallDir){	// ���̃{�[�����Ƃ���
					// �����Ɖ��(�E���)
					this->AngVelTgt = -0.02;
				}else{
					// �����Ɖ��(�����)
					this->AngVelTgt = 0.02;
				}
			}
			break;
			
			
			
			
		case 99:	// �ꎞ��~	
			if(StartTimeActionManage + 1000 < getTime_ms() ){
				//ManageActionTansakuState_BallFind = ManageActionTansakuState_BallFind_last+1;
			}else{
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
		}
		break;
		
		
	case INROF_ACTION_LINE:	// ���C���g���[�X
		float x, y;
		
		if(SeqInit){
#ifdef DEBUGSCI0
			Sci0.print("%08d LineTrace \r\n", getTime_ms());
#endif
			StartTimeActionManage = getTime_ms();
		}
		
		
		PsdCenter->getGlobalPos(&MachinePos, &x, &y);
		if((x > 580.0F) && (y < 1500.0F)){	// �t�B�[���h��ł͂�����聕�{�[���T���L��
			BallFindEnable = true;
			setTgtVel_LineTrace(RMB_VEL_NORMAL-50.0F);
		}else{
			BallFindEnable = false;	// �t�B�[���h�O�ł̓{�[���T�����Ȃ�
			setTgtVel_LineTrace(RMB_VEL_NORMAL);
		}
		
		
		// 3�b�Ɉ����
		if(StartTimeActionManage + 3000 < getTime_ms()){
			resetBallSensors();
			StartTimeActionManage = getTime_ms();
		}	
		
		
		// �{�[�����񂾁I�I�I
		if(Rmb->isCliff()){
			ActionState = INROF_ACTION_OVERBALL;
			ManageActionTansakuState_BallFind = 0;
			ManageActionTansakuState_BallFind_Last = 0xFF;
			ManageActionTansakuState_BallOver = 0;
		}
		
		break;
	case INROF_ACTION_OVERBALL:	
		// �}�V�����{�[���ɏ��グ�����̏���
		if(ManageActionTansakuState_BallOver!=ManageActionTansakuState_BallOver_Last){	// ���񏈗�
			SeqInitSub = true;
		}
		ManageActionTansakuState_BallOver_Last = ManageActionTansakuState_BallOver;
		
		switch(ManageActionTansakuState_BallOver){
		case 0:
			if(SeqInitSub){
				MachinePosActionManage = MachinePos;
			}
			this->SpdTgt = -50.0;		// [mmps]
			this->AngVelTgt = 0.0;	// [rps]
			
			if(!Rmb->isCliff() || ((MachinePosActionManage - MachinePos)>300)){	// ���܂Ȃ��Ȃ邩300mm��������
			
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				ActionState = INROF_ACTION_LINE;
				ManageActionTansakuState_BallFind_Last = 0xFF;
				
				ManageActionTansakuState_BallOver = 0;
				ManageActionTansakuState_BallOver_Last = 0xFF;
				
			}
		break;
		}
	}
	
	
	
	return false;
}
	
	
// �{�[����u���ɍs������
bool_t inrof2016::manageAction_goToGoal(void){
	bool_t SeqInit = false;
	bool_t SeqEnd = false;
	
	ActionStateLast = ActionState;
	
	// ����
	switch(ActionState){
	case INROF_ACTION_IDLE:	// �ŏ��ɓ���
		ActionState = INROF_ACTION_BALLRELEASE;
		ManageActionTansakuState_BallRelease = 0;
		ManageActionTansakuState_BallReleaseLast = 0xff;
		MachinePosActionManage = MachinePos;	// ���̏�Ԃɓ��������̏ꏊ����xxmm�ȓ��͏ꏊ�␳���Ȃ��p
		ManageActionLineTraceDir = LineTraceDir;
		Goal = INROF_GOAL_NONE;		// �܂��s���挈�܂�Ȃ�
		
		// �����ĉ������s
	case INROF_ACTION_BALLRELEASE:
			
		if(ManageActionTansakuState_BallReleaseLast != ManageActionTansakuState_BallRelease){	// ����t���O���Ă�
			SeqInit = true;
#ifdef DEBUGSCI0
Sci0.print("manageAction_goToGoal %d\r\n", ManageActionTansakuState_BallRelease);
#endif
		}
		ManageActionTansakuState_BallReleaseLast = ManageActionTansakuState_BallRelease;
		
		switch(ManageActionTansakuState_BallRelease){
		case 0:	// �S�[���ʒu�܂Ń��C���g���[�X�A�A�[���ړ��v��
			/* ���񏈗� */
			if(SeqInit){
				Goal = INROF_GOAL_NONE;
				
#ifdef DEBUGSCI0
				Sci0.print("Forward? %d\r\n", ManageActionLineTraceDir);
#endif
			}
		
			/* �ʏ펞���� */
			// ��{�̓��C���g���[�X�ŖڕW�S�[���ʒu�̌�_��ڎw��
			
			// �ڕW�S�[�����菈��
			if((Goal == INROF_GOAL_NONE)&&(UPPER_STATE_IDLE == InrofUpper.getState())){	// �܂����܂��ĂȂ� && ����ƃA�[���ɂɂȂ���
			// �A�[������ɉ��F�̃{�[�������Ă邩����
				inrof_color TrashColor;
				
				if(INROF_LINEDIR_FORWARD == ManageActionLineTraceDir){
					if(0 < InrofUpper.numRed()){	// �ԃ`�F�b�N
						TrashColor = INROF_RED;		// �̂Ă�{�[���̐F
						this->Goal = INROF_GOAL_RED;		// �������S�[��
					}else if(0 < InrofUpper.numYellow()){	// ���`�F�b�N
						TrashColor = INROF_YELLOW;	// �̂Ă�{�[���̐F
						this->Goal = INROF_GOAL_YELLOW;		// �������S�[��
					}else if(0 < InrofUpper.numBlue()){	// �`�F�b�N
						TrashColor = INROF_BLUE;	// �̂Ă�{�[���̐F
						this->Goal = INROF_GOAL_BLUE;		// �������S�[��
					}else{	// �{�[������ -> ��������
						TrashColor = INROF_NONE;
						this->Goal = INROF_GOAL_NONE;		// �������S�[��
						
						// �T���ĊJ
						setState(INROF_TANSAKU);
						break;
					}
				}else{	
					if(0 < InrofUpper.numBlue()){		// �`�F�b�N
						TrashColor = INROF_BLUE;	// �̂Ă�{�[���̐F
						this->Goal = INROF_GOAL_BLUE;		// �������S�[��
					}else if(0 < InrofUpper.numYellow()){	// ���`�F�b�N
						TrashColor = INROF_YELLOW;	// �̂Ă�{�[���̐F
						this->Goal = INROF_GOAL_YELLOW;		// �������S�[��
					}else if(0 < InrofUpper.numRed()){	// �ԃ`�F�b�N
						TrashColor = INROF_RED;	// �̂Ă�{�[���̐F
						this->Goal = INROF_GOAL_RED;		// �������S�[��
					}else{	// �{�[������ -> ��������
						TrashColor = INROF_NONE;
						this->Goal = INROF_GOAL_NONE;		// �������S�[��
						
						// �T���ĊJ
						setState(INROF_TANSAKU);
						break;
					}
				}
				
				// �A�[������Ƀ{�[����p�ӂ���悤�v��
				if(INROF_LINEDIR_FORWARD == ManageActionLineTraceDir){	// ���C���������Ȃ�}�V�������Ɏ̂Ă�
					InrofUpper.reqBallTrashPre(TrashColor, PI);
				}else{							// ���C���t�����Ȃ�}�V���E���Ɏ̂Ă�
					if(INROF_GOAL_BLUE == Goal){			// ���C���t�����ŐȂ�}�V���O���Ɏ̂Ă�
						InrofUpper.reqBallTrashPre(TrashColor, PI/2);
					}else{
						InrofUpper.reqBallTrashPre(TrashColor, 0);
					}
				}
				
				Sci0.print("ArmReqBallTrash. Col %d\r\n", Goal);
				
				switch(Goal){
				case INROF_GOAL_BLUE:
					Rmb->outLedDispAscii("BLUE");
					break;
				case INROF_GOAL_YELLOW:
					Rmb->outLedDispAscii("YLOW");
					break;
				case INROF_GOAL_RED:
					Rmb->outLedDispAscii("RED");
					break;
				}
				
			}
			
			
			
			// �ړ�����
			
			if(200.0f > (MachinePosActionManage - MachinePos)){	// ���C���g���[�X���n�܂���200mm�ȉ������i��łȂ���Έʒu�␳���Ȃ��B
				fAdjustMachinePos = false;
			}else{
				fAdjustMachinePos = true;
			}
			if(INROF_CROSS_START==Cross || INROF_CROSS_NONE==Cross){
				// �Ԍ����_���X�^�[�g�ʒu���������璼�i
				SpdTgt = RMB_VEL_NORMAL;
				AngVelTgt = 0.0;
				//fAdjustMachinePos = true;
				
			}else if(INROF_GOAL_BLUE == Goal){
				// �ړI�n���̏ꍇ�̏���
				if(INROF_LINEDIR_FORWARD == ManageActionLineTraceDir){
				// Forward���F	�����C�����ނ܂łƁA�����C���ʉߌ�(450-�Z���T��)mm�̓��C���g���[�X
				//		���̌�͒��i�B�����C������650mm�i�ނ܂�
					
					if(INROF_CROSS_YELLOW == Cross){	// ���F��������ňȍ~
					
							
						if((MachinePos - PosCrossYellow) > 650.0F){	/* ������ 3 */	// ��������650mm�i��
							// �������{�[���u���ʒu�B
#ifdef DEBUGSCI0
							Sci0.print("se mX%f Y%f, yX%f Y%f\r\n", MachinePos.X, MachinePos.Y, PosCrossYellow.X, PosCrossYellow.Y);
#endif
							SeqEnd = true;
							
						}else if((MachinePos - PosCrossYellow) > (450.0F - LINESENS_POS_X)){	/* ������ 2 */	// ��������450mm-�Z���T���i��
							
							SpdTgt = RMB_VEL_NORMAL;
							AngVelTgt = 0.0;		// ���i
							
#ifdef DEBUGSCI0
							Sci0.print("st mX%f Y%f, yX%f Y%f\r\n", MachinePos.X, MachinePos.Y, PosCrossYellow.X, PosCrossYellow.Y);
#endif							
						}else{		/* ������ 1 */
							setTgtVel_LineTrace(RMB_VEL_NORMAL);	// ���C���g���[�X
						}
					}else{
						setTgtVel_LineTrace(RMB_VEL_NORMAL);	// ���C���g���[�X
					}
					
				}else{
				// Backward���F	���C���ʉߌ�(150-�Z���T��)mm�̓��C���g���[�X
				//		���̌�͒��i�B���C������300mm�i�ނ܂�
					
					if((INROF_CROSS_BLUE == Cross)||(MachinePos.X<FIELD_INROF_CROSS_BLUE_X)){	// ��������ňȍ~
						if((MachinePos - PosCrossBlue) > 300.0F){		// ������300mm�i��
							// �������{�[���u���ʒu�B
							SeqEnd = true;
						}else if((MachinePos - PosCrossBlue) > (150.0F - LINESENS_POS_X)){	// ������150mm�i��(���C���Ȃ��Ȃ�ӂ�)
															// ���l��A��������ł�����Ɛi�ނ܂Œ��i�����Ⴄ���ǂ܂�������
							SpdTgt = RMB_VEL_NORMAL;
							AngVelTgt = 0.0;		// ���i
							
						}else{
							setTgtVel_LineTrace(RMB_VEL_NORMAL);	// ���C���g���[�X
						}
					}else{
						setTgtVel_LineTrace(RMB_VEL_NORMAL);	// ���C���g���[�X
					}
				}
				
			}else{
				// �ړI�n�����܂��͐ԁA�������͌��܂��Ă��Ȃ��̎��̏���
				setTgtVel_LineTrace(RMB_VEL_NORMAL);	// ���C���g���[�X
				
				// �̂Ă��ゾ������c
				
				
				//fAdjustMachinePos = true;
				if((INROF_GOAL_RED == Goal) && (INROF_CROSS_RED == Cross)){	// �ړI�̃��C���z����
					if((MachinePos - PosCrossRed) < 40.0F){	// �ړI���C����܂Ői��
						SeqEnd = true;					// ��������~�ʒu
					}
				}else if((INROF_GOAL_YELLOW == Goal) && (INROF_CROSS_YELLOW == Cross)){	// �ړI�̃��C���z����
					if((MachinePos - PosCrossYellow) < 40.0F){	// �ړI���C����܂Ői��
						SeqEnd = true;					// ��������~�ʒu
					}
				}else{
				}
				
			}
			
			
			/* �J�ڏ��� */
			// �S�[���̌����_�ɗ����[
			if(	/*onCross() 	// 2��ڈȏ�A�����Ď̂Ă�Ƃ��A���C�����������Ă�Ƒ���o�����Ⴄ�̂Ń��C����ł��邱�Ƃ͌��m���Ȃ��B�����r���[�Ȉʒu�̎���A���������S�[���������Ă������Ȃ�������
				&&*/
				SeqEnd
			){	
				//adjustMachinePos();	// �p�x���A-PI�`PI�͈̔͂ɂȂ�̂Œ���
				
				ManageActionTansakuState_BallRelease ++;	// ���̏�����
				MachinePosActionManageSub = MachinePos;		// �S�[���̏ꏊ�ɂ������̈ʒu���o����
				
				ManageActionLineTraceDir = LineTraceDir;
				
#ifdef DEBUGSCI0
Sci0.print("Obj Goal Pos. \r\n");
#endif
			}
			break;
			
			
		case 1:	// �A�[���ړ�
			/* ���񏈗� */
			if(SeqInit){
				
				// �}�V���~�߂�
				SpdTgt = 0.0;
				AngVelTgt = 0.0;
#ifdef DEBUGSCI0
Sci0.print("Stop machine. \r\n");
#endif
				
			}
			/* �ʏ펞���� */
			// notthing to do (wait for arm move. )
			
			
			/* �J�ڏ��� */
			// �A�[���ړ���������
			if(UPPER_STATE_IDLE == InrofUpper.getState()){
				ManageActionTansakuState_BallRelease ++;	// ���̏�����
#ifdef DEBUGSCI0
Sci0.print("UPPER_STATE_BALL_TRASH_PRE_DONE\r\n");
#endif
			}
			break;
			
		case 2:	// �{�[���̂Ă�
			/* ���񏈗� */
			if(SeqInit){
				
				Rmb->outLedDispAscii("TRSH");
				
				// �{�[���̂Ă�v��
				InrofUpper.reqBallTrash();
				
			}
			/* �ʏ펞���� */
			// notthing to do
			
			
			/* �J�ڏ��� */
			// �{�[���̂Ă�
			if(UPPER_STATE_IDLE == InrofUpper.getState()){
				
				if(InrofUpper.numBlue()
				 || InrofUpper.numYellow()
				 || InrofUpper.numRed()){	// �܂��̂Ă���̂���
					
					if((INROF_GOAL_BLUE == Goal) && (InrofUpper.numBlue()==0)){	// ���̂Ă��B�͎̂ďI������B
						
					#ifdef DEBUGSCI0
						Sci0.print("trash bule fin.\r\n");
						#endif
						ManageActionTansakuState_BallRelease  = 3;	// �ԂƂ��A���F�̃S�[��������
					}else{
#ifdef DEBUGSCI0
						Sci0.print("trash same ball.\r\n");
#endif
						ManageActionTansakuState_BallRelease = 0;	// �܂��̂Ă�
					}
				}else{	// �����̂Ă���̂Ȃ�				
					// �t�B�[���h����Ȃ�
					if(FieldBall==0){
						if(INROF_GOAL_BLUE == Goal){
#ifdef DEBUGSCI0
							Sci0.print("to start. from blue\r\n");
							#endif
							ManageActionTansakuState_BallRelease = 3;	// �ԂƂ��A���F�̃S�[��������
						}else{
#ifdef DEBUGSCI0
							Sci0.print("to start. from yellow or red\r\n");
							#endif
							// �X�^�[�g��
							setState(INROF_TOSTART);
						}
					}else{
						ManageActionTansakuState_BallRelease = 4;	// �܂��T�����邽�߂ɏ������
						
					}
				}
			}
			break;
		case 3: // ����ԂƂ��̂ق�����
			
			this->SpdTgt = 0;
			
			if(INROF_LINEDIR_FORWARD == ManageActionLineTraceDir){	// �X�^�[�g�����痈����
				// ���̌����̋t������
				if(fabs(fabs(MachinePos.Th - (MachinePosActionManageSub.Th))-PI) < 0.1){
					if((MachinePosActionManageSub - MachinePos) > 200.0F){	// �{�[���̂Ă��Ƃ����烉�C���ڂ��Ă邾�낤�Ƃ��܂Ői��
						SeqEnd = true;
					}else{	// ���C�����܂Ői��
						this->SpdTgt = RMB_VEL_NORMAL;
						this->AngVelTgt = 0.0F;
					}
				}else{	
					// �t�����܂ŉ�]
					this->AngVelTgt = 0.15F * ( (MachinePosActionManageSub.Th - PI) - MachinePos.Th );  // �E180����]
				}
			}else{	// �t�B�[���h�����痈��
				// ���̌���+90��������
				
				if(fabs(fabs(MachinePos.Th - MachinePosActionManageSub.Th) - PI/2.0F) < 0.1F){
					if((MachinePosActionManageSub - MachinePos) > 200.0F){	// �{�[���̂Ă��Ƃ����烉�C���ڂ��Ă邾�낤�Ƃ��܂Ői��
						SeqEnd = true;
					}else{
						this->SpdTgt = RMB_VEL_NORMAL;
						this->AngVelTgt = 0.0F;
					}
				}else{
					// ����+1/2PI�����܂ŉ�]
					this->AngVelTgt =  0.15F * ( (MachinePosActionManageSub.Th + PI/2.0F) - MachinePos.Th );  // ��90����]
				}
			}
			//��]���x�ɐ���
			if(0.04<AngVelTgt){
				this->AngVelTgt = 0.04;
			}else if(-0.04>AngVelTgt){
				this->AngVelTgt = -0.04;
			}
			
			if(SeqEnd){
				if(InrofUpper.numBlue()
				 || InrofUpper.numYellow()
				 || InrofUpper.numRed()){
					
					ManageActionTansakuState_BallRelease = 0;	// �ԂƂ����F���̂Ă�
					
#ifdef DEBUGSCI0
					Sci0.print("next trash red or yellow. \r\n");
					#endif
				}else{	// �����̂Ă���̂Ȃ�
					
					// �t�B�[���h����Ȃ�
					// �X�^�[�g��
					setState(INROF_TOSTART);
#ifdef DEBUGSCI0
					Sci0.print("to start. \r\n");
					#endif
				}
			}
			
			break;
		case 4: 	// �{�[���̂ďI���������t�B�[���h�������ĒT���ĊJ
			/* ���񏈗� */
			if(SeqInit){
				InrofUpper.reqArmToCenter();	// �A�[�����[
			}
			/* �ʏ펞���� */
		
			// �ȉ�������
			
			this->SpdTgt = 0.0;
			
			// �I������ �� ���񐧌�
			if(INROF_GOAL_BLUE == Goal){
				if(INROF_LINEDIR_BACKWARD == ManageActionLineTraceDir){	// �t�B�[���h�����痈����
					// ���̌����Ƌt������
					if(fabs(fabs(MachinePos.Th - (MachinePosActionManageSub.Th))-PI) < 0.1){
						if((MachinePosActionManageSub - MachinePos) > 200.0F){	// �{�[���̂Ă��Ƃ����烉�C���ڂ��Ă邾�낤�Ƃ��܂Ői��
							SeqEnd = true;
						}else{
							this->SpdTgt = RMB_VEL_NORMAL;
							this->AngVelTgt = 0.0F;
						}
					}else{	
						// �t�����܂ŉ�]
						this->AngVelTgt = 0.15F * ( (MachinePosActionManageSub.Th - PI) - MachinePos.Th );  // �E180����]
					}
				}else{	// �X�^�[�g�����痈��
					// ���̌���-90��������
					if(fabs(fabs(MachinePos.Th - MachinePosActionManageSub.Th) - PI/2.0F) < 0.1F){
						if((MachinePosActionManageSub - MachinePos) > 200.0F){	// �{�[���̂Ă��Ƃ����烉�C���ڂ��Ă邾�낤�Ƃ��܂Ői��
							SeqEnd = true;
						}else{
							this->SpdTgt = RMB_VEL_NORMAL;
							this->AngVelTgt = 0.0F;
						}
					}else{
						// ����-1/2PI�����܂ŉ�]
						this->AngVelTgt = 0.15F * ( (MachinePosActionManageSub.Th - PI/2.0F) - MachinePos.Th );  // �E90����]
					}
				}
				
			}else{	// �� or ��
				// ���C����ɂ���̂ŁA���񂷂邾����ok
				if(((fabs(fabs(MachinePos.Th - (MachinePosActionManageSub.Th))-PI) < 0.1)&&(INROF_LINEDIR_BACKWARD == ManageActionLineTraceDir))		// �t�B�[���h���痈�����F���̌����̔��΂�����
					|| (INROF_LINEDIR_FORWARD == ManageActionLineTraceDir)){		// �X�^�[�g���痈�����F���̌����Ɠ���
					
					// ���A���C����Ȃ�B�ł��������B
					
					SeqEnd = true;
				
				}else{
					
					this->AngVelTgt = 0.15 * ( (MachinePosActionManageSub.Th - PI) - MachinePos.Th );  // �E180����]
			
				}
			}
	
			if(0.04<AngVelTgt){
				this->AngVelTgt = 0.04;
			}else if(-0.04>AngVelTgt){
				this->AngVelTgt = -0.04;
			}
			
			
			/* �J�ڏ��� */
			// �E��]����
			if( 
				SeqEnd
			){
				// �T���ĊJ
				setState(INROF_TANSAKU);
				
				// �A�[��������
				//InrofUpper.reqArmToCenter();
				
			}
			break;
		
		}
		break;
		
	}
	
	
	
	return false;
}

bool_t inrof2016::manageAction_goToStart(void){
	bool_t SeqInit = false;
	bool_t SubSeqInit = false;
	bool_t SeqEnd = false;
	
	
	if(ActionStateLast != ActionState){
		SeqInit = true;
	}
	ActionStateLast = ActionState;
	
	if(ManageActionToStartState_ToFinish_Last != ManageActionToStartState_ToFinish){
		SubSeqInit = true;
	}
	ManageActionToStartState_ToFinish_Last = ManageActionToStartState_ToFinish;
	
	
	switch(ActionState){
	case INROF_ACTION_IDLE:	// ������
		Sci0.print("To Start \r\n");
		ActionState = INROF_ACTION_LINE;
		
		InrofUpper.reqArmToCenter();	// �A�[���^�񒆂ɂ����
		
		ManageActionToStartState_ToFinish_Last = 0xff;
		ManageActionToStartState_ToFinish = 0x00;
		
		
		Rmb->outLedDispAscii("FIN.");
		
		break;
		
	case INROF_ACTION_LINE:	// ���C���g���[�X
		
		if(SeqInit){
			Sci0.print("LineTrace \r\n");
			StartTimeActionManage = getTime_ms();
		}
		
		setTgtVel_LineTrace(RMB_VEL_NORMAL);
		
		
		// �J�ڏ���
		if(MachinePos.Y < -200.0F){	// �X�^�[�g�]�[��������
			Sci0.print("StartZone in \r\n");
			ActionState = INROF_ACTION_TOFINISH;
			ManageActionToStartState_ToFinish = 0;
			ManageActionToStartState_ToFinish_Last = 0xFF;
		}
		
		break;
	case INROF_ACTION_TOFINISH:	// �X�^�[�g�]�[���߂���
		switch(ManageActionToStartState_ToFinish){
		case 0:	
			if(SubSeqInit){
				Sci0.print("StartZone \r\n");
				StartTimeActionManage = getTime_ms();
				
				MachinePosActionManageSub = MachinePos;
				
			}
			
			// ���̏�ŉ��
			this->SpdTgt = 0.0F;
			
			if(fabs(fabs(MachinePos.Th - (MachinePosActionManageSub.Th))-PI) < 0.1){
				this->AngVelTgt = 0.0F;
				SeqEnd = true;
			}else{	
				// �t�����܂ŉ�]
				this->AngVelTgt = 0.2F * ( (MachinePosActionManageSub.Th - PI) - MachinePos.Th );  // �E180����]
			}
			
			if(0.04<AngVelTgt){
				this->AngVelTgt = 0.04;
			}else if(-0.04>AngVelTgt){
				this->AngVelTgt = -0.04;
			}
			
			// �����I�����
			if(SeqEnd){
				
				// ����SE
				
				Inrof.Rmb->outNoteSingle(0, 69, 8);
				
				// �����o�N���Ă邩�킩��Ȃ����߁A�S������
				Inrof.Rmb->outLeds(0xFF);	// �S�_��
				Inrof.Rmb->outSchedulingLeds(0xFF);	// �S�_��
				Inrof.disableOutTyre();
				
				setState(INROF_IDLE);
			}
			break;
			
		default:
			break;
		}
		
		
		break;
	}	
	
	return false;
}

// ���C���Z���T�̃L�����u���[�V����
// ���A���̍��̊Ԃ��A���L�薳���̂������l�ɂ���eeprom�ɐݒ�
bool_t inrof2016::manageAction_calibLine(void){
	
	setState(INROF_IDLE);
	
	return false;
}





/*************
���C���g���[�X����悤�ɖڕW���x�ݒ�
���C�����Ȃ��A�������̓X�^�[�g�̌����_�߂����Ƃ��Ȃ璼�i
*************/
void inrof2016::setTgtVel_LineTrace(void){
	
	
	setTgtVel_LineTrace(VEL_NORMAL);
	
}
void inrof2016::setTgtVel_LineTrace(float SpdTgt){
	
	
	this->SpdTgt = SpdTgt;
	
	
	if(INROF_CROSS_START==Cross || INROF_CROSS_NONE == Cross){	// �X�^�[�g���C���𒴂����Ƃ��B
		this->AngVelTgt = 0;	
		PidCtrlLineTrace->resetStatus(getLineSensAd(2) - getLineSensAd(1));	//pid�̓��Z�b�g���Ă���
	}else{
		
		
		
		if((getLineSensAd(1)<LINESENS_THRESHOLD_CENTER)||(getLineSensAd(2)<LINESENS_THRESHOLD_CENTER)){
			// ����2�̃Z���T�����C������Ă�
			this->AngVelTgt = PidCtrlLineTrace->calc(getLineSensAd(2) - getLineSensAd(1));	// ���C���g���[�X
			if(fAdjustMachinePos){
				adjustMachinePos();
			}
		}else if((getLineSensAd(0)<LINESENS_THRESHOLD_SIDE)||(getLineSensAd(3)<LINESENS_THRESHOLD_SIDE)){
			// ���E2�̃Z���T�ǂ��炩���C������Ă�
			this->AngVelTgt = PidCtrlLineTrace->calc(getLineSensAd(3) - getLineSensAd(0));	// ���C���g���[�X
			this->AngVelTgt = this->AngVelTgt * 2.0F;
			this->SpdTgt = SpdTgt*0.5F;
		}else{
			// �ǂ���������ĂȂ��̂ł܂�����
			this->AngVelTgt = 0;	
			
			PidCtrlLineTrace->resetStatus(getLineSensAd(2) - getLineSensAd(1));	//pid�̓��Z�b�g���Ă���
		}
		/*
		if(	(getLineSensAd(1)<LINESENS_THRESHOLD_CENTER)||(getLineSensAd(2)<LINESENS_THRESHOLD_CENTER)
			||(getLineSensAd(0)<LINESENS_THRESHOLD_SIDE)||(getLineSensAd(3)<LINESENS_THRESHOLD_SIDE)
		){
			this->AngVelTgt = 0.5*(min(getLineSensAd(2), getLineSensAd(3)) - min(getLineSensAd(0),getLineSensAd(1)));
		}else{
			// �ǂ������ĂȂ��̂ł܂�����
			this->AngVelTgt = 0;	
		}
		*/
		
	}
	
}
/*************
�w�蔼�a�Ń^�[������ڕW���x�ݒ�
�T�v
 ���1/4�͂Ȃ��̒萔�B���ꂩ���Ȃ��ƋȂ���߂����Ⴄ�́B
�����F
 ���i���x [mm/s]
 ���񔼌a(���ō���], ���ŉE��]) [mm]
*************/
void inrof2016::setTgtVel_Turn(float SpdTgt, float Radius){
	
	this->SpdTgt = SpdTgt;							// [mm/s]
	//this->AngVelTgt = SpdTgt / Radius / (2*PI) /4.0F ;	// [rev/s]
	this->AngVelTgt = this->Spd / Radius / (2.0F*PI) /2.0F;	// [rev/s]
	
}








/*********************
���s����
�T�v�F
 PID����ŁA���i�����}��]������Duty���o�͂���B
 Duty�́Asigned�̃v���X�}�C�i�X�őO�i��i��\���B
**********************/
void inrof2016::ctrlRun(float SpdTgt_mmps, float AngVelTgt_rps, int16_t* RTireDutyTgt, int16_t* LTireDutyTgt){
	float RTireSpdTgt_rps, LTireSpdTgt_rps;
	
	// �e�^�C���̖ڕW���x
	//  �ڕW���i���x[mm/s]�Ɗp���x[r/s]����e�^�C���̖ڕW�p���x[r/s]�����߂�
	RTireSpdTgt_rps = SpdTgt_mmps/(PI*TIRE_DIA_MM) + AngVelTgt_rps * 4.0F * TIRE_TREAD_MM/TIRE_DIA_MM;
	LTireSpdTgt_rps = SpdTgt_mmps/(PI*TIRE_DIA_MM) - AngVelTgt_rps * 4.0F * TIRE_TREAD_MM/TIRE_DIA_MM;

	// �t�B�[�h�o�b�N����
	if(fMotorOut){
		// PID����
		*RTireDutyTgt += (int16_t)PidCtrlRTire->calc(RTireSpdTgt_rps - RTireSpd_rps);
		*LTireDutyTgt += (int16_t)PidCtrlLTire->calc(LTireSpdTgt_rps - LTireSpd_rps);
	}else{
		// ���[�^�[�o�͖������́APID����Ƀ��Z�b�g�������A�ڕW�f���[�e�B�[��0�Ƃ���B
		PidCtrlRTire->resetStatus(RTireSpdTgt_rps - RTireSpd_rps);
		PidCtrlLTire->resetStatus(LTireSpdTgt_rps - LTireSpd_rps);
		*RTireDutyTgt = 0;
		*LTireDutyTgt = 0;
	}
	
}



/*********************
���[�^�[�o��(PWM duty)
�T�v�F
 Duty����(+-255)�ɉ����Đ��]�t�]��؂�ւ���B
 fMotorOut��false�̎��A���[�^�[���t���[��ԂƂ���B
**********************/
void inrof2016::outTyres(int16_t RTireDutyTgt, int16_t LTireDutyTgt, bool_t fMotorOut){
	
	if(fMotorOut){
		// �����o�I�ɂ̓o�b�N�Ȃ̂ŋt�ɂ���
		Rmb->drivePwm(-LTireDutyTgt, -RTireDutyTgt);
	}else{
		Rmb->drivePwm(0, 0);	// �~�܂�I
	}
	
}



/*********************
���C���Z���T�ʒu���O���[�o�����W�n�Ŏ擾
�����F
	(�Ԃ�)*X
	(�Ԃ�)*Y
**********************/
void inrof2016::getLineSenseGlobalPos(position* Pos){
	float cosMTh = cosf(MachinePos.Th);
	float sinMTh = sinf(MachinePos.Th);
	
	Pos->X = MachinePos.X + (LINESENS_POS_X*cosMTh - LINESENS_POS_Y*sinMTh);
	Pos->Y = MachinePos.Y + (LINESENS_POS_X*sinMTh + LINESENS_POS_Y*cosMTh);
	
}







/*********************
�o�b�e���d�����擾��BattVoltage,fBattEmpty�ɃZ�b�g
�T�[�{����擾���������g���܂�
�����F
**********************/
void inrof2016::getBattVoltage(void){
	
	this->BattVoltage = InrofUpper.ServoPitch->getPresentVolts();	// �d���v���l�擾
	if(BATT_LOW>=(this->BattVoltage)){
		this->fBattEmpty = true;
	}else{
		this->fBattEmpty = false;
	}
	
	InrofUpper.ServoPitch->updatePresentVolts();	// ���̍X�V(�v���v��
	
}




/*********************
�������`�F�b�N
�����F�Ȃ�
�ߒl�F�������������������Ă�
��������������
�@�ʐM�o���Ă�
�@��M�v����������S����M����
**********************/
bool_t inrof2016::checkInitialize(void){
	bool_t Init = true;
	
	// �ʐM�`�F�b�N
	Init = Init && Rmb->isConnect();	// �ʐM�o���Ă� 
	Init = Init && Rmb->isRcvOnceAtLeast();	// ��M�v����������S����M����
	
	if(Init && (INROF_INITIALIZE==this->State)){
		this->State = INROF_IDLE;
	}
	
	
	return Init;
}




void inrof2016::drivePs3Con(void){
	int16_t outDutyR,outDutyL;	// -255 ~ 255
	
	
	
	if(ExtSci.isPs3ConAvailable()){
		
		// �|�����[�^�[�쓮on/off
		// START�{�^��(�����オ��)
		if(ExtSci.ControllerData.button.Start && !Ps3ConDataLast.button.Start){
			if(isCleaning()){
				stopClean();
			//	Inrof.ArmMotor->setDuty(200);
			}else{
				startClean();
			//	Inrof.ArmMotor->setDuty(-200);
			}
		}
		// Full���[�h��
		// ���{�^��(�����オ��)
		if(ExtSci.ControllerData.button.Circle && !Ps3ConDataLast.button.Circle){
			Rmb->setMode(ROI_MODE_PASSIVE);
			Rmb->setMode(ROI_MODE_FULL);
		}
		// �p�b�V�u���[�h��
		// Select�{�^��(�����オ��)
		if(ExtSci.ControllerData.button.Select && !Ps3ConDataLast.button.Select){
			Rmb->setMode(ROI_MODE_PASSIVE);
		}
		// ���Ƃ֋A��
		// ���{�^��(�����オ��)
		if(ExtSci.ControllerData.button.Square && !Ps3ConDataLast.button.Square){
			Rmb->sendCommand(ROI_OPCODE_FORCE_SEEKING_DOG);
		}
		
		
		// ����
		//-100 ~ 150
		GrabAngleTgt += ((int16_t)ExtSci.ControllerData.button.AnalogRight - (int16_t)ExtSci.ControllerData.button.AnalogLeft)/20;
		if(GrabAngleTgt>SERVOPOS_MAX_G){
			GrabAngleTgt = SERVOPOS_MAX_G;
		}else if(GrabAngleTgt<SERVOPOS_MIN_G){
			GrabAngleTgt = SERVOPOS_MIN_G;
		}
		InrofUpper.ServoGrab->setGoalPosition(GrabAngleTgt);
		
		// �����グ��
		//-700 ~ 380
		PitchAngleTgt += ((int16_t)ExtSci.ControllerData.button.AnalogUp - (int16_t)ExtSci.ControllerData.button.AnalogDown)/20;
		if(PitchAngleTgt>SERVOPOS_MAX_P){
			PitchAngleTgt = SERVOPOS_MAX_P;
		}else if(PitchAngleTgt<SERVOPOS_MIN_P){
			PitchAngleTgt = SERVOPOS_MIN_P;
		}
		InrofUpper.ServoPitch->setGoalPosition(PitchAngleTgt);
		
		
		// �A�[�����[�^�[
		// R2�FCW
		// L2�FCCW
		if(ExtSci.ControllerData.button.R1 && !Ps3ConDataLast.button.R1){
			Ps3ConArmMode = 1;
			
			if(ArmPosTgt>0){
				ArmPosTgt--;
			}
		}
		if(ExtSci.ControllerData.button.L1 && !Ps3ConDataLast.button.L1){
			Ps3ConArmMode = 1;
			
			if(ArmPosTgt<9){
				ArmPosTgt++;
			}
		}
		
		if((ExtSci.ControllerData.button.AnalogL2 > 5) || (Ps3ConDataLast.button.AnalogR2 > 5)){
			Ps3ConArmMode = 0;
		}
		
		switch(Ps3ConArmMode){
		case 0:	// �A�i���O�I�ɓ��������[�h
			InrofUpper.moveArmDuty((int16_t)ExtSci.ControllerData.button.AnalogL2 - (int16_t)ExtSci.ControllerData.button.AnalogR2);
			break;
		case 1:	
			InrofUpper.moveArmBallPos(ArmPosTgt);
			break;
		default:
			InrofUpper.moveArmDuty(0);
		}
		
		
		// ���E�X�e�B�b�N�ő���(+-128 -> +-255)
		//outDutyL = ((int16_t)0x0080 - (int16_t)ExtSci.ControllerData.button.LStickVertical)*2;
		//outDutyR = ((int16_t)0x0080 - (int16_t)ExtSci.ControllerData.button.RStickVertical)*2;
		
		
		int16_t Straight = ((int16_t)0x0080 - (int16_t)ExtSci.ControllerData.button.RStickVertical);
		int16_t Turn = ((int16_t)0x0080 - (int16_t)ExtSci.ControllerData.button.LStickHrizontal)/1.5;
		
		if((Straight<20) && (Straight>-20)){
			Straight = 0;
		}
		if((Turn<30) && (Turn>-30)){
			Turn = 0;
		}
		
		outDutyL = Straight - Turn;
		outDutyR = Straight + Turn;
		// �����ɕs����
		if((outDutyL<30) && (outDutyL>-30)){
			outDutyL = 0;
		}
		if((outDutyR<30) && (outDutyR>-30)){
			outDutyR = 0;
		}
		
		outTyres(outDutyR, outDutyL, fMotorOut);
		
		// ����̃{�^�������L��(�����オ�蔻��̂���)
		Ps3ConDataLast = ExtSci.ControllerData;
	}else{
		InrofUpper.ArmMotor->setDuty(0);
		outTyres(0, 0, 0);
	}
}










// 1ms���Ƃ̊��荞�ݐݒ�
// TMR0
// ���荞�ݎ��� 46 / (PCLK/1024) = 0.981msec
void inrof2016::setup1msInterrupt(void){
	uint16_t SamplingFreq = 1000;

	SYSTEM.PRCR.WORD = 0xA503u;		// �������݋���
	MSTP(TMR0) = 0;					// ���W���[���N��
	
	TMR0.TCR.BYTE = 0x00;
	TMR0.TCR.BIT.CCLR = 1;			// �R���y�A�}�b�`A�ŃJ�E���^�N���A
	TMR0.TCR.BIT.CMIEA = 1;			// �R���y�A�}�b�`A���荞�݋���(CMIAn)
	
	
	TMR0.TCSR.BYTE = 0x00;
	
	
	// �J�E���^
	uint16_t cnt = PCLK/1024/SamplingFreq;
	TMR0.TCNT = 0;
	TMR0.TCORA = cnt;		//�R���y�A�}�b�`�p(�g��Ȃ�)
	TMR0.TCORB = cnt+1;		//�R���y�A�}�b�`�p(�g��Ȃ�)
	
	// ���荞�݋���
	IEN(TMR0,CMIA0) = 1;
	IPR(TMR0,CMIA0) = 2;
	
	// �N���b�N�ݒ�
	TMR0.TCCR.BYTE = 0x00;
	TMR0.TCCR.BIT.CKS = 0x05;	// �N���b�N�I��			PCLK/1024
	TMR0.TCCR.BIT.CSS = 0x01;	// �N���b�N�\�[�X�I�� �����N���b�N
	
	// ����~�߂�̂͂��ꂵ���Ȃ��H
	//TMR0.TCCR.BIT.CKS = 0x00;
	//TMR0.TCCR.BIT.CSS = 0x00;	
}

/*************
TPU6���g�����t���[�J�E���^�B
���Ԍv���p
16bit, PCLK/1 -> 48MHz/64 = 750kHz
�^�C�}�J�E���^ : TPU6.TCNT
�I�[�o�[�t���[����� TPU6.TSR.BIT.TCFV������
*************/
void inrof2016::setupFreeCounter(void){
	MSTP(TPU6) = 0;					// ���W���[���N��
	TPU6.TCR.BIT.TPSC = 3;	// PCLK/4
	TPU6.TCR.BIT.CKEG = 0;	// �����オ��G�b�W�ŃJ�E���g
	TPU6.TCR.BIT.CCLR = 0;	// �N���A�Ȃ�
	TPU6.TMDR.BIT.MD = 0;	// �ʏ퓮��
	
	TPUB.TSTR.BIT.CST6 = 1;	// �J�E���g�J�n
	
}


//extern void isr1ms(void);
// ���荞�݊֐��̐ݒ�
#pragma interrupt (Excep_TMR0_CMIA0(vect=VECT(TMR0,CMIA0), enable))
void Excep_TMR0_CMIA0(void){
	
	Inrof.isr1ms();
	//isr1ms();
	
}