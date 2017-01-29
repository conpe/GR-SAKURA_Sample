/********************************************/
/*	STmicro VL6180 (I2C)		    */
/*			for RX63n @ CS+     */
/*			Wrote by conpe_	    */
/*			2016/05/23	    */
/********************************************/

//�y�g�����z
// �܂�begin();��I2C������
// �Z���T����v���l���擾���邽�߁A����I��measure()���Ă�(I2C����M�J�n)�B
// getDistance()�Ōv���l��Ԃ��B

// �f�t�H���g�ݒ�
// I2C�A�h���X 0x29

//�y�X�V�����z
// 2016.05.23 �V�K�쐬

//�y�X�V�������z
// attach�̖߂�l�́A�A�^�b�`���s�ɉ����āA�f�o�C�X�ƒʐM���ł��Ȃ��������Ƃ�Ԃ�����
// ���󂾂ƃA�^�b�`���s�݂̂̔���Ȃ̂ŁA�f�o�C�X�ƒʐM�ł��Ȃ��Ă�ok����ɂȂ�B
// �f�o�C�X�ƒʐM�ł��Ȃ��ꍇ�A�o�b�t�@�N���A�����Ⴄ�̂ŁA�A�^�b�`���̂͐�������B
// ��C�ɑ��肷�������������s����B

#ifndef __STVL53L0X_H__
#define __STVL53L0X_H__

#include "RIIC.h"
#include "CommonDataType.h"
// debug
#include "SCI.h"

// �f�t�H���g�ݒ�
#define STVL5_ADRS_DEFAULT		0x29
#define STVL5_DISTANCE_DEFAULT	-1
#define STVL5_FAIL_DISTANCE_MM	0x1FFF

// �ǂ����ɂ���
extern uint32_t getTime_ms(void);

class stvl5_comus_t;

class stmicro_VL53L0X{
public:
	enum register_e{
		SYSRANGE_START			= 0x00,
		SYSTEM_THRESH_HIGH		= 0x0C,
		SYSTEM_THRESH_LOW		= 0x0E,
		SYSTEM_SEQUENCE_CONFIG		= 0x01,
		SYSTEM_RANGE_CONFIG		= 0x09,
		SYSTEM_INTERMEASUREMENT_PERIOD	= 0x04,
		SYSTEM_INTERRUPT_CONFIG_GPIO	= 0x0A,
		GPIO_HV_MUX_ACTIVE_HIGH		= 0x84,
		SYSTEM_INTERRUPT_CLEAR		= 0x0B,
		RESULT_INTERRUPT_STATUS		= 0x13,
		RESULT_RANGE_STATUS		= 0x14,
		RESULT_RANGE			= 0x1E,
		RESULT_CORE_AMBIENT_WINDOW_EVENTS_RTN	= 0xBC,
		RESULT_CORE_RANGING_TOTAL_EVENTS_RTN	= 0xC0,
		RESULT_CORE_AMBIENT_WINDOW_EVENTS_REF	= 0xD0,
		RESULT_CORE_RANGING_TOTAL_EVENTS_REF	= 0xD4,
		RESULT_PEAK_SIGNAL_RATE_REF		= 0xB6,
		ALGO_PART_TO_PART_RANGE_OFFSET_MM	= 0x28,
		I2C_SLAVE_DEVICE_ADDRESS		= 0x8A,
		MSRC_CONFIG_CONTROL			= 0x60,
		PRE_RANGE_CONFIG_MIN_SNR		= 0x27,
		PRE_RANGE_CONFIG_VALID_PHASE_LOW	= 0x56,
		PRE_RANGE_CONFIG_VALID_PHASE_HIGH	= 0x57,
		PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT	= 0x64,
		FINAL_RANGE_CONFIG_MIN_SNR		= 0x67,
		FINAL_RANGE_CONFIG_VALID_PHASE_LOW	= 0x47,
		FINAL_RANGE_CONFIG_VALID_PHASE_HIGH	= 0x48,
		FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT = 0x44,
		PRE_RANGE_CONFIG_SIGMA_THRESH_HI	= 0x61,
		PRE_RANGE_CONFIG_SIGMA_THRESH_LO	= 0x62,
		PRE_RANGE_CONFIG_VCSEL_PERIOD		= 0x50,
		PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI	= 0x51,
		PRE_RANGE_CONFIG_TIMEOUT_MACROP_LO	= 0x52,
		SYSTEM_HISTOGRAM_BIN			= 0x81,
		HISTOGRAM_CONFIG_INITIAL_PHASE_SELECT	= 0x33,
		HISTOGRAM_CONFIG_READOUT_CTRL		= 0x55,
		FINAL_RANGE_CONFIG_VCSEL_PERIOD		= 0x70,
		FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI	= 0x71,
		FINAL_RANGE_CONFIG_TIMEOUT_MACROP_LO	= 0x72,
		CROSSTALK_COMPENSATION_PEAK_RATE_MCPS	= 0x20,
		MSRC_CONFIG_TIMEOUT_MACROP		= 0x46,
		SOFT_RESET_GO2_SOFT_RESET_N		= 0xBF,
		IDENTIFICATION_MODEL_ID			= 0xC0,
		IDENTIFICATION_REVISION_ID		= 0xC2,
		OSC_CALIBRATE_VAL			= 0xF8,
		GLOBAL_CONFIG_VCSEL_WIDTH		= 0x32,
		GLOBAL_CONFIG_SPAD_ENABLES_REF_0	= 0xB0,
		GLOBAL_CONFIG_SPAD_ENABLES_REF_1	= 0xB1,
		GLOBAL_CONFIG_SPAD_ENABLES_REF_2	= 0xB2,
		GLOBAL_CONFIG_SPAD_ENABLES_REF_3	= 0xB3,
		GLOBAL_CONFIG_SPAD_ENABLES_REF_4	= 0xB4,
		GLOBAL_CONFIG_SPAD_ENABLES_REF_5	= 0xB5,
		GLOBAL_CONFIG_REF_EN_START_SELECT	= 0xB6,
		DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD	= 0x4E,
		DYNAMIC_SPAD_REF_EN_START_OFFSET	= 0x4F,
		POWER_MANAGEMENT_GO1_POWER_FORCE	= 0x80,
		VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV	= 0x89,
		ALGO_PHASECAL_LIM			= 0x30,
		ALGO_PHASECAL_CONFIG_TIMEOUT		= 0x30,
	};
	
