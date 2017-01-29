
// E2�f�[�^�t���b�V���ǂݏ���
// �f�[�^�^�Fuint16_t
// �e�ʁF32kbyte = 16word
// �A�h���X�F0�`16383


//�u���b�N�F32byte * 1024�u���b�N = 32kbyte
//�C���[�X 32byte�P��
//���C�g 2byte�P��


#include "drvDataFlash.h"
#include <string.h>

// �h���C�o
static int8_t initE2flash(void);
static int8_t writeE2flash(uint16_t Address, uint16_t Data);
static int8_t writeE2flash32byte(uint16_t* Address, uint8_t* Data, uint8_t Num);
static int8_t readE2flash(uint16_t Address, uint16_t* RetData);

static int8_t fld_blank_check_2B(uint32_t addr);
static int8_t fld_erase_32B(uint32_t addr);
static int8_t fld_program_word(uint32_t addr, uint16_t Data);
static int8_t fld_enable_read(void);
static int8_t fld_disable_read(void);

static void (*AddedWriteTask)(void) = 0x00000000;

#define E2_START_ADRS 0x00100000
#define E2_DF_WRITE_BUF_NUM 64
#define E2_DF_AREA_DATA_NUM 32		// 1�G���A�̃f�[�^��

// �������݃o�b�t�@
static uint8_t drvDataFlashWriteBufPt = 0;
static uint16_t drvDataFlashWriteBufAdrs[E2_DF_WRITE_BUF_NUM];
static uint8_t drvDataFlashWriteBufData[E2_DF_WRITE_BUF_NUM];

static uint16_t WriteBufAdrs[E2_DF_AREA_DATA_NUM];	// �������ݗp�ꎞ�o�b�t�@
static uint8_t WriteBufData[E2_DF_AREA_DATA_NUM];

/*****************************************
�f�[�^�t���b�V����������
�T�v�F
	�f�[�^�t���b�V���ɏ������ރf�[�^�o�^
	�������݃o�b�t�@�Ɋi�[���A�������݃^�X�N�ɂ���ď������܂��
�����F
	�Ȃ�
�Ԓl�F
	�������݌���
	DF_OK�F����
******************************************/
int8_t drvDataFlash_write(uint16_t Adrs, uint8_t Data){
	
	if(drvDataFlashWriteBufPt < E2_DF_WRITE_BUF_NUM){	// �o�b�t�@�󂫂���
		drvDataFlashWriteBufAdrs[drvDataFlashWriteBufPt] = Adrs;
		drvDataFlashWriteBufData[drvDataFlashWriteBufPt] = Data;
		++drvDataFlashWriteBufPt;
		
		return DF_OK;
	}else{
		return DF_BUFF_FULL;
	}
}

