#ifndef BUTTON_H_
#define BUTTON_H_

typedef enum
{
	BUT_PIN_BUTTON_OP = 0,
	BUT_PIN_BUTTON_RESET,

	BUT_PIN_BUT_MAX,
}e_button_pins;

typedef enum
{
	BUT_ST_PRESSED = 0,
	BUT_ST_UPRESSED,

	BUT_ST_MAX,
}e_but_states;

typedef struct
{
	e_but_states but_state;
	int isButPressed;
}t_st_but_info;

#define PIN_LED_RED_OP1			(30)
#define PIN_LED_YELLOW_OP2		(28)
#define PIN_LED_GREEN_OP3		(26)

#define PIN_BUTTON_OP			(24)

#define	PIN_BUTTON_RESET_BAT1	(2)//25
#define	PIN_BUTTON_RESET_BAT2	(3)//35
#define	PIN_BUTTON_RESET_BAT3	(4)//41
#define	PIN_BUTTON_RESET_BAT4	(5)//47
#define	PIN_BUTTON_RESET_BAT5	(6)//53

void p_but_task(void);
void p_but_init(void);

#endif
