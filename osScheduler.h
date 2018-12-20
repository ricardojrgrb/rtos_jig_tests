#ifndef OSSCHEDULER_H_
#define OSSCHEDULER_H_

#define MAX_QUEE		(10)
#define INVALID_ID		(0xF)

void p_os_printQuees(void);

typedef enum
{
	TIME_1MS_BASE		=(1),
	TIME_10MS_BASE		=(10 * TIME_1MS_BASE),
	TIME_100MS_BASE 	=(10 * TIME_10MS_BASE),
	TIME_1S_BASE		=(10 * TIME_100MS_BASE),
	TIME_10S_BASE		=(10 * TIME_1S_BASE),

	TIME_MAX_BASE,
	TIME_FORCE_u32 = 0xFFFFFFFF, // force u32
}e_os_timeBases;

typedef enum
{
	MMI_OS_DUMMY = 0,

	MMI_BAT_RES_READ,	 			//1
	MMI_BAT_RES_BUTTON, 			//2
	MMI_BAT_OP_BUTTON,				//3

	MMI_BAT_BLINK_EVENT, 			//4

	MMI_BAT_IS_CONNECTED, 			//5
	MMI_BAT_IS_DISCONNECTED, 		//6

	MMI_BAT_FULL_CHARGED, 			//7
	MMI_BAT_START_CHARGING, 		//8
	MMI_BAT_DISCHARGE_VOL_ACHIEVED, //9

	MMI_BAT_NEEDS2CHARGE, 			//10
	MMI_BAT_NEEDS2DISCHARGE, 		//11

	MMI_BAT_HOT_TEMPERATURE, 		//12

	MMI_BAT_TEST_RESULT_OK, 		//13
	MMI_BAT_END_TMR_DISC,			//14

	MMI_BAT_HOLD_RES_TMR, 			//15	

	MMI_BAT_LED_GREEN_BLINK, 		//16

	MMI_BAT_CHECKCHARGE,			//17

	MMI_MAX, 						//18
}e_os_Events;

typedef enum
{
	SYS_PROC_DUMMY= 0,
	SYS_PROC_MMI,
	SYS_PROC_GATE,
	SYS_PROC_OTHERS,

	SYS_PROC_MAX,
}e_os_sysProc;

typedef enum
{
	HIGH_PRIOR_QUEE = 0,
	NORM_PRIOR_QUEE,
}e_quee_prior;

typedef struct
{
	u16 osInfo;
	e_os_Events osEvent;
}t_os_scheduler;

typedef struct
{
	u32 execuTime;
	unsigned long startTime;
	e_os_Events osEvent;
	u8 instance;
	u16 osInfo;
}t_os_timer;

void p_os_osScheduler(void);
void p_os_clearOsGlolbalData(void);
bool p_os_stopTimer(e_os_Events ev);
bool p_os_putOsEvent(e_os_Events ev);
bool p_os_putOsEventInfo(e_os_Events, u16 info);
volatile t_os_scheduler * p_os_getActual(void);
bool p_os_startTimer(e_os_Events, u32 ms_t, u16 info);
void p_os_initOs(void);

#endif /* OSSCHEDULER_H_ */