/*****************************************
�f�[�^�t���b�V���������݃^�X�N
�T�v�F
	�o�b�t�@�ɂ���f�[�^��������������
�����F
	�Ȃ�
�Ԓl�F
	�������݌���
	DF_OK�F����
******************************************/
int8_t drvDataFlash_WriteTask(void){
	int8_t ret = DF_OK;
	uint8_t WriteNum;
	int16_t PtTmp;
	uint8_t fNotUniqueAdrs;
	uint8_t i;
	uint32_t AreaHeadAdrs;
	
	uint16_t NotWriteBufAdrs[E2_DF_WRITE_BUF_NUM-1];	// �����Ȃ������f�[�^
	uint8_t NotWriteBufData[E2_DF_WRITE_BUF_NUM-1];
	uint16_t NotWriteNum;
	
	// ���s�^�X�N�o�^����Ă����
	if(0 != AddedWriteTask){
		(*AddedWriteTask)();	// ���s����
	}
	
	while((0 < drvDataFlashWriteBufPt)&&(DF_OK==ret)){	// �������݃f�[�^�܂����� ���� �O�񏑍�����OK
		// �������݃f�[�^����
		// 1�o�C�g(�A�v���������ݒP��)��2�o�C�g(E2�f�[�^�t���b�V���������ݒP��)
		// �����������ݗ̈���܂Ƃ߂đ���
		
		
		PtTmp = drvDataFlashWriteBufPt-1;
		WriteNum = 0;
		NotWriteNum = 0;
		AreaHeadAdrs = ((uint32_t)drvDataFlashWriteBufAdrs[PtTmp])&0xFFFFFFE0;
		while((PtTmp>=0)&&(WriteNum<E2_DF_AREA_DATA_NUM)){	// �������݃o�b�t�@�ɂ���f�[�^���B�������́A��x�ɏ������߂�f�[�^��
			
			if(AreaHeadAdrs == ((uint32_t)drvDataFlashWriteBufAdrs[PtTmp]&0xFFFFFFE0)){	// �����G���A���̃f�[�^
				
			
				// �����A�h���X�ɏ����ꍇ�A���Ƃ���o�b�t�@�ɒǉ��������݂̂̂�����
				i = 0;
				fNotUniqueAdrs = false;
				while(i<WriteNum){
					if(WriteBufAdrs[i]==drvDataFlashWriteBufAdrs[PtTmp]){
						fNotUniqueAdrs = true;
					}
					++i;
				}
				if(false == fNotUniqueAdrs){	// ���񏉂߂ď����A�h���X
					WriteBufAdrs[WriteNum] = drvDataFlashWriteBufAdrs[PtTmp];
					WriteBufData[WriteNum] = drvDataFlashWriteBufData[PtTmp];
					--PtTmp;
					++WriteNum;
				}else{				// ����A���łɏ����Ă���A�h���X
								// �������݃f�[�^�𖳎�����
					--PtTmp;
				}
			}else{		// �Ⴄ�������ݗ̈�
					// ���񏑂��悤�Ɋo���Ă���
				NotWriteBufAdrs[NotWriteNum] = drvDataFlashWriteBufAdrs[PtTmp];
				NotWriteBufData[NotWriteNum] = drvDataFlashWriteBufData[PtTmp];
				++NotWriteNum;
				--PtTmp;
			}
		}
		
		ret = writeE2flash32byte(WriteBufAdrs, WriteBufData, WriteNum);	// �������ݎ��{
		
		if(FLD_OK == ret){
			// �������ݐ����Ȃ�
			drvDataFlashWriteBufPt -= WriteNum;	// ���������|�C���^�����炷
			
			// �����Ȃ������f�[�^���o������
			for(i=0; i<NotWriteNum; i++){
				drvDataFlashWriteBufAdrs[drvDataFlashWriteBufPt-1-i] = NotWriteBufAdrs[i];
				drvDataFlashWriteBufData[drvDataFlashWriteBufPt-1-i] = NotWriteBufData[i];
			}
			
			ret = DF_OK;
		}
	}
	
	return ret;
}

/*****************************************
�������ݎ����s�^�X�N�o�^
�T�v�F
	����I�ɌĂяo���������ݏ����Ŏ��s����^�X�N��o�^
�����F
	���s�֐��|�C���^
�Ԓl�F
	�Ȃ�
******************************************/
void drvDataFlash_attachWriteTask( void (*Tsk)(void)){
	AddedWriteTask = Tsk;
}

/*****************************************
�������ݎ����s�^�X�N�o�^����
�T�v�F
	����I�ɌĂяo���������ݏ����Ŏ��s����^�X�N������
�����F
	�Ȃ�
�Ԓl�F
	�Ȃ�
******************************************/
void drvDataFlash_dettachWriteTask(void){
	AddedWriteTask = 0x00000000;
}


