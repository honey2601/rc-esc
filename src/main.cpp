// inspired by http://rcarduino.blogspot.com/2012/05/interfacing-rc-channels-to-l293d-motor.html

#include <Arduino.h>
#include <EnableInterrupt.h>
#include <EEPROM.h>
#include "declarations.h"

static constexpr uint16_t rc_neutral = 1500;
static constexpr uint16_t rc_max = 2000;
static constexpr uint16_t rc_min = 1000;
static constexpr uint16_t rc_deadband = 50;

static constexpr uint16_t rc_deadlow = 1150;
static constexpr uint16_t rc_deadhigh = 1775;
static constexpr uint16_t deadlo = rc_neutral - rc_deadband;
static constexpr uint16_t deadhi = rc_neutral + rc_deadband;
static constexpr uint32_t correction_val = 3000;

static constexpr int chR = 7;  // rechts hoch 1100 / runter 1820 // CH 1
// static constexpr int chLhori = 8z`; // links links 1820 / rechts 1100
static constexpr int chL = 8;  // links hoch 1800 / runter 1100 // CH 3
// static constexpr int chRhori = 10; // rechts links 1100 / rechts 1820

static constexpr uint8_t pwm_motor_1A = 5;
static constexpr uint8_t pwm_motor_1B = 6;
static constexpr uint8_t pwm_motor_2A = 9;
static constexpr uint8_t pwm_motor_2B = 10;

static constexpr uint8_t enable_motor_1 = 14;
static constexpr uint8_t enable_motor_2 = 15;

volatile uint32_t timerL;
volatile uint32_t timerR;
volatile uint32_t timerLRef;
volatile uint32_t timerRRef;
volatile uint32_t tempR;
volatile uint32_t tempL;

#define debug 1

// define local functions
void convertLeftStick();
void convertRightStick();
void waitForTransmitter();
void cleanup (channel_e stick);


void setup() {
  // Set input pins
  pinMode(chR, INPUT);
  pinMode(chL, INPUT);

  // Setup output pins
  pinMode(pwm_motor_1A, OUTPUT);
  pinMode(pwm_motor_1B, OUTPUT);
  pinMode(pwm_motor_2A, OUTPUT);
  pinMode(pwm_motor_2B, OUTPUT);
  pinMode(enable_motor_1, OUTPUT);
  pinMode(enable_motor_2, OUTPUT);


  enableInterrupt(chR, convertRightStick, CHANGE);
  enableInterrupt(chL, convertLeftStick, CHANGE);

  // WAIT FOR INIT SEQUENCE LEFT HIGH RIGHT LOW FOR 2 SECONDS
  delay(2000);
  waitForTransmitter();
  Serial.begin(115200);

}

void loop() {
#if debug
  Serial.print("LRef:");
  Serial.print(timerLRef);

  Serial.print("RRef:");
  Serial.println(timerRRef);
#endif
}
// TODO add conversion of time to pwm values
void convertRightStick() {
  if(digitalRead(chR) == HIGH) {
    timerR = micros();
  }
    tempR = micros() - timerR;
    timerRRef = correction_val - tempR;
    cleanup(RIGHT);
}
// TODO add conversion of time to pwm values

void convertLeftStick() {
  if(digitalRead(chL) == HIGH) {
    timerL = micros();
  }
  else {

    timerLRef = micros() - timerL;
    cleanup(LEFT);
  }
}

void waitForTransmitter() {
  volatile uint32_t count = micros();
  volatile uint32_t count_start = micros();
  uint32_t count_end = 10000;
  while(1) {
    if (count > (count_start + count_end) ) {
      return;
    }

    if ((timerLRef < 1200) && (timerRRef > 1700)) {
      count = micros();
      #if debug
      Serial.print("Init LRef:");
      Serial.print(timerLRef);

      Serial.print("Init RRef:");
      Serial.println(timerRRef);
      #endif
    }
    else {
      count_start = micros();
      count = micros();
      #if debug
      Serial.print("Error LRef:");
      Serial.print(timerLRef);

      Serial.print("Error RRef:");
      Serial.println(timerRRef);
      #endif
    }
  }
}

// cleanup inputs regarding dead space
void cleanup(channel_e stick) {
  if(stick == LEFT) {
    uint32_t temp = timerLRef;
    if((temp > (deadlo)) && (temp < (deadhi))) {
      timerLRef = rc_neutral;
    }
    else if ((temp > rc_deadhigh) && (temp < rc_max)) {
      timerLRef = rc_max;
    }
    else if ((temp < rc_deadlow) && (temp > rc_min)) {
      timerLRef = rc_min;
    }
    else if ((temp > rc_max) || (temp < rc_min)){ // no valid value set neutral
      timerLRef = rc_neutral;
    }
  }
  if(stick == RIGHT) {
    uint32_t temp = timerRRef;
    if((temp > (deadlo)) && (temp < (deadhi))) {
      timerRRef = rc_neutral;
    }
    else if ((temp > rc_deadhigh) && (temp < rc_max)) {
      timerRRef = rc_max;
    }
    else if ((temp < rc_deadlow) && (temp > rc_min)) {
      timerRRef = rc_min;
    }
    else if ((temp > rc_max) || (temp < rc_min)){ // no valid value set neutral
      timerRRef = rc_neutral;
    }
  }
}


// add arduino pwm functionality  see here for examples
// https://arduino-projekte.webnode.at/registerprogrammierung/fast-pwm/
// pwm setup
// cli(); // disable global interrupts
//
//      // init output pins
//      DDRC |= (1<<DDC6); // PC6 as output / OC3A as output / D5 as output // OC3A als PWM-Pin / Timer/Counter1
//      DDRC |= (1<<DDC7); // onboard LED - Arduino Pin 13
//
//      //PRR1 |= 1<<PRUSB; // disable usb
//
//      TCCR3A |= (1<<COM3A0); // Toggle OC3A on compare match
//      TCCR3B |= (1<<WGM32); // turn on CTC-Mode // Mode 4: CTC-Mode mit OCR3A als TOP
//      TCCR3B |= (1<<CS31); // Pre-Scaler N=8
//      TIMSK3 |= (1<<OCIE3A); // Output Compare A Match Interrupt Enable
//
//      // set 25 Hz -- OCR3A --> 39.999
//      OCR3A = 60000;
//
//      sei(); // enable global inte
// ISR(TIMER3_COMPA_vect)
// {
//   PINC |= (1<<PINC7);
// }
void generatePwm(uint32_t value, uint8_t pin) {

}
