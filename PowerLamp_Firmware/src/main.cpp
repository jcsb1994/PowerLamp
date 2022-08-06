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

uint8_t gPwmMin = 0;
uint8_t gPwmMax = 0;
uint8_t gPwmCurr = 0;
uint8_t incr = 1;
uint8_t dir = 0;

void setLinear(uint8_t y) {
  gPwmMin = y;
  gPwmMax = y;
  gPwmCurr = y; // should not, should move at set
}

void setPulse(uint8_t min, uint8_t max, uint16_t periodMs) {
  float div = (periodMs/1000);
  uint16_t dist = (max-min)*2;
  uint8_t pwmPerSec = dist / div;

  if (pwmPerSec < MIN_PWM_STEPS_PER_SEC) { return; }
  gPwmMin = min;
  gPwmMax = max;
  incr = pwmPerSec / MIN_PWM_STEPS_PER_SEC;
}

void run() {
  static unsigned long timer = 0;
  if(!SudoArduino::isTimeOut(timer,LIGHT_RUN_MS)) { return; }
  timer = millis();
  analogWrite(LED_PWM_GPIO, gPwmCurr);
  if (gPwmMin != gPwmMax) {

    if (dir == 1) {
      if (gPwmCurr < gPwmMax) {
        gPwmCurr += incr;
      } else {
        dir = 0;
      }
    } else if ((dir == 0)) {
      if (gPwmCurr > gPwmMin) {
        gPwmCurr -= incr;
      } else {
        dir = 1;
      }
    }
  }
}

tact tactSwitch = tact(TACT_GPIO);



void setup() {
  pinMode(LED_PWM_GPIO, OUTPUT);
}

int lightMode = NO_LIGHT;

void loop() {


  tactSwitch.poll([]{gPwmCurr = gPwmCurr>20?gPwmCurr-20:0; setPulse(gPwmCurr+1, gPwmCurr+20, 200); }, 
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
      setPulse(50, 200, 2000);
      break;
    case NO_LIGHT:
    default:
      setLinear(0);
      lightMode = NO_LIGHT;
      break;
    }

  run();

}
