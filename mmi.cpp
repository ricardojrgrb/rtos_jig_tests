#include "Arduino.h"
#include "string.h"
#include "types.h"
#include "bat.h"
#include "osScheduler.h"
#include "mmi.h"
#include "button.h"
#include "blinkLed.h"

#define G_os_actual	p_os_getActual()

typedef struct
{
	int g_id;
	int r_id;
}t_st_mmi_leds_returns;

int mmi_id;	
e_mmi_states mmi_st[MMI_NUM_BATTERIES]; 
e_mmi_bat_res mmi_bat_res[MMI_NUM_BATTERIES]; 
t_st_mmi_leds_returns l_ret[MMI_NUM_BATTERIES];

const char * const mmi_st_names[STM_MMI_MAX]
={
	(char*)"STM_MMI_STANDBY",
	(char*)"STM_MMI_CHECKCHARGE",
	(char*)"STM_MMI_CHARGING",
	(char*)"STM_MMI_DISCHARGING",
	(char*)"STM_MMI_RESULT",
};

const int mmi_led_pins[MMI_PINsLED_MAX][MMI_NUM_BATTERIES]
={
	{
		PIN_LED_GREEN_BAT1,
		PIN_LED_GREEN_BAT2,
		PIN_LED_GREEN_BAT3,
		PIN_LED_GREEN_BAT4,
		PIN_LED_GREEN_BAT5,
	},{
		PIN_LED_RED_BAT1,
		PIN_LED_RED_BAT2,
		PIN_LED_RED_BAT3,
		PIN_LED_RED_BAT4,
		PIN_LED_RED_BAT5,
	}
};

static void p_mmi_changeLedModes(e_bat_mode mode)
{
	digitalWrite(PIN_LED_RED_OP1, LOW);
	digitalWrite(PIN_LED_YELLOW_OP2, LOW);
	digitalWrite(PIN_LED_GREEN_OP3, LOW);

	switch (mode)
	{
	case E_BAT_MODO0:
		digitalWrite(PIN_LED_RED_OP1, HIGH);
	break;

	case E_BAT_MODO1:
		digitalWrite(PIN_LED_YELLOW_OP2, HIGH);
	break;

	case E_BAT_MODO2:
		digitalWrite(PIN_LED_GREEN_OP3, HIGH);
	break;

	default: break;
	}
}

static void p_mmi_clearResultLeds(t_e_pinsLed led, int i)
{
	digitalWrite(mmi_led_pins[led][i], LOW);
}

static void p_mmi_setResultLeds(t_e_pinsLed led, int i)
{
	digitalWrite(mmi_led_pins[led][i], HIGH);
}

static void p_mmi_showResult(int i)
{
	p_mmi_clearResultLeds(MMI_PINsLED_RED, i);
	p_mmi_clearResultLeds(MMI_PINsLED_GREEN, i);

	switch(mmi_bat_res[i])
	{
	case E_MMI_BAT_RES_REPPROVED: 
		p_mmi_setResultLeds(MMI_PINsLED_RED, i); 
		break;
	case E_MMI_BAT_RES_APPROVED: 
		p_mmi_setResultLeds(MMI_PINsLED_GREEN, i); 
		break;
	case E_MMI_BAT_RES_ABORTED: 
		p_mmi_clearResultLeds(MMI_PINsLED_RED, i); 
		p_mmi_clearResultLeds(MMI_PINsLED_GREEN, i);
		break;
	case E_MMI_BAT_FINAL_CHARGE:
		t_st_bled_create bLedGreen;

		bLedGreen.timeDuration = 0;
		bLedGreen.cadence = BLED_BLINK_LED_100MS;

		bLedGreen.pinPort = mmi_led_pins[MMI_PINsLED_GREEN][i];
		l_ret[i].g_id = p_create_bled_info(bLedGreen);
		break;
	case E_MMI_BAT_RES_HOT:
		t_st_bled_create bLed;

		bLed.timeDuration = 0;
		bLed.cadence = BLED_BLINK_LED_100MS;
		bLed.pinPort = mmi_led_pins[MMI_PINsLED_RED][i];
		l_ret[i].r_id = p_create_bled_info(bLed);

		bLed.pinPort = mmi_led_pins[MMI_PINsLED_GREEN][i];
		l_ret[i].g_id = p_create_bled_info(bLed);
		break;
	default:
		break;
	}
}

static bool p_mmi_stopLedBlinking(int led_id)
{
	if (!p_bLed_removeBlink(led_id))
		return FALSE;

	led_id = MAX_VEC_BLED;
	return TRUE;
}

