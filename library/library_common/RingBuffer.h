/********************************************/
/*		�����O�o�b�t�@						*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/12/29		*/
/********************************************/

// �e���v���[�g�g���Ăǂ�ȃf�[�^�^�ł��g����B
// �o�b�t�@�̒�����uint16_t(65535)�܂ŁB

//�y�X�V�����z
// 2015.11.28 �o�b�t�@�m�ێ��s�������̓o�b�t�@�T�C�Y��0�ɂ���


//�y�X�V�\��z
// �o�b�t�@�������ς��ł��������Ⴄ���[�h

#ifndef RING_BUFFER_HPP_
#define RING_BUFFER_HPP_

#include "CommonDataType.h"
#include <string.h>	// memset�Ɏg��


// debug
#include "iodefine.h"


#define BUFFER_SIZE_DEFAULT  255U

template <typename DATATYPE> class RingBuffer
{
private : 
	DATATYPE *Buffer;	// buffer
	uint16_t BufferSize;
	DATATYPE *pWrite;	// �������ꏊ
	DATATYPE *pRead;	// ���ǂޏꏊ
	
public:
	RingBuffer(uint16_t BufferSize);	// Constructor
	RingBuffer(void);
	~RingBuffer(void);
	
	int8_t add(DATATYPE data);		// �l�ǉ�
	int8_t read(DATATYPE *data);		// �l���o��
	DATATYPE read(void){DATATYPE tmp; read(&tmp); return tmp;};		// �l���o��
	int8_t watch(DATATYPE *data);	// �l���邾��
	int8_t watch(uint16_t IndexRel, DATATYPE *data);	//�l���邾��(IndexRel��̂�����)
	
	uint16_t getNumElements(void);
	uint16_t getBufferSize(void){return BufferSize;};
	uint16_t getFreeSpace(void){return getBufferSize()-getNumElements();};
	uint16_t getWriteIndex(void){return (uint16_t)(pWrite-Buffer);};		//�o�b�t�@�擪���牽�Ԗڂ��w���Ă��邩
	uint16_t getReadIndex(void){return (uint16_t)(pRead-Buffer);};
	
	bool_t isFull(void){return (getNumElements()>=BufferSize);};
	bool_t isEmpty(void){return (pWrite==pRead);};
	void clear(void);
	
};




/*********************
 �R���X�g���N�^
**********************/
template <class DATATYPE> RingBuffer<DATATYPE>::RingBuffer(uint16_t BufferSize){
	Buffer = new DATATYPE[BufferSize+1];
	if(NULL!=Buffer){
		this->BufferSize = BufferSize;
	}else{
		this->BufferSize = 0;
	}
	clear();
}

template <class DATATYPE>
RingBuffer<DATATYPE> ::RingBuffer(void){
	RingBuffer(BUFFER_SIZE_DEFAULT);
}

/*********************
 �f�X�g���N�^
**********************/
template <class DATATYPE>
RingBuffer<DATATYPE> ::~RingBuffer(void){
	delete[] Buffer;
}


/*********************
 �o�b�t�@�ɒǉ�����
�o�b�t�@�������ς��Ȃ�-1��Ԃ�
**********************/
template <class DATATYPE>
int8_t RingBuffer<DATATYPE>::add(DATATYPE data){
	DATATYPE *pNext = pWrite+1;
	
	if(pNext > &Buffer[BufferSize]){	// ��Ԍ��܂ōs������
		pNext = Buffer;					// �擪��
	}
	
	if(pNext != pRead){	// not reach to pRead
		*pWrite = data;		// Write data
		pWrite = pNext;		// Update pointer
		return 0;
	}
	
	return -1;
}

/*********************
 �o�b�t�@�ǂ�
�o�b�t�@�ɂȂ����-1��Ԃ�
**********************/
template <class DATATYPE>
int8_t RingBuffer<DATATYPE>::read(DATATYPE *data){
	
	if(pRead != pWrite){
		*data = *pRead;		// Read data
		
		pRead++;			// Update pointer
		if(pRead > &Buffer[BufferSize]){
			pRead = Buffer;
		}
		return 0;
	}
	
	return -1;	// no data
}

/*********************
 �o�b�t�@�ǂނ��Ǔǂ񂾂��Ƃɂ��Ȃ�
**********************/
template <class DATATYPE>
int8_t RingBuffer<DATATYPE>::watch(DATATYPE *data){
	
	if(pRead != pWrite){
		*data = *pRead;		// Read data
		return 0;
	}
	
	return -1;	// no data
}

/*********************
 �o�b�t�@�ǂނ��Ǔǂ񂾂��Ƃɂ��Ȃ�
IndexRel�̐������������
**********************/
template <class DATATYPE>
int8_t RingBuffer<DATATYPE>::watch(uint16_t IndexRel, DATATYPE *data){
	DATATYPE* pReadRet = pRead;
	
	if(IndexRel<=getNumElements()){
		pReadRet += IndexRel;		// �|�C���^�̌v�Z�Ȃ̂ŏ���Ƀf�[�^�^���̃T�C�Y�ړ����Ă����
		while(pReadRet>&Buffer[BufferSize]){
			pReadRet = pReadRet-BufferSize-1;	
		}
		
		*data = *pReadRet
		return 0;
		
	}
	
	return -1;	// no data
}

/*********************
 �o�b�t�@������
**********************/
template <class DATATYPE>
void RingBuffer<DATATYPE>::clear(void){
	//memset(Buffer, 0U, BufferSize+1);	// Clear the buffer
	pWrite = Buffer;					// Set pointer at start.
	pRead = Buffer;
}

/*********************
 �o�b�t�@�ɂ��܂��Ă���擾
**********************/
template <class DATATYPE>
uint16_t RingBuffer <DATATYPE> ::getNumElements(void){
	
	if(pWrite>=pRead){
		return (pWrite-pRead);
	}else{
		return (uint16_t)(&Buffer[BufferSize] - pRead + pWrite - Buffer)+1;		// �v�f���Ő�����
	}
	
	return 0;
}


#endif