	uint8_t I2cAddress;		// I2C�A�h���X
	
	// �R���X�g���N�^
	stmicro_VL53L0X(I2c_t* I2Cn, uint8_t I2cAddress = STVL5_ADRS_DEFAULT);
	// �f�X�g���N�^
	virtual ~stmicro_VL53L0X(void);
	
	//����J�n
	int8_t begin(void);
	
	// �o�[�X�g�v���J�n
	int8_t startMeasurement(void);
	// �o�[�X�g�v����~
	int8_t stopMeasurement(void);
	
	
	// �v�����ʎ擾
	int8_t getDistance(int16_t* Dist);
	int16_t getDistance(void){return Distance_mm;};
	
	// ���茋�ʎ擾�v��
	int8_t measure(void);
	// ��������Z�b�g
	int8_t setMeasurePeriod(uint16_t msec);
	// ���荞�݃N���A
	int8_t clearInterrupt(void);
	//I2C�A�h���X�ݒ�
	int8_t setI2cAddress(uint8_t NewAdrs);
	
	// �e��ʐM
	int8_t setReg(uint8_t ComuReg, uint8_t SetData);	// ���W�X�^�Z�b�g(���荞��)
	int8_t reqDat(uint8_t ComuReg, uint16_t ReqNum);	// �f�[�^��M(���荞��)
	
	int8_t writeReg(uint8_t ComuReg, uint8_t SetData);		// �f�[�^�����M(1byte)
	int8_t writeReg(uint8_t ComuReg, uint8_t* SetData, uint8_t SendNum);	// �f�[�^�����M(nbyte)
	int8_t writeRegWord(uint8_t ComuReg, uint16_t SetData);	// �f�[�^�����M(2byte)
	int8_t writeRegDWord(uint8_t ComuReg, uint32_t SetData);	// �f�[�^�����M(4byte)
	uint8_t readReg(uint8_t ComuReg);				// �f�[�^����M(1byte)
	int8_t readReg(uint8_t ComuReg, uint8_t* RcvData, uint8_t RcvNum);	// �f�[�^����M(nbyte)
	uint16_t readRegWord(uint8_t ComuReg);			// �f�[�^����M(2byte)
	uint32_t readRegDWord(uint8_t ComuReg);			// �f�[�^����M(4byte)
	
	
	// �����Ă邩����
	bool_t isComuErr(void){return fI2cErr;};
	bool_t isAlive(void){return RangeStatus;};
	//bool_t isAlive(void){return RangeReturnSignalCount;};
	int8_t checkAlive(void){ return reqDat(RESULT_RANGE_STATUS, 1); };
	//int8_t checkAlive(void){ return reqDat(RESULT_RANGE_RETURN_SIGNAL_COUNT, 1); };
	
