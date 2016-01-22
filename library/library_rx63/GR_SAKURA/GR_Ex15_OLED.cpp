
#include "GR_Ex15_OLED.h"



void gr_ex_oled::begin(void){
	
	// switch
	setPinMode(OLED_SW0, PIN_INPUT);
	setPinMode(OLED_SW1, PIN_INPUT);
	setPinMode(OLED_SW2, PIN_INPUT);
	setPinMode(OLED_SW3, PIN_INPUT);
	
	// OLED
	Oled.begin(OLED_SPI_MODULE, MOSIB_PE6, RSPCKB_PE5, SSLB0_PE4, OLED_PIN_DC, OLED_TxCallback);
	
}

// OLED CD??
void OLED_TxCallback(void){
	Oled.callbackTxIsr();
}



gr_ex_oled GrExOled;
OledDriverSSD1306 Oled;