void p_mmi_mainStm(void)
{
	
	switch(mmi_st[mmi_id])
	{
	case STM_MMI_STANDBY:
		
		if ((G_os_actual->osEvent == MMI_BAT_OP_BUTTON) && p_bat_isChangeModeAllowed()) 
		{
			p_mmi_changeLedModes((e_bat_mode)G_os_actual->osInfo); 
		}
		else if(G_os_actual->osEvent == MMI_BAT_IS_CONNECTED)
		{
			p_mmi_clearResultLeds(MMI_PINsLED_GREEN, mmi_id); 
			p_mmi_clearResultLeds(MMI_PINsLED_RED, mmi_id);
			p_os_startTimer(MMI_BAT_CHECKCHARGE, BAT_WAIT_CHECKCHARGE, mmi_id);
			mmi_st[mmi_id] = STM_MMI_BATCONNECTED;
		}
		else if(G_os_actual->osEvent == MMI_BAT_IS_DISCONNECTED)
		{
			p_mmi_clearResultLeds(MMI_PINsLED_GREEN, mmi_id); 
			p_mmi_clearResultLeds(MMI_PINsLED_RED, mmi_id);
			mmi_st[mmi_id] = STM_MMI_BATDISCONNECTED;
		}

		break;
	case STM_MMI_BATCONNECTED:

		if(G_os_actual->osEvent == MMI_BAT_CHECKCHARGE)
		{
			mmi_st[mmi_id] = STM_MMI_CHECKCHARGE;
		}

		break;
	case STM_MMI_BATDISCONNECTED:

		if(G_os_actual->osEvent == MMI_BAT_IS_DISCONNECTED)
		{
			mmi_st[mmi_id] = STM_MMI_STANDBY;
		}

		break;
	case STM_MMI_CHECKCHARGE:
		
		if(G_os_actual->osEvent == MMI_BAT_NEEDS2CHARGE)
		{
			mmi_st[mmi_id] = STM_MMI_CHARGING;
			p_bat_startCharging(mmi_id);
		}
		else if(G_os_actual->osEvent == MMI_BAT_NEEDS2DISCHARGE) 
		{
			mmi_st[mmi_id] = STM_MMI_DISCHARGING;
			p_os_startTimer(MMI_BAT_END_TMR_DISC, BAT_DISCHARGE_TIME, mmi_id);
			p_bat_startTestDisCharging(mmi_id);
		}
		
		break;
	case STM_MMI_CHARGING:

		if(G_os_actual->osEvent == MMI_BAT_IS_DISCONNECTED) 
		{
			p_bat_genericalEnd(mmi_id, "Bateria Desconectada!"); 
			mmi_bat_res[mmi_id] = E_MMI_BAT_RES_ABORTED; 
			mmi_st[mmi_id] = STM_MMI_STANDBY; 
			p_mmi_showResult(mmi_id); 
		}
		else if(G_os_actual->osEvent == MMI_BAT_HOT_TEMPERATURE) 
		{
			p_bat_genericalEnd(mmi_id, "Bateria Quente!"); 
			mmi_bat_res[mmi_id] = E_MMI_BAT_RES_HOT; 
			mmi_st[mmi_id] = STM_MMI_RESULT;
			p_mmi_showResult(mmi_id);
		}
		else if(G_os_actual->osEvent == MMI_BAT_FULL_CHARGED) 
		{
			p_os_startTimer(MMI_BAT_END_TMR_DISC, BAT_DISCHARGE_TIME, mmi_id);
			p_bat_startTestDisCharging(mmi_id); 
			mmi_st[mmi_id] = STM_MMI_DISCHARGING;
		}

		break;
	case STM_MMI_DISCHARGING:

		if(G_os_actual->osEvent == MMI_BAT_IS_DISCONNECTED) 
		{
			p_bat_genericalEnd(mmi_id, "Bateria Desconectada!"); 
			mmi_bat_res[mmi_id] = E_MMI_BAT_RES_ABORTED;
			mmi_st[mmi_id] = STM_MMI_STANDBY; 
			p_mmi_showResult(mmi_id); 
		}
		else if(G_os_actual->osEvent == MMI_BAT_HOT_TEMPERATURE) 
		{
			p_bat_genericalEnd(mmi_id, "Bateria Quente!"); 
			mmi_bat_res[mmi_id] = E_MMI_BAT_RES_HOT;
			mmi_st[mmi_id] = STM_MMI_RESULT;
			p_mmi_showResult(mmi_id); 
		}
		else if(G_os_actual->osEvent == MMI_BAT_DISCHARGE_VOL_ACHIEVED) 
		{
			p_bat_genericalEnd(mmi_id, "Reprovada!"); 
			mmi_bat_res[mmi_id] = E_MMI_BAT_RES_REPPROVED; 
			mmi_st[mmi_id] = STM_MMI_RESULT; 
			p_mmi_showResult(mmi_id); 
		}
		else if(G_os_actual->osEvent == MMI_BAT_END_TMR_DISC) 
		{
			p_os_putOsEventInfo(MMI_BAT_TEST_RESULT_OK, mmi_id); // Manda carregar quando aprovado!
			p_bat_genericalEnd(mmi_id, "Aprovada!");
			mmi_bat_res[mmi_id] = E_MMI_BAT_RES_APPROVED; 
			mmi_st[mmi_id] = STM_MMI_RESULT; 
			p_mmi_showResult(mmi_id);
		}

		break;
	case STM_MMI_RESULT:

		if (G_os_actual->osEvent == MMI_BAT_RES_BUTTON) 
		{
			mmi_bat_res[mmi_id] = E_MMI_BAT_RES_NONE; 
			mmi_st[mmi_id] = STM_MMI_STANDBY; 
			p_mmi_clearResultLeds(MMI_PINsLED_RED, mmi_id);
			p_mmi_clearResultLeds(MMI_PINsLED_GREEN, mmi_id);
		}
		else if(G_os_actual->osEvent == MMI_BAT_IS_DISCONNECTED) 
		{
			mmi_bat_res[mmi_id] = E_MMI_BAT_RES_NONE; 
			mmi_st[mmi_id] = STM_MMI_STANDBY; 
			p_mmi_stopLedBlinking(l_ret[mmi_id].g_id);
			p_mmi_stopLedBlinking(l_ret[mmi_id].r_id);
		}
		else if(G_os_actual->osEvent == MMI_BAT_TEST_RESULT_OK)
		{
			p_mmi_clearResultLeds(MMI_PINsLED_GREEN, mmi_id);
			p_bat_startCharging(mmi_id);
			mmi_bat_res[mmi_id] = E_MMI_BAT_FINAL_CHARGE;
			mmi_st[mmi_id] = STM_MMI_FINAL_CHARGE;
			p_mmi_showResult(mmi_id);
		}

		break;
	case STM_MMI_FINAL_CHARGE:

		if(G_os_actual->osEvent == MMI_BAT_IS_DISCONNECTED)
		{
			p_bat_genericalEnd(mmi_id, "Bateria Desconectada!");
			mmi_bat_res[mmi_id] = E_MMI_BAT_RES_ABORTED;
			mmi_st[mmi_id] = STM_MMI_STANDBY;
			p_mmi_showResult(mmi_id);
		}
		else if(G_os_actual->osEvent == MMI_BAT_FULL_CHARGED)
		{
			p_bat_genericalEnd(mmi_id, "Aprovada!");
			p_mmi_stopLedBlinking(l_ret[mmi_id].g_id);
			mmi_bat_res[mmi_id] = E_MMI_BAT_RES_APPROVED;
			p_mmi_showResult(mmi_id);
		}
		else if(G_os_actual->osEvent == MMI_BAT_RES_BUTTON)
		{
			mmi_bat_res[mmi_id] = E_MMI_BAT_RES_NONE;
			mmi_st[mmi_id] = STM_MMI_STANDBY;
			p_mmi_clearResultLeds(MMI_PINsLED_RED, mmi_id);
			p_mmi_clearResultLeds(MMI_PINsLED_GREEN, mmi_id);
		}

		break;
	default:
		break;
	}
}

