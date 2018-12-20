#ifndef TYPES_H_
#define TYPES_H_

#define DBG_OS_WARNINGS		TRUE
#define DBG_OS_SCHEDULER	FALSE
#define DBG_MMI				FALSE
#define DBG_UTIL			FALSE
#define DBG_GATE			FALSE
#define DBG_KB_EXTERNALBTN	FALSE
#define DBG_KB_MATRIXKB		FALSE
#define DBG_EEP				TRUE

#define FALSE     			(0)
#define TRUE      			(!FALSE)
#define NULL				((void*)0)
#define TP_INVALID_INF		(0xFF)


//8-bit types.
typedef signed char 		s8;
typedef unsigned char 		u8;

//16-bit types.
typedef int 				s16;
typedef unsigned short 		u16; // already defined in USBAPI.h:30

//32-bit types.
typedef long 				s32;
typedef unsigned long 		u32;

//64-bit types.
typedef long long 			s64;
typedef unsigned long long 	u64;

#endif /* TYPES_H_ */
