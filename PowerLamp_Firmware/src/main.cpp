#include <Arduino.h>
#include "tact.h"
#include "SudoArduino.h"

#define TACT_GPIO (2)
#define LED_PWM_GPIO (9) // 500Hz PWM freq
#define LIGHT_RUN_MS (25)
#define MIN_PWM_STEPS_PER_SEC (1000 / LIGHT_RUN_MS)
//max freq 25*256 = 6400 ms per 256 ticks or 25ms per tick or 

enum Event {
  NOTHING,
  PRESS_FLICKER,
  TURN_OFF,
  NEXT_LIGHT_MODE,
};

enum LightingMode {
  NO_LIGHT,
  HIGH_INTENSITY,
  LOW_INTENSITY,
  SLOW_PULSE,
  LIGHTING_MODE_SIZE
};

uint8_t globalPwmMin = 0;
uint8_t globalPwmMax = 0;
uint8_t globalPwmCurr = 0;
uint8_t incr = 0;

void setLinear(uint8_t y) {
  globalPwmMin = y;
  globalPwmMax = y;
  globalPwmCurr = y; // should not, should move at set
}

void setPulse(uint8_t min, uint8_t max, uint16_t periodMs) {
  auto getPwmPerSec = [](uint8_t min, uint8_t max, uint16_t periodMs)
  {
    return (((max-min)*2) / (periodMs/1000));
  };
  if ((getPwmPerSec(min,max,periodMs)) < MIN_PWM_STEPS_PER_SEC) { return; }
  globalPwmMin = min;
  globalPwmMax = max;
  incr = getPwmPerSec(min,max,periodMs) / MIN_PWM_STEPS_PER_SEC;
}

void run() {
  static unsigned long timer = 0;
  if(!SudoArduino::isTimeOut(timer,LIGHT_RUN_MS)) { return; }
  timer = millis();
  analogWrite(LED_PWM_GPIO, globalPwmCurr);
  if (globalPwmMin != globalPwmMax) {
    (globalPwmCurr > globalPwmMin) ? (globalPwmCurr--) : (globalPwmCurr++) % globalPwmMax;
  }
}
tact tactSwitch = tact(TACT_GPIO);



void setup() {
  pinMode(LED_PWM_GPIO, OUTPUT);
}

int lightMode = NO_LIGHT;

void loop() {


  tactSwitch.poll([]{setPulse(globalPwmCurr>20?globalPwmCurr-20:0, globalPwmCurr, 200); }, 
                  []{lightMode++; lightMode % LIGHTING_MODE_SIZE; }, 
                  []{lightMode = NO_LIGHT; });


  switch (lightMode)
  {
    case HIGH_INTENSITY:
      setLinear(255);
      break;
    case LOW_INTENSITY:
      setLinear(127);
      break;
    case SLOW_PULSE:
    // setLinear(127);
      setPulse(50, 200, 3);
      break;
    case NO_LIGHT:
    default:
      setLinear(0);
      lightMode = NO_LIGHT;
      break;
    }

  run();

}
