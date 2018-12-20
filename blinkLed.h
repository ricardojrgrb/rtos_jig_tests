#ifndef BLINKLED_H_
#define BLINKLED_H_

#define MAX_VEC_BLED	(0xF)
#define LED_DISABLE_ID	(MAX_VEC_BLED)
#define pin13			(13)

typedef enum
{
	BLED_BLINK_LED_DISABLED	=	0,
	BLED_BLINK_LED_1MS		=	(TIME_1MS_BASE),
	BLED_BLINK_LED_10MS		=	(BLED_BLINK_LED_1MS * 10),
	BLED_BLINK_LED_100MS	=	(BLED_BLINK_LED_10MS * 10),
	BLED_BLINK_LED_1S		=	(BLED_BLINK_LED_100MS * 10),
	BLED_BLINK_LED_10S		=	(BLED_BLINK_LED_1S * 10),
}e_bled_times;

typedef struct
{
	int pinPort;
	int timeDuration;
	e_bled_times cadence;
}t_st_bled_create;

void p_bLed_all(void);
void p_bLed_init(void);
bool p_bLed_removeBlink(int i);
int p_create_bled_info(t_st_bled_create index);

#endif
