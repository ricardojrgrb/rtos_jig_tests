#include "Arduino.h"
#include "string.h"
#include "types.h"
#include "osScheduler.h"
#include "mmi.h"
#include "blinkLed.h"

#define G_os_actual			p_os_getActual()

typedef struct
{
	t_st_bled_create info;
	bool pinState;
	u32 init;
	u64 count;
}t_st_bLed_info;

t_st_bLed_info vec_st_bLed[MAX_VEC_BLED];

int p_create_bled_info(t_st_bled_create index)
{
	for(int i = 0; i < MAX_VEC_BLED; i++)
	{
		if (vec_st_bLed[i].info.cadence == BLED_BLINK_LED_DISABLED)
		{
			vec_st_bLed[i].pinState = 0;
			vec_st_bLed[i].init = millis();
			vec_st_bLed[i].count = 0;
			vec_st_bLed[i].info.cadence = index.cadence;
			vec_st_bLed[i].info.pinPort = index.pinPort;
			vec_st_bLed[i].info.timeDuration = index.timeDuration * BLED_BLINK_LED_1S;
			return i;
		}
	}
	return MAX_VEC_BLED;
}

bool p_bLed_removeBlink(int i)
{
	if ((i < MAX_VEC_BLED) && (vec_st_bLed[i].info.cadence != BLED_BLINK_LED_DISABLED))
	{
		digitalWrite(vec_st_bLed[i].info.pinPort, LOW);
		memset(&vec_st_bLed[i], 0x00, sizeof(vec_st_bLed[i]));
		return TRUE;
	}
	return FALSE;
}

void p_bLed_all(void)
{
	u32 actual_millis = millis();

	for(int i = 0; i < MAX_VEC_BLED; i++)
	{
		if (vec_st_bLed[i].info.cadence == BLED_BLINK_LED_DISABLED)
		{
			continue;
		}

		if ((vec_st_bLed[i].info.timeDuration != 0) && (actual_millis + vec_st_bLed[i].init) >= vec_st_bLed[i].info.timeDuration)
		{
			digitalWrite(vec_st_bLed[i].info.pinPort, LOW);
			memset(&vec_st_bLed[i], 0x00, sizeof(vec_st_bLed[i]));
		}
		else
		{
			if ((actual_millis - vec_st_bLed[i].init) >= vec_st_bLed[i].info.cadence * vec_st_bLed[i].count)
			{
				digitalWrite(vec_st_bLed[i].info.pinPort, !vec_st_bLed[i].pinState);
				vec_st_bLed[i].pinState = !vec_st_bLed[i].pinState;
				vec_st_bLed[i].count++;
			}
		}
	}
}

void p_bLed_init(void)
{
	pinMode(pin13, OUTPUT);

	memset(vec_st_bLed, 0x00, sizeof(vec_st_bLed));

	t_st_bled_create p;

	p.cadence = BLED_BLINK_LED_100MS;
	p.pinPort = pin13;
	p.timeDuration = 0;

	p_create_bled_info(p);
}