/*****************************************
E2�f�[�^�t���b�V����������(word�T�C�Y)
�T�v�F
	�w��A�h���X�ɒl������
	�f�[�^��16bit�B
�����F
	address: �A�h���X 0�`16383 (1����)
�Ԓl�F
	��������
	�A�h���X�͈͊O�̏ꍇ�͒l����������FLD_ERROR��Ԃ�
******************************************/
int8_t drvDataFlash_writeImmWord(uint16_t Address, uint16_t Data){
	switch(writeE2flash(Address, Data)){
	case FLD_OK:
		return DF_OK;
		break;
	case FLD_OUT_OF_ADDRESS:
		return DF_OUT_OF_ADDRESS;
		break;
	default:
		return DF_ERROR;
	}
}
static int8_t writeE2flash(uint16_t Address, uint16_t Data){
	
	uint16_t DataTmp[16];
	bool_t DataExist[16] = {0};
	uint32_t CheckingAdrs;
	uint32_t i;
	int8_t ret;
	uint32_t AreaHeadAdrs = ((uint32_t)Address*2)&0xFFFFFFE0;
	
	// �C���[�X���u���b�N(32byte)�P�ʂȂ̂ŁA
	// ���������A�h���X���u�����N����Ȃ��ꍇ�A
	// 32byte�̂��A�u�����N����Ȃ��ꏊ�̃f�[�^��ǂ݂����āA
	// �C���[�X��A�܂������Ă����B
	
	if(Address>=E2_DF_SIZE_BYTE){	// �A�h���X�͈͊O
		return FLD_OUT_OF_ADDRESS;
	}
	
	if(1==FLASH.FSTATR0.BIT.FRDY){	// �t���b�V�����쒆����Ȃ��t���O
		// �u�����N�`�F�b�N(�u�����N����Ȃ��Ə����Ȃ�)
	
		switch(fld_blank_check_2B(E2_START_ADRS + (uint32_t)Address*2)){
		case FLD_BLANK:	
			// �u�����N�Ȃ̂�
			// ���̃A�h���X�̂ݏ�������
			ret = fld_program_word(E2_START_ADRS + (uint32_t)Address*2, Data);
			break;
		case FLD_NOBLANK:
			// �u�����N����Ȃ��̂�
			// �������ݍς݂̒l��ǂݍ���
			for(i=0; i<16; i++){
				CheckingAdrs = E2_START_ADRS + AreaHeadAdrs + i*2;
				
				if(CheckingAdrs == (E2_START_ADRS + ((uint32_t)Address*2))){
					// �������ݑΏۃA�h���X�͎w��f�[�^���Z�b�g
					DataTmp[i] = Data;
					DataExist[i] = true;
				}else{	// ����ȊO�͏����Ă���f�[�^���Z�b�g
					if(readE2flash(((uint32_t)Address&0xFFFFFFF0)+i, &DataTmp[i]) == FLD_OK){
						DataExist[i] = true;
					}else{
						DataExist[i] = false;
					}
				}
			}
			
			// �C���[�X����
			if(fld_erase_32B(E2_START_ADRS + AreaHeadAdrs) != FLD_OK){
				return FLD_ERROR;
			}
			
			// �������ݍς݂̒l�ƐV�K�̃f�[�^���Z�b�g
			for(i=0; i<16; i++){
				if(DataExist[i]){
					CheckingAdrs = E2_START_ADRS + AreaHeadAdrs + i*2;
					ret = fld_program_word(CheckingAdrs, DataTmp[i]);
					if(ret != FLD_OK){
						return FLD_ERROR;
					}
				}
			}
			break;
		//default:
			ret = FLD_ERROR | 0x10;
		}
	}else{
		ret = FLD_BUSY;
	}
	
	return ret;
}

