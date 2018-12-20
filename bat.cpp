#define CHARGE_VALUE 	(0.0048)

#include "types.h"
#include "button.h"
#include "osScheduler.h"
#include "mmi.h"
#include "Arduino.h"
#include "bat.h"

#define BAT_HOLD_RESULT			(TIME_1S_BASE * 5) /*x seconds*/
#define G_os_actual				p_os_getActual()
#define N_AMOSTRAS				(50) 

typedef struct
{
	float value[N_AMOSTRAS];
	int id;
}t_st_bat_Average;

bool bat_conn_flag[MMI_NUM_BATTERIES];
e_bat_st  bat_st[MMI_NUM_BATTERIES];	

t_st_bat_Average isBatteryHot[MMI_NUM_BATTERIES];
t_st_bat_Average isBatCharged[MMI_NUM_BATTERIES];
t_st_bat_Average checkNeedOfCharge[MMI_NUM_BATTERIES];
t_st_bat_Average isBatteryConnected[MMI_NUM_BATTERIES];
t_st_bat_Average isDischargeVoltageAchieved[MMI_NUM_BATTERIES];
t_st_bat_Average showChargeBatteries[MMI_NUM_BATTERIES];

#define IS_BAT_CONNECTED(v)			(v <  1.70)
#define IS_BAT_HOT(v)				(v <= 0.60)
#define IS_BAT_DISC_VOLT_ACHIE(v)	(v <= 2.00)
#define IS_BAT_FULL_CHARGED(v)		(v >= 4.20)
#define IS_BAT_NEED2CHARGE(v)		(v <  4.20)

const char * const bat_st_names[E_ST_BAT_MAX]
={
	(char*)"E_ST_BAT_DISCONNECTED",
	(char*)"E_ST_BAT_CHARGING",
	(char*)"E_ST_BAT_DISCHARGING",
	(char*)"E_ST_BAT_RESULT",
	(char*)"E_ST_BAT_CHECKCHARGE",
};

const int io_bat_pins[MMI_NUM_BATTERIES][E_IO_BAT_MAX]
={
	{/*BAT 1*/
		PIN_CTRL1_BAT1,
		PIN_CTRL2_BAT1,
		PIN_VOLTAGE_BAT1,
		PIN_TEMPARATURE_BAT1,
	},{
		/*BAT 2*/
		PIN_CTRL1_BAT2,
		PIN_CTRL2_BAT2,
		PIN_VOLTAGE_BAT2,
		PIN_TEMPARATURE_BAT2,
	},{
		/*BAT 3*/
		PIN_CTRL1_BAT3,
		PIN_CTRL2_BAT3,
		PIN_VOLTAGE_BAT3,
		PIN_TEMPARATURE_BAT3,
	},{
		/*BAT 4*/
		PIN_CTRL1_BAT4,
		PIN_CTRL2_BAT4,
		PIN_VOLTAGE_BAT4,
		PIN_TEMPARATURE_BAT4,
	},{
		/*BAT 5*/
		PIN_CTRL1_BAT5,
		PIN_CTRL2_BAT5,
		PIN_VOLTAGE_BAT5,
		PIN_TEMPARATURE_BAT5,
	},					
};

static void p_bat_setPinCtrl(int bat_id, int pin_id, e_bat_pinCtrl on_off)
{
	digitalWrite(io_bat_pins[bat_id][pin_id], on_off);
}

static float readValues(int bat_id, int pin_id)
{
	return (CHARGE_VALUE * analogRead(io_bat_pins[bat_id][pin_id]));
}

void p_bat_clearPinCharge(int bat_id)
{
	checkNeedOfCharge[bat_id].value[checkNeedOfCharge[bat_id].id] = 0.00;
}

static void p_bat_isBatteryConnected(int bat_id)
{
	isBatteryConnected[bat_id].value[isBatteryConnected[bat_id].id] = readValues(bat_id, E_IO_BAT_TEMPERATURE);
	isBatteryConnected[bat_id].id++;

	if (isBatteryConnected[bat_id].id >= N_AMOSTRAS)
	{
		float t = 0.0;
		for (int i=0; i < N_AMOSTRAS; i++)
		{
			t += isBatteryConnected[bat_id].value[i];
			isBatteryConnected[bat_id].id = 0;
			delay(100); // tempo de 100ms entre uma leitura e outra;
		}

		if (IS_BAT_CONNECTED(t/N_AMOSTRAS))
        {
        	p_os_putOsEventInfo(MMI_BAT_IS_CONNECTED, bat_id);
            bat_conn_flag[bat_id] = TRUE;
        }
        else if (!IS_BAT_CONNECTED(t/N_AMOSTRAS))
        {
            p_os_putOsEventInfo(MMI_BAT_IS_DISCONNECTED, bat_id);
            bat_conn_flag[bat_id] = FALSE;
        }
	}
}