void p_mmi_init(void)
{
	pinMode(PIN_LED_RED_OP1, OUTPUT);
	pinMode(PIN_LED_YELLOW_OP2, OUTPUT);
	pinMode(PIN_LED_GREEN_OP3, OUTPUT);

	pinMode(13, OUTPUT); //teste led

	pinMode(PIN_LED_GREEN_BAT1, OUTPUT);
	pinMode(PIN_LED_GREEN_BAT2, OUTPUT);
	pinMode(PIN_LED_GREEN_BAT3, OUTPUT);
	pinMode(PIN_LED_GREEN_BAT4, OUTPUT);
	pinMode(PIN_LED_GREEN_BAT5, OUTPUT);

	pinMode(PIN_LED_RED_BAT1, OUTPUT);
	pinMode(PIN_LED_RED_BAT2, OUTPUT);
	pinMode(PIN_LED_RED_BAT3, OUTPUT);
	pinMode(PIN_LED_RED_BAT4, OUTPUT);
	pinMode(PIN_LED_RED_BAT5, OUTPUT);

	memset(mmi_st, 0x00, sizeof(mmi_st)); 
	memset(mmi_bat_res, 0x00, sizeof(mmi_bat_res)); 
	mmi_id = 0xFF; 

	p_mmi_changeLedModes(E_BAT_MODO0); 
	p_os_putOsEventInfo(MMI_BAT_BLINK_EVENT, INVALID_ID);

	memset(l_ret, LED_DISABLE_ID, sizeof(l_ret));
}

bool p_mmi_isMMiEvent(int ev)
{
	switch(ev)
	{
	case MMI_BAT_OP_BUTTON:
	case MMI_BAT_RES_BUTTON:
	case MMI_BAT_CHECKCHARGE:
	case MMI_BAT_NEEDS2CHARGE:
	case MMI_BAT_FULL_CHARGED:
	case MMI_BAT_END_TMR_DISC:
	case MMI_BAT_TEST_RESULT_OK:
	case MMI_BAT_NEEDS2DISCHARGE:
	case MMI_BAT_HOT_TEMPERATURE:
	case MMI_BAT_IS_CONNECTED:
	case MMI_BAT_IS_DISCONNECTED:
	case MMI_BAT_DISCHARGE_VOL_ACHIEVED:
		return ((mmi_id = (G_os_actual->osInfo & 0xF)) < MMI_NUM_BATTERIES);
	default: break;
	}
	return FALSE;
}

void p_mmi_all(void)
{
	if (p_mmi_isMMiEvent(G_os_actual->osEvent))
	{
		p_mmi_mainStm();
	}
	mmi_id = 0xFF;
}
