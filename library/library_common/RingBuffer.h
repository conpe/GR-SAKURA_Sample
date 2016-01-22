/********************************************/
/*		リングバッファ						*/
/*					for RX63n @ CS+			*/
/*					Wrote by conpe_			*/
/*							2015/12/29		*/
/********************************************/

// テンプレート使ってどんなデータ型でも使える。
// バッファの長さはuint16_t(65535)個まで。

//【更新履歴】
// 2015.11.28 バッファ確保失敗した時はバッファサイズを0にする


//【更新予定】
// バッファがいっぱいでも足しちゃうモード

#ifndef RING_BUFFER_HPP_
#define RING_BUFFER_HPP_

#include "CommonDataType.h"
#include <string.h>	// memsetに使う


// debug
#include "iodefine.h"


#define BUFFER_SIZE_DEFAULT  255U

template <typename DATATYPE> class RingBuffer
{
private : 
	DATATYPE *Buffer;	// buffer
	uint16_t BufferSize;
	DATATYPE *pWrite;	// 次書く場所
	DATATYPE *pRead;	// 次読む場所
	
public:
	RingBuffer(uint16_t BufferSize);	// Constructor
	RingBuffer(void);
	~RingBuffer(void);
	
	int8_t add(DATATYPE data);		// 値追加
	int8_t read(DATATYPE *data);		// 値取り出し
	DATATYPE read(void){DATATYPE tmp; read(&tmp); return tmp;};		// 値取り出し
	int8_t watch(DATATYPE *data);	// 値見るだけ
	int8_t watch(uint16_t IndexRel, DATATYPE *data);	//値見るだけ(IndexRel個先のを見る)
	
	uint16_t getNumElements(void);
	uint16_t getBufferSize(void){return BufferSize;};
	uint16_t getFreeSpace(void){return getBufferSize()-getNumElements();};
	uint16_t getWriteIndex(void){return (uint16_t)(pWrite-Buffer);};		//バッファ先頭から何番目を指しているか
	uint16_t getReadIndex(void){return (uint16_t)(pRead-Buffer);};
	
	bool_t isFull(void){return (getNumElements()>=BufferSize);};
	bool_t isEmpty(void){return (pWrite==pRead);};
	void clear(void);
	
};




/*********************
 コンストラクタ
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
 デストラクタ
**********************/
template <class DATATYPE>
RingBuffer<DATATYPE> ::~RingBuffer(void){
	delete[] Buffer;
}


/*********************
 バッファに追加する
バッファがいっぱいなら-1を返す
**********************/
template <class DATATYPE>
int8_t RingBuffer<DATATYPE>::add(DATATYPE data){
	DATATYPE *pNext = pWrite+1;
	
	if(pNext > &Buffer[BufferSize]){	// 一番後ろまで行ったら
		pNext = Buffer;					// 先頭へ
	}
	
	if(pNext != pRead){	// not reach to pRead
		*pWrite = data;		// Write data
		pWrite = pNext;		// Update pointer
		return 0;
	}
	
	return -1;
}

/*********************
 バッファ読む
バッファになければ-1を返す
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
 バッファ読むけど読んだことにしない
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
 バッファ読むけど読んだことにしない
IndexRelの数だけ先を見る
**********************/
template <class DATATYPE>
int8_t RingBuffer<DATATYPE>::watch(uint16_t IndexRel, DATATYPE *data){
	DATATYPE* pReadRet = pRead;
	
	if(IndexRel<=getNumElements()){
		pReadRet += IndexRel;		// ポインタの計算なので勝手にデータ型分のサイズ移動してくれる
		while(pReadRet>&Buffer[BufferSize]){
			pReadRet = pReadRet-BufferSize-1;	
		}
		
		*data = *pReadRet
		return 0;
		
	}
	
	return -1;	// no data
}

/*********************
 バッファ初期化
**********************/
template <class DATATYPE>
void RingBuffer<DATATYPE>::clear(void){
	//memset(Buffer, 0U, BufferSize+1);	// Clear the buffer
	pWrite = Buffer;					// Set pointer at start.
	pRead = Buffer;
}

/*********************
 バッファにたまってる個数取得
**********************/
template <class DATATYPE>
uint16_t RingBuffer <DATATYPE> ::getNumElements(void){
	
	if(pWrite>=pRead){
		return (pWrite-pRead);
	}else{
		return (uint16_t)(&Buffer[BufferSize] - pRead + pWrite - Buffer)+1;		// 要素数で数える
	}
	
	return 0;
}


#endif