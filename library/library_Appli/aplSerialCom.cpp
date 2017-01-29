/**************************************************
aplSerialCom.cpp
	�V���A���ʐM����
**************************************************/

#include "aplSerialCom.h"

// �w�b�_
#define HEADER_RCV 0xFBBF
#define HEADER_RET_OK 0xBFFB
#define HEADER_RET_NG 0xCFFC

#define SERIAL_COM_LENGTH 13


void (*SerialCom_t::AddedRcvTask)(void) = NULL;
Serial_t* SerialCom_t::SerialObj;
SerialComMode_t SerialCom_t::Mode = SELCOM_MODE_COMMAND;


#ifndef __APL_SERIALCOM_SPEC_H__
// ���[�U�[�ݒ肪�����ꍇ
const ST_SERIAL_COM_COMMAND_TABLE_T stSerialComCommandTable[] = {
	{	0x0000,		0,	NULL	},	// 
};
#endif

// �V���A���ʐM�f�t�H���g�R�}���h�����e�[�u��
const ST_SERIAL_COM_COMMAND_TABLE_T SerialCom_t::stSerialComDefauldCommandTable[] = {
	/*	ComId,	DataLength,	Func	*/
	{	0xFF00,		1,	setModeSerialCom	},	// ���[�h�Z�b�g
	{	0x0000,		0,	NULL	},	// 
};


// ������
int8_t SerialCom_t::init(Serial_t *SerialObj, SerialComMode_t InitialMode){
	SerialCom_t::SerialObj = SerialObj;
	SerialCom_t::Mode = InitialMode;
	
	return 0;
}

// �^�X�N����
// �R�}���h���[�h��
//  �R�}���h���� ���[�U�[�R�}���h->�f�t�H���g�R�}���h�̏�
//  �^�X�N�o�^����Ă���΂�������s
void SerialCom_t::task(void){
	uint16_t Sel;
	uint16_t ComId;
	uint16_t RcvDataNum;		// ��M�o�b�t�@�f�[�^��
//	uint16_t RcvDataNumCom;		// �R�}���h�܂ł̃f�[�^��
	volatile uint16_t SelRcvData=0;
	uint16_t i;
	uint8_t DataTmp;
	uint8_t RetDataNum;
	static uint8_t fHead = 0;
	static uint8_t RcvData[SERIAL_COM_LENGTH];	// �w�b�_���珇�Ɋo���Ă��� �w�b�_2byte, �R�}���hID 2byte, �f�[�^8byte, �`�F�b�N�T��1byte
	static uint8_t RcvDataPos = 0;	// ���ۑ�����ʒu
	int8_t FuncRet = FUNC_RET_NO_RET;
	bool_t IsSumOk = 0;	// �`�F�b�N�T��
	bool_t Breaked = FALSE;
	const uint16_t RcvHeader = HEADER_RCV;
	
	
	
	// ��M�f�[�^����,�R�}���h���s
	// ��M�f�[�^�R�s�[
	
	
	// �R�}���h���
	// �w�b�_��T��
	// ��M��
	if(RcvDataNum = SerialObj->available()){	// ��M�f�[�^����
		for(SelRcvData=0; SelRcvData<RcvDataNum; ++SelRcvData){	// ��M������������͂������߂�
			DataTmp = SerialObj->watch(SelRcvData);
			
			if(0==fHead){	// �܂��w�b�_�����ĂȂ�
				if((uint8_t)((RcvHeader>>8)&0x00FF) == DataTmp){	// �w�b�_1��
					fHead = 1;
				}else{	// �w�b�_����Ȃ������̂Ń��Z�b�g
				}
			}else if(1==fHead){	// 1�ڂ̃w�b�_�����Ă���
				if((uint8_t)((RcvHeader)&0x00FF) == DataTmp){	// �w�b�_2��
					fHead = 2;
					RcvData[0] = (uint8_t)((RcvHeader>>8)&0x00FF);
					RcvData[1] = (uint8_t)((RcvHeader)&0x00FF);
					
					RcvDataPos = 2;
				}else{	// 1�ڂ̎���2�ڂ���Ȃ������̂Ń��Z�b�g
					fHead = 0;
					RcvDataPos = 0;
				}
			}else if(2==fHead){	// �w�b�_��������ԂŎ��̎�����
				RcvData[RcvDataPos] = DataTmp;
				++RcvDataPos;
				if(SERIAL_COM_LENGTH <= RcvDataPos ){	// ��M����
					fHead = 0;
					RcvDataPos = 0;
					//�`�F�b�N�T���m�F
					IsSumOk = isValidCheckSum(RcvData);
					if(IsSumOk){
						SelRcvData;
						break;
					}else{
						SelRcvData -= (SERIAL_COM_LENGTH-2);	// ���炵�ĉ�͂�����
					}
				}
			}else{
				
			}
		}
	}
	
	if(IsSumOk){
		
		ComId = ((uint16_t)RcvData[3]<<8) | (uint16_t)RcvData[2];
		
		// �֐����s
		// ���[�U�[�R�}���h
		Sel = 0;
		
		if(SELCOM_MODE_COMMAND==Mode){	// �R�}���h���[�h�Ȃ��
			while(NULL != stSerialComCommandTable[Sel].Func){				// �֐��o�^����(�e�[�u���̍Ō�����m)
				if(ComId == stSerialComCommandTable[Sel].ComId){			// �R�}���hID��v
					FuncRet = (*stSerialComCommandTable[Sel].Func)(&RcvData[4]);	// �֐����s				
					Breaked = TRUE;
					break;
				}
				++Sel;
			}
		}
		if(!Breaked){	// ���[�U�[�R�}���h���s����Ă��Ȃ��Ȃ��
			// �f�t�H���g�R�}���h�����s
			Sel = 0;
			if((0xFF00==ComId)||(SELCOM_MODE_COMMAND==Mode)){		// ���[�h�Z�b�g�R�}���h�������̓R�}���h���[�h�Ȃ��
				while(NULL != stSerialComDefauldCommandTable[Sel].Func){			// �֐��o�^����(�e�[�u���̍Ō�����m)
					if(ComId == stSerialComDefauldCommandTable[Sel].ComId){			// �R�}���hID��v
						FuncRet = (*stSerialComDefauldCommandTable[Sel].Func)(&RcvData[4]);	// �֐����s
						Breaked = TRUE;
						break;
					}
					++Sel;
				}
			}else{
				FuncRet = FUNC_RET_NO_RET;
			}
		}
	}
	
	// �R�}���h����
	if(Breaked){	// �R�}���h��v����
		if(FUNC_RET_OK == FuncRet){
			// �w�b�_���Z�b�g
			RcvData[0] = (uint8_t)((HEADER_RET_OK>>8)&0x00FF);
			RcvData[1] = (uint8_t)((HEADER_RET_OK)&0x00FF);
			
			// �������M
			RetDataNum = SERIAL_COM_LENGTH;
			
		}else if(FUNC_RET_NG == FuncRet){
			// �w�b�_���Z�b�g
			RcvData[0] = (uint8_t)((HEADER_RET_NG>>8)&0x00FF);
			RcvData[1] = (uint8_t)((HEADER_RET_NG)&0x00FF);
			
			// �������M
			RetDataNum = SERIAL_COM_LENGTH;
		}else{
			// �����Ԃ��Ȃ�
			RetDataNum = 0;
		}
		
		// �`�F�b�N�T���X�V
		setCheckSum(RcvData);
		
		// �����f�[�^���M
		i = 0;
		while(i<RetDataNum){
			SerialObj->transmit(RcvData[i]);
			++i;
		}
		
		// �R�}���h�܂ł̎�M�f�[�^����
		for(i = 0; i<(SelRcvData+1); ++i){
			SerialObj->read();
		}
		
	}
	
	
	// ���s�^�X�N�o�^����Ă����
	if(NULL != AddedRcvTask){
		//��M�f�[�^����΁H
		(*AddedRcvTask)();	// ���s����
	}
	
}


/*****************************************
���[�h�Z�b�g
�T�v�F
	���샂�[�h��ݒ�
�����F
	�V�������샂�[�h
�Ԓl�F
	�Ȃ�
******************************************/
void SerialCom_t::setMode(SerialComMode_t NewMode){
	Mode = NewMode;
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
int8_t SerialCom_t::setModeSerialCom(uint8_t *RcvData){
	if((SELCOM_MODE_COMMAND == RcvData[0])||(SELCOM_MODE_THROUGH == RcvData[0])){
		setMode((SerialComMode_t)RcvData[0]);
		
		return FUNC_RET_OK;
	}else{
		return FUNC_RET_NG;
	}
}

/*****************************************
��M���e�����^�X�N�o�^
�T�v�F
	����I�ɌĂяo���������ݏ����Ŏ��s����^�X�N��o�^
�����F
	���s�֐��|�C���^
�Ԓl�F
	�Ȃ�
******************************************/
void SerialCom_t::attachRcvTask( void (*Tsk)(void)){
	AddedRcvTask = Tsk;
}

/*****************************************
��M���e�����^�X�N�o�^����
�T�v�F
	����I�ɌĂяo���������ݏ����Ŏ��s����^�X�N������
�����F
	�Ȃ�
�Ԓl�F
	�Ȃ�
******************************************/
void SerialCom_t::dettachRcvTask(void){
	AddedRcvTask = NULL;
}

/*****************************************
�`�F�b�N�T���m�F
�T�v�F
	��M�f�[�^��SERIAL_COM_LENGTH-1�o�C�g�𑫂����l��Ԃ�
�����F
	�Ȃ�
�Ԓl�F
	uint8_t �`�F�b�N�T��
******************************************/
uint8_t SerialCom_t::calcCheckSum(uint8_t* RcvData){
	uint8_t DataSum = 0;
	uint8_t i;
	
	for(i=0;i<SERIAL_COM_LENGTH-1;++i){
		DataSum += RcvData[i];
	}
	
	return DataSum;
}

/*****************************************
�`�F�b�N�T�������Ă邩�m�F
�T�v�F
	
�����F
	�Ȃ�
�Ԓl�F
	�Ȃ�
******************************************/
uint8_t SerialCom_t::isValidCheckSum(uint8_t* RcvData){
	return RcvData[SERIAL_COM_LENGTH-1] == calcCheckSum(RcvData);
}

/*****************************************
�`�F�b�N�T�����Z�b�g
�T�v�F
	�ŏI�o�C�g�Ƀ`�F�b�N�T�����Z�b�g
�����F
	
�Ԓl�F
	�Ȃ�
******************************************/
void SerialCom_t::setCheckSum(uint8_t* RcvData){
	RcvData[SERIAL_COM_LENGTH-1] = calcCheckSum(RcvData);
}

