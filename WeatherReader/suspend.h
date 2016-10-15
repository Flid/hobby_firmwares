

#ifndef SLEEP_H
#define SLEEP_H

#include <avr/sleep.h>
#include <avr/wdt.h>
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

extern "C" void WDT_vect(void) __attribute__ ((signal));
extern "C" void sleepHandler(void) __attribute__ ((signal));

class Sleep {

public:

	friend void WDT_vect(void);
	friend void sleepHandler(void);

Sleep();

	//------------------------------------------------------
	// Description: sets the Arduino into idle Mode sleep,
	// the least power saving, The idle mode stops the MCU
	// but leaves peripherals and timers running.
	//------------------------------------------------------
	void idleMode() { setSleepMode(SLEEP_MODE_IDLE);}

	//------------------------------------------------------
	// Description: sets the Arduino into adc Mode sleep,
	// This mode makes the MCU enter ADC Noise Reduction mode,
	// stopping the CPU but allowing the ADC, the external interrupts,
	// the 2-wire Serial Interface address watch, Timer/Counter2
	// and the Watchdog to continue operating.
	//------------------------------------------------------
	void adcMode() {setSleepMode(SLEEP_MODE_ADC);}

	//------------------------------------------------------
	// Description: sets the Arduino into power Save Mode sleep,
	// The timer crystal will continue to operate in this mode,
	// Timer2 still active.
	//------------------------------------------------------
	void pwrSaveMode() {setSleepMode(SLEEP_MODE_PWR_SAVE);}

	//------------------------------------------------------
	// Description: sets the Arduino into power Down Mode sleep,
	// The most power saving, all systems are powered down
	// except the watch dog timer and external reset
	//------------------------------------------------------
	void pwrDownMode(){setSleepMode(SLEEP_MODE_PWR_DOWN);}

	//------------------------------------------------------
	// Description: Works like the Arduino delay function, sets the
	// Arduino into sleep mode for a specified time.
	// Parameters: (unsigned long) time in ms of the sleep cycle
	//------------------------------------------------------
	void sleepDelay(unsigned long sleepTime);

	//------------------------------------------------------
	// Description: Works like the Arduino delay function, sets the
	// Arduino into sleep mode for a specified time.
	// Parameters: (unsigned long) time in ms of the sleep cycle
	//            (boolean) prevents the Arduino from entering sleep
	//------------------------------------------------------
	void sleepDelay(unsigned long sleepTime,boolean &abortCycle);

	//------------------------------------------------------
	// Description: the WDT needs to be calibrated against timer 0
	// periodically to keep the sleep time accurate. Default calibration
	// occurs every 100 wake/sleep cycles. recalibrate too often will
	// waste power and too rarely will make the sleep time inaccurate.
	// Parameters: (int) set the # of wake/sleep cycles between calibrations
	//------------------------------------------------------
	void setCalibrationInterval(int interval){ sleepCycleInterval = interval; }

	//------------------------------------------------------
	// Description: set the Arduino into sleep mode until an interrupt is
	// triggered. The interrupt pin is passed in as parameter
	// Parameters: (int) interrupt pin value, 2, 3, etc, see attachinterrupt()
	//      (int) mode of trigger, HIGH,LOW,RISING,CHANGE
	//------------------------------------------------------
	void sleepPinInterrupt(int interrupt,int mode);





private:

	int sleepMode_;
	unsigned long timeSleep;
	float calibv;
	volatile byte isrcalled;
	static Sleep* pSleep; //static ptr to Sleep class for the ISR
	int sleepCycleCount;
	int sleepCycleInterval;

   void setSleepMode(int mode);
   void WDT_Off();
   void WDT_On(byte psMask);
   int sleepWDT(unsigned long remainTime,boolean &abortCycle);
   void calibrateTime(unsigned long sleepTime,boolean &abortCycle); //calibrate the time keeping difference between WDT and Timer0
   unsigned long WDTMillis();	// Estimated millis is real clock + calibrated sleep time

};

#endif

