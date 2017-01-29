
#include "Inrof2016_aplSci.h"



inrof2016_aplSci_t::inrof2016_aplSci_t(Sci_t *SciObject){
	Sci = SciObject;
	Mode = PC_THROUGH;
	Ps3ConAvailable = false;
	Ps3ConDisableCnt = 0;
};


/*********************
�ʐM�f�[�^���
�T�v�F
	���[�h�ɉ����Ď�M�o�b�t�@�����߂���
	����I�ɌĂяo������
**********************/
void inrof2016_aplSci_t::task(void){
	uint8_t cnt;
	uint8_t tmp[8];
	uint8_t Sum = 0;
	
	switch(Mode){
	case PS3:
		if(Ps3ConDisableCnt>SBDBT_PS3CON_DISABLECNT){
			Ps3ConAvailable = false;
		}else{
			Ps3ConDisableCnt ++;
		}
		// ��M�f�[�^���R���g���[���f�[�^�ɂԂ�����
		while(SBDBT_PS3CON_DATALENGTH <= Sci->available()){	// ��M�����R���g���[���S�f�[�^���ȏゾ������
			// �f�[�^�擪����������
			tmp[0] = 0xFF;
			while(0x80 != tmp[0]){
				if(Sci->receive(&tmp[0])){
					return;		// �擪�o�C�g�����邱�ƂȂ���M�o�b�t�@�����Ȃ���
							// ����Ȍ��肠�肦�Ȃ�
				}
			}
			
			// RCB3�t�H�[�}�b�g���ǂ�
			Sum = 0;
			for(cnt=1;cnt<7;cnt++){		// 1~6byte
				Sci->receive(&tmp[cnt]);
				Sum += tmp[cnt];
			}
			Sci->receive(&tmp[cnt]);	// 7byte
			Sum = Sum&0x7F;	// �`�F�b�N�T���͉���7bit
			
			
			// �`�F�b�N�T���m�F
			if(tmp[7] == Sum){
				// �T��ok
				Ps3ConAvailable = true;
				Ps3ConDisableCnt = 0;		// �R���g���[�������J�E���g���Z�b�g
				// �c��̃f�[�^��˂�����
				for(cnt=8;cnt<SBDBT_PS3CON_DATALENGTH;cnt++){
					Sci->receive(&ControllerData.Data[cnt]);
				}
			}else{
				// �T��ng	
				// ���̃f�[�^�擪���܂ō���ďI���
				while(1){
					if(Sci->watch(&tmp[0])){
						return;		// �o�b�t�@�Ȃ�
					}
					if(0x80 == tmp[0]){	// �擪�o�C�g����
						break;		// 
					}else{			// �擪�o�C�g����Ȃ�
						Sci->receive(&tmp[0]);	// ����
					}
				}
			}
		}
		
		break;
	case PC:
		
		break;
	case PC_THROUGH:
		// nothing to do
		break;
	default:
		// nothing to do
		break;
	}
	
}


// ���[�h���Z�b�g
void inrof2016_aplSci_t::setMode(mode_t NewMode){
	
	if(Mode!=NewMode){	// ���[�h�ς������o�b�t�@�N���A
		Sci->clearRxBuff();
	}
	
	Mode = NewMode;
}