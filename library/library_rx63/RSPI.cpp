/********************************************/
/*	RSPIモジュールを使用したSPI通信			*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/02/25		*/
/********************************************/
#include "RSPI.h"

// RSPI module 
// Rx63n is master. 

//絶対beginするんだぞっ

//【更新予定】
// 

// Constructor
Spi_t::Spi_t(spi_module SpiModule)
{
	_SpiConfig.SpiModule = SpiModule;
	
	//レジスタ
	switch(SpiModule){
	case RSPI_SPI0:
		Spi_t::SPIreg = &RSPI0;
		break;
	case RSPI_SPI1:
		Spi_t::SPIreg = &RSPI1;
		break;
	case RSPI_SPI2:
		Spi_t::SPIreg = &RSPI2;
		break;
	}
	
	// 初期化
	switch(SpiModule){
	case RSPI_SPI0:
		_pinSSLn0 = SSLA0_NONE;
		_pinSSLn1 = SSLA1_NONE;
		_pinSSLn2 = SSLA2_NONE;
		_pinSSLn3 = SSLA3_NONE;
		break;
	case RSPI_SPI1:
		_pinSSLn0 = SSLB0_NONE;
		_pinSSLn1 = SSLB1_NONE;
		_pinSSLn2 = SSLB2_NONE;
		_pinSSLn3 = SSLB3_NONE;
		break;
	}
	
	// コールバック関数
	CallBackFuncTx = NULL;
	CallBackFuncRx = NULL;
	
	TxBuff = NULL;
	RxBuff = NULL;
}

Spi_t::~Spi_t(void){
	if(NULL!=TxBuff)	delete TxBuff;
	if(NULL!=RxBuff)	delete RxBuff;
}

// 
int8_t Spi_t::begin(spi_pin PinMOSI, spi_pin PinMISO, spi_pin PinRSPCK, spi_pin PinSSLn0, spi_pin PinSSLn1, spi_pin PinSSLn2, spi_pin PinSSLn3){
	
	begin(PinMOSI, PinMISO, PinRSPCK, PinSSLn0, PinSSLn1, PinSSLn2, PinSSLn3, SPI_BUFFER_SIZE_DEFAULT);
	
	return 0;
}

int8_t Spi_t::begin(spi_pin PinMOSI, spi_pin PinMISO, spi_pin PinRSPCK, spi_pin PinSSLn0, spi_pin PinSSLn1, spi_pin PinSSLn2, spi_pin PinSSLn3, uint16_t BuffNum){
	
	// Buffer
	if(NULL!=TxBuff)	delete TxBuff;
	TxBuff = new RingBuffer<uint8_t>(BuffNum);
	if(NULL==TxBuff) return -1;
	fUseRx = ((PinMISO!=MISOA_NONE) && (PinMISO!=MISOB_NONE));
	if(fUseRx){
		if(NULL!=RxBuff)	delete RxBuff;
		RxBuff = new RingBuffer<uint8_t>(BuffNum);
		if(NULL==RxBuff) return -1;
	}
	fBufferAttached = 1;
	
	// SPI setting
	fUseRx = ((PinMISO!=MISOA_NONE) && (PinMISO!=MISOB_NONE));
	initRegister(fUseRx);	
	
	// pin mode setting
	// Set pin mode to MOSI, MISO, RSPCK , SSL
	setPinModeSpi(PinMOSI, PinMISO, PinRSPCK, PinSSLn0, PinSSLn1, PinSSLn2, PinSSLn3);
	
	
	fEmptySpiTxBuff = true;
	
	return 0;
}

// 
int8_t Spi_t::begin(spi_pin PinMOSI, spi_pin PinMISO, spi_pin PinRSPCK, spi_pin PinSSLn){
	
	begin(PinMOSI, PinMISO, PinRSPCK, PinSSLn, SPI_BUFFER_SIZE_DEFAULT);
	return 0;
}