void p_bat_showChargeBatteries(int bat_id)
{
	showChargeBatteries[bat_id].value[showChargeBatteries[bat_id].id] = readValues(bat_id, E_IO_BAT_VOLTAGE);
	showChargeBatteries[bat_id].id++;

	if(showChargeBatteries[bat_id].id >= N_AMOSTRAS)
	{
		float t = 0.0;
		for(int i=0; i < N_AMOSTRAS; i++)
		{
			t += showChargeBatteries[bat_id].value[i];
			showChargeBatteries[bat_id].id = 0;
		}

		Serial.print("[ ");
		Serial.print(bat_id);
		Serial.print(" ] - Carga: ");
		Serial.print(t/N_AMOSTRAS);
		Serial.print("\n");

	}
}


static void p_bat_checkNeedOfCharge(int bat_id)
{

	checkNeedOfCharge[bat_id].value[checkNeedOfCharge[bat_id].id] = readValues(bat_id, E_IO_BAT_VOLTAGE);
	checkNeedOfCharge[bat_id].id++;

	if (checkNeedOfCharge[bat_id].id >= N_AMOSTRAS)
	{
		float t = 0.0;
		for (int i=0; i < N_AMOSTRAS; i++)
		{
			t += checkNeedOfCharge[bat_id].value[i];
			checkNeedOfCharge[bat_id].id = 0;
		}

		if (IS_BAT_NEED2CHARGE((t/N_AMOSTRAS)))
			p_os_putOsEventInfo(MMI_BAT_NEEDS2CHARGE, bat_id);
		else
			p_os_putOsEventInfo(MMI_BAT_NEEDS2DISCHARGE, bat_id);
	}
}

static void p_bat_isBatteryHot(int bat_id)
{
	isBatteryHot[bat_id].value[isBatteryHot[bat_id].id] = readValues(bat_id, E_IO_BAT_TEMPERATURE);
	isBatteryHot[bat_id].id++;

	if (isBatteryHot[bat_id].id >= N_AMOSTRAS)
	{
		float t = 0.0;
		for (int i=0; i < N_AMOSTRAS; i++)
		{
			t += isBatteryHot[bat_id].value[i];
			isBatteryHot[bat_id].id = 0;
		}

		if (IS_BAT_HOT((t/N_AMOSTRAS)))
			p_os_putOsEventInfo(MMI_BAT_HOT_TEMPERATURE, bat_id);
	}
}

static void p_bat_isDischargeVoltageAchieved(int bat_id)
{
	isDischargeVoltageAchieved[bat_id].value[isDischargeVoltageAchieved[bat_id].id] = readValues(bat_id, E_IO_BAT_VOLTAGE);
	isDischargeVoltageAchieved[bat_id].id++;

	if (isDischargeVoltageAchieved[bat_id].id >= N_AMOSTRAS)
	{
		float t = 0.0;
		for (int i=0; i < N_AMOSTRAS; i++)
		{
			t += isDischargeVoltageAchieved[bat_id].value[i];
			isDischargeVoltageAchieved[bat_id].id = 0;
		}

		if (IS_BAT_DISC_VOLT_ACHIE((t/N_AMOSTRAS)))
			p_os_putOsEventInfo(MMI_BAT_DISCHARGE_VOL_ACHIEVED, bat_id);
	}
}

static void p_bat_isBatCharged(int bat_id)
{
	isBatCharged[bat_id].value[isBatCharged[bat_id].id] = readValues(bat_id, E_IO_BAT_VOLTAGE);
	isBatCharged[bat_id].id++;

	if (isBatCharged[bat_id].id >= N_AMOSTRAS)
	{
		float t = 0.0;
		for (int i=0; i < N_AMOSTRAS; i++)
		{
			t += isBatCharged[bat_id].value[i];
			isBatCharged[bat_id].id = 0;
		}

		if (IS_BAT_FULL_CHARGED((t/N_AMOSTRAS)))
			p_os_putOsEventInfo(MMI_BAT_FULL_CHARGED, bat_id);
	}
}