/*****************************************
E2�f�[�^�t���b�V����������(byte�T�C�Y 32byte�܂�)
�T�v�F
	�w��A�h���X�ɒl������
	�f�[�^��8bit�B
	�A�h���X�͂��ׂē����̈�ɂ���K�v������
�����F
	address: �A�h���X 0�`32768
�Ԓl�F
	��������
	�A�h���X�͈͊O�̏ꍇ�͒l����������FLD_ERROR��Ԃ�
******************************************/
static int8_t writeE2flash32byte(uint16_t* Address, uint8_t* Data, uint8_t Num){
	uint16_t DataTmp[16];
	bool_t DataExist[16] = {0};
	uint8_t CheckingIdx;
	uint8_t WritingIdx;
	uint32_t i;
	int8_t ret = FLD_OK;
	uint32_t AreaHeadAdrs = ((uint32_t)Address[0])&0xFFFFFFE0;
	int8_t Blank;
	uint32_t fWriteDone = 0;
	
	// �C���[�X���u���b�N(32byte)�P�ʂȂ̂ŁA
	// ���������A�h���X���u�����N����Ȃ��ꍇ�A
	// 32byte�̂��A�u�����N����Ȃ��ꏊ�̃f�[�^��ǂ݂����āA
	// �C���[�X��A�܂������Ă����B
	
	if(0<Num){
		for(i=0;i<Num;++i){
			if((Address[i]>=E2_DF_SIZE_BYTE)||(AreaHeadAdrs!=((uint32_t)Address[i])&0xFFFFFFE0)){
				// �A�h���X�͈͊O �������� �̈悪�܂������Ă���
				return FLD_OUT_OF_ADDRESS;
			}
		}
		if(1==FLASH.FSTATR0.BIT.FRDY){	// �t���b�V�����쒆����Ȃ��t���O
			
			Blank = FLD_BLANK;
			// �������ݍς݂̒l��ǂݍ���
			for(i=0; i<16; i++){
				DataTmp[i] = 0x0000;
				switch(readE2flash(AreaHeadAdrs/2 + i, &DataTmp[i])){	// �l�ǂݍ���
				case FLD_OK:
					DataExist[i] = true;					// �l�������ݍς݂�����
					Blank = FLD_NOBLANK;
					break;
				case FLD_BLANK:
					DataExist[i] = false;					// �u�����N������
					break;
				default:
					return FLD_ERROR - 20;					// �ǂݍ��ݎ��s
					break;
				}
			}
			
			// �������݃f�[�^�ݒ�
			fWriteDone = 0;
			for(WritingIdx = 0; WritingIdx<Num; ++WritingIdx ){
				if(!(fWriteDone&(0x00000001<<WritingIdx))){	// �܂����ĂȂ�
					fWriteDone |= (0x00000001<<WritingIdx);	// �������t���O���Ă�
					//AdrsTmp = Address[WritingIdx]&0xFFFFFFFE;	// ��������E2�f�[�^�t���b�V���A�h���X�ݒ�(2byte����)
					CheckingIdx = Address[WritingIdx]>>1;		// �������ރA�h���X�̃C���f�b�N�X(AreaHeadAdrs��0�Ƃ���WORD�T�C�Y�̃C���f�b�N�X)
					DataExist[CheckingIdx] = true;			// �������ݑΏۂƂ��ă`�F�b�N
					
					if(Address[WritingIdx]&0x00000001){	// Word�̉�����
						DataTmp[CheckingIdx] = (DataTmp[CheckingIdx]&0xFF00) | (uint16_t)Data[WritingIdx];	// �������ɏ���
					}else{					// Word�̏㔼��
						DataTmp[CheckingIdx] = (DataTmp[CheckingIdx]&0x00FF) | (((uint16_t)Data[WritingIdx])<<8);	// �㔼���ɏ���
					}
					
					// 2�o�C�g�܂Ƃ߂ď������݃f�[�^���
					for(i=0; i<Num;++i){	// ����E2�t���b�V���A�h���X�̂�T��
						if(!(fWriteDone&(0x00000001<<i))){	// �܂����ĂȂ�
							if((Address[i]>>1) == CheckingIdx){	// ����1bit������ē����Ȃ瓯��
								// �f�[�^���\�z
								// �����A�h���X�Ȃ���ɂ���̂�D�悷��(�㏑������)
								if(Address[i]&0x00000001){	// �A�h���X+1
									DataTmp[CheckingIdx] = (DataTmp[CheckingIdx]&0xFF00) | (uint16_t)Data[i];	// �������ɏ���
								}else{
									DataTmp[CheckingIdx] = (DataTmp[CheckingIdx]&0x00FF) | (((uint16_t)Data[i])<<8);	// �㔼���ɏ���
								}
								
								fWriteDone |= (0x00000001<<i);	// �������t���O���Ă�
							}
						}
					}
				}
			}
			
			// �������ݍς݂̂Ƃ��낪����ꍇ�͏����Ȃ��̂ŃC���[�X����
			if(FLD_NOBLANK == Blank){
				if(FLD_OK != fld_erase_32B(E2_START_ADRS + AreaHeadAdrs)){
					return FLD_ERROR - 30;
				}
			}
			
			// 16word�̏������ݎ��{
			for(i=0; i<16; i++){
				if(true == DataExist[i]){	// �������ݑΏ�
					ret = fld_program_word(E2_START_ADRS + AreaHeadAdrs + i*2, DataTmp[i]);
					if(FLD_OK != ret){
						return FLD_ERROR - 40 - (int8_t)i;
					}
				}
			}
			
		}else{
			ret = FLD_BUSY;
		}
	}
	return ret;
}

/*****************************************
E2�f�[�^�t���b�V���ǂݍ���(byte�T�C�Y)
�T�v�F
	�w��A�h���X�ɂ�����Ă���l��Ԃ�
�����F
	address: �A�h���X 0�`32768 (1����)
�Ԓl�F
	�w��A�h���X�ɏ�����Ă���l
	�A�h���X�͈͊O�̏ꍇ��0xFFFF��Ԃ�
******************************************/
int8_t drvDataFlash_read(uint16_t Address, uint8_t* RetData){
	uint16_t RetDataTmp; 
	
	switch(readE2flash(Address/2, &RetDataTmp)){
	case FLD_OK:
		if(Address&0x00000001){	// ��ʃA�h���X (�f�[�^�̉�����)
			*RetData = (uint8_t)(RetDataTmp&0x00FF);
		}else{			// ���ʃA�h���X (�f�[�^�̏㔼��)
			*RetData = (uint8_t)((RetDataTmp>>8)&0x00FF);
		}
		
		return DF_OK;
	case FLD_OUT_OF_ADDRESS:
		return DF_OUT_OF_ADDRESS;
	case FLD_BLANK:
		return DF_BLANK;
	default:
		return DF_ERROR;
	}
}