int8_t Spi_t::begin(spi_pin PinMOSI, spi_pin PinMISO, spi_pin PinRSPCK, spi_pin PinSSLn, uint16_t BuffNum){
	spi_pin PinSSLn0, PinSSLn1, PinSSLn2, PinSSLn3;
	
	fUseRx = ((PinMISO!=MISOA_NONE) && (PinMISO!=MISOB_NONE));
	
	// read current SSLn setting
	PinSSLn0 = _pinSSLn0;
	PinSSLn1 = _pinSSLn1;
	PinSSLn2 = _pinSSLn2;
	PinSSLn3 = _pinSSLn3;
	
	switch(_SpiConfig.SpiModule){
	case RSPI_SPI0:
		if(PinSSLn==SSLA_NONE){
			// 
		}else if(PinSSLn<SSLA1_NONE){
			PinSSLn0 = PinSSLn;
		}else if(PinSSLn<SSLA2_NONE){
			PinSSLn1 = PinSSLn;
		}else if(PinSSLn<SSLA3_NONE){
			PinSSLn2 = PinSSLn;
		}else{
			PinSSLn3 = PinSSLn;
		}
		break;
	case RSPI_SPI1:
		if(PinSSLn==SSLB_NONE){
			// 
		}else if(PinSSLn<SSLB1_NONE){
			PinSSLn0 = PinSSLn;
		}else if(PinSSLn<SSLB2_NONE){
			PinSSLn1 = PinSSLn;
		}else if(PinSSLn<SSLB3_NONE){
			PinSSLn2 = PinSSLn;
		}else{
			PinSSLn3 = PinSSLn;
		}
		break;
	}
	
	
	// Buffer	
	if(NULL!=TxBuff)	delete TxBuff;
	TxBuff = new RingBuffer<uint8_t>(BuffNum);
	if(NULL==TxBuff) return -1;
	if(fUseRx){
		if(NULL!=RxBuff)	delete RxBuff;
		RxBuff = new RingBuffer<uint8_t>(BuffNum);
		if(NULL==RxBuff) return -1;
	}
	fBufferAttached = 1;
	
	// SPI setting
	initRegister((PinMISO!=MISOA_NONE) && (PinMISO!=MISOB_NONE));	
	
	// pin mode setting
	// Set pin mode to MOSI, MISO, RSPCK , SSL
	setPinModeSpi(PinMOSI, PinMISO, PinRSPCK, PinSSLn0, PinSSLn1, PinSSLn2, PinSSLn3);
	
	
	fEmptySpiTxBuff = true;
	
	return 0;
}

