
#include "aplMemCtrl.h"

#define DATAID_OK	0
#define DATAID_NG	1


static void readMemEep(uint16_t Id, void* ReadData);
static int8_t readMemEep_UB(uint16_t Id, uint8_t* ReadData);
static int8_t readMemEep_UH(uint16_t Id, uint16_t* ReadData);
static int8_t readMemEep_UW(uint16_t Id, uint32_t* ReadData);
static int8_t readMemEep_FL(uint16_t Id, float* ReadData);
static int8_t readMemEep_DB(uint16_t Id, double* ReadData);

// �e�[�u���C���f�b�N�X�ŏ�������
static int8_t writeMemEep(uint16_t Idx, void* WriteData);		// �������݃f�[�^�̃A�h���X
static int8_t writeMemEepFloatVal(uint16_t Idx, float WriteData);	// float�l����L���X�g���ď���
static int8_t writeMemEep_UB(uint16_t Idx, uint8_t Data);		// �w��f�[�^�^�̒l
static int8_t writeMemEep_UH(uint16_t Idx, uint16_t Data);
static int8_t writeMemEep_UW(uint16_t Idx, uint32_t Data);
static int8_t writeMemEep_FL(uint16_t Idx, float Data);
static int8_t writeMemEep_DB(uint16_t Idx, double Data);

// EEPROM����RAM�ϐ��֓ǂݍ���
static void EepToRamAll(void);

static int8_t searchTableIdx_MemTable(uint16_t Id, uint16_t *TableIdx);		// �f�[�^ID�ɑΉ�����f�[�^�e�[�u���C���f�b�N�X���擾


static void AllInitTask(void);

static uint8_t fAllFormatting = 0;		// �S�萔��������


// E2�f�[�^�t���b�V����������
int8_t initMem(void){
	int8_t ret;
	
	// �h���C�o������
	ret = drvDataFlash_init();
	
	// �h���C�o�Ƀ^�X�N�o�^
	drvDataFlash_attachWriteTask(AllInitTask);
	
	// EEPROM���e��RAM�֓ǂݏo��
	EepToRamAll();
	
	return ret;
}

// EEPROM���e�����ׂ�RAM�֓ǂݏo��
void EepToRamAll(void){
	uint32_t Id;
	uint32_t dummy;
	for(Id=0;Id<MEM_USED_NUM;Id++){
		readMemEep(Id, &dummy);
	}
}

// EEPROM����f�[�^�ǂݍ���
// RAM���X�V
static void readMemEep(uint16_t Id, void* ReadData){
	
	switch(stMemTable[Id].DataType){
	case enUB:
	case enB:
		if(FLD_OK == readMemEep_UB(Id, (uint8_t *)ReadData)){
			*(uint8_t *)stMemTable[Id].Val = *(uint8_t *)ReadData;
		}
		break;
	case enUH:
	case enH:
		if(FLD_OK == readMemEep_UH(Id, (uint16_t *)ReadData)){
			*(uint16_t *)stMemTable[Id].Val = *(uint16_t *)ReadData;
		}
		break;
	case enUW:
	case enW:
		if(FLD_OK == readMemEep_UW(Id, (uint32_t *)ReadData)){
			*(uint32_t *)stMemTable[Id].Val = *(uint32_t *)ReadData;
		}
		break;
	case enFL:
		if(FLD_OK == readMemEep_FL(Id, (float *)ReadData)){
			*(float *)stMemTable[Id].Val = *(float *)ReadData;
		}
		break;
	case enDB:
		if(FLD_OK == readMemEep_DB(Id, (double *)ReadData)){
			*(double *)stMemTable[Id].Val = *(double *)ReadData;
		}
		break;
	}
	
}