/*****************************************
E2�f�[�^�t���b�V���ǂݍ���(word�T�C�Y)
�T�v�F
	�w��A�h���X�ɂ�����Ă���l��Ԃ�
�����F
	address: �A�h���X 0�`16383 (1����)
�Ԓl�F
	�w��A�h���X�ɏ�����Ă���l
	�A�h���X�͈͊O�̏ꍇ��0xFFFF��Ԃ�
******************************************/
int8_t readE2flash(uint16_t address, uint16_t* RetData){
	int8_t ret;
	
	if(address>=E2_DF_SIZE_BYTE){	// �A�h���X�͈͊O
		return FLD_OUT_OF_ADDRESS;
	}
	
	ret = fld_blank_check_2B(E2_START_ADRS + (uint32_t)address*2);	// �u�����N�`�F�b�N
	if(FLD_NOBLANK == ret){
		// �u�����N����Ȃ�
		fld_enable_read();
		*RetData = *(uint16_t *)(E2_START_ADRS + (uint32_t)address*2);	// �w��ӏ��Ԃ�
		fld_disable_read();
		return FLD_OK;
	}else{
		// �u�����N �� �G���[��
		return ret;
	}
	
	return ret;
}


// ������
int8_t drvDataFlash_init(void){
	return initE2flash();
}

static int8_t initE2flash(void){
	volatile uint32_t i;
	static const int fcu_ram_size = 8*1024;	// 8k byte
	void *fcu_ram = (uint32_t *)0x007F8000;
	void *fcu_fw = (uint32_t *)0xFEFFE000;
	
	volatile uint8_t *addr_b = (uint8_t *)E2_START_ADRS;
	volatile uint16_t *addr_w = (uint16_t *)E2_START_ADRS;
	
	
	/* ROM�Ɋi�[����Ă���FCU�̃t�@�[���E�F�A��RAM�փR�s�[ */
	// ���[�h���[�h��
	if((FLASH.FENTRYR.WORD & 0x00FF)!=0x0000){
		FLASH.FENTRYR.WORD = 0xAA00;
	}
	// FCU RAM�G���A�A�N�Z�X����
	FLASH.FCURAME.WORD = 0xC401;
	
	// RAM��FCU�t�@�[�����R�s�[
	memcpy(fcu_ram, fcu_fw, fcu_ram_size);
	
	/* ���ӃN���b�N��ݒ� */
	// P/E���[�h��
	if((FLASH.FENTRYR.WORD & 0x00FF)!=0x0080){
		FLASH.FENTRYR.WORD = 0xAA80;
	}
	// ���ӃN���b�N�l���Z�b�g
	FLASH.PCKAR.BIT.PCKA = 48;	//48MHz (�ЂƂ܂��Œ�l;;;)
	
	// ���ӃN���b�N�ݒ�����s
	*addr_b = 0xE9;
	*addr_b = 0x03;
	*addr_w = 0x0f0f;
	*addr_w = 0x0f0f;
	*addr_w = 0x0f0f;
	*addr_b = 0xD0;
	
	// tPCKA���ԑ҂�
	//if( wait_FRDY(120) == FLD_TMOUT){	
	//}
	for(i=0;i<65535;++i){
	}
	
	
	if(FLASH.FSTATR0.BIT.ILGLERR == 1){
		return FLD_ERROR;
	}
	
	return FLD_OK;
}


// �ׂ����֐� //

