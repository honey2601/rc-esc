// inspired by http://rcarduino.blogspot.com/2012/05/interfacing-rc-channels-to-l293d-motor.html

#include <Arduino.h>
#include <EnableInterrupt.h>
#include <EEPROM.h>
#include "declarations.h"

const uint16_t rc_neutral = 1500;
const uint16_t rc_max = 2000;
const uint16_t rc_min = 1000;
const uint16_t rc_deadband = 50;

const uint16_t rc_deadlow = 1150;
const uint16_t rc_deadhigh = 1775;
const uint16_t deadlo = rc_neutral - rc_deadband;
const uint16_t deadhi = rc_neutral + rc_deadband;
const uint32_t correction_val = 3000;

const int chR = 7;  // rechts hoch 1100 / runter 1820 // CH 1
// const int chLhori = 8z`; // links links 1820 / rechts 1100
const int chL = 8;  // links hoch 1800 / runter 1100 // CH 3
// const int chRhori = 10; // rechts links 1100 / rechts 1820

const uint8_t pwm_motor_1A = 5;
const uint8_t pwm_motor_1B = 6;
const uint8_t pwm_motor_2A = 9;
const uint8_t pwm_motor_2B = 10;

const uint8_t enable_motor_1 = 14;
const uint8_t enable_motor_2 = 15;

volatile uint32_t timerL;
volatile uint32_t timerR;
volatile uint32_t timerLRef;
volatile uint32_t timerRRef;
volatile uint32_t tempR;
volatile uint32_t tempL;





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
  // waitForTransmitter();
  delay(2000);
  Serial.begin(115200);

}

void loop() {
  Serial.print("LRef:");
  Serial.print(timerLRef);

  Serial.print("RRef:");
  Serial.println(timerRRef);

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
  uint32_t count_end = 1000000;
  while(1) {
    if (count > (count_start + count_end) ) {
      return;
    }

    if ((timerLRef < 1200) && (timerRRef > 1700)) {
      count = micros();
    }
    else {
      count_start = micros();
      count = micros();
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
    // else if ((temp > rc_max) || (temp < rc_min)){ // no valid value set neutral
    //   timerLRef = rc_neutral;
    // }
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
    // else if ((temp > rc_max) || (temp < rc_min)){ // no valid value set neutral
    //   timerRRef = rc_neutral;
    // }
  }
}


void generatePwm(uint32_t value, uint8_t pin) {

}

// todo function here to calibrate high low and neutral values
// potinetially use button to trigger and led to indicate next

// todo add function to stop if values are outside of transmitter range


// todo add sketch with kicad

// todo add capacitors to handle direction changes