void Spi_t::initRegister(bool_t fUseRx){
	
	// SPI module wakeup. 
	/* Protection off */
	SYSTEM.PRCR.WORD = 0xA503u;
	/* Wake RSPI unit from standby mode */
	if(_SpiConfig.SpiModule == RSPI_SPI0){
		MSTP(RSPI0) = 0u;
	}else if(_SpiConfig.SpiModule == RSPI_SPI1){
		MSTP(RSPI1) = 0u;
	}
	/* Protection on */
	//SYSTEM.PRCR.WORD = 0xA500u; // gr-sakura用にonしない

	SPIreg->SPCR.BYTE = 0x08u; //SPI Stop, Set to Master Mode
	if(fUseRx){
		SPIreg->SPCR.BIT.TXMD = 0;
		//IEN
		if(_SpiConfig.SpiModule == RSPI_SPI0){
			IEN(RSPI0,SPRI0) = 1;
			IPR(RSPI0,SPRI0) = 10;
		}else if(_SpiConfig.SpiModule == RSPI_SPI1){
			IEN(RSPI1,SPRI1) = 1;
			IPR(RSPI1,SPRI1) = 10;
		}
    	SPIreg->SPCR.BIT.SPRIE = 1; // Enable Rx Interrupt Request
	}else{	// only Tx									// Tx and Rx
		SPIreg->SPCR.BIT.TXMD = 1;
    	SPIreg->SPCR.BIT.SPRIE = 0; // Disable Rx Interrupt Request
	}
    /* Set SSL pin to active low */
    SPIreg->SSLP.BIT.SSL0P = 0u;
    /* Set SPPCR register */
    SPIreg->SPPCR.BYTE = 0u;
    /* Set bit rate to 12Mbit/s, by setting SPBR to 0 */
    //SPIreg->SPBR = RSPI_CLOCK_DIV16;
    //SPIreg->SPBR = RSPI_CLOCK_DIV128;		// ちょっと遅くした。
	SPIreg->SPBR = 0x09;	//2.4Mbps
    /* Set SPDCR register */
	//SPIreg->SPDCR.BYTE = 0x20u;
	SPIreg->SPDCR.BYTE = 0x0u;
    /* Set RSPI sequence control pointer to SPCMD0 */
    SPIreg->SPSCR.BYTE = 0u;
    /* Set SPCMD0 register (command register 0) */
	SPIreg->SPCMD0.WORD = 0x0700u;	// MSB first, 8bit, SPI MODE0, SSL0(External)
	SPIreg->SPCMD0.BIT.SSLKP = 0;		// negate a SSL when end transmit
	//setCS(RSPI_SSL3);
	
	SPIreg->SPCMD0.BIT.CPHA = 1;		// 奇数エッジでデータ変化、偶数エッジでデータサンプル
	SPIreg->SPCMD0.BIT.CPOL = 1;		// アイドル時のRSPCK = high
	
	SPIreg->SPDCR.BIT.SPRDTD = 0;	// read SPDR is access to read buffer.
	
	SPIreg->SPCKD.BIT.SCKDL = 0x04;	// SSLからxRSPCK遅れてRSPCK発振開始
	//SPIreg->SPCMD0.BIT.SCKDEN = 1;	// ↑有効
	//SPIreg->SPCKD.BIT.SCKDL = 0x00;	// SSLから0RSPCK遅れてRSPCK発振開始
	SPIreg->SPCMD0.BIT.SCKDEN = 0;	// ↑無効
	//SPIreg->SSLND.BIT.SLNDL = 0x03;	// RSPCK発振終了から4RSPCK遅れてSSLネゲート
	SPIreg->SSLND.BIT.SLNDL = 0x00;	// RSPCK発振終了から0RSPCK遅れてSSLネゲート
	//SPIreg->SPND.BIT.SPNDL = 0x00;	// 次アクセス遅延
	//SPIreg->SSLND.BIT.SLNDL = 0x01;	// RSPCK発振終了から0RSPCK遅れてSSLネゲート
	SPIreg->SPND.BIT.SPNDL = 0x01;	// 次アクセス遅延
	SPIreg->SPCMD0.BIT.SPNDEN = 1;	// ↑無効
	
	// Interrupt enable
	if(_SpiConfig.SpiModule == RSPI_SPI0){
		IEN(RSPI0,SPTI0) = 1;
		IPR(RSPI0,SPTI0) = 1;
	}else if(_SpiConfig.SpiModule == RSPI_SPI1){
		IEN(RSPI1,SPTI1) = 1;
		IPR(RSPI1,SPTI1) = 1;
	}
	enableTxBuffEmptyInterrupt(); // Tx Interrupt Request enable
	
	// Start
	SPIreg->SPSR.BIT.MODF = 0;
    SPIreg->SPCR.BIT.SPE = 1; //Start SPI
}


void Spi_t::end() {
	
	SPIreg->SPCR.BIT.SPE = 0;		// RSPI end
														// Tx interrupt is occur.
	
	// SPI off
	switch(_SpiConfig.SpiModule){
	case RSPI_SPI0:
		MSTP(RSPI0) = 1;
		break;
	case RSPI_SPI1:
		MSTP(RSPI1) = 1;
		break;
	case RSPI_SPI2:
//		MSTP(RSPI2) = 1;
		break;
	}
}


void Spi_t::setBitOrder(spi_bit_order bitOrder)
{
    SPIreg->SPCR.BIT.SPE = 0; //Stop SPI

    if(bitOrder == RSPI_LSBFIRST) {
        SPIreg->SPCMD0.WORD |=  (1 << 12);
    } else {
        SPIreg->SPCMD0.WORD &= ~(1 << 12);
    }

    SPIreg->SPCR.BIT.SPE = 1; //Start SPI
}