// �u�����N�`�F�b�N 2B
// addr : ��΃A�h���X
static int8_t fld_blank_check_2B(uint32_t addr){
	
	volatile uint8_t *adrr_b = (uint8_t *)(addr&0xFFFFFC00);	// �u���b�N�擪�A�h���X
									// 2kB�P��
	volatile uint16_t adrr_ofs = (uint16_t)(addr&0x000003FF);	// �u���b�N�擪�A�h���X����̃I�t�Z�b�g
	
	
	// P/E���[�h��
	if((FLASH.FENTRYR.WORD & 0x0080) != 0x0080){
		FLASH.FENTRYR.WORD = 0xAA80;
	}
	
	FLASH.FWEPROR.BIT.FLWE = 1; // Unptorect
	
	// use the blank checking command
	FLASH.FMODR.BIT.FRDMD = 1;
	
	// set the blank check size (2B)
	FLASH.DFLBCCNT.BIT.BCSIZE = 0;
	FLASH.DFLBCCNT.BIT.BCADR = adrr_ofs;		// 2kB�P�ʂɂ������̗]��
	
	// execute the blank checking command
	*adrr_b = 0x71;
	*adrr_b = 0xD0;
	
	while(FLASH.FSTATR0.BIT.FRDY == 0);
	//wait for tDBC2K time (timeout is 770us)
	// if(wait_FRDY(700 * 1.1) == FLD_TMOUT)
	// {
	// reset_fcu();
	// }
	// error check
	
	FLASH.FWEPROR.BIT.FLWE = 2; // protect
	
	if(FLASH.FSTATR0.BIT.ILGLERR){
		return FLD_ERROR;
	}
	
	// get result of blank checking command
	if(0 == FLASH.DFLBCSTAT.BIT.BCST){
		return FLD_BLANK;
	}
	
	return FLD_NOBLANK;
}

// ���� 32B
// addr : ��΃A�h���X
static int8_t fld_erase_32B(uint32_t addr){
	volatile uint8_t *addr_b = (uint8_t *)(addr&0xFFFFFFE0);	// 32byte�P��
	int32_t ret = FLD_OK;
	
	// P/E���[�h��
	if((FLASH.FENTRYR.WORD & 0x0080) != 0x0080){
		FLASH.FENTRYR.WORD = 0xAA80;
	}
	
	FLASH.FWEPROR.BIT.FLWE = 1; // Unptorect
	FLASH.DFLWE0.WORD = 0x1EFF; //1Eh is Key
	FLASH.DFLWE1.WORD = 0xE1FF; //E1h is Key
	// execute the block erase command
	*addr_b = 0x20;
	*addr_b = 0xD0;
	while(FLASH.FSTATR0.BIT.FRDY == 0);	// 11ms�ʂ����邩��
	if((FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.ERSERR == 1)){ // error check
		ret = FLD_ERROR;
	}else{
		/* nothing to do */
	}
	FLASH.FWEPROR.BIT.FLWE = 2; // protect
	FLASH.DFLWE0.WORD = 0x1E00; // disable
	FLASH.DFLWE1.WORD = 0xE100; // disable
	return ret;
}

// 2byte��������
// addr : ��΃A�h���X
static int8_t fld_program_word(uint32_t addr, uint16_t Data){
	volatile uint8_t *addr_b = (uint8_t *)addr;
	 volatile uint16_t *addr_w = (uint16_t *)addr;
	int32_t ret = FLD_OK;
	
	// P/E���[�h��
	if ((FLASH.FENTRYR.WORD & 0x0080) != 0x0080) {
		FLASH.FENTRYR.WORD = 0xAA80;
	}
	
	FLASH.FWEPROR.BIT.FLWE = 1; // Unprotect
	FLASH.DFLWE0.WORD = 0x1EFF; // 1Eh is Key
	FLASH.DFLWE1.WORD = 0xE1FF; // 1Eh is Key
	
	// execute the 2-byte programming command
	*addr_b = 0xE8;
	*addr_b = 0x01;
	*addr_w = Data;
	*addr_b = 0xD0;
	
	while(FLASH.FSTATR0.BIT.FRDY == 0);	//11ms���炢�����邩��
	
	if((FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.ERSERR == 1)){ //error check
		ret = FLD_ERROR;
	}else{
		/* nothing to do */
	}
	
	FLASH.FWEPROR.BIT.FLWE = 0x02;// protect
	FLASH.DFLWE0.WORD = 0x1E00;// disable
	FLASH.DFLWE1.WORD = 0xE100;// disable
	
	return ret;
}

// �ǂݏo������
static int8_t fld_enable_read(void){
	// ���[�h���[�h��
	if((FLASH.FENTRYR.WORD & 0x00FF) != 0x0000){
		FLASH.FENTRYR.WORD = 0xAA00;
	}
	
	FLASH.DFLRE0.WORD = 0x2DFF;
	FLASH.DFLRE1.WORD = 0xD2FF;
	
	return FLD_OK;
}
static int8_t fld_disable_read(void){
	// ���[�h���[�h��
	if((FLASH.FENTRYR.WORD & 0x00FF) != 0x0000){
		FLASH.FENTRYR.WORD = 0xAA00;
	}
	
	FLASH.DFLRE0.WORD = 0x2D00;
	FLASH.DFLRE1.WORD = 0xD200;
	
	return FLD_OK;
}