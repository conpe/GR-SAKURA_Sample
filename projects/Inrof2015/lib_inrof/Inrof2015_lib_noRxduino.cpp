
#include "Inrof2015_lib_noRxduino.h"

inrof2015 Inrof;
//SDMMC MMC;

inrof2015::inrof2015(void){
//	SdWav = new sd_wav;
	
	fBattEmpty = false;
	fCnt200 = false;
	fJudgeColor = true;
	
	SpdTgt = 0;
	AngVelTgt = 0;
	
	
	MachinePos.X = 0.0;
	MachinePos.Y = -115.0;
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
	
	
	BallColorFixed = INROF_YELLOW;	// ���R�{�[����u���ɍs��(�ǂ��̃S�[���ł�����)
	
	State = INROF_IDLE;
	ActionState = INROF_ACTION_IDLE;
	
	fFinishManageAction = false;
	fFinishAction = false;
	
	fAdjustMachinePos = true;
	
	Time_msec = 0;
}


void inrof2015::begin(void){
	
	GrEx.begin();		// �g���������
	GrExOled.begin();	// �g�����OLED������

// �I�u�W�F�N�g�p��
	// �G���R�[�_
	EncR = GrEx.Enc1;
	EncL = GrEx.Enc0;
	
	// ���[�^�[
	MotorR = GrEx.Motor1;
	MotorL = GrEx.Motor0;
	
	// I2C
	
	// SCI
	// �V���A���T�[�{
	ServoF = new futaba_cmd_servo(1, &Sci5, 115200);
	ServoR = new futaba_cmd_servo(2, &Sci5, 115200);
	
	// �����Z���T
	PsdLeft = new inrof2015_psd(&I2C0, PSDSENS_LEFT_ADRS, PSDSENS_LEFT_POS_X, PSDSENS_LEFT_POS_Y, PSDSENS_LEFT_POS_TH);
	PsdLeftSide = new inrof2015_psd(&I2C0, PSDSENS_LEFTSIDE_ADRS, PSDSENS_LEFTSIDE_POS_X, PSDSENS_LEFTSIDE_POS_Y, PSDSENS_LEFTSIDE_POS_TH);
	PsdFront = new inrof2015_psd(&I2C0, PSDSENS_FRONT_ADRS, PSDSENS_FRONT_POS_X, PSDSENS_FRONT_POS_Y, PSDSENS_FRONT_POS_TH);
	PsdRightSide = new inrof2015_psd(&I2C0, PSDSENS_RIGHTSIDE_ADRS, PSDSENS_RIGHTSIDE_POS_X, PSDSENS_RIGHTSIDE_POS_Y, PSDSENS_RIGHTSIDE_POS_TH);
	PsdRight = new inrof2015_psd(&I2C0, PSDSENS_RIGHT_ADRS, PSDSENS_RIGHT_POS_X, PSDSENS_RIGHT_POS_Y, PSDSENS_RIGHT_POS_TH);
	
	// �J���[�Z���T
	ColorSens = new hamamatsu_S11059(&I2C0);
	// MPU6050
	//Mpu = new invensense_MPU(&I2C0, 0x68);
	
// ������
	//�G���R�[�_�v���J�n
	EncR->startCnt();
	EncL->startCnt();
	
	
	ServoF->begin();
	ServoR->begin();
	// �Z���T�ސݒ�
	PsdLeftSide->begin();
	PsdLeft->begin();
	PsdFront->begin();
	PsdRightSide->begin();
	PsdRight->begin();
	PsdLeft->setMedianFilter(9);
	PsdLeftSide->setMedianFilter(9);
	PsdFront->setMedianFilter(9);
	PsdRightSide->setMedianFilter(9);
	PsdRight->setMedianFilter(9);
	//�J���[�Z���T
	ColorSens->begin();
	setPinMode(COLORSENS_PORT_LED, PIN_OUTPUT);
	outPin(COLORSENS_PORT_LED, 1);
	ColorSens->wakeup();	// ���W���[���N����
	ColorSens->startMeasurement();	// �v���J�n
	//Mpu->setup();
	
	
	// AD�ϊ��J�n
	GrEx.startAD();
	
	
	// ����p
	RateLimitSpdTgt = new ctrl_ratelimiter_abs(SAMPLETIME_CTRL_S, 500, 1000);			// 500 mm/s/s
	RateLimitAngVelTgt = new ctrl_ratelimiter_abs(SAMPLETIME_CTRL_S, 0.5);	// 1 rev/s/s	
	
	
	// PSD�Z���T��I2C�A�h���X����������
	//Oled.println("%d", PsdLeft->setI2cAddress(0x70, PD0));
	//while(1);
	
	setup1msInterrupt();	//1ms���Ɗ��荞�ݐݒ�
}

void inrof2015::step(void){
	if(fExeReq1ms){
		step1ms();
		fExeReq1ms = false;
	}
	if(fExeReq10ms){
		step10ms();
		fExeReq10ms = false;
	}
	if(fExeReq200ms){	// 10ms�̂���ɂ��Ȃ��ƁA�J���[�Z���T��I2C�ł��Ȃ�(new���s����)
		step200ms();
		fExeReq200ms = false;
	}
}

// 1ms���ƂɎ��s����֐�
void inrof2015::isr1ms(void){
	
	Time_msec++;
	
	fExeReq1ms = true;
	
	if(Cnt200ms<199){
		Cnt200ms++;
	}else{
		fCnt200 = 1;
		Cnt200ms = 0;
		fExeReq200ms = true;
	}
	
	
	if(Cnt10ms<9){
		Cnt10ms ++;
	}else{
		Cnt10ms = 0;
		fExeReq10ms = true;
	}
	
	
	
}

void inrof2015::step1ms(void){
	int32_t RTireDutyTgt, LTireDutyTgt;
	
	// test
	// 1ms���ƂɃ`�J�`�J
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

	/***********
	 ���͏���
	************/
	// ���C���Z���T�X�V
	updateLineSense();
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
	
	// PSD�Z���T/�}�V���ʒu�X�V
	PsdLeft->setMachinePos(&MachinePos);
	PsdLeftSide->setMachinePos(&MachinePos);
	PsdFront->setMachinePos(&MachinePos);
	PsdRightSide->setMachinePos(&MachinePos);
	PsdRight->setMachinePos(&MachinePos);
	
	// ���C���Z���T��_����
	IsCrossing = judgeCross();
	// ���s��������
	judgeRunOrient();
	// �{�[������������
	IsBallFinding = judgeBallFind();
	
	//SpdTgt = 200;
	//AngVelTgt = 0.000005*(getLineSensAd(1) - getLineSensAd(2));	// ���C���g���[�X
	
	// ��ԊǗ�
	// ����Ǘ�
	// ���� (SpdTgt��AngVelTgt�����肷��)
	manageState();
	
	/***********
	 �o��
	************/
	//SpdTgt = 200;
	//AngVelTgt = 0.000005*(getLineSensAd(1) - getLineSensAd(2));	
	
	
	//RateLimitSpdTgt->setRate(500, 1000);	// �������͋��߂ŁB
	//RateLimitAngVelTgt->setRate(0.5);
	RateLimitSpdTgt->limitRate(&SpdTgt);
	RateLimitAngVelTgt->limitRate(&AngVelTgt);
	
	// ���x����ŖڕW���x�𖞂����f���[�e�B�[��v�Z
	ctrlRun(SpdTgt, AngVelTgt, &RTireDutyTgt, &LTireDutyTgt);	// ���x����
	
	// �f���[�e�B�[�o��
	// �o�b�e���Ȃ��Ƃ��͏o�͂��Ȃ�
	if(!fBattEmpty){	// �o�b�e���オ���ĂȂ����
		outTyres(RTireDutyTgt, LTireDutyTgt);				// �f���[�e�B�[�o��
	}else{
		fMotorOut = false;
		outTyres(0, 0);	//���[�^�[off
	}
}
	
