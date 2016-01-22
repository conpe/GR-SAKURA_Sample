
#ifndef __COMMON_DATA_TYPE_H__
#define __COMMON_DATA_TYPE_H__

// datatype


typedef signed char     int8_t;
typedef unsigned char   uint8_t;
typedef signed short    int16_t;
typedef unsigned short  uint16_t;
typedef signed long     int32_t;
typedef unsigned long   uint32_t;
//typedef bool bool_t;		// <- OLED動かなくなる(´・ω・｀)
typedef uint8_t bool_t;	// バッファがおかしいと読んで、バッファのみuint8_tにして挑戦。


#endif