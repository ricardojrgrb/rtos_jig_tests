#include "types.h"
#include "osScheduler.h"
#include "button.h" 
#include "bat.h"
#include "mmi.h"
#include "blinkLed.h"


void setup()
{
	Serial.begin(9600);	
		
	p_os_initOs();	/*Init OS*/
	p_bLed_init();	/*Init drvier Leds*/	
	p_but_init();	/*Init drvier Buttons*/	
	p_bat_init();	/*Init drvier Batteries*/	
	p_mmi_init();	/*Init mmi*/	
}

void loop()
{	
	p_os_osScheduler(); 		/*OS THREATMENT*/
	p_but_task();				/*HARDWARE*/
	p_bat_loop();				/*HARDWARE*/	
	p_bLed_all();				/*HARDWARE*/
	p_mmi_all();				/*3_MMI*/
	p_os_clearOsGlolbalData();  /*4_CLEAR DATA AFTER IT BEEN USED*/
}
