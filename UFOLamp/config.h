#ifndef _CONFIG_H_
#define _CONFIG_H_

#define DEBUG true
#define LED_PIN 6
#define NUM_LEDS 37
#define STEPPER_SPEED 25
#define CONTROLS_PROCESS_INTERVAL 10
// Should be disconnected
#define RANDOM_SEED_PIN A4

#define STEPPER_PIN1 2
#define STEPPER_PIN2 3
#define STEPPER_PIN3 4
#define STEPPER_PIN4 5

#define CONTROL_POTENTIOMETER 0
#define CONTROL_ROTARY_ENCODER 1
#define CONTROL_TYPE CONTROL_POTENTIOMETER

#define POTENTIOMETER_PIN A0
#define POTENTIOMETER_INVERT true 
// When analog value drops bellow this threshold, it will be set to 0
#define POTENTIOMETER_LOW_CUT_OFF 10

#endif // _CONFIG_H_