	// �����̋V��
	int8_t reset(void);
	
	// �ʐM�A�C�h����Ԃ�
	// I2C�A�h���X�ݒ莞�ɏ��ɋN�������邽�߂ɕK�v
	bool_t isIdleComu(void){return I2Cn->isIdle();};
	
	
	// �^�C���A�E�g���Ԑݒ� [msec]
	void setTimeout(uint32_t timout){io_timeout = timout;};
	
	
	
	// ��M�f�[�^�����p�R�[���o�b�N�֐�
	// I2C��M�f�[�^���擾���ĒʐM���e���Ƃɏ��������
	int8_t fetchI2cRcvData(const stvl5_comus_t* Comu);
	
	
	
	/*************************/
	//	VL53L0 API
	/*************************/
	bool_t init(bool_t io_2v8 = true);
	
	enum vcselPeriodType { VcselPeriodPreRange, VcselPeriodFinalRange };
	
	bool setSignalRateLimit(float limit_Mcps);
    float getSignalRateLimit(void);

    bool setMeasurementTimingBudget(uint32_t budget_us);
    uint32_t getMeasurementTimingBudget(void);

    bool setVcselPulsePeriod(vcselPeriodType type, uint8_t period_pclks);
    uint8_t getVcselPulsePeriod(vcselPeriodType type);

    void startContinuous(uint32_t period_ms = 0);
    void stopContinuous(void);
    uint16_t readRangeContinuousMillimeters(void);
    uint16_t readRangeSingleMillimeters(void);

    inline void setTimeout(uint16_t timeout) { io_timeout = timeout; }
    inline uint16_t getTimeout(void) { return io_timeout; }
    bool timeoutOccurred(void);
	
    
private:
	I2c_t *I2Cn;
	uint8_t I2cAddressPre;		// I2C�A�h���X(�m��O)
	uint8_t *RcvBuff;
	int16_t Distance_mm;	// �v����������[mm]
	bool_t fI2cErr;			// I2C�ʐM�G���[ ->����M��߂�
	uint8_t *FuseRcv;	// 
	uint8_t cntget;
	
	uint8_t RangeStatus;
	uint32_t RangeReturnSignalCount;
	
	
	// I2C�N���X�ɒʐM���e���A�^�b�`
	int8_t attachI2cComu(uint8_t ComuReg, uint8_t SetData, uint16_t RxNum);
	
	
	
	// VL53L0 API
	
	// TCC: Target CentreCheck
    // MSRC: Minimum Signal Rate Check
    // DSS: Dynamic Spad Selection

    struct SequenceStepEnables
    {
      bool_t tcc, msrc, dss, pre_range, final_range;
    };

    struct SequenceStepTimeouts
    {
      uint16_t pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks;

      uint16_t msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks;
      uint32_t msrc_dss_tcc_us,    pre_range_us,    final_range_us;
    };

    uint8_t address;
    uint32_t io_timeout;
    bool did_timeout;
    uint32_t timeout_start_ms;

    uint8_t stop_variable; // read by init and used when starting measurement; is StopVariable field of VL53L0X_DevData_t structure in API
    uint32_t measurement_timing_budget_us;

    bool getSpadInfo(uint8_t * count, bool * type_is_aperture);

    void getSequenceStepEnables(SequenceStepEnables * enables);
    void getSequenceStepTimeouts(SequenceStepEnables const * enables, SequenceStepTimeouts * timeouts);

    bool performSingleRefCalibration(uint8_t vhv_init_byte);

    static uint16_t decodeTimeout(uint16_t value);
    static uint16_t encodeTimeout(uint16_t timeout_mclks);
    static uint32_t timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks);
    static uint32_t timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks);
    
};

class stvl5_comus_t:public I2c_comu_t {
public:
	stvl5_comus_t(uint8_t DestAddress, uint8_t* TxData, uint16_t TxNum, uint16_t RxNum)	: I2c_comu_t(DestAddress, TxData, TxNum, RxNum){DevId = (0x03);};	// ����M
	
	//uint8_t DevId;	// �f�o�C�XID
	stmicro_VL53L0X* STVL5;
	uint8_t ComuReg;	// �Ώۃ��W�X�^

	void callBack(void){
		STVL5->fetchI2cRcvData(this);
	};
};

#endif
