/********************************************/
/*		RX63nの汎用IOポート操作				*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2014/12/30		*/
/********************************************/

#ifndef __PORTREG_RX63_H__
#define __PORTREG_RX63_H__

#include "iodefine.h"
#include "CommonDataType.h"

#define bit(b)                         (1UL << (b))
#define bitRead(value, bit)            (((value) >> (bit)) & 0x01)
#define bitSet(value, bit)             ((value) |= (1UL << (bit)))
#define bitClear(value, bit)           ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#define  GET_ADR_PDR(a)  ((volatile uint8_t *)&PORT0.PDR  + ((a)       / sizeof(uint8_t)) )
#define  GET_ADR_PODR(a) ((volatile uint8_t *)&PORT0.PODR + ((a)       / sizeof(uint8_t)) )
#define  GET_ADR_PIDR(a) ((volatile uint8_t *)&PORT0.PIDR + ((a)       / sizeof(uint8_t)) )
#define  GET_ADR_PMR(a)  ((volatile uint8_t *)&PORT0.PMR  + ((a)       / sizeof(uint8_t)) )
#define  GET_ADR_ODR0(a) ((volatile uint8_t *)&PORT0.ODR0 + (((a) * 2) / sizeof(uint8_t)) )
#define  GET_ADR_ODR1(a) ((volatile uint8_t *)&PORT0.ODR1 + (((a) * 2) / sizeof(uint8_t)) )
#define  GET_ADR_PCR(a)  ((volatile uint8_t *)&PORT0.PCR  + ((a)       / sizeof(uint8_t)) )
#define  GET_ADR_DSCR(a) ((volatile uint8_t *)&PORT0.DSCR + ((a)       / sizeof(uint8_t)) )

enum pins{
	P05 = 0x05,
	P07 = 0x07,
	P12 = 0x12,
	P13,
	P14,
	P15,
	P16,
	P17,
	P20 = 0x20,
	P21,
	P22,
	P23,
	P24,
	P25,
	P26,
	P27,
	P30 = 0x30,	
	P31,
	P32,
	P33,
	P34,
	P35,
	P40 = 0x40,
	P41,
	P42,
	P43,
	P44,
	P45,
	P46,
	P47,
	P50 = 0x50,
	P51,
	P52,
	P53,
	P54,
	P55,
	PA0 = 0xA0,
	PA1,
	PA2,
	PA3,
	PA4,
	PA5,
	PA6,
	PA7,
	PC0 = 0xC0,
	PC1,
	PC2,
	PC3,
	PC4,
	PC5,
	PC6,
	PC7,
	PD0 = 0xD0,
	PD1,
	PD2,
	PD3,
	PD4,
	PD5,
	PD6,
	PD7,
	PE0 = 0xE0,
	PE1,
	PE2,
	PE3,
	PE4,
	PE5,
	PE6,
	PE7,
	PJ3 = 0x0123
};

enum pin_mode{
	PIN_INPUT = 0,
	PIN_OUTPUT = 1,
	PIN_INPUT_PULLUP,
	PIN_OUTPUT_OPENDRAIN
};

void setPinMode(pins Pin, pin_mode PinMode);
bool_t readPin(pins Pin);
void outPin(pins Pin, bool_t Out);

/*
static inline void BSET(volatile byte* port, int bit)
{
    __asm __volatile(
        "bset %1, [%0].b\n"
        :
        : "r" (port), "r" (bit)
        :
    );
}

static inline void BCLR(volatile byte* port, int bit)
{
    __asm __volatile(
        "bclr %1, [%0].b\n"
        :
        : "r" (port), "r" (bit)
        :
    );
}

static inline int BTST(volatile byte* port, int bit)
{
    int ret;
    __asm __volatile(
        "btst %2, [%1].b\n"
        "scnz.l %0\n"
        : "=r" (ret)
        : "r" (port), "r" (bit)
        :
    );
    return ret;
}
*/




#endif