// int8_t�ǂݍ���
static int8_t readMemEep_UB(uint16_t Id, uint8_t* ReadData){
	int8_t ret;
	
	ret = drvDataFlash_read(stMemTable[Id].Adrs, ReadData);
	
	return ret;
}
// int16_t�ǂݍ���
static int8_t readMemEep_UH(uint16_t Id, uint16_t* ReadData){
	int8_t ret;
	uint8_t tmp[2];
	
	drvDataFlash_read(stMemTable[Id].Adrs, &tmp[0]);
	ret = drvDataFlash_read(stMemTable[Id].Adrs+1, &tmp[1]);
	
	*ReadData = (((uint16_t)tmp[0])<<8) | (uint16_t)tmp[1];
	
	return ret;
}
// int32_t�ǂݍ���
static int8_t readMemEep_UW(uint16_t Id, uint32_t* ReadData){
	int8_t ret;
	uint8_t tmp[4];
	
	drvDataFlash_read(stMemTable[Id].Adrs, &tmp[0]);
	drvDataFlash_read(stMemTable[Id].Adrs+1, &tmp[1]);
	drvDataFlash_read(stMemTable[Id].Adrs+2, &tmp[2]);
	ret = drvDataFlash_read(stMemTable[Id].Adrs+3, &tmp[3]);
	
	*ReadData = (((uint32_t)tmp[0])<<8) | (((uint32_t)tmp[1])<<8) | (((uint32_t)tmp[2])<<8) | (uint32_t)tmp[3];
	
	return ret;
}
// float�ǂݍ���
static int8_t readMemEep_FL(uint16_t Id, float* ReadData){
	int8_t ret;
	UN_FL_DATA flTmp;
	
	drvDataFlash_read(stMemTable[Id].Adrs,   &flTmp.Bit[0]);
	drvDataFlash_read(stMemTable[Id].Adrs+1, &flTmp.Bit[1]);
	drvDataFlash_read(stMemTable[Id].Adrs+2, &flTmp.Bit[2]);
	ret = drvDataFlash_read(stMemTable[Id].Adrs+3, &flTmp.Bit[3]);
	
	*ReadData = flTmp.Val;
	
	return ret;
}
// double�ǂݍ���
static int8_t readMemEep_DB(uint16_t Id, double* ReadData){
	int8_t ret;
	UN_DB_DATA dbTmp;
	
	drvDataFlash_read(stMemTable[Id].Adrs,   &dbTmp.Bit[0]);
	drvDataFlash_read(stMemTable[Id].Adrs+1, &dbTmp.Bit[1]);
	drvDataFlash_read(stMemTable[Id].Adrs+2, &dbTmp.Bit[2]);
	drvDataFlash_read(stMemTable[Id].Adrs+3, &dbTmp.Bit[3]);
	drvDataFlash_read(stMemTable[Id].Adrs+4, &dbTmp.Bit[4]);
	drvDataFlash_read(stMemTable[Id].Adrs+5, &dbTmp.Bit[5]);
	drvDataFlash_read(stMemTable[Id].Adrs+6, &dbTmp.Bit[6]);
	ret = drvDataFlash_read(stMemTable[Id].Adrs+7, &dbTmp.Bit[7]);
	
	*ReadData = dbTmp.Val;
	
	return ret;
}


// E2�f�[�^�t���b�V������ǂ�
// RAM����
int8_t readMem(uint16_t Id, void* ReadData){
	
	switch(stMemTable[Id].DataType){
	case enUB:
	case enB:
		*(uint8_t *)ReadData = *(uint8_t *)stMemTable[Id].Val;
		break;
	case enUH:
	case enH:
		*(uint16_t *)ReadData = *(uint16_t *)stMemTable[Id].Val;
		break;
	case enUW:
	case enW:
		*(uint32_t *)ReadData = *(uint32_t *)stMemTable[Id].Val;
		break;
	case enFL:
		*(float *)ReadData = *(float *)stMemTable[Id].Val;
		break;
	case enDB:
		*(double *)ReadData = *(double *)stMemTable[Id].Val;
		break;
	}
	
	return 0;
}


// E2�f�[�^�t���b�V���֏���
int8_t writeMem(uint16_t Id, void* WriteData){
	uint16_t Idx;
	
	// ID����
	if(DATAID_OK == searchTableIdx_MemTable(Id, &Idx)){
		return writeMemEep(Idx, WriteData);
	}else{
		return -4;	// outofaddress
	}
}

// �e�[�u���C���f�b�N�X�Ə������݃f�[�^�̃A�h���X�ŏ���
static int8_t writeMemEep(uint16_t Idx, void* WriteData){
	int8_t ret;
	
	
	switch(stMemTable[Idx].DataType){
	case enUB:
	case enB:
		ret = writeMemEep_UB(Idx, *(uint8_t *)WriteData);		// EEPROM�f�[�^�X�V -> ������������X�V���Ȃ��悤�ɂ�����
		*(uint8_t *)stMemTable[Idx].Val = *(uint8_t *)WriteData;	// RAM�X�V
		break;
	case enUH:
	case enH:
		ret = writeMemEep_UH(Idx, *(uint16_t *)WriteData);
		*(uint16_t *)stMemTable[Idx].Val = *(uint16_t *)WriteData;
		break;
	case enUW:
	case enW:
		ret = writeMemEep_UW(Idx, *(uint32_t *)WriteData);
		*(uint32_t *)stMemTable[Idx].Val = *(uint32_t *)WriteData;
		break;
	case enFL:
		ret = writeMemEep_FL(Idx, *(float *)WriteData);
		*(float *)stMemTable[Idx].Val = *(float *)WriteData;
		break;
	case enDB:
		ret = writeMemEep_DB(Idx, *(double *)WriteData);
		*(double *)stMemTable[Idx].Val = *(double *)WriteData;
		break;
	}
	
	return ret;
}
// �e�[�u���C���f�b�N�X�Ə������݃f�[�^��float�l�ŏ���
static int8_t writeMemEepFloatVal(uint16_t Idx, float WriteData){
	int8_t ret;
	
	
	switch(stMemTable[Idx].DataType){
	case enUB:
	case enB:
		ret = writeMemEep_UB(Idx, (uint8_t)WriteData);		// EEPROM�f�[�^�X�V -> ������������X�V���Ȃ��悤�ɂ�����
		*(uint8_t *)stMemTable[Idx].Val = (uint8_t)WriteData;	// RAM�X�V
		break;
	case enUH:
	case enH:
		ret = writeMemEep_UH(Idx, (uint16_t)WriteData);
		*(uint16_t *)stMemTable[Idx].Val = (uint16_t)WriteData;
		break;
	case enUW:
	case enW:
		ret = writeMemEep_UW(Idx, (uint32_t)WriteData);
		*(uint32_t *)stMemTable[Idx].Val = (uint32_t)WriteData;
		break;
	case enFL:
		ret = writeMemEep_FL(Idx, (float)WriteData);
		*(float *)stMemTable[Idx].Val = (float)WriteData;
		break;
	case enDB:
		ret = writeMemEep_DB(Idx, (double)WriteData);
		*(double *)stMemTable[Idx].Val = (double)WriteData;
		break;
	}
	
	return ret;
}



int8_t writeMemUB(uint16_t Id, uint8_t Data){
	uint16_t Idx;
	
	// ID����
	if(DATAID_OK == searchTableIdx_MemTable(Id, &Idx)){
		return writeMemEep_UB(Idx, Data);
	}else{
		return -4;	// outofaddress
	}
}
int8_t writeMemUH(uint16_t Id, uint16_t Data){
	uint16_t Idx;
	
	// ID����
	if(DATAID_OK == searchTableIdx_MemTable(Id, &Idx)){
		return writeMemEep_UH(Idx, Data);
	}else{
		return -4;	// outofaddress
	}
}
int8_t writeMemUW(uint16_t Id, uint32_t Data){
	uint16_t Idx;
	
	// ID����
	if(DATAID_OK == searchTableIdx_MemTable(Id, &Idx)){
		return writeMemEep_UW(Idx, Data);
	}else{
		return -4;	// outofaddress
	}
}
int8_t writeMemFL(uint16_t Id, float Data){
	uint16_t Idx;
	
	// ID����
	if(DATAID_OK == searchTableIdx_MemTable(Id, &Idx)){
		return writeMemEep_FL(Idx, Data);
	}else{
		return -4;	// outofaddress
	}
}
int8_t writeMemDB(uint16_t Id, double Data){
	uint16_t Idx;
	
	// ID����
	if(DATAID_OK == searchTableIdx_MemTable(Id, &Idx)){
		return writeMemEep_DB(Idx, Data);
	}else{
		return -4;	// outofaddress
	}
}

// �e�[�u���C���f�b�N�X�ŏ�������
static int8_t writeMemEep_UB(uint16_t Idx, uint8_t Data){
	return drvDataFlash_write(stMemTable[Idx].Adrs, Data);
}
static int8_t writeMemEep_UH(uint16_t Idx, uint16_t Data){
	int8_t ret;
	
	ret = drvDataFlash_write(stMemTable[Idx].Adrs, (uint8_t)((Data>>8)&0x00FF));
	ret = drvDataFlash_write(stMemTable[Idx].Adrs+1, (uint8_t)(Data&0x00FF));
	
	return ret;
}
static int8_t writeMemEep_UW(uint16_t Idx, uint32_t Data){
	int8_t ret;
	
	ret = drvDataFlash_write(stMemTable[Idx].Adrs, (uint8_t)((Data>>24)&0x000000FF));
	ret = drvDataFlash_write(stMemTable[Idx].Adrs+1, (uint8_t)((Data>>16)&0x000000FF));
	ret = drvDataFlash_write(stMemTable[Idx].Adrs+2, (uint8_t)((Data>>8)&0x000000FF));
	ret = drvDataFlash_write(stMemTable[Idx].Adrs+3, (uint8_t)(Data&0x000000FF));
	
	return ret;
}
static int8_t writeMemEep_FL(uint16_t Idx, float Data){
	int8_t ret;
	uint8_t i;
	UN_FL_DATA Tmp;
	
	Tmp.Val = Data;
	
	for(i=0;i<4;i++){
		ret = drvDataFlash_write(stMemTable[Idx].Adrs+i, Tmp.Bit[i]);
	}
	
	return ret;
}
static int8_t writeMemEep_DB(uint16_t Idx, double Data){
	int8_t ret;
	uint8_t i;
	UN_DB_DATA Tmp;
	
	Tmp.Val = Data;
	for(i=0;i<8;i++){
		ret = drvDataFlash_write(stMemTable[Idx].Adrs+i, Tmp.Bit[i]);
	}
	
	return ret;
}




// �e�[�u���C���f�b�N�X����(������)
static int8_t searchTableIdx_MemTable(uint16_t Id, uint16_t *TableIdx){		// �f�[�^ID�ɑΉ�����f�[�^�e�[�u���C���f�b�N�X���擾
	*TableIdx = 0xFFFF;
	uint16_t i;
	
	for(i=0; i<MEM_USED_NUM; i++){
		if(Id == stMemTable[i].DataId){
			*TableIdx = i;
			return DATAID_OK;
		}
	}

	return DATAID_NG;
}


// �萔������
int8_t clearAllMem(void){
	int8_t ret;
	static uint16_t Id = 0;
	
	fAllFormatting = 1;
	
	for(; Id<MEM_USED_NUM; Id++){
		switch(stMemTable[Id].DataType){
		case enUB:
		case enB:
			ret = writeMemUB(Id, *(uint8_t *)stMemTable[Id].InitVal);
			*(uint8_t *)stMemTable[Id].Val = *(uint8_t *)stMemTable[Id].InitVal;	// RAM�Z�b�g
			break;
		case enUH:
		case enH:
			ret = writeMemUH(Id, *(uint16_t *)stMemTable[Id].InitVal);
			*(uint16_t *)stMemTable[Id].Val = *(uint16_t *)stMemTable[Id].InitVal;
			break;
		case enUW:
		case enW:
			ret = writeMemUW(Id, *(uint32_t *)stMemTable[Id].InitVal);
			*(uint32_t *)stMemTable[Id].Val = *(uint32_t *)stMemTable[Id].InitVal;
			break;
		case enFL:
			ret = writeMemFL(Id, *(float *)stMemTable[Id].InitVal);
			*(float *)stMemTable[Id].Val = *(float *)stMemTable[Id].InitVal;
			break;
		case enDB:
			ret = writeMemDB(Id, *(double *)stMemTable[Id].InitVal);
			*(double *)stMemTable[Id].Val = *(double *)stMemTable[Id].InitVal;
			break;
		}
		
		if(ret != DF_OK){
			return -1;
		}
	}
	
	fAllFormatting = 0;	// ���ׂĊ���������t���O������
	Id = 0;			// ���͓�����
	
	return 0;
}

// �萔�������^�X�N
void AllInitTask(void){	
	if(fAllFormatting){	// �܂��������r���Ȃ�
		clearAllMem();	// ���s����
	}
}

// �V���A���ʐM�C���^�[�t�F�[�X
// ��M�����f�[�^��Ώۂ�EEPROM�ɏ���
// �f�[�^ID 2byte, tmp 2byte, �f�[�^ 0�`4byte
int8_t MemRcvCom_Write(uint8_t* RcvData){
	uint16_t DataId;
	uint16_t TableIdx;
	uint32_t Data;
	float flDataPys;	// �f�[�^�����l
	double dbDataPys;	// �f�[�^�����l
	uint8_t Data_u8;	// 8bit�f�[�^
	int8_t Data_s8;	// 8bit�f�[�^
	uint16_t Data_u16;	// 16bit�f�[�^
	int16_t Data_s16;	// 16bit�f�[�^
	uint32_t Data_u32;	// 16bit�f�[�^
	int32_t Data_s32;	// 16bit�f�[�^
	int8_t i;
	
	// ID����
	DataId = (uint16_t)RcvData[0] | ((uint16_t)RcvData[1]<<8);
	if(DATAID_OK == searchTableIdx_MemTable(DataId, &TableIdx)){	// ID�ƍ�OK
		
		// �g�p���Ă���f�[�^���Ńf�[�^���擾
		Data = 0;
		for(i=i; i<stMemTable[TableIdx].DataLength; ++i){
			Data |= ((uint32_t)RcvData[i + 3] << (8*i));
		}
		
		// Digit�l���畨���n�ɒ����ď���
		switch(stMemTable[TableIdx].DataType){
		case enUB:
			Data_u8 = (uint8_t)(stMemTable[TableIdx].Resolution * Data + stMemTable[TableIdx].Offset);
			writeMemEep_UB(TableIdx, Data_u8);
			break;
		case enB:
			Data_s8 = (int8_t)(stMemTable[TableIdx].Resolution * Data + stMemTable[TableIdx].Offset);
			writeMemEep_UB(TableIdx, Data_s8);	// UB�ŏ���������đ��v���ȁH
			break;
		case enUH:
			Data_u16 = (uint16_t)(stMemTable[TableIdx].Resolution * Data + stMemTable[TableIdx].Offset);
			writeMemEep_UH(TableIdx, Data_u16);
			break;
		case enH:
			Data_s16 = (int16_t)(stMemTable[TableIdx].Resolution * Data + stMemTable[TableIdx].Offset);
			writeMemEep_UH(TableIdx, Data_s16);
			break;
		case enUW:
			Data_u32 = (uint32_t)(stMemTable[TableIdx].Resolution * Data + stMemTable[TableIdx].Offset);
			writeMemEep_UW(TableIdx, Data_u32);
			break;
		case enW:
			Data_s32 = (int32_t)(stMemTable[TableIdx].Resolution * Data + stMemTable[TableIdx].Offset);
			writeMemEep_UW(TableIdx, Data_s32);
			break;
		case enFL:
			flDataPys = stMemTable[TableIdx].Resolution * (float)Data + stMemTable[TableIdx].Offset;
			
			// min,max
			
			writeMemEep_FL(TableIdx, flDataPys);
			break;
		case enDB: // ���������_�^
			dbDataPys = stMemTable[TableIdx].Resolution * (double)Data + stMemTable[TableIdx].Offset;
			
			// min,max
			
			writeMemEep_DB(TableIdx, dbDataPys);
			break;
		default:
			return -1;
		}
		
		return 0;
	}else{
		return -1;
	}
	
}

// �Ώۂ�EEPROM�f�[�^��Ԃ�
// �f�[�^ID 2byte, tmp 2byte, �f�[�^ 0�`4byte
int8_t MemRcvCom_Read(uint8_t* Data){
	uint16_t DataId;
	// ID����
	DataId = Data[0] | ((uint16_t)Data[1]<<8);
	
	// �ǂݍ���
	//readMem();
	
	// �����l->Digit�l
	
	// Data�ɃZ�b�g
	
	return 0;
}
// �萔������
int8_t MemRcvCom_clearAllMem(uint8_t* Data){
	clearAllMem();
	return 0;
}