void Spi_t::setDataMode(spi_mode mode)
{
    SPIreg->SPCR.BIT.SPE = 0; //Stop SPI

    SPIreg->SPCMD0.WORD = (SPIreg->SPCMD0.WORD & ~SPI_MODE_MASK) | ((uint16_t)mode);
	
    SPIreg->SPCR.BIT.SPE = 1; //Start SPI
}

void Spi_t::setClockDivider(spi_clock_div rate)
{
    SPIreg->SPCR.BIT.SPE = 0; //Stop SPI

    SPIreg->SPBR = rate;

    SPIreg->SPCR.BIT.SPE = 1; //Start SPI
}


// this is Rx63n specical
void Spi_t::setPinModeSpi(spi_pin PinMOSI, spi_pin PinMISO, spi_pin PinRSPCK, spi_pin PinSSLn0, spi_pin PinSSLn1, spi_pin PinSSLn2, spi_pin PinSSLn3){
	// set PDR, PMR, PFS 
	
	_pinSSLn0 = PinSSLn0;
	_pinSSLn1 = PinSSLn1;
	_pinSSLn2 = PinSSLn2;
	_pinSSLn3 = PinSSLn3;
	
	
  /* PWPR.PFSWE write protect off */
    MPC.PWPR.BYTE = 0x00u;
  /* PFS register write protect off */
    MPC.PWPR.BYTE = 0x40u;
  /* Protection off */
    SYSTEM.PRCR.WORD = 0xA503u;
	
	if(_SpiConfig.SpiModule == RSPI_SPI0){
		// MOSI port config
		switch(PinMOSI){
		case MOSIA_NONE:
			break;
		case MOSIA_P16:					//P16
			PORT1.PDR.BIT.B6 = 1;		// OUT
		    MPC.P16PFS.BIT.PSEL = 0x0d;	// MOSIA
		    PORT1.PMR.BIT.B6 = 1;		// notGPIO
			break;
		case MOSIA_PA6:					//PA6
			PORTA.PDR.BIT.B6 = 1;		// OUT
		    MPC.PA6PFS.BIT.PSEL = 0x0d;	// MOSIA
		    PORTA.PMR.BIT.B6 = 1;		// notGPIO
			break;
		case MOSIA_PC6:					//PC6
			PORTC.PDR.BIT.B6 = 1;		// OUT
		    MPC.PC6PFS.BIT.PSEL = 0x0d;	// MOSIA
		    PORTC.PMR.BIT.B6 = 1;		// notGPIO
			break;
		}
		// MISO port config
		switch(PinMISO){
		case MISOA_NONE:
			break;
		case MISOA_P17:					//P17
			PORT1.PDR.BIT.B7 = 0;		// IN
		    MPC.P17PFS.BIT.PSEL = 0x0d;	// MISOA
		    PORT1.PMR.BIT.B7 = 1;		// notGPIO
			break;
		case MISOA_PA7:					//PA7
			PORTA.PDR.BIT.B7 = 0;		// IN
		    MPC.PA7PFS.BIT.PSEL = 0x0d;	// MISOA
		    PORTA.PMR.BIT.B7 = 1;		// notGPIO
			break;
		case MISOA_PC7:					//PC7
			PORTC.PDR.BIT.B7 = 0;		// IN
		    MPC.PC7PFS.BIT.PSEL = 0x0d;	// MISOA
		    PORTC.PMR.BIT.B7 = 1;		// notGPIO
			break;
		}
		// RSPCKA port config
		switch(PinRSPCK){
		case RSPCKA_PA5:				//PA5
			PORTA.PDR.BIT.B5 = 1;		// OUT
		    MPC.PA5PFS.BIT.PSEL = 0x0d;	// RSPCKA
		    PORTA.PMR.BIT.B5 = 1;		// notGPIO
			break;
		case RSPCKA_PB0:				//PB0
			PORTB.PDR.BIT.B0 = 1;		// OUT
		    MPC.PB0PFS.BIT.PSEL = 0x0d;	// RSPCKA
		    PORTB.PMR.BIT.B0 = 1;		// notGPIO
			break;
		case RSPCKA_PC5:				//PC5
			PORTC.PDR.BIT.B5 = 1;		// OUT
		    MPC.PC5PFS.BIT.PSEL = 0x0d;	// RSPCKA
		    PORTC.PMR.BIT.B5 = 1;		// notGPIO
			break;
		}
		// SSLA0 port config
		switch(PinSSLn0){
		case SSLA0_NONE:				//
			break;
		case SSLA0_PA4:				//PA4
			PORTA.PDR.BIT.B4 = 1;		// OUT
		    MPC.PA4PFS.BIT.PSEL = 0x0d;	// SSLA0
		    PORTA.PMR.BIT.B4 = 1;		// notGPIO
			break;
		case SSLA0_PC4:				//PC4
			PORTC.PDR.BIT.B4 = 1;		// OUT
		    MPC.PC4PFS.BIT.PSEL = 0x0d;	// SSLA0
		    PORTC.PMR.BIT.B4 = 1;		// notGPIO
			break;
		}
		// SSLA1 port config
		switch(PinSSLn1){
		case SSLA1_NONE:			
			break;
		case SSLA1_PA0:				//PA0
			PORTA.PDR.BIT.B0 = 1;		// OUT
		    MPC.PA0PFS.BIT.PSEL = 0x0d;	// SSLA1
		    PORTA.PMR.BIT.B0 = 1;		// notGPIO
			break;
		case SSLA1_PC0:				//PC0
			PORTC.PDR.BIT.B0 = 1;		// OUT
		    MPC.PC0PFS.BIT.PSEL = 0x0d;	// SSLA1
		    PORTC.PMR.BIT.B0 = 1;		// notGPIO
			break;
		}
		// SSLA2 port config
		switch(PinSSLn2){
		case SSLA2_NONE:			
			break;
		case SSLA2_PA1:				//PA1
			PORTA.PDR.BIT.B1 = 1;		// OUT
		    MPC.PA1PFS.BIT.PSEL = 0x0d;	// SSLA2
		    PORTA.PMR.BIT.B1 = 1;		// notGPIO
			break;
		case SSLA2_PC1:				//PC1
			PORTC.PDR.BIT.B1 = 1;		// OUT
		    MPC.PC1PFS.BIT.PSEL = 0x0d;	// SSLA2
		    PORTC.PMR.BIT.B1 = 1;		// notGPIO
			break;
		}
		// SSLA3 port config
		switch(PinSSLn3){
		case SSLA3_NONE:			
			break;
		case SSLA3_PA2:				//PA2
			PORTA.PDR.BIT.B2 = 1;		// OUT
		    MPC.PA2PFS.BIT.PSEL = 0x0d;	// SSLA3
		    PORTA.PMR.BIT.B2 = 1;		// notGPIO
			break;
		case SSLA3_PC2:				//PC2
			PORTC.PDR.BIT.B2 = 1;		// OUT
		    MPC.PC2PFS.BIT.PSEL = 0x0d;	// SSLA3
		    PORTC.PMR.BIT.B2 = 1;		// notGPIO
			break;
		}
	}else if(_SpiConfig.SpiModule == RSPI_SPI1){
		// MOSI port config
		switch(PinMOSI){
		case MOSIB_NONE:
			break;
		case MOSIB_P26:					//P26
			PORT2.PDR.BIT.B6 = 1;		// OUT
		    MPC.P26PFS.BIT.PSEL = 0x0d;	// MOSIB
		    PORT2.PMR.BIT.B6 = 1;		// notGPIO
			break;
		case MOSIB_PE2:					//PE2
			PORTE.PDR.BIT.B2 = 1;		// OUT
		    MPC.PE2PFS.BIT.PSEL = 0x0e;	// MOSIB
		    PORTE.PMR.BIT.B2 = 1;		// notGPIO
			break;
		case MOSIB_PE6:					//PE6
			PORTE.PDR.BIT.B6 = 1;		// OUT
		    MPC.PE6PFS.BIT.PSEL = 0x0d;	// MOSIB
		    PORTE.PMR.BIT.B6 = 1;		// notGPIO
			break;
		}
		// MISO port config
		switch(PinMISO){
		case MISOB_NONE:
			break;
		case MISOB_P30:					//P30
			PORT3.PDR.BIT.B0 = 0;		// IN
		    MPC.P30PFS.BIT.PSEL = 0x0d;	// MISOB
		    PORT3.PMR.BIT.B0 = 1;		// notGPIO
			break;
		case MISOB_PE3:					//PE3
			PORTE.PDR.BIT.B3 = 0;		// IN
		    MPC.PE3PFS.BIT.PSEL = 0x0d;	// MISOB
		    PORTE.PMR.BIT.B3 = 1;		// notGPIO
			break;
		case MISOB_PE7:					//PE7
			PORTE.PDR.BIT.B7 = 0;		// IN
		    MPC.PE7PFS.BIT.PSEL = 0x0d;	// MISOB
		    PORTE.PMR.BIT.B7 = 1;		// notGPIO
			break;
		}
		// RSPCKA port config
		switch(PinRSPCK){
		case RSPCKB_P27:				//P27
			PORT2.PDR.BIT.B7 = 1;		// OUT
		    MPC.P27PFS.BIT.PSEL = 0x0d;	// RSPCKB
		    PORT2.PMR.BIT.B7 = 1;		// notGPIO
			break;
		case RSPCKB_PE1:				//PE1
			PORTE.PDR.BIT.B1 = 1;		// OUT
		    MPC.PE1PFS.BIT.PSEL = 0x0e;	// RSPCKB
		    PORTE.PMR.BIT.B1 = 1;		// notGPIO
			break;
		case RSPCKB_PE5:				//PE5
			PORTE.PDR.BIT.B5 = 1;		// OUT
		    MPC.PE5PFS.BIT.PSEL = 0x0d;	// RSPCKB
		    PORTE.PMR.BIT.B5 = 1;		// notGPIO
			break;
		}
		// SSLA0 port config
		switch(PinSSLn0){
		case SSLB0_NONE:				//PA5
			break;
		case SSLB0_P31:				//P31
			PORT3.PDR.BIT.B1 = 1;		// OUT
		    MPC.P31PFS.BIT.PSEL = 0x0d;	// SSLB0
		    PORT3.PMR.BIT.B1 = 1;		// notGPIO
			break;
		case SSLB0_PE4:				//PE4
			PORTE.PDR.BIT.B4 = 1;		// OUT
		    MPC.PE4PFS.BIT.PSEL = 0x0d;	// SSLB0
		    PORTE.PMR.BIT.B4 = 1;		// notGPIO
			break;
		}
		// SSLA1 port config
		switch(PinSSLn1){
		case SSLB1_NONE:			
			break;
		case SSLB1_P50:				//P50
			PORT5.PDR.BIT.B0 = 1;		// OUT
		    MPC.P50PFS.BIT.PSEL = 0x0d;	// SSLB1
		    PORT5.PMR.BIT.B0 = 1;		// notGPIO
			break;
		case SSLB1_PE0:				//PE0
			PORTE.PDR.BIT.B0 = 1;		// OUT
		    MPC.PE0PFS.BIT.PSEL = 0x0d;	// SSLB1
		    PORTE.PMR.BIT.B0 = 1;		// notGPIO
			break;
		}
		// SSLA2 port config
		switch(PinSSLn2){
		case SSLB2_NONE:			
			break;
		case SSLB2_P51:				//P51
			PORT5.PDR.BIT.B1 = 1;		// OUT
		    MPC.P51PFS.BIT.PSEL = 0x0d;	// SSLB2
		    PORT5.PMR.BIT.B1 = 1;		// notGPIO
			break;
		case SSLB2_PE1:				//PE1
			PORTE.PDR.BIT.B1 = 1;		// OUT
		    MPC.PE1PFS.BIT.PSEL = 0x0d;	// SSLB2
		    PORTE.PMR.BIT.B1 = 1;		// notGPIO
			break;
		}
		// SSLA3 port config
		switch(PinSSLn3){
		case SSLB3_NONE:			
			break;
		case SSLB3_P52:				//P52
			PORT5.PDR.BIT.B2 = 1;		// OUT
		    MPC.P52PFS.BIT.PSEL = 0x0d;	// SSLB3
		    PORT5.PMR.BIT.B2 = 1;		// notGPIO
			break;
		case SSLB3_PE2:				//PE2
			PORTE.PDR.BIT.B2 = 1;		// OUT
		    MPC.PE2PFS.BIT.PSEL = 0x0d;	// SSLB3
		    PORTE.PMR.BIT.B2 = 1;		// notGPIO
			break;
		}
		
	}
	
	
	 // gr-sakura用にonしない
  /* PWPR.PFSWE write protect on */
  /* PFS register write protect on */
//    MPC.PWPR.BYTE = 0x80u;
  /* Protection on */
//    SYSTEM.PRCR.WORD = 0xA500u;
	
}

void Spi_t::setCS(spi_ssl ssl){
    SPIreg->SPCMD0.BIT.SSLA = ssl;	// select active device. 
}


int8_t Spi_t::transmit(uint8_t TrData){
	// add data to buffer. 
	// when transmit is stop, transmit once.
	// if buffer is full, return false.
	
	int8_t ack;
	ack = TxBuff->add(TrData);
	if(!ack){
		if(fEmptySpiTxBuff || isIdle()){		
			isrTx();	// Transmit one time
		}
	}
	return ack;
}

int8_t Spi_t::receive(uint8_t *RcData){
	
	return RxBuff->read(RcData);
}

// コールバック関数登録
void Spi_t::setCallBackFuncTx(void (*CallBackFuncTx)(void)){
	Spi_t::CallBackFuncTx = CallBackFuncTx;

}

void Spi_t::setCallBackFuncRx(void (*CallBackFuncRx)(void)){
	Spi_t::CallBackFuncRx = CallBackFuncRx;
	
}

void Spi_t::isrTx(void){
	uint8_t Data;
	
	// 送信終了割り込み
	if(CallBackFuncTx!=NULL){
		(*CallBackFuncTx)();
	}
	
	if(!TxBuff->read(&Data)){	//バッファ読む。ちゃんと読めたらsetする
		setSpiTxData(Data);		// Transmit data
		fEmptySpiTxBuff = false;
	}else{						// 読み終わってる
		fEmptySpiTxBuff = true;
	}
}

void Spi_t::isrRx(void){

	// 受信終了割り込み
	if(CallBackFuncRx!=NULL){
		(*CallBackFuncRx)();
	}
	RxBuff->add(getSpiRxData());
	
}


Spi_t SPI0(RSPI_SPI0);	//MOSIA, MISOA
Spi_t SPI1(RSPI_SPI1);	//MOSIB, MOSIB
//Spi_t SPI2(RSPI_SPI2, &rx_buffer2, &tx_buffer2);	//MOSIB, MOSIB

// interrupt
#pragma section IntPRG
// tx
/*
#pragma interrupt (Excep_RSPI0_SPTI0(enable, vect=VECT(RSPI0,SPTI0)))
// SDカードで使うため、RSPI0の割り込みは設定しない
void Excep_RSPI0_SPTI0(void){
	setpsw_i();	//多重割り込み許可
	SPI0.isrTx();
}
*/

#pragma interrupt (Excep_RSPI1_SPTI1(enable, vect=VECT(RSPI1,SPTI1)))
void Excep_RSPI1_SPTI1(void)
{
	setpsw_i();	//多重割り込み許可
	//PORTD.PODR.BIT.B0 = 1;
	SPI1.isrTx();
	//PORTD.PODR.BIT.B0 = 0;
}

//rx
/*
#pragma interrupt (Excep_RSPI0_SPRI0(enable, vect=VECT(RSPI0,SPRI0)))
void Excep_RSPI0_SPRI0(void){
	setpsw_i();	//多重割り込み許可
	SPI0.isrRx();
}
*/	

#pragma interrupt (Excep_RSPI1_SPRI1(enable, vect=VECT(RSPI1,SPRI1)))
void Excep_RSPI1_SPRI1(void){
	setpsw_i();	//多重割り込み許可
	//PORTD.PODR.BIT.B1 = 1;
	SPI1.isrRx();
	//PORTD.PODR.BIT.B1 = 0;
}

#pragma section 



