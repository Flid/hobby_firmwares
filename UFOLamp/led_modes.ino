#include "config.h"
#include "led_modes.h"

bool mode_reset_happened = true;
uint8_t speed = 100;
LEDModes mode;


void set_mode(LEDModes new_mode) {
  #if DEBUG
  Serial.print("Setting mode to ");
  Serial.println(new_mode);
  #endif
  mode = new_mode;
  mode_reset_happened = true;
}

LEDModes get_mode() {
  return mode;
}

uint32_t currentColors[NUM_LEDS];

inline void process_mode_constant() {
  // Set all pixels white
  for (uint8_t i=0; i < strip.numPixels(); i++) {
    currentColors[i] = strip.Color(255, 255, 255);
  }
}

void set_random_color_targets(uint32_t* colors, uint8_t count) {
  uint32_t color = strip.Color(random(256), random(256), random(256));
    
  for(uint8_t px=0; px<NUM_LEDS; px++) {
    if(mode == LEDModes::FADE_RANDOM_MULTICOLOR)
      color = strip.Color(random(256), random(256), random(256));
       
     colors[px] = color;
  }

  // Random colors make the image chaotic, let's make it a bit smoother.
  if(mode == LEDModes::FADE_RANDOM_MULTICOLOR) {
    for(uint8_t px=1; px<NUM_LEDS; px+=2) {

      uint8_t* prevBytes = reinterpret_cast<uint8_t*>(&colors[px-1]);
      uint8_t* targetBytes = reinterpret_cast<uint8_t*>(&colors[px]);
      uint8_t* nextBytes = reinterpret_cast<uint8_t*>(&colors[(px+1)%NUM_LEDS]);

      for(uint8_t i=0; i<4; i++) {
        targetBytes[i] = (prevBytes[i] + nextBytes[i]) / 2;
      }
    }
  }

  #if DEBUG
  Serial.print("Target colors has been reset. New target for pixel 0 is ");
  Serial.println(colors[0]);
  #endif
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t wheel(uint8_t wheelPos) {
  wheelPos = 255 - wheelPos;
  if(wheelPos < 85) {
    return strip.Color(255 - wheelPos * 3, 0, wheelPos * 3);
  }
  if(wheelPos < 170) {
    wheelPos -= 85;
    return strip.Color(0, wheelPos * 3, 255 - wheelPos * 3);
  }
  wheelPos -= 170;
  return strip.Color(wheelPos * 3, 255 - wheelPos * 3, 0);
}


inline void process_mode_fade_random() {
  static uint32_t targetColors[NUM_LEDS];
  
  if(mode_reset_happened) {
      set_random_color_targets(targetColors, NUM_LEDS);
      set_random_color_targets(currentColors, NUM_LEDS);
  }
      
  
  bool changed = false;

  for(uint8_t px=0; px<NUM_LEDS; px++) {
    uint8_t* targetBytes = reinterpret_cast<uint8_t*>(&targetColors[px]);
    uint32_t &current = currentColors[px];
    uint8_t* currentBytes = reinterpret_cast<uint8_t*>(&current);

    for(uint8_t i=0; i<4; i++) {
      if (currentBytes[i] < targetBytes[i]) {
        currentBytes[i]++;
        changed = true;
      } else if (currentBytes[i] > targetBytes[i]) {
        currentBytes[i]--;
        changed = true;
      }
    }
    
    
  }
  
  if(!changed)
      set_random_color_targets(targetColors, NUM_LEDS);

  delay(speed/10);
}


inline void process_mode_moving_rainbow() {
  static uint16_t iteration = 0;
  uint8_t colorPerPixel = 255 / (NUM_LEDS-1);

  for(uint8_t px=0; px<NUM_LEDS; px++) {
    currentColors[px] = wheel((px * colorPerPixel + iteration/256) & 255);
  }

  delay(1);
  iteration += speed/4;
}


inline void led_step() {
  switch(mode) {
    case LEDModes::CONSTANT:
      process_mode_constant();
      break;
    case LEDModes::FADE_RANDOM:
    case LEDModes::FADE_RANDOM_MULTICOLOR:
      process_mode_fade_random();
      break;
    case LEDModes::MOVING_RAINBOW:
      process_mode_moving_rainbow();
      break;
  }
  mode_reset_happened = false;

  for(uint8_t px=0; px<NUM_LEDS; px++) {
    strip.setPixelColor(px, currentColors[px]);
  }
  strip.show();
}