void inrof2015::step10ms(void){
	
	
	// �Z���T�X�V
	PsdLeft->measure();
	PsdLeftSide->measure();
	PsdFront->measure();
	PsdRightSide->measure();
	PsdRight->measure();
	
	//Mpu->measure();
	
	// �T�[�{�f�[�^�X�V
	ServoF->updatePresentPosition();	// ���[���[ (����ĂȂ��Ă�getpresentposition�ɂ͉e���Ȃ�)
	ServoR->updatePresentPosition();	
	
	outPin(GR_LED3, readPin(OLED_SW3));
}

void inrof2015::step200ms(void){
	// �{�[���F�X�V
	updateBallColor();
	ColorSens->updateSens();
	
	// �o�b�e�����X�V
	getBattVoltage();
}

/*********************
���C���Z���T�A�b�v�f�[�g
�T�v�F
 �G���R�[�_�l���A�b�v�f�[�g
�����F

**********************/
void inrof2015::updateLineSense(void){
	static bool_t fLineSensOn;							// LED���点�Ă�t���O
	static uint16_t LineSensAdOn[LINESENS_NUM_SENSE];	// ���C���Z���TAD�l LED on��
	static uint16_t LineSensAdOff[LINESENS_NUM_SENSE];	// ���C���Z���TAD�l LED off��
	
	
	if(fLineSensOn){	// on������
		for(uint8_t i=0; i<LINESENS_NUM_SENSE; i++){
			LineSensAdOn[i] = GrEx.getAD((grex_ad_ch)i);
		}
		outPin(LINESENS_PORT_LED, 0); 
		fLineSensOn = 0;
	}else{
		for(uint8_t i=0; i<LINESENS_NUM_SENSE; i++){
			LineSensAdOff[i] = GrEx.getAD((grex_ad_ch)i);
		}
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
 �G���R�[�_�l���A�b�v�f�[�g
�����F

**********************/
void inrof2015::updateEncoder(void){
	
	REncCntDiff = EncR->getCntDiff();
	LEncCntDiff = EncL->getCntDiff();
}

/*********************
�^�C����ԃA�b�v�f�[�g
�T�v�F
 �G���R�[�_�l����p�x�Ɗp���x���A�b�v�f�[�g
�����F

**********************/
void inrof2015::updateTyre(void){
	
	// �^�C���p���x[rps]
	RTireSpd_rps = -(float)REncCntDiff/ENC_CPR/GEAR_RATIO/SAMPLETIME_CTRL_S;
	LTireSpd_rps =  (float)LEncCntDiff/ENC_CPR/GEAR_RATIO/SAMPLETIME_CTRL_S;
	
	// �^�C���p�x[rev]
	RTirePos_r = RTirePos_r + (float)REncCntDiff/ENC_CPR/GEAR_RATIO;
	LTirePos_r = LTirePos_r - (float)LEncCntDiff/ENC_CPR/GEAR_RATIO;
	
}
	
/*********************
���Ȉʒu�X�V
**********************/
void inrof2015::updateOdmetry(void){
	CalcOdmetry.REncCntDiff[1] = CalcOdmetry.REncCntDiff[0];
	CalcOdmetry.REncCntDiff[0] = -REncCntDiff;
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
�{�[���̐F�X�V
�T�v�F
 �F�Z���T�̊e�F�̋����F�Ƃ���B
 �����ȂƂ���INROF_UNKNOWN�Ƃ���B
 �Z���T�X�V�Ɠ������ČĂ�łˁB(LED���点��^�C�~���O�̓s��)
**********************/
void inrof2015::updateBallColor(void){
	int16_t BallColorValDiff[3];
	
	if(fJudgeColor){
		
		if(BallColorLed){
			ColorSens->getLuminance(&BallColorVal[0][1], &BallColorVal[1][1], &BallColorVal[2][1]);
		}else{
			ColorSens->getLuminance(&BallColorVal[0][0], &BallColorVal[1][0], &BallColorVal[2][0]);
		}
		// ���邢���ƈÂ����̍����Ƃ�
		BallColorValDiff[0] = abs(BallColorVal[0][1] - BallColorVal[0][0]);
		BallColorValDiff[1] = abs(BallColorVal[1][1] - BallColorVal[1][0]);
		BallColorValDiff[2] = abs(BallColorVal[2][1] - BallColorVal[2][0]);
		
		if((BallColorValDiff[0]>50)||(BallColorValDiff[1]>50)||(BallColorValDiff[2]>50)){
			
			if(BallColorValDiff[0] > BallColorValDiff[1]){
				if(BallColorValDiff[0] > BallColorValDiff[2]){
					BallColor = INROF_RED;	// Red
				}else{
					BallColor = INROF_BLUE;	// Blue
				}
			}else if(BallColorValDiff[1] > BallColorValDiff[2]){
				if(BallColorValDiff[1] > BallColorValDiff[0]){
					BallColor = INROF_YELLOW;	// Green
				}else{
					BallColor = INROF_RED;	// Red
				}
			}else{
				if(BallColorValDiff[2] > BallColorValDiff[1]){
					BallColor = INROF_BLUE;	// Blue
				}else{
					BallColor = INROF_YELLOW;	// Green
				}
			}
			outPin(GR_LED1, 1);
		}else{
			outPin(GR_LED1, 0);
			BallColor = INROF_NONE;
		}
		
		if(BallColorLed){
			
			outPin(COLORSENS_PORT_LED, 0);
			BallColorLed = false;
		}else{
			
			outPin(COLORSENS_PORT_LED, 1);
			BallColorLed = true;
		}
	}else{
		
		BallColor = INROF_NONE;
		outPin(COLORSENS_PORT_LED, 0);
		BallColorLed = false;
	}
	
}



/*********************
��_����
�T�v�F��_�ɗ������Ƃ𔻒肷��
		�V������_�ɗ������Ɍ�_�C���f�b�N�X���X�V���Atrue��Ԃ�
		����ȊO�ł�false��Ԃ��B
�����F
�ߒl�F�V���������_�ɗ���
**********************/
bool_t inrof2015::judgeCross(void){
	bool_t IsNewCross = false;
	position LineSensPos;
	
	// �����_����
	if((getLineSensAd(0)<LINESENS_THRESHOLD_SIDE) && (getLineSensAd(3)<LINESENS_THRESHOLD_SIDE)){
		
		if(100.0<(MachinePos - LastFindCrossPos)){	// �O���������100mm�ȏ㉓���Ƃ��낾������(�A������h�~)
			LastFindCrossPos = MachinePos;	// �����������ꏊ�o���Ă���
			IsNewCross = true;
			//�ǂ��̌����_�����H
			getLineSenseGlobalPos(&LineSensPos);	// ���C���Z���T�̃O���[�o���Ȉʒu���擾
			if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossStart)){		// ���C���Z���T�ƁA�t�B�[���h��̌����_�̈ʒu���߂����
				Cross = INROF_CROSS_START;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossRed)){
				Cross = INROF_CROSS_RED;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossYellow)){
				Cross = INROF_CROSS_YELLOW;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR>(LineSensPos - PosCrossBlue)){
				Cross = INROF_CROSS_BLUE;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR>(LineSensPos - PosCrossBallFieldBegin)){
				Cross = INROF_CROSS_BALLFIELD_BEGIN;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD_FAR>(LineSensPos - PosCrossBallFieldEnd)){
				Cross = INROF_CROSS_BALLFIELD_END;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossGoalRed)){
				Cross = INROF_CROSS_GOAL_RED;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossGoalYellow)){
				Cross = INROF_CROSS_GOAL_YELLOW;
			}else if(FIELD_INROF_CROSS_JUDGE_THRESHOLD>(LineSensPos - PosCrossGoalBlue)){
				Cross = INROF_CROSS_GOAL_BLUE;
			}else{
				// �ǂ��Ƃ��߂��Ȃ��̂Ƀ��C����H�H�H�H
				Cross = INROF_CROSS_NONE;
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
bool_t inrof2015::judgeBallFind(void){
	
	
	if(true==BallFindEnable){	// enable�̂Ƃ��B
		//if((INROF_CROSS_BALLFIELD_BEGIN == Cross) || (INROF_CROSS_BALLFIELD_END == Cross)){	// �{�[���]�[����
			if( (1500.0 > MachinePos.Y) && ( (PI/6 > fabs(MachinePos.getNormalizeTh()-PI/2))||(PI/6 > fabs(MachinePos.getNormalizeTh()+PI/2) ) ) ){		// �{�[���]�[����, �قڏ㉺�����Ă���
				
				// �����Z���T
				if( (BALLFIND_MAXDISTANCE_SIDE > PsdLeftSide->getDistance())		// �w�肵���������߂�
					&& (PSD_MIN < PsdLeftSide->getDistance())						// �L���͈�
					&& PsdLeftSide->isObjectBallArea()								// �{�[���̓t�B�[���h��(�Z���T�l�擾�ł��ĂȂ����false�Ԃ�)
				){
					return true;		
				}
				// �E���Z���T
				if( (BALLFIND_MAXDISTANCE_SIDE > PsdRightSide->getDistance())		// �w�肵���������߂�
					&& (PSD_MIN < PsdRightSide->getDistance())						// �L���͈�
					&& PsdRightSide->isObjectBallArea()								// �{�[���̓t�B�[���h��(�Z���T�l�擾�ł��ĂȂ����false�Ԃ�)
				){
					return true;		
				}
				// ���N���X�Z���T
				if( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdLeft->getDistance())		// �w�肵���������߂�
					&& (PSD_MIN < PsdLeft->getDistance())						// �L���͈�
					&& PsdLeft->isObjectBallArea()								// �{�[���̓t�B�[���h��(�Z���T�l�擾�ł��ĂȂ����false�Ԃ�)
				){
					return true;		
				}
				// �E�N���X�Z���T
				if( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdRight->getDistance())		// �w�肵���������߂�
					&& (PSD_MIN < PsdRight->getDistance())						// �L���͈�
					&& PsdRight->isObjectBallArea()								// �{�[���̓t�B�[���h��(�Z���T�l�擾�ł��ĂȂ����false�Ԃ�)
				){
					return true;		
				}
				
			}
		//}
	}
	
	return false;
}
	
void inrof2015::judgeRunOrient(void){
	
	// �قڊm���ȂƂ������X�V����
	if(MachinePos.Y < 1400){
		if((MachinePos.X < 100) && (MachinePos.X > -100)){	// ���̃��C����ɂ���
			if((MachinePos.getNormalizeTh() > PI/4)&&(MachinePos.getNormalizeTh() < PI/4*3)){
				LineTraceDir = INROF_LINEDIR_FORWARD;
			}else if((MachinePos.getNormalizeTh() < -PI/4)&&(MachinePos.getNormalizeTh() > -PI/4*3)){
				LineTraceDir = INROF_LINEDIR_BACKWARD;
			}
		}else if((MachinePos.X < 100) && (MachinePos.X > -100)){	// �E�̃��C����ɂ���
			if((MachinePos.getNormalizeTh() > PI/4)&&(MachinePos.getNormalizeTh() < PI/4*3)){			// �k�����Ă�
				LineTraceDir = INROF_LINEDIR_BACKWARD;
			}else if((MachinePos.getNormalizeTh() < -PI/4)&&(MachinePos.getNormalizeTh() > -PI/4*3)){	// ������Ă�
				LineTraceDir = INROF_LINEDIR_FORWARD;
			}
		}
	}else{	// ��̃��C����ɂ���
		
		if((MachinePos.getNormalizeTh() < PI/4)&&(MachinePos.getNormalizeTh() > -PI/4)){			// �������Ă�
			LineTraceDir = INROF_LINEDIR_FORWARD;
		}else if((MachinePos.getNormalizeTh() > PI/4*3)||(MachinePos.getNormalizeTh() < -PI/4*3)){	// �������Ă�
			LineTraceDir = INROF_LINEDIR_BACKWARD;
		}
	}
	
}


// �}�V���ʒu�C��
// �����_���W���g��

void inrof2015::adjustMachinePos(void){
	
	if(IsCrossing){	// �����_�������Ȃ�
		switch(Cross){
		case INROF_CROSS_YELLOW:	// ���F�]�[���̌����_�Ȃ�
			if(ORIENT_NORTH==MachineOrientation){			// �k�����Ă�
				MachinePos.X = PosCrossYellow.X;	
				MachinePos.Y = PosCrossYellow.Y - LINESENS_POS_X - 9;
				MachinePos.Th = PI/2;
		
				outPin(GR_LED2, 1);
				
			}else if(ORIENT_SOUTH==MachineOrientation){	// ������Ă�
				MachinePos.X = PosCrossYellow.X;
				MachinePos.Y = PosCrossYellow.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;
		
				outPin(GR_LED2, 1);
			}
			break;
		case INROF_CROSS_BLUE:	// �]�[���̌����_�Ȃ�
			if(ORIENT_WEST==MachineOrientation){			// �������Ă�
				MachinePos.X = PosCrossBlue.X + LINESENS_POS_X + 9;	
				MachinePos.Y = PosCrossBlue.Y;
				MachinePos.Th = PI;
		
				outPin(GR_LED2, 1);
			}else if(ORIENT_EAST==MachineOrientation){			// �������Ă�
				MachinePos.X = PosCrossBlue.X - LINESENS_POS_X - 9;	
				MachinePos.Y = PosCrossBlue.Y;
				MachinePos.Th = 0;
		
				outPin(GR_LED2, 1);
			}
			break;
		case INROF_CROSS_RED:	// �ԃ]�[���̌����_�Ȃ�
			if(ORIENT_SOUTH==MachineOrientation){			// ������Ă�
				MachinePos.X = PosCrossRed.X;	
				MachinePos.Y = PosCrossRed.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;
		
				outPin(GR_LED2, 1);
			}else if(ORIENT_NORTH==MachineOrientation){			// �k�����Ă�
				MachinePos.X = PosCrossRed.X;	
				MachinePos.Y = PosCrossRed.Y - LINESENS_POS_X - 9;
				MachinePos.Th = PI/2;
		
				outPin(GR_LED2, 1);
				
			}
			break;
		case INROF_CROSS_BALLFIELD_BEGIN:	// �{�[���]�[���J�n�̌����_�Ȃ�
			if(ORIENT_NORTH==MachineOrientation){			// �k�����Ă�
				MachinePos.X = PosCrossBallFieldBegin.X;	
				MachinePos.Y = PosCrossBallFieldBegin.Y - LINESENS_POS_X - 9;
				MachinePos.Th = PI/2;
		
				outPin(GR_LED2, 1);
			}else if(ORIENT_SOUTH==MachineOrientation){	// ������Ă�
			//	MachinePos.X = PosCrossBallFieldBegin.X;
			//	MachinePos.Y = PosCrossBallFieldBegin.Y + LINESENS_POS_X + 9;
			//	MachinePos.Th = -PI/2;
		
			//	outPin(GR_LED2, 1);
			}
			break;
		case INROF_CROSS_BALLFIELD_END:	// �{�[���]�[���I���̌����_�Ȃ�
			if(ORIENT_SOUTH==MachineOrientation){	// ������Ă�
				MachinePos.X = PosCrossBallFieldEnd.X;
				MachinePos.Y = PosCrossBallFieldEnd.Y + LINESENS_POS_X + 9;
				MachinePos.Th = -PI/2;
		
				outPin(GR_LED2, 1);
			}
			break;
		}
	}else{
		outPin(GR_LED2, 0);
	}
	
}




//��ԊǗ�
int8_t inrof2015::manageState(void){
	
	// �Ђ�����{�[���Ƃ��ăS�[���������������獡�̂Ƃ�����Ȃ��Ȃ��B
	
	fFinishManageAction = manageAction(State);
	return 0;
}
	
// ����Ǘ�
// ����
//  �ړI�n�Ƃ��T�����[�h�Ƃ�
bool_t inrof2015::manageAction(inrof_state State){
	
	
	switch(State){
	case INROF_IDLE:
		SpdTgt = 0;
		AngVelTgt = 0;
		break;
	case INROF_TANSAKU:
		return manageAction_tansaku();
		break;
	case INROF_TOGOAL:
		return manageAction_goToGoal(BallColorFixed);
		break;
	case INROF_TOSTART:
		return manageAction_goToStart();
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
bool_t inrof2015::manageAction_tansaku(void){
	
	
	// ��ԑJ��(��_)
	if(IsCrossing){
		if(Cross==INROF_CROSS_BALLFIELD_END){	// �t�B�[���h�̒[�ɂ����炿����Ɖ�������90����]����
			// 180����]����
			ActionState = INROF_ACTION_TURN;
			ManageActionTansakuState_BallFind_turn = 0;	// ������Ɖ�����
			MachinePosActionManageSub = MachinePos;
		}else if(Cross==INROF_CROSS_BALLFIELD_BEGIN){
			if((INROF_ACTION_LINE == ActionState) && (fabs(MachinePos.getNormalizeTh() - PI/2)) < 0.5 ){	// ���C���g���[�X���Ăď�ނ��Ă鎞�Ƀ{�[���t�B�[���h�̏�̐����z����
				ActionState = INROF_ACTION_TURN;
				ManageActionTansakuState_BallFind_turn = 1;
				MachinePosActionManageSub = MachinePos;
			}
		}else if(Cross==INROF_CROSS_START){
			if((INROF_ACTION_LINE == ActionState) && (fabs(MachinePos.getNormalizeTh() + PI/2)) < 0.5 ){	// ���C���g���[�X���Ăĉ��ނ��Ă鎞�ɃX�^�[�g�]�[��	// �����g��Ȃ�
				ActionState = INROF_ACTION_TURN;
				MachinePosActionManageSub = MachinePos;
			}
		}
	}
	
	// ��ԑJ��(�{�[�������鏈��)
	if(IsBallFinding){	// �{�[��������
		ActionState = INROF_ACTION_BALLGET;		// �{�[���擾�X�e�[�g�ֈڍs
		BallFindEnable = false;					// �{�[���T���I��
		MachinePosActionManage = MachinePos;	// 
		MachinePosActionManageSub = MachinePos;
		ManageActionTansakuState_BallFind = 0;	// �{�[���T���p�X�e�[�g������
		// �ǂ̃Z���T�H
		BallFindDistance = 999;
		if( (BALLFIND_MAXDISTANCE_SIDE > PsdLeftSide->getDistance()) && PsdLeftSide->isObjectBallArea() ){
			if(BallFindDistance > PsdLeftSide->getDistance()){	// ���̃Z���T���߂���΂����K�p
				BallDir = 1;	// �����
				BallFindDistance = PsdLeftSide->getDistance();
			}
		}
		if( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdRight->getDistance()) && PsdRight->isObjectBallArea() ){
		// �������A�E�N���X�������獶��肷��
			if(BallFindDistance > PsdRight->getDistance()){	// ���̃Z���T���߂���΂����K�p
				BallDir = 1;
				BallFindDistance = PsdRight->getDistance();
			}
		}
		if( (BALLFIND_MAXDISTANCE_SIDE > PsdRightSide->getDistance()) && PsdRightSide->isObjectBallArea() ){
			if(BallFindDistance > PsdRightSide->getDistance()){
				BallDir = 0;	// �E
				BallFindDistance = PsdRightSide->getDistance();
			}
		}
		if ( (BALLFIND_MAXDISTANCE_SIDECLOSS > PsdLeft->getDistance()) && PsdLeft->isObjectBallArea() ){
		// �E�����A���N���X��������E��肷��
			if(BallFindDistance > PsdLeft->getDistance()){	
				BallDir = 0;	// �E
				BallFindDistance = PsdLeft->getDistance();
			}
		}
	}
	
	
	
	// ����
	switch(ActionState){
	case INROF_ACTION_IDLE:		// 
		ActionState = INROF_ACTION_LINE;
		setTgtVel_LineTrace();
		ManageActionTansakuState_BallFind = 0;
		break;
	case INROF_ACTION_TURN:		// 180�����
		//adjustMachinePos();	// ���C���g���[�X�ł͂Ȃ����ǌ����_�Ŕ��肷�邱�Ƃ������̂�
		switch(ManageActionTansakuState_BallFind_turn){
		case 0:	// ������Ɖ�����
			if(100.0 < (MachinePos-MachinePosActionManageSub)){	// 100mm�ȏ㗣�ꂽ
				ManageActionTansakuState_BallFind_turn = 1;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{		// �����܂ŉ�����
				this->SpdTgt = -100;		// [mmps]
				this->AngVelTgt = 0;	// [rps]
				BallFindEnable = false;
			}
			break;
		case 1:	//180�x����
			if(0.05 > fabs(MachinePos.Th - (MachinePosActionManageSub.Th + PI))){
				//fFinishAction = 1;				// 180��������̂Ŏ��̃X�e�[�g��
				ManageActionTansakuState_BallFind_turn = 0;
				ActionState = INROF_ACTION_LINE;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				BallFindEnable = true;
			}else{		// 180�����܂ŉ�]
				this->SpdTgt = 0;		// [mmps]
				this->AngVelTgt = 0.2 * ((MachinePosActionManageSub.Th + PI) - MachinePos.Th);	// [rps]
				BallFindEnable = false;
			}
			break;
		}
		break;
	case INROF_ACTION_BALLGET:	// �{�[���������̂Œ͂܂���
		switch(ManageActionTansakuState_BallFind){
		case 0:		// �{�[���Ȃ��Ȃ�܂Ŕ��Ό���(���Ƃ��Ɩڂ̑O�Ƀ{�[�������������p)
			if((BallFindDistance+30 < PsdFront->getDistance()) || (0>PsdFront->getDistance())){ 	// �w��l���傫�� or �����l
				ManageActionTansakuState_BallFind = 1;				// 20��������̂Ŏ��̃X�e�[�g��
				this->SpdTgt = 0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{
				this->SpdTgt = 0;		// [mmps]
				if(BallDir){	// �{�[���͍�����
					this->AngVelTgt = -0.05;	// �E��][rps]
				}else{
					this->AngVelTgt = 0.05;	// [rps]
				}
			}
			
			break;
			
		case 1:		// �{�[��������܂ŉ��
		
			if(fabs(MachinePos.Th - MachinePosActionManageSub.Th) > PI/2){		// 90�x�ȏ��������߂ĒT���ĊJ
				ManageActionTansakuState_BallFind = 40;
		
			//}else if(( BallFindDistance+30 > PsdFront->getDistance()) && PsdFront->isObjectBallArea()){		//�w��l��菬���� and �L���͈�
			}else if(( (BallFindDistance+30) > PsdFront->getDistance()) && (0 < PsdFront->getDistance())){		// �{�[��������(�w��l��菬���� and �L���͈�)
			//}else if(( BallFindDistance+30 > PsdFront->getDistance())){		//�w��l��菬���� -> �����l�������甽�������Ⴄ�̂Ń{�c
					// �Ώۂ̃{�[���ʒu��菬������Ό������Ɣ���B������Ɨ]�T������+30mm���Ă�
				ManageActionTansakuState_BallFind = 2;				// �{�[���������̂Ŏ��̃X�e�[�g��
				BallFindTh = MachinePos.Th;
			}else{
				if(BallDir){	// �{�[���͍�����
					//����
					this->SpdTgt = 0;		// [mmps]
					this->AngVelTgt = 0.04;	// [rps]
				}else{			// �{�[���͉E����
				//-����
					this->SpdTgt = 0;		// [mmps]
					this->AngVelTgt = -0.04;	// [rps]
				}
			}
			break;
			
			
		case 2:		// �t�����g�Z���T�Ń{�[���������B�{�[���Ȃ��Ȃ�܂ōX�ɉ���
			if(fabs(MachinePos.Th - MachinePosActionManageSub.Th) > (PI*2/3)){		// 120�x�ȏ��������߂ĒT���ĊJ
				ManageActionTansakuState_BallFind = 40;
		
			}else if((BallFindDistance+30 < PsdFront->getDistance()) || (0>PsdFront->getDistance())){ 	// �w��l��艓�� or �����l(�����Ȃ����炢����)
				ManageActionTansakuState_BallFind = 3;				// �{�[���Ȃ��Ȃ����̂Ŏ��̃X�e�[�g��
				BallFindTh = (MachinePos.Th + BallFindTh)/2;
				if(BallDir){	// ����
					BallFindTh = BallFindTh - 0.05;
				}else{
					BallFindTh = BallFindTh + 0.05;
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
			if(fabs(BallFindTh - MachinePos.Th) < 0.01){ 	// �{�[���̂ق�������
				ManageActionTansakuState_BallFind = 4;				// �{�[���̂ق��������̂Ŏ��̃X�e�[�g��
				this->SpdTgt = 0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{
				this->SpdTgt = 0;		// [mmps]
				if(!BallDir){
					this->AngVelTgt = 0.02;	// [rps]
				}else{
					this->AngVelTgt = -0.02;
				}
			}
			break;
			
			
		case 4:		// �{�[���̂ق��������B�O�㋗�����킹���
			//����
			if((BALLFIND_MAXDISTANCE_FRONT<PsdFront->getDistance()) || (0>PsdFront->getDistance() )){	// �{�[������ or �����l
				ManageActionTansakuState_BallFind = 40;
			}else if(fabs(PsdFront->getDistance() - BALLGET_DISTANCE_FRONT) < 10){ 	// �{�[���Ƃ̋������킹��
				ManageActionTansakuState_BallFind = 5;				// �������킹���̂Ŏ��̃X�e�[�g��
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				StartTimeActionManage = getTime();	// ����
				
				// �A�[�����낷
				Inrof.ServoF->setGoalTime(50);
				Inrof.ServoR->setGoalTime(50);
				ServoF->setGoalPosition(SERVOPOS_BALLGET_F);
				ServoR->setGoalPosition(SERVOPOS_BALLGET_R);
				
			}else{
				// �t�����g�����Z���T�Ń{�[���Ƃ̋��������킹��
				this->SpdTgt = 5 * (PsdFront->getDistance() - BALLGET_DISTANCE_FRONT);		// [mmps]
				// �Œᑬ�x�K���H
				if(100<SpdTgt && 0<SpdTgt){
					this->SpdTgt = 100;
				}else if(-100>SpdTgt && 0>SpdTgt){
					this->SpdTgt = -100;
				}
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
			
			
		case 5:		// �O�㋗�����킹���B�A�[�����낷��
			if(StartTimeActionManage + 800 < getTime() /* �T�[�{���슮(500msec) */){ 	// �T�[�{������
				ManageActionTansakuState_BallFind = 6;
				MachinePosActionManageSub = MachinePos;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
			}
			break;
			
			
		case 6:		// �A�[�����낵���B�O�i�����
			if((BALLGET_DISTANCE_FRONT-20) < (MachinePosActionManageSub - MachinePos)){ 	// �O�i��
				ManageActionTansakuState_BallFind = 7;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				StartTimeActionManage = getTime();	// ����
				
				// �A�[���グ��
				Inrof.ServoF->setGoalTime(50);
				Inrof.ServoR->setGoalTime(200);
				ServoF->setGoalPosition(SERVOPOS_BALLUP_F);
				ServoR->setGoalPosition(SERVOPOS_BALLUP_R);
				
			}else{
				this->SpdTgt = 1.0*(BALLGET_DISTANCE_FRONT - (MachinePosActionManageSub - MachinePos));		// [mmps]
				//SpdTgt = 30;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
			
			
		case 7:		// �O�㋗�����킹���B�A�[���グ���
			if(StartTimeActionManage + 800 < getTime()){ 	// �T�[�{�グ��
				ManageActionTansakuState_BallFind = 8;
				this->SpdTgt = -10.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
				// ���s�p�� -> �{�[���F����p��
				Inrof.ServoF->setGoalTime(50);
				Inrof.ServoR->setGoalTime(50);
				//Inrof.ServoF->setGoalPosition(SERVOPOS_RUN_F);
				//Inrof.ServoR->setGoalPosition(SERVOPOS_RUN_R);
				Inrof.ServoF->setGoalPosition(SERVOPOS_COLORCHECK_F);
				Inrof.ServoR->setGoalPosition(SERVOPOS_COLORCHECK_R);
				
				// �F����J�n
				fJudgeColor = true;
				StartTimeActionManage = getTime();	//����̂��߂̎��ԑ���p
				
			}else{
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
				// �A�[���グ��
				
			}
			break;
			
			
		case 8:		// �{�[���Ƃ����I�I�I�I���C���֖߂��
			if( 5.0 > (((ORIENT_WEST == MachineOrientation) || (ORIENT_EAST == MachineOrientation))?fabs(MachinePosActionManage.X - MachinePos.X):fabs(MachinePosActionManage.Y - MachinePos.Y)) ){
			//if( ((MachinePos.X > MachinePosActionManage.X) && ((MachinePos.getNormalizeTh()>PI/2) || (MachinePos.getNormalizeTh()<(-PI/2))))
			//	||  ((MachinePos.X < MachinePosActionManage.X) && ((MachinePos.getNormalizeTh()<=PI/2) && (MachinePos.getNormalizeTh()>=(-PI/2))))
			//){ 	// ���C����ɖ߂��Ă���
				ManageActionTansakuState_BallFind = 9;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
				
				
			}else{
				// �܂������o�b�N
				/*
				if((MachinePos.getNormalizeTh() > PI/2) || (MachinePos.getNormalizeTh() < -PI/2)){	// �������Ă�
					SpdTgt = -3 * (MachinePosActionManage.X - MachinePos.X);		// [mmps]
				}else{																				// �E�����Ă�
					SpdTgt = 3 * (MachinePosActionManage.X - MachinePos.X);
				}*/
				if(ORIENT_WEST == MachineOrientation){	// �������Ă�
					this->SpdTgt = -3 * (MachinePosActionManage.X - MachinePos.X);		// [mmps]
				}else if(ORIENT_EAST == MachineOrientation){	// ��																		// �E�����Ă�
					this->SpdTgt = 3 * (MachinePosActionManage.X - MachinePos.X);
				}else if(ORIENT_SOUTH == MachineOrientation){	// ������Ă�
					this->SpdTgt = -3 * (MachinePosActionManage.Y - MachinePos.Y);		// [mmps]
				}else{											// �k�����Ă�
					this->SpdTgt = 3 * (MachinePosActionManage.Y - MachinePos.Y);
				}
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
		case 9:		// �k����
			if( (fabs(MachinePos.getNormalizeTh()-PI/2) < 0.15) && (StartTimeActionManage + 1000 < getTime())){	// ���������k���������{�[�����莞�Ԍo�����B							// ���A�{�[���F�Z���T�N������1000ms�ȏ�o���Ă��邱�ƁB
				ManageActionTansakuState_BallFind = 10;
				
				//return true;		
			}else{
				// �k������(P����)
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.2 * (PI/2 - ((MachinePos.getNormalizeTh()< -PI/2)?MachinePos.getNormalizeTh()+2*PI:MachinePos.getNormalizeTh()));	// [rps]
				//��]���x�ɐ���
				if(0.2<AngVelTgt){
					this->SpdTgt = 0.2;
				}else if(-0.2>AngVelTgt){
					this->SpdTgt = -0.2;
				}
			}
			
			break;
		
		case 10:	// ���C�����܂ł���ɉ��
		
				// ���C�����邩�`�F�b�N
			if((getLineSensAd(1)<LINESENS_THRESHOLD_CENTER)||(getLineSensAd(2)<LINESENS_THRESHOLD_CENTER)){	// �����Q�̃Z���T�����ꂩ�������Ȃ�܂�
				//�{�[���F�`�F�b�N
				BallColorFixed = BallColor;
				
				// �{�[�������ĂȂ�������I�I�I
				// �܂��T���I�I�I
				if(INROF_NONE == BallColorFixed){
					ManageActionTansakuState_BallFind = 40;
				}else{
					// �{�[�������Ă邩��u���ɍs��
					this->SpdTgt = 0.0;		// [mmps]
					this->AngVelTgt = 0.0;	// [rps]
					Inrof.ServoF->setGoalTime(500);
					Inrof.ServoR->setGoalTime(500);
					Inrof.ServoF->setGoalPosition(SERVOPOS_RUN_F);
					Inrof.ServoR->setGoalPosition(SERVOPOS_RUN_R);
					
					
					State = INROF_TOGOAL;	// ���̏�ԂցB�{�[����u���ɍs���B
					ActionState = INROF_ACTION_IDLE;
					ManageActionTansakuState_BallFind = 0;
				}
				
				fJudgeColor = false;
			}else{
				//���C���Ȃ���΂܂��
				//  ��肷�����������ǂ����悤
				this->SpdTgt = 0.0;		// [mmps]
				if(BallDir){	// ���̃{�[�����Ƃ���
					// �����Ɖ��(�����)
					this->AngVelTgt = 0.05;
				}else{
					// �����Ɖ��(�E���)
					this->AngVelTgt = -0.05;
				}
			}
			break;
			
// �{�[�����������ǌ����������p�X�e�[�g
		case 40:	//�{�[���������Ǝv�������ǌ�������
			// ���C����ɖ߂�
			if( 5.0 > (((ORIENT_WEST == MachineOrientation) || (ORIENT_EAST == MachineOrientation))?fabs(MachinePosActionManage.X - MachinePos.X):fabs(MachinePosActionManage.Y - MachinePos.Y)) ){
			//if( 5.0 > fabs(MachinePosActionManage.X - MachinePos.X) ){
			//if( ((5.0 > fabs(MachinePosActionManage.X - MachinePos.X)) && ((MachinePos.getNormalizeTh()>PI/2) || (MachinePos.getNormalizeTh()<(-PI/2))))
			//	||  ((MachinePos.X < MachinePosActionManage.X) && ((MachinePos.getNormalizeTh()<=PI/2) && (MachinePos.getNormalizeTh()>=(-PI/2))))
		//	){ 	// ���C����ɖ߂��Ă���
				ManageActionTansakuState_BallFind = 41;
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}else{
				/*
				if((MachinePos.getNormalizeTh() > PI/2) || (MachinePos.getNormalizeTh() < -PI/2)){	// �������Ă�
					SpdTgt = -5 * (MachinePosActionManage.X - MachinePos.X);		// [mmps]
				}else{																				// �E�����Ă�
					SpdTgt = 5 * (MachinePosActionManage.X - MachinePos.X);
				}
				*/
				if(ORIENT_WEST == MachineOrientation){	// �������Ă�
					this->SpdTgt = -3 * (MachinePosActionManage.X - MachinePos.X);		// [mmps]
				}else if(ORIENT_EAST == MachineOrientation){	// ��																		// �E�����Ă�
					this->SpdTgt = 3 * (MachinePosActionManage.X - MachinePos.X);
				}else if(ORIENT_SOUTH == MachineOrientation){	// ������Ă�
					this->SpdTgt = -3 * (MachinePosActionManage.Y - MachinePos.Y);		// [mmps]
				}else{											// �k�����Ă�
					this->SpdTgt = 3 * (MachinePosActionManage.Y - MachinePos.Y);
				}
				
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
		case 41:	//���E�{�[���������Ǝv�������ǈ����
			// ���������ɖ߂��ĒT���ĊJ
			if(0.05 > fabs(MachinePosActionManage.Th - MachinePos.Th)){
				ActionState = INROF_ACTION_LINE;	// ���������T�����܂�
				ManageActionTansakuState_BallFind = 0;
			}else{
				SpdTgt = 0.0;		// [mmps]
				AngVelTgt = 0.15*(MachinePosActionManage.Th - MachinePos.Th);	// [rps]
			}
			break;
			
			
		case 99:	// �ꎞ��~	
			if(StartTimeActionManage + 1000 < getTime() ){
				ManageActionTansakuState_BallFind = ManageActionTansakuState_BallFind_last+1;
			}else{
				this->SpdTgt = 0.0;		// [mmps]
				this->AngVelTgt = 0.0;	// [rps]
			}
			break;
		}
		break;
		
		
	case INROF_ACTION_LINE:	// ���C���g���[�X
		float x, y;
		PsdFront->getGlobalPos(&x, &y);
		if((x > 980) && (y < 1500)){	// �t�B�[���h��ł͂�����聕�{�[���T���L��
			BallFindEnable = true;
			setTgtVel_LineTrace(200.0);
		}else{
			setTgtVel_LineTrace(240.0);
		}
		
		
		// ������H�{�[�������Ă��I
		
		
		break;
	
		
	
	}
	
	
	
	return false;
}
	
bool_t inrof2015::manageAction_goToGoal(inrof_color Color){
	
	// ����
	switch(ActionState){
	case INROF_ACTION_IDLE:	// �ŏ��ɓ���
		ActionState = INROF_ACTION_BALLRELEASE;
		ManageActionTansakuState_BallRelease = 0;
		MachinePosActionManage = MachinePos;	// ���̏�Ԃɓ��������̏ꏊ����xxmm�ȓ��͏ꏊ�␳���Ȃ��p
		//break;
		/*
	case INROF_ACTION_LINE:		// 
		setTgtVel_LineTrace();	// �Ƃ肠�������C���g���[�X
		break;
		*/
	case INROF_ACTION_BALLRELEASE:
		switch(ManageActionTansakuState_BallRelease){
		case 0:	// �ڕW�����_�܂Ń��C���g���[�X
			
			// �S�[���̌����_�ɗ����[
			if(	IsCrossing 
				&&(	((INROF_RED == BallColorFixed) && (INROF_CROSS_RED == Cross))
					|| ((INROF_YELLOW == BallColorFixed) && (INROF_CROSS_YELLOW == Cross))
					|| ((INROF_BLUE == BallColorFixed) && (INROF_CROSS_BLUE == Cross))
				)
			){	
				adjustMachinePos();	// �p�x���A-PI�`PI�͈̔͂ɂȂ�̂Œ���
				
				ManageActionTansakuState_BallRelease = 1;
				MachinePosActionManageSub = MachinePos;
				
				ManageActionLineTraceDir = LineTraceDir;
				
				
			}else{	
				if(200 > (MachinePosActionManage - MachinePos)){	// ���C���g���[�X���n�܂���200mm�ȉ������i��łȂ���Έʒu�␳���Ȃ��B
					fAdjustMachinePos = false;
				}
				if(INROF_CROSS_START==Cross || INROF_CROSS_NONE==Cross){
					// �Ԍ����_���X�^�[�g�ʒu���������璼�i
					SpdTgt = 150.0;
					AngVelTgt = 0.0;
					fAdjustMachinePos = true;
				}else{
					setTgtVel_LineTrace(240.0);	// ���C���g���[�X
					fAdjustMachinePos = true;
				}
			}
			break;
			
		case 1:	// ��or���F�̌����_�ɗ����BBackWord�Ȃ�E�^�[�������BForward�Ȃ獶�^�[������(���R�{�[����)
			// -90����� or �Ȃ�̂Ă�(�������̏�Ԃ�)
			//if( ((fabs(fabs(MachinePos.Th - MachinePosActionManageSub.Th)-PI/2) < 0.1)&&(INROF_LINEDIR_FORWARD != ManageActionLineTraceDir))
			if( (fabs(fabs(MachinePos.Th - MachinePosActionManageSub.Th)-PI/2) < 0.1)
				//|| (((MachinePos.Th - MachinePosActionManageSub.Th) > (PI/2 - 0.1))&&(INROF_LINEDIR_BACKWARD != ManageActionLineTraceDir))
				|| (INROF_BLUE == BallColorFixed)
				//|| (INROF_YELLOW == BallColorFixed)
			){
				ManageActionTansakuState_BallRelease = 2;
				StartTimeActionManage = getTime();
				
				SpdTgt = 0.0;
				AngVelTgt = 0.0;
				
				Inrof.ServoR->enableTorque(CMDSV_TORQUE_OFF);
				Inrof.ServoF->setGoalTime(50);
				//Inrof.ServoR->setGoalTime(40);
				Inrof.ServoF->setGoalPosition(SERVOPOS_BALLTRASH_F);
				Inrof.ServoR->setGoalPosition(SERVOPOS_BALLTRASH_R);
				
		outPin(GR_LED3, 0);
			}else{
		outPin(GR_LED3, 1);
				if(INROF_LINEDIR_FORWARD == ManageActionLineTraceDir){	// ���C���������Ȃ�
					setTgtVel_Turn(VEL_NORMAL, LINESENS_POS_X+50);		// ������
				}else{
					setTgtVel_Turn(VEL_NORMAL, -(LINESENS_POS_X+50));	// ���x�A���񔼌a�w��
				}
			}
			break;
			
		case 2:	// ��or���F�̌����_�Ń^�[�����������B�{�[�����o
			if( StartTimeActionManage + 1500 < getTime() ){	// �{�[��������
				ManageActionTansakuState_BallRelease = 3;
				SpdTgt = 0.0;
				AngVelTgt = 0.0;
				
				
				Inrof.ServoR->enableTorque(CMDSV_TORQUE_ON);
				Inrof.ServoF->setGoalTime(500);
				Inrof.ServoR->setGoalTime(500);
				Inrof.ServoF->setGoalPosition(SERVOPOS_RUN_F);
				Inrof.ServoR->setGoalPosition(SERVOPOS_RUN_R);
				
			}else{
				//setTgtVel_LineTrace();
				this->SpdTgt = 0.0;
				this->AngVelTgt = 0.0;
			}
			break;
		case 3: 	// �{�[���̂Ă����牺����Ȃ���E��]����
			if( ((fabs(fabs(MachinePos.Th - (MachinePosActionManageSub.Th))-PI) < 0.1)&&(INROF_LINEDIR_FORWARD != ManageActionLineTraceDir))		// �k(�ȊO)�E��(��)������(���̌����̔��΂�����)
				|| (((fabs(MachinePos.Th - MachinePosActionManageSub.Th) < 0.1))&&(INROF_LINEDIR_BACKWARD != ManageActionLineTraceDir))	// �k����(FORWARD�Ȃ̂Ō��̌����Ɠ���)
			){
				ActionState = INROF_ACTION_LINE;			// �T���ĊJ
				
				State = INROF_TANSAKU;
				ActionState = INROF_ACTION_IDLE;
				ManageActionTansakuState_BallRelease = 0;
				
			}else{
				if(INROF_BLUE == BallColorFixed){
					this->SpdTgt = 0.0;
					this->AngVelTgt = 0.2 * ( (MachinePosActionManageSub.Th - PI) - MachinePos.Th );  // �E180����]
					if(0.2<AngVelTgt){
						this->SpdTgt = 0.2;
					}else if(-0.2>AngVelTgt){
						this->SpdTgt = -0.2;
					}
				}else{
					setTgtVel_Turn(-VEL_NORMAL, (LINESENS_POS_X+50));	// ������Ȃ���E��]�B
				}
			}
			break;
			
		
		}
		break;
		
	}
	
	return false;
}

bool_t inrof2015::manageAction_goToStart(void){
	
	return false;
}


/*************
���C���g���[�X����悤�ɖڕW���x�ݒ�
���C�����Ȃ��A�������̓X�^�[�g�̌����_�߂����Ƃ��Ȃ璼�i
*************/
void inrof2015::setTgtVel_LineTrace(void){
	
	
	setTgtVel_LineTrace(VEL_NORMAL);
	
}
void inrof2015::setTgtVel_LineTrace(float SpdTgt){
	
	
	this->SpdTgt = SpdTgt;
	
	
	if(INROF_CROSS_START==Cross){
		this->AngVelTgt = 0;	
	}else{
		if((getLineSensAd(1)<LINESENS_THRESHOLD_CENTER)||(getLineSensAd(2)<LINESENS_THRESHOLD_CENTER)){
			// ����2�̃Z���T�����C������Ă�
			this->AngVelTgt = 0.000005*(getLineSensAd(1) - getLineSensAd(2));	// ���C���g���[�X
			
			if(fAdjustMachinePos){
				adjustMachinePos();
			}
			
		}else if((getLineSensAd(0)<LINESENS_THRESHOLD_SIDE)||(getLineSensAd(3)<LINESENS_THRESHOLD_SIDE)){
			// ���E2�̃Z���T�ǂ��炩���C������Ă�
			this->AngVelTgt = 0.000004*(getLineSensAd(0) - getLineSensAd(3));	// ���C���g���[�X
			
			this->SpdTgt = SpdTgt/2;
		}else{
			// �ǂ���������ĂȂ��̂ł܂�����
			this->AngVelTgt = 0;	
		}
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
void inrof2015::setTgtVel_Turn(float SpdTgt, float Radius){
	
	this->SpdTgt = SpdTgt;							// [mm/s]
	//this->AngVelTgt = SpdTgt / Radius / (2*PI) /4 ;	// [rev/s]
	this->AngVelTgt = this->Spd / Radius / (2*PI) /2;	// [rev/s]
	
}








/*********************
���s����
�T�v�F
 PID����ŁA���i�����}��]������Duty���o�͂���B
 Duty�́Asigned�̃v���X�}�C�i�X�őO�i��i��\���B
**********************/
void inrof2015::ctrlRun(float SpdTgt_mmps, float AngVelTgt_rps, int32_t* RTireDutyTgt, int32_t* LTireDutyTgt){
	static ctrl_pid PidCtrlRTire(SAMPLETIME_CTRL_S, 200000,1000,1, -MotorR->getPeriodCnt(),MotorR->getPeriodCnt(), -1000, 1000);
	static ctrl_pid PidCtrlLTire(SAMPLETIME_CTRL_S, 200000,1000,1, -MotorL->getPeriodCnt(),MotorL->getPeriodCnt(), -1000, 1000);
	float RTireSpdTgt_rps, LTireSpdTgt_rps;
	
	// �e�^�C���̖ڕW���x
	//  �ڕW���i���x[mm/s]�Ɗp���x[r/s]����^�C���̖ڕW�p���x[r/s]�����߂�
	RTireSpdTgt_rps = SpdTgt_mmps/(PI*TIRE_DIA_MM) + AngVelTgt_rps * 4 * TIRE_TREAD_MM/TIRE_DIA_MM;
	LTireSpdTgt_rps = SpdTgt_mmps/(PI*TIRE_DIA_MM) - AngVelTgt_rps * 4 * TIRE_TREAD_MM/TIRE_DIA_MM;
	
	// �t�B�[�h�o�b�N����
	if(fMotorOut){
		// PID����
		*RTireDutyTgt = (int32_t)PidCtrlRTire.calc(RTireSpdTgt_rps - RTireSpd_rps);
		*LTireDutyTgt = (int32_t)PidCtrlLTire.calc(LTireSpdTgt_rps - LTireSpd_rps);
	}else{
		// ���[�^�[�o�͖������́APID����Ƀ��Z�b�g�������A�ڕW�f���[�e�B�[��0�Ƃ���B
		PidCtrlRTire.resetStatus(RTireSpdTgt_rps - RTireSpd_rps);
		PidCtrlLTire.resetStatus(LTireSpdTgt_rps - LTireSpd_rps);
		*RTireDutyTgt = 0;
		*LTireDutyTgt = 0;
	}
}



/*********************
���[�^�[�o��
�T�v�F
 Duty�����ɉ����Đ��]�t�]��؂�ւ���B
 fMotorOut��false�̎��A���[�^�[���t���[��ԂƂ���B
**********************/
void inrof2015::outTyres(int32_t RTireDutyTgt, int32_t LTireDutyTgt){
	
	if(fMotorOut){
		if(RTireDutyTgt<0){
			MotorR->setDir(RTIRE_DIR_BK);
			MotorR->setDutyCnt(-RTireDutyTgt);
		}else{
			MotorR->setDir(RTIRE_DIR_FW);
			MotorR->setDutyCnt(RTireDutyTgt);
		}
		if(LTireDutyTgt<0){
			MotorL->setDir(LTIRE_DIR_BK);
			MotorL->setDutyCnt(-LTireDutyTgt);
		}else{
			MotorL->setDir(LTIRE_DIR_FW);
			MotorL->setDutyCnt(LTireDutyTgt);
		}
	}else{
		MotorR->setDir(FREE);
		MotorR->setDutyCnt(0);
		MotorL->setDir(FREE);
		MotorL->setDutyCnt(0);
	}
	
}



/*********************
���C���Z���T�ʒu���O���[�o�����W�n�Ŏ擾
�����F
	(�Ԃ�)*X
	(�Ԃ�)*Y
**********************/
void inrof2015::getLineSenseGlobalPos(position* Pos){
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
void inrof2015::getBattVoltage(void){
	
	this->BattVoltage = ServoF->getPresentVolts();	// �d���v���l�擾
	if(BATT_LOW>=(this->BattVoltage)){
		this->fBattEmpty = true;
	}else{
		this->fBattEmpty = false;
	}
	
	ServoF->updatePresentVolts();	// ���̍X�V(�v���v��
	
}





// 1ms���Ƃ̊��荞�ݐݒ�
// TMR0
// ���荞�ݎ��� 46 / (PCLK/1024) = 0.981msec
void inrof2015::setup1msInterrupt(void){
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
void inrof2015::setupFreeCounter(void){
	MSTP(TPU6) = 0;					// ���W���[���N��
	TPU6.TCR.BIT.TPSC = 3;	// PCLK/4
	TPU6.TCR.BIT.CKEG = 0;	// �����オ��G�b�W�ŃJ�E���g
	TPU6.TCR.BIT.CCLR = 0;	// �N���A�Ȃ�
	TPU6.TMDR.BIT.MD = 0;	// �ʏ퓮��
	
	TPUB.TSTR.BIT.CST6 = 1;	// �J�E���g�J�n
	
}


extern void isr1ms(void);
// ���荞�݊֐��̐ݒ�
#pragma interrupt (Excep_TMR0_CMIA0(vect=VECT(TMR0,CMIA0), enable))
void Excep_TMR0_CMIA0(void){
	
	Inrof.isr1ms();
	//isr1ms();
	
}
