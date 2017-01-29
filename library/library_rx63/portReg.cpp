
#include "portReg.h"

// XV‚µ‚½‚¢
// ‘¶Ý‚µ‚È‚¢ƒsƒ“”Ô†‚¾‚Á‚½‚ç‰½‚à‚µ‚È‚¢


void setPinMode(pins Pin, pin_mode PinMode){
	
	uint8_t port = (Pin>>4)&0xFF;	
	uint8_t bit = (Pin&0x0F);
	
	volatile uint8_t *adr_pdr = GET_ADR_PDR(port);
	volatile uint8_t *adr_pcr = GET_ADR_PCR(port);
	
	// Strip the pin of its peripheral functions, if any.
	bool_t PmrTmp = bitRead(*GET_ADR_PMR(port), bit);
	bitClear(*GET_ADR_PMR(port), bit);
	
	// Affect the mode.
	switch (PinMode)
	{
	// --------------------------------------------------------------------/
	case PIN_INPUT:
		bitClear(*adr_pdr, bit);
		bitClear(*adr_pcr, bit);
		break;
	// --------------------------------------------------------------------/
	case PIN_OUTPUT:
		if (Pin != P35)
		{
			bitSet(*adr_pdr, bit);
			bitClear(*adr_pcr, bit);
		}
		else
		{
		//	LOG_INFO(MODULE, "pinMode: IO54 is input only");
		}
		break;
	// --------------------------------------------------------------------/
	case PIN_INPUT_PULLUP:
		bitClear(*adr_pdr, bit);
		bitSet(*adr_pcr, bit);
		break;
	// --------------------------------------------------------------------/
	case PIN_OUTPUT_OPENDRAIN:
		if (Pin != P35)
		{
			bitSet(*adr_pdr, bit);
			bitClear(*adr_pcr, bit);
			if (bit <= 3) {
				bitSet(*GET_ADR_ODR0(port), 2 * bit);
			} else {
				bitSet(*GET_ADR_ODR1(port), 2 * bit);
			}
		}
		else
		{
			//LOG_INFO(MODULE, "pinMode: IO54 is input only");
		}

		break;
	default:
		//LOG_ERROR(MODULE, "pinMode: Unknown mode");
		break;
	} // switch : mode.

	//g_pinsCurrentAttachFunction[pin] = _ATTACH_OTHER;
	
	if(PmrTmp){
		bitSet(*GET_ADR_PMR(port), bit);
	}else{
		bitClear(*GET_ADR_PMR(port), bit);
	}
}

bool_t readPin(pins Pin){
	
	uint8_t port = (Pin>>4)&0xFF;
	uint8_t bit = (Pin&0x0F);

	if (bitRead(*GET_ADR_PIDR(port), bit)){
		return 1;
	}else{
		return 0;
	}
}

void outPin(pins Pin, bool_t Out){
	
	if (Pin != P35)	{
		uint8_t port = (Pin>>4)&0xFF;
		uint8_t bit = (Pin&0x0F);
		volatile uint8_t* adr_podr = GET_ADR_PODR(port);
		
		switch (Out)
		{
		// ----------------------------------------------------------------/
		case 1:
			bitSet(*adr_podr, bit);
			break;
		// ----------------------------------------------------------------/
		case 0:
			bitClear(*adr_podr, bit);
			break;
		// ----------------------------------------------------------------/
		default:
		//	LOG_ERROR(MODULE, "digitalWrite: Unknown value");
			break;
		} // switch : value.
	}else{
	//	LOG_INFO(MODULE, "digitalWrite: IO54 is input only");
	}
}

