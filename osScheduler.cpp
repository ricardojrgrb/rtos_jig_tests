#include "Arduino.h"
#include "types.h"
#include "string.h"
#include "stdio.h"
#include "osScheduler.h"

static u32 currentSample = 0;
static u32 previouSample = 0;
static bool timeSampleLocker = FALSE;

volatile t_os_timer os_timer[MAX_QUEE]; // MAX_QUEE = 10
volatile t_os_scheduler os_quee[MAX_QUEE]; // MAX_QUEE = 10
volatile t_os_scheduler G_os_actual;		//actual - init.

static void p_os_initQuee(void);
static void p_os_queeForward(void);
static void p_os_getQueeEvent(void);
static void p_os_timerForward(void);
static void p_os_TimerScheduler(void);
static void p_os_resetTimerValues(u8);

static char _log[100] = " ";

static void p_os_queeForward(void)
{
	u8 i = 0;
	while(i+1 < MAX_QUEE)
	{
		if (os_quee[i+1].osEvent != MMI_OS_DUMMY)
		{
			os_quee[i].osEvent = os_quee[i+1].osEvent;
			os_quee[i].osInfo = os_quee[i+1].osInfo;
			os_quee[i+1].osInfo = MMI_OS_DUMMY;
			os_quee[i+1].osEvent = MMI_OS_DUMMY;
		}
		else
		{
			os_quee[i].osEvent = MMI_OS_DUMMY;
			os_quee[i].osInfo = 0;
		}
		i++;
	}
}

static void p_os_getQueeEvent(void)
{
	if (os_quee[0].osEvent != MMI_OS_DUMMY)
	{
		memset(_log, 0, sizeof(_log));
		G_os_actual.osEvent = os_quee[0].osEvent;
		G_os_actual.osInfo  = os_quee[0].osInfo;
		p_os_queeForward();
	}
}

static void p_os_timerForward(void)
{
	u8 i = MAX_QUEE;
	while (--i > 0)
	{
		if (os_timer[i].osEvent != MMI_OS_DUMMY)
		{
			os_timer[i-1].osInfo = os_timer[i].osInfo;
			os_timer[i-1].osEvent = os_timer[i].osEvent;
			os_timer[i-1].startTime = os_timer[i].startTime;
			os_timer[i-1].execuTime = os_timer[i].execuTime;
		}
	}
}

static void p_os_TimerScheduler(void)
{
	for (u8 i = 0; i < MAX_QUEE; i++)
	{
		if((os_timer[i].osEvent > MMI_OS_DUMMY) &&
				((millis() - os_timer[i].startTime) >= os_timer[i].execuTime))
		{
			p_os_putOsEventInfo(os_timer[i].osEvent, os_timer[i].osInfo);;
			p_os_resetTimerValues(i);
		}
	}
}

static void p_os_taskScheduler(void)
{
	if((millis() - previouSample) >= TIME_1MS_BASE)
	{
		p_os_getQueeEvent();
		timeSampleLocker = FALSE;
	}
}

void p_os_osScheduler(void)
{
	if (timeSampleLocker == FALSE)
	{
		previouSample = millis();
		timeSampleLocker = TRUE;
	}

	p_os_taskScheduler();

	p_os_TimerScheduler();
}

bool p_os_putOsEvent(e_os_Events ev)
{
	return p_os_putOsEventInfo(ev, INVALID_ID);
}

bool p_os_putOsEventInfo(e_os_Events ev, u16 info)
{
	u8 i;
	for ( i=0; i < MAX_QUEE; i++)
	{
		if(os_quee[i].osEvent == MMI_OS_DUMMY)
		{
			os_quee[i].osEvent = ev;
			os_quee[i].osInfo = info;
			return TRUE;
		}
	}
	return FALSE;
}

bool p_os_startTimer(e_os_Events ev, u32 ms_t, u16 info)
{
	u8 i, x;
	bool ret = FALSE;

	for (i=0; i < MAX_QUEE; i++)
	{
		if(os_timer[i].osEvent == ev)
		{
			x = i;
			ret = TRUE;
			break;
		}
		else if(os_timer[i].osEvent == MMI_OS_DUMMY)
		{
			x = i;
			ret = TRUE;
			break;
		}
	}
	if (ret)
	{
		os_timer[x].osEvent = ev;
		os_timer[x].osInfo = info;
		os_timer[x].execuTime = ms_t;
		os_timer[x].startTime = millis();
	}
	return ret;
}

bool p_os_stopTimer(e_os_Events ev)
{
	for (u8 i=0; i < MAX_QUEE; i++)
	{
		if(os_timer[i].osEvent == ev)
		{
			os_timer[i].osEvent = MMI_OS_DUMMY;
			os_timer[i].osInfo = 0;
			os_timer[i].execuTime = 0;
			os_timer[i].startTime = 0;
			return TRUE;
		}
	}
	return FALSE;
}

void p_os_clearOsGlolbalData(void)
{
	memset((void*)&G_os_actual, 0, sizeof(G_os_actual));
	G_os_actual.osInfo = 0xFFFF;
}

volatile t_os_scheduler * p_os_getActual(void)
{
	return &G_os_actual;
}

static void p_os_initQuee(void)
{
	for (u8 i=0; i < MAX_QUEE; i++)
	{
		os_timer[i].osEvent  = os_quee[i].osEvent = MMI_OS_DUMMY;
		os_quee[i].osInfo =  os_timer[i].execuTime = os_timer[i].startTime = 0;
	}
}

#if DBG_OS_SCHEDULER || DBG_OS_WARNINGS
void p_os_printQuees(void)
{
	for (u8 i = 0; i < MAX_QUEE ; i++)
	{
		memset(_log, 0, sizeof(_log));
	}
}
#endif

static void p_os_resetTimerValues(u8 i)
{
	os_timer[i].osEvent = MMI_OS_DUMMY;
	os_timer[i].execuTime = 0;
	os_timer[i].osInfo = 0;
	os_timer[i].startTime = 0;
}
void p_os_initOs(void)
{
	p_os_initQuee();

	p_os_clearOsGlolbalData();

	timeSampleLocker = FALSE;
}
