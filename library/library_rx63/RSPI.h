/********************************************/
/*	RSPIモジュールを使用したSPI通信			*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/02/25		*/
/********************************************/

/*
【使い方】




めっちゃタイミングが悪いと、1バイトのみ単発で送ると送られないかも！
(「バッファすべての送信終了の瞬間に新しく1バイト追加」で発生する可能性あり)
もう1バイト送れば流れだすのであまり気にしなくてもいいかも。


【更新履歴】
2015.03.01 送受信割り込み時にコールバック関数呼べるように。

*/

#ifndef __RSPI_H__
#define __RSPI_H__

#include <machine.h>	// マイコンコンパイラ固有関数

#include "iodefine.h"
#include "CommonDataType.h"

#include "RingBuffer.h"

// for debug
#include "portReg.h"
//#include "GR_define.h"

// リングバッファのサイズ
#define SPI_BUFFER_SIZE_DEFAULT 64

enum spi_module
{
	RSPI_SPI0 = 0,
	RSPI_SPI1 = 1,
	RSPI_SPI2 = 2
};

enum spi_pin
{
	MOSIA_NONE,	// not use output
	MOSIA_P16,
	MOSIA_PA6,
	MOSIA_PC6,
	MISOA_NONE,	// not use input
	MISOA_P17,
	MISOA_PA7,
	MISOA_PC7,
	RSPCKA_PA5,
	RSPCKA_PB0,
	RSPCKA_PC5,
	SSLA_NONE,
	SSLA0_NONE,
	SSLA0_PA4,
	SSLA0_PC4,
	SSLA1_NONE,
	SSLA1_PA0,
	SSLA1_PC0,
	SSLA2_NONE,
	SSLA2_PA1,
	SSLA2_PC1,
	SSLA3_NONE,
	SSLA3_PA2,
	SSLA3_PC2,
	MOSIB_NONE,
	MOSIB_P26,
	MOSIB_PE2,
	MOSIB_PE6,
	MISOB_NONE,
	MISOB_P30,
	MISOB_PE3,
	MISOB_PE7,
	RSPCKB_P27,
	RSPCKB_PE1,
	RSPCKB_PE5,
	SSLB_NONE,
	SSLB0_NONE,
	SSLB0_P31,
	SSLB0_PE4,
	SSLB1_NONE,
	SSLB1_P50,
	SSLB1_PE0,
	SSLB2_NONE,
	SSLB2_P51,
	SSLB2_PE1,
	SSLB3_NONE,
	SSLB3_P52,
	SSLB3_PE2
};

enum spi_bit_order{
	RSPI_LSBFIRST, 
	RSPI_MSBFIRST  
};

enum spi_clock_div
{
	RSPI_CLOCK_DIV2 = 0x00,  
	RSPI_CLOCK_DIV4 = 0x01,  
	RSPI_CLOCK_DIV8 = 0x03,  
	RSPI_CLOCK_DIV16 = 0x07, 
	RSPI_CLOCK_DIV32 = 0x0F, 
	RSPI_CLOCK_DIV64 = 0x1F, 
	RSPI_CLOCK_DIV128 = 0x3F
};

enum spi_mode
{
	RSPI_MODE0 = 0x00,
	RSPI_MODE1 = 0x01,
	RSPI_MODE2 = 0x02,
	RSPI_MODE3 = 0x03	// <- normarly kore!
};

#define SPI_MODE_MASK 0x0003  // PHA = bit 0, POL = 1

enum spi_ssl
{
	RSPI_SSL0 = 0x00,
	RSPI_SSL1 = 0x01,
	RSPI_SSL2 = 0x02,
	RSPI_SSL3 = 0x03
};

struct spi_config
{
	bool_t			TxEnable;
	spi_module		SpiModule;
	spi_bit_order	SpiBitOrder;
	spi_clock_div	SpiDiv;
	spi_mode		SpiMode;
};

class Spi_t
{
private:
	spi_config _SpiConfig;
	RingBuffer<uint8_t> *TxBuff;
	RingBuffer<uint8_t> *RxBuff;
	bool_t fEmptySpiTxBuff;
	spi_pin _pinSSLn0, _pinSSLn1, _pinSSLn2, _pinSSLn3;	// enabled pin for SSLn0
	bool_t fBufferAttached;
	bool_t fUseRx;
	
	// コールバック関数
	void (*CallBackFuncTx)();
	void (*CallBackFuncRx)();

	// レジスタ
	volatile __evenaccess struct st_rspi *SPIreg;
	//volatile __evenaccess struct st_rspi *SPIreg[3];
	
	void initRegister(bool_t fUseRx);
	void setPinModeSpi(spi_pin PinMOSI, spi_pin PinMISO, spi_pin PinRSPCK, spi_pin PinSSLn0, spi_pin PinSSLn1, spi_pin PinSSLn2, spi_pin PinSSLn3);
	// TxBuffEmptyInterrupt
	void enableTxBuffEmptyInterrupt(void){ SPIreg->SPCR.BIT.SPTIE = 1; }
	void disableTxBuffEmptyInterrupt(void){ SPIreg->SPCR.BIT.SPTIE = 0; }
	bool_t isEnableTxInterrupt(void){ return SPIreg->SPCR.BIT.SPTIE; }
	// SpiIdle
	void enableSpiIdleInterrupt(void){ SPIreg->SPCR2.BIT.SPIIE = 1; }
	void disableSpiIdleInterrupt(void){ SPIreg->SPCR2.BIT.SPIIE = 0; }
	bool_t isEnableSpiIdleInterrupt(void){ return SPIreg->SPCR2.BIT.SPIIE; }
	
	// tx, rx spi module register
	void setSpiTxData(uint8_t Data){ SPIreg->SPDR.WORD.H = Data; }
	uint8_t getSpiRxData(void){ return SPIreg->SPDR.WORD.H; }
	
public:
	// Constructor
	Spi_t(spi_module SpiModule);
	~Spi_t(void);
	
	// Configration
	int8_t begin(spi_pin PinMOSI, spi_pin PinMISO, spi_pin PinRSPCK, spi_pin PinSSLn0, spi_pin PinSSLn1, spi_pin PinSSLn2, spi_pin PinSSLn3);	// set all SSLn pin to use.
	int8_t begin(spi_pin PinMOSI, spi_pin PinMISO, spi_pin PinRSPCK, spi_pin PinSSLn0, spi_pin PinSSLn1, spi_pin PinSSLn2, spi_pin PinSSLn3, uint16_t BuffNum);
	int8_t begin(spi_pin PinMOSI, spi_pin PinMISO, spi_pin PinRSPCK, spi_pin PinSSLn0);													// set only SSLn pin to use. already used SSLn pin is sonomama. 
	int8_t begin(spi_pin PinMOSI, spi_pin PinMISO, spi_pin PinRSPCK, spi_pin PinSSLn, uint16_t BuffNum);
	void end(void);
	void setBitOrder(spi_bit_order bitOrder);
	void setDataMode(spi_mode mode);
	void setClockDivider(spi_clock_div rate);
	void setCS(spi_ssl ssl);	//SSLを設定する. 変更するときはisIdleで通信が終わっていることを確認してから。 
	
	// Transmit/Receive
	int8_t transmit(uint8_t TrData);
	int8_t receive(uint8_t *RcData);
	
	// get status
	bool_t isIdle(void){ return !SPIreg->SPSR.BIT.IDLNF; };
	//bool_t isEmptySpiTxBuff(void){return fEmptySpiTxBuff;};					// RSPIの送信バッファSPDRが空か(送信割り込み入った時にバッファがなければ1になる)
	bool_t isTxBuffEmpty(void){ return TxBuff->isEmpty(); };
	uint16_t getTxBuffNum(void){ return TxBuff->getNumElements(); };
	uint16_t getTxBuffSize(void){ return TxBuff->getBufferSize(); };
	uint16_t getTxBuffWriteIndex(void){ return TxBuff->getWriteIndex(); };
	uint16_t getTxBuffReadIndex(void){ return TxBuff->getReadIndex(); };
	uint16_t getRxBuffWriteIndex(void){ return RxBuff->getWriteIndex(); };
	uint16_t getRxBuffReadIndex(void){ return RxBuff->getReadIndex(); };
	
	// コールバック関数を登録
	void setCallBackFuncTx(void (*CallBackFuncTx)(void));
	void setCallBackFuncRx(void (*CallBackFuncRx)(void));
	
	// isr
	void isrTx(void);
	void isrRx(void);
	//void isrIdle(void);
	
};


extern Spi_t SPI0;
extern Spi_t SPI1;


#endif