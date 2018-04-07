#include <Adafruit_NeoPixel.h>
#include <AccelStepper.h>
#include "led_modes.h"

#ifdef __AVR__
  #include <avr/power.h>
#endif

#include "config.h"
bool isOff;

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

AccelStepper stepper(4, STEPPER_PIN1, STEPPER_PIN2, STEPPER_PIN3, STEPPER_PIN4); 


SIGNAL(TIMER0_COMPA_vect) 
{
  stepper.runSpeed();
}


void setup() {
  strip.begin();
  strip.setBrightness(0);
  strip.show();

  #if DEBUG
  Serial.begin(9600);
  #endif

  stepper.setSpeed(STEPPER_SPEED);

  // Enable interrrupts for stepper
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  
  randomSeed(analogRead(RANDOM_SEED_PIN));
}

inline void process_controls() {
  uint8_t newBrightness;
  
  #if CONTROL_TYPE == CONTROL_POTENTIOMETER
    uint16_t value = analogRead(POTENTIOMETER_PIN);
    
    #if POTENTIOMETER_INVERT
      value = 1023 - value;
    #endif

    if(value < POTENTIOMETER_LOW_CUT_OFF)
      value = 0;
    else
      value = map(value, 0, 1023, 0, 255);
    
    newBrightness = value;
    
  #elif CONTROL_TYPE == CONTROL_ROTARY_ENCODER
    // TODO
  #endif  //CONTROL_POTENTIOMETER

  strip.setBrightness(newBrightness);

  if(newBrightness == 0) {
    isOff = true;
  }
  else {
    if(isOff) {
      LEDModes new_mode = (LEDModes)((get_mode() + 1) % LEDModes::LED_MODES_COUNT);
      set_mode(new_mode);
    }
    isOff = false;
  }
}

inline bool _should_process(uint64_t &counter, uint16_t process_every) {
  uint64_t diff = millis() - counter;
  
  if(diff > process_every) {
    counter = millis();
    return true;
  }
  
  return false;
}

inline bool should_process_controls() {
  static uint64_t last_controls_updated = 0;
  return _should_process(last_controls_updated, CONTROLS_PROCESS_INTERVAL);
}

void loop() {
  if(should_process_controls())
    process_controls();

  led_step();
}


