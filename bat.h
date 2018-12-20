#ifndef BAT_H_
#define BAT_H_

#define BAT_DISCHARGE_TIME		(TIME_1S_BASE * 12600) // x seconds
#define BAT_WAIT_CHECKCHARGE	(TIME_1S_BASE * 5)

#define PIN_VOLTAGE_BAT1		(A0)
#define PIN_VOLTAGE_BAT2		(A2)
#define PIN_VOLTAGE_BAT3		(A4)
#define PIN_VOLTAGE_BAT4		(A6)
#define PIN_VOLTAGE_BAT5		(A8)

#define PIN_TEMPARATURE_BAT1	(A1)
#define PIN_TEMPARATURE_BAT2	(A3)
#define PIN_TEMPARATURE_BAT3	(A5)
#define PIN_TEMPARATURE_BAT4	(A7)
#define PIN_TEMPARATURE_BAT5	(A9)

#define PIN_CTRL1_BAT1			(34)
#define PIN_CTRL1_BAT2			(38)
#define PIN_CTRL1_BAT3			(42)
#define PIN_CTRL1_BAT4			(46)
#define PIN_CTRL1_BAT5			(50)

#define PIN_CTRL2_BAT1			(36)
#define PIN_CTRL2_BAT2			(40)
#define PIN_CTRL2_BAT3			(44)
#define PIN_CTRL2_BAT4			(48)
#define PIN_CTRL2_BAT5			(52)

typedef enum
{
	E_IO_BAT_CTRL1,
	E_IO_BAT_CTRL2,
	E_IO_BAT_VOLTAGE,
	E_IO_BAT_TEMPERATURE,

	E_IO_BAT_MAX,
}e_io_bat_pins;

typedef enum
{
	E_BAT_PIN_OFF = 0,
	E_BAT_PIN_ON,

	E_BAT_PIN_MAX,
}e_bat_pinCtrl;

typedef enum
{
	E_BAT_MODO0 = 0,
	E_BAT_MODO1,
	E_BAT_MODO2,

	E_BAT_MODMAX,
}e_bat_mode;

typedef enum
{
	E_ST_BAT_DISCONNECTED = 0,
	E_ST_BAT_CONNECTED,
	E_ST_BAT_CHECKCHARGE,
	E_ST_BAT_CHARGING,
	E_ST_BAT_DISCHARGING,
	E_ST_BAT_RESULT,

	E_ST_BAT_MAX,
}e_bat_st;

void p_bat_init(void);
void p_bat_loop(void);
void p_bat_showChargeBatteries(int bat_id);
void p_bat_clearPinCharge(int bat_id);
bool p_bat_getBatConnFlag(int i);
bool p_bat_isChangeModeAllowed(void);
void p_bat_startCharging(int bat_id);
void p_bat_startTestDisCharging(int bat_id);
void p_bat_genericalEnd(int bat_id, char * msg);

#endif
