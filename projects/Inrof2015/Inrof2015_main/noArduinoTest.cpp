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
#include "Inrof2015_lib_noRxduino.h"



void setup()
{
	
	
	// GR-SAKURA関係初期化
	GR_begin();
	
	// 周辺機器初期化
	Inrof.begin();
	
	
	
	// シリアル送受信
	Sci2.begin(115200, true, true, 1024, 128);
	
	Sci2.print("Hello Rx63n\r\n");
	Sci2.print(" ...ServoTest\r\n");

	
	Inrof.MotorR->setDutyCnt(0);
	Inrof.MotorL->setDutyCnt(0);
	Inrof.MotorR->setDir(RTIRE_DIR_FW);
	Inrof.MotorL->setDir(LTIRE_DIR_FW);
	
	Inrof.fMotorOut = false;
	
	
	//サーボon
	Inrof.ServoF->enableTorque(CMDSV_TORQUE_ON);
	Inrof.ServoR->enableTorque(CMDSV_TORQUE_ON);
	// サーボ速度
	Inrof.ServoF->setGoalTime(500);
	Inrof.ServoR->setGoalTime(5600);
	// 走行姿勢
	Inrof.ServoF->setGoalPosition(SERVOPOS_RUN_F);
	Inrof.ServoR->setGoalPosition(SERVOPOS_RUN_R);
	
}



void loop()
{
	static bool_t sw[2];
	
	static bool_t sw0[2];
	static bool_t sw1[2];
	static bool_t sw2[2];
	static bool_t sw3[2];
	
	static uint8_t LoopCnt;
	
	position* MachinePos;
	
	
	
	// スイッチ読み取り
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

	
	//outPin(GR_LED1, sw0[0]);
	
	if(Inrof.fCnt200){
	//if(0){
		Inrof.fCnt200 = 0;
		
		Oled.initRegister();	// 表示ずれちゃったりするので、レジスタセットし直す
		
		Oled.setCurNewLine(0,0);
		Oled.print("Servo=");
		Oled.setCurNewLine(1,0);
		Oled.print("%3d,%3d",
						Inrof.ServoF->getPresentPosition(),
						Inrof.ServoR->getPresentPosition()
					);
					
		Oled.setCurNewLine(2,0);
		Oled.print("PSD=");
		Oled.setCurNewLine(3,0);
		Oled.print("%3d,%3d,%3d,%3d,%3d\n",
						Inrof.PsdLeft->getDistance(),
						Inrof.PsdLeftSide->getDistance(),
						Inrof.PsdFront->getDistance(),
						Inrof.PsdRightSide->getDistance(),
						Inrof.PsdRight->getDistance()
					);
		Oled.setCurNewLine(4,0);
		Oled.print("Color(RGBIr)=");
		Oled.setCurNewLine(5,0);
		Oled.print("%4d,%4d,%4d,%4d\n",
						Inrof.ColorSens->getLuminanceRed(),
						Inrof.ColorSens->getLuminanceGreen(),
						Inrof.ColorSens->getLuminanceBlue(),
						Inrof.ColorSens->getLuminanceIr()
					);
	
		MachinePos = Inrof.getMachinePos();
		Oled.setCurNewLine(6);
		Oled.print("Pos=%3.0f,%3.0f,%3.0f",
					MachinePos->X,
					MachinePos->Y,
					MachinePos->getNormalizeTh() * 10.0
					);
					
		
		Oled.setCurNewLine(7,16);
		Oled.print("%d\n", LoopCnt++);
		
		
		
	}
	
	if(sw0[0] != sw0[1]){
		if(sw0[0]){
		// スイッチ0押した
			I2C0.resetI2C();
		}
	}
	if(sw1[0] != sw1[1]){
		if(sw1[0]){
			Sci2.print("enableTorque %d\r\n", Inrof.ServoF->enableTorque(CMDSV_TORQUE_BRAKE));
			Sci2.print("enableTorque %d\r\n", Inrof.ServoR->enableTorque(CMDSV_TORQUE_BRAKE));
		}else{
			Inrof.ServoF->enableTorque(CMDSV_TORQUE_OFF);
			Inrof.ServoR->enableTorque(CMDSV_TORQUE_OFF);
		}
	}
		
	if(sw2[0] != sw2[1]){
		if(sw2[0]){
		}else{
			Inrof.ServoF->enableTorque(CMDSV_TORQUE_ON);
			Inrof.ServoR->enableTorque(CMDSV_TORQUE_ON);
		}
		
	}
	
	if(sw3[0] != sw3[1]){
		if(sw3[0]){
			
		}else{
			Inrof.fMotorOut = !Inrof.fMotorOut;
			if(Inrof.getState() == INROF_IDLE){
				Inrof.setState(INROF_TOGOAL);
			}
		}
	}
	
	/*
	IWDT.IWDTRR = 0x00;
	IWDT.IWDTRR = 0xFF;
	WDT.WDTRR = 0x00;
	WDT.WDTRR = 0xFF;
	*/
	
	//PORTD.PODR.BIT.B7 = ServoF->fAttaching;
	
	// 制御実行
	Inrof.step();
	
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

void main(void)
{
	setupRx();
	setup();
	
	Sci2.print("main sp 0x%08p\r\n", get_usp());
	
	while(1){
		loop();
	}
}

#ifdef __cplusplus
void abort(void)
{

}
#endif
