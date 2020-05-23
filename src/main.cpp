// inspired by
// http://rcarduino.blogspot.com/2012/05/interfacing-rc-channels-to-l293d-motor.html

// ecksteinimg.de/Datasheet/Pololu/PO2135.pdf

#include "declarations.h"s
#include <Arduino.h>
#include <EEPROM.h>
#include <EnableInterrupt.h>

static constexpr uint32_t rc_neutral = 1500;
static constexpr uint32_t rc_max = 2000;
static constexpr uint32_t rc_min = 1000;
static constexpr uint32_t rc_deadband = 50;

static constexpr uint32_t rc_deadlow = 1150;
static constexpr uint32_t rc_deadhigh = 1850;
static constexpr uint32_t deadlo = rc_neutral - rc_deadband;
static constexpr uint32_t deadhi = rc_neutral + rc_deadband;
static constexpr uint32_t correction_val = 3000;

static constexpr int chR = 7; // rechts hoch 1100 / runter 1820 // CH 1
// static constexpr int chLhori = 8z`; // links links 1820 / rechts 1100
static constexpr int chL = 8; // links hoch 1800 / runter 1100 // CH 3
// static constexpr int chRhori = 10; // rechts links 1100 / rechts 1820

static constexpr uint8_t pwm_motor_1 = 5;
static constexpr uint8_t pwm_motor_2 = 9;

static constexpr uint8_t phase_motor_1 = 14;
static constexpr uint8_t phase_motor_2 = 15;

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
void convertRightStick2();

void waitForTransmitter();
void cleanup(channel_e stick);
void generatePwm(uint32_t value, channel_e channel, direction_e direction);
uint8_t convertToPWMRange(uint32_t value);

void setup();
void loop();

void setup()
{
  // Set input pins
  pinMode(chR, INPUT);
  pinMode(chL, INPUT);

  // Setup output pins
  pinMode(pwm_motor_1, OUTPUT);
  pinMode(pwm_motor_1, OUTPUT);
  pinMode(phase_motor_1, OUTPUT);
  pinMode(phase_motor_2, OUTPUT);

  enableInterrupt(chR, convertRightStick, CHANGE);
  enableInterrupt(chL, convertLeftStick, CHANGE);

  // WAIT FOR INIT SEQUENCE LEFT HIGH RIGHT LOW FOR 2 SECONDS
  delay(5000);
  waitForTransmitter();
  Serial.begin(115200);
  generatePwm(1750, channel_e::LEFT, direction_e::FORWARD);
  delay(5000);
  generatePwm(1500, channel_e::LEFT, direction_e::FORWARD);
}

void loop()
{
  // #if debug
  //   Serial.print("LRef:");
  //   Serial.print(timerLRef);

  //   Serial.print("RRef:");
  //   Serial.println(timerRRef);
  // #endif
  //   if (timerLRef <= 1500)
  //   {
  //     // left back
  //     generatePwm(timerLRef, channel_e::LEFT, direction_e::BACKWARD);
  //   }
  //   else
  //   {
  //     generatePwm(timerLRef, channel_e::LEFT, direction_e::FORWARD);
  //   }
  //   if (timerRRef <= 1500)
  //   {
  //     // right back
  //     generatePwm(timerRRef, channel_e::RIGHT, direction_e::BACKWARD);
  //   }
  //   else
  //   {
  //     generatePwm(timerRRef, channel_e::RIGHT, direction_e::FORWARD);
  //   }
}

void convertRightStick()
{
  if (digitalRead(chR) == HIGH)
  {
    timerR = micros();
  }
  tempR = micros() - timerR;
  if ((tempR >= rc_min) && (tempR <= rc_max))
  {
    timerRRef = correction_val - tempR;
    cleanup(channel_e::RIGHT);
  }
}

void convertLeftStick()
{
  if (digitalRead(chL) == HIGH)
  {
    timerL = micros();
  }
  else
  {
    timerLRef = micros() - timerL;
    cleanup(channel_e::LEFT);
  }
}

void waitForTransmitter()
{
  volatile uint32_t count = micros();
  volatile uint32_t count_start = micros();
  uint32_t count_end = 10000;
  while (1)
  {
    if (count > (count_start + count_end))
    {
      return;
    }

    if ((timerLRef < 1200) && (timerRRef > 1700))
    {
      count = micros();
#if debug
      Serial.print("Init LRef:");
      Serial.print(timerLRef);

      Serial.print("Init RRef:");
      Serial.println(timerRRef);
#endif
    }
    else
    {
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

void cleanup(channel_e stick)
{
  if (stick == channel_e::LEFT)
  {
    uint32_t temp = timerLRef;
    if ((temp > (deadlo)) && (temp < (deadhi)))
    {
      timerLRef = rc_neutral;
    }
    else if ((temp > rc_deadhigh) && (temp < rc_max))
    {
      timerLRef = rc_max;
    }
    else if ((temp < rc_deadlow) && (temp > rc_min))
    {
      timerLRef = rc_min;
    }
    timerLRef = constrain(timerLRef, rc_min, rc_max);
  }
  else if (stick == channel_e::RIGHT)
  {
    uint32_t temp = timerRRef;
    if ((temp > (deadlo)) && (temp < (deadhi)))
    {
      timerRRef = rc_neutral;
    }
    else if ((temp > rc_deadhigh) && (temp < rc_max))
    {
      timerRRef = rc_max;
    }
    else if ((temp < rc_deadlow) && (temp > rc_min))
    {
      timerRRef = rc_min;
    }
    timerRRef = constrain(timerRRef, rc_min, rc_max);
  }
}

// alternatively occr1a etc.. check out atmega32u4 timers
void generatePwm(uint32_t value, channel_e channel, direction_e direction)
{
  uint8_t val = convertToPWMRange(value);
  if (channel == channel_e::LEFT)
  {
    if (direction == direction_e::FORWARD)
    {
      // phase to forward
      digitalWrite(phase_motor_1, 0);
      analogWrite(pwm_motor_1, val);
    }
    else
    {
      // phase to backward
      analogWrite(phase_motor_1, 1);
      analogWrite(pwm_motor_1, val);
    }
  }
  else
  {
    if (direction == direction_e::FORWARD)
    {
      // phase to forward
      analogWrite(phase_motor_2, 0);
      analogWrite(pwm_motor_2, val);
    }
    else
    {
      // phase to backward
      analogWrite(phase_motor_2, 1);
      analogWrite(pwm_motor_2, val);
    }
  }
}

uint8_t convertToPWMRange(uint32_t value)
{
  uint32_t val = 0;
  double result = 0.0;
  if (value < rc_neutral)
  {
    val = 3000 - value;     //  3000 - 1200 = 1800
    val = val - rc_neutral; // 1800 - 1500 = 300
  }
  else if (value > rc_neutral)
  {
    val = value - rc_neutral; // 1800 - 1500 = 300
  }
  else if (value == rc_neutral)
  {
    return 0;
  }
  result = ((val / 500) * 175) + 75; // motor only drives at high pwms give it a boost of 75

  return static_cast<uint8_t>(result);
}
