#include "types.h"
#include "mmi.h"
#include "osScheduler.h"
#include "Arduino.h"
#include "bat.h"
#include "button.h"

#define G_os_actual				(p_os_getActual())

#define BUTTON_READ_TIMES		(5)
#define BUTTON_READ_10MS_TIME	(TIME_10MS_BASE)
#define IS_BUTTON_PRESSED(b)	(digitalRead(b) == 0)
#define IS_BUTTON_RELEASED(b)	(digitalRead(b) == 1)

e_bat_mode bat_mode;
t_st_but_info mode_but;
t_st_but_info reset_but[MMI_NUM_BATTERIES];

const int reset_but_pins[MMI_NUM_BATTERIES]
={
	PIN_BUTTON_RESET_BAT1,
	PIN_BUTTON_RESET_BAT2,
	PIN_BUTTON_RESET_BAT3,
	PIN_BUTTON_RESET_BAT4,
	PIN_BUTTON_RESET_BAT5,
};

static void p_but_checkBtn(void)
{	/*RESET BUTTON DEBAUNCE*/
	for (int i=0; i < MMI_NUM_BATTERIES; i++)
	{
		if (IS_BUTTON_PRESSED(reset_but_pins[i]))
		{

			if ((reset_but[i].isButPressed++ >= BUTTON_READ_TIMES) && (reset_but[i].but_state == BUT_ST_UPRESSED))
			{

				p_os_putOsEventInfo(MMI_BAT_RES_BUTTON, i);
				reset_but[i].but_state = BUT_ST_PRESSED;
			}
		}
		else
		{
			reset_but[i].but_state = BUT_ST_UPRESSED;
			reset_but[i].isButPressed = 0;
		}
	}

	/*OPERATION MODE BUTTON DEBAUNCE*/
	if (IS_BUTTON_PRESSED(PIN_BUTTON_OP))
	{

		if ((mode_but.isButPressed++ >= BUTTON_READ_TIMES) && (mode_but.but_state == BUT_ST_UPRESSED))
		{
			mode_but.but_state = BUT_ST_PRESSED;
			bat_mode = (e_bat_mode)((((int)bat_mode)+1) % E_BAT_MODMAX);
			p_os_putOsEventInfo(MMI_BAT_OP_BUTTON, bat_mode);
		}
	}
	else
	{
		mode_but.but_state = BUT_ST_UPRESSED;
		mode_but.isButPressed = 0;
	}
}

void p_but_task(void)
{	/*Check if button is pressed*/
	if (G_os_actual->osEvent == MMI_BAT_RES_READ)
	{
		p_but_checkBtn();
		p_os_startTimer(MMI_BAT_RES_READ, BUTTON_READ_10MS_TIME, 0);
	}
}

void p_but_init(void)
{	/*Buttons*/
	pinMode(PIN_BUTTON_OP, INPUT);

	for (int i=0; i < MMI_NUM_BATTERIES; i++)
	{
		pinMode(reset_but_pins[i], INPUT);
		digitalWrite(reset_but_pins[i], HIGH);
	}

	bat_mode = E_BAT_MODO0;

	p_os_startTimer(MMI_BAT_RES_READ, BUTTON_READ_10MS_TIME, 0);
	memset(reset_but, 0x00, sizeof(reset_but));
	memset(&mode_but, 0x00, sizeof(mode_but));
}