void p_bat_startTestDisCharging(int bat_id)
{
	p_bat_setPinCtrl(bat_id, E_IO_BAT_CTRL1, E_BAT_PIN_OFF);
	p_bat_setPinCtrl(bat_id, E_IO_BAT_CTRL2, E_BAT_PIN_ON);
	p_os_startTimer(MMI_BAT_END_TMR_DISC, BAT_DISCHARGE_TIME, bat_id);
	bat_st[bat_id] = E_ST_BAT_DISCHARGING;
}

void p_bat_genericalEnd(int bat_id, char * msg)
{
	bat_st[bat_id] = E_ST_BAT_DISCONNECTED;
	p_bat_setPinCtrl(bat_id, E_IO_BAT_CTRL1, E_BAT_PIN_OFF);
	p_bat_setPinCtrl(bat_id, E_IO_BAT_CTRL2, E_BAT_PIN_OFF);
}

void p_bat_startCharging(int bat_id)
{
	bat_st[bat_id] = E_ST_BAT_CHARGING;
	p_bat_setPinCtrl(bat_id, E_IO_BAT_CTRL1, E_BAT_PIN_ON);
	p_bat_setPinCtrl(bat_id, E_IO_BAT_CTRL2, E_BAT_PIN_OFF);

}

bool p_bat_isChangeModeAllowed(void)
{
	for (int i=0; i < MMI_NUM_BATTERIES; i++)
	{
		if (bat_st[i] != E_ST_BAT_DISCONNECTED)
			return FALSE;
	}
	return TRUE;
}

void p_bat_loop(void)
{	
	for (int i=0; i<MMI_NUM_BATTERIES; i++)
	{
		switch(bat_st[i])
		{
		case E_ST_BAT_DISCONNECTED:
			p_bat_isBatteryConnected(i);
			if(bat_conn_flag[i] == TRUE)
			{
				bat_st[i] = E_ST_BAT_CONNECTED;
			}
			break;
		case E_ST_BAT_CONNECTED:
			p_bat_isBatteryConnected(i);
 			if(bat_conn_flag[i] == TRUE)
 			{
 				bat_st[i] = E_ST_BAT_CHECKCHARGE;    
 			}
 			if(bat_conn_flag[i] == FALSE)
 			{
 				bat_st[i] = E_ST_BAT_DISCONNECTED;
 			}
			break;
		case E_ST_BAT_CHECKCHARGE:
			p_bat_isBatteryConnected(i);
			p_bat_checkNeedOfCharge(i);
			break;
		case E_ST_BAT_CHARGING:
			p_bat_isBatteryConnected(i); 
			p_bat_isBatCharged(i); 
			p_bat_isBatteryHot(i);
			p_bat_showChargeBatteries(i);
			break;
		case E_ST_BAT_DISCHARGING:
			p_bat_isBatteryConnected(i); 
			p_bat_isDischargeVoltageAchieved(i); 
			p_bat_isBatteryHot(i);
			p_bat_showChargeBatteries(i);
			break;
		default: break;
		}
	}
}

void p_bat_init(void)
{
	memset(bat_st, 0x00, sizeof(bat_st));

	pinMode(PIN_VOLTAGE_BAT1, INPUT);
	pinMode(PIN_VOLTAGE_BAT2, INPUT);
	pinMode(PIN_VOLTAGE_BAT3, INPUT);
	pinMode(PIN_VOLTAGE_BAT4, INPUT);
	pinMode(PIN_VOLTAGE_BAT5, INPUT);

	pinMode(PIN_TEMPARATURE_BAT1, INPUT);
	pinMode(PIN_TEMPARATURE_BAT2, INPUT);
	pinMode(PIN_TEMPARATURE_BAT3, INPUT);
	pinMode(PIN_TEMPARATURE_BAT4, INPUT);
	pinMode(PIN_TEMPARATURE_BAT5, INPUT);

	pinMode(PIN_CTRL1_BAT1, OUTPUT);
	pinMode(PIN_CTRL1_BAT2, OUTPUT);
	pinMode(PIN_CTRL1_BAT3, OUTPUT);
	pinMode(PIN_CTRL1_BAT4, OUTPUT);
	pinMode(PIN_CTRL1_BAT5, OUTPUT);

	pinMode(PIN_CTRL2_BAT1, OUTPUT);
	pinMode(PIN_CTRL2_BAT2, OUTPUT);
	pinMode(PIN_CTRL2_BAT3, OUTPUT);
	pinMode(PIN_CTRL2_BAT4, OUTPUT);
	pinMode(PIN_CTRL2_BAT5, OUTPUT);

	memset(bat_conn_flag, 0x00, sizeof(bat_conn_flag));
}




