/*
 * Copyright (c) 2017 Evan Kale
 * Email: EvanKale91@gmail.com
 * Web: www.youtube.com/EvanKale
 * Social: @EvanKale91
 *
 * This file is part of FidgetUSBControllerOfKickassery.
 *
 * FidgetUSBControllerOfKickassery is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
* Compilation note:
* - Requires MHeironimus's Joystick library
* - Get it here: https://github.com/MHeironimus/ArduinoJoystickLibrary
*/

#include <Joystick.h>
#include <math.h>

#define JOYSTICK_MID_VALUE 0.0
#define JOYSTICK_RANGE 127.0
#define JOYSTICK_SENSITIVITY 10
#define JOYSTICK_FN_MAX_Y 5.0
#define EXP_SMOOTHING_ALPHA 0.05

#define MOTOR_MID_VALUE 512
#define MOTOR_RANGE 250.0
#define MOTOR0_PIN A0
#define MOTOR1_PIN A1

#define NUM_BUTTONS 6
#define BUTTON0_PIN 15
#define BUTTON1_PIN 14
#define BUTTON2_PIN 16
#define BUTTON3_PIN 10
#define BUTTON4_PIN 8
#define BUTTON5_PIN 9

#define LED_PIN 3

int buttonPin[NUM_BUTTONS];
bool buttonState[NUM_BUTTONS];

float motorLow, motorHigh;
float joystickLow, joystickHigh;
float joystickFnMaxX;
float oneMinusExpSmoothingAlpha;

float filteredMotorValue0 = MOTOR_MID_VALUE;
float filteredMotorValue1 = MOTOR_MID_VALUE;

void setup()
{
  Joystick.begin();

  motorLow = MOTOR_MID_VALUE - MOTOR_RANGE;
  motorHigh = MOTOR_MID_VALUE + MOTOR_RANGE;

  joystickLow = JOYSTICK_MID_VALUE - JOYSTICK_RANGE;
  joystickHigh = JOYSTICK_MID_VALUE + JOYSTICK_RANGE;

  joystickFnMaxX = ((pow(M_E, JOYSTICK_FN_MAX_Y) - 1)) / JOYSTICK_SENSITIVITY;
  oneMinusExpSmoothingAlpha = 1 - EXP_SMOOTHING_ALPHA;

  buttonPin[0] = BUTTON0_PIN;
  buttonPin[1] = BUTTON1_PIN;
  buttonPin[2] = BUTTON2_PIN;
  buttonPin[3] = BUTTON3_PIN;
  buttonPin[4] = BUTTON4_PIN;
  buttonPin[5] = BUTTON5_PIN;

  for (int i = 0; i < NUM_BUTTONS; ++i)
  {
    pinMode(buttonPin[i], INPUT_PULLUP);
    buttonState[i] = false;
  }

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
}

void loop()
{
  int motorValue0 = analogRead(MOTOR0_PIN);
  filteredMotorValue0 = (EXP_SMOOTHING_ALPHA * motorValue0 + oneMinusExpSmoothingAlpha * filteredMotorValue0);
  Joystick.setXAxis((int)motorToJoystickValue(filteredMotorValue0));

  int motorValue1 = analogRead(MOTOR1_PIN);
  filteredMotorValue1 = (EXP_SMOOTHING_ALPHA * motorValue1 + oneMinusExpSmoothingAlpha * filteredMotorValue1);
  Joystick.setYAxis((int)motorToJoystickValue(filteredMotorValue1));

  for (int i = 0; i < NUM_BUTTONS; ++i)
  {
    bool btnStateRead = !digitalRead(buttonPin[i]);
    if (btnStateRead != buttonState[i])
    {
      Joystick.setButton(i, btnStateRead);
      buttonState[i] = btnStateRead;
    }
  }
}

float motorToJoystickValue(float filteredMotorValue)
{
  float normalizedMotorValue = (filteredMotorValue - MOTOR_MID_VALUE) / MOTOR_RANGE;

  if (normalizedMotorValue > 1.0)
    normalizedMotorValue = 1.0;
  else if (normalizedMotorValue < -1.0)
    normalizedMotorValue = -1.0;

  boolean isNegative = false;
  if (normalizedMotorValue < 0)
  {
    isNegative = true;
    normalizedMotorValue = -normalizedMotorValue;
  }

  float fnX = joystickFnMaxX * normalizedMotorValue;
  float fnY = log((fnX * JOYSTICK_SENSITIVITY) + 1);
  float normalizedFnY = (fnY / JOYSTICK_FN_MAX_Y);
  float joystickValue = normalizedFnY * JOYSTICK_RANGE;

  if (isNegative)
    joystickValue = -joystickValue;

  if (joystickValue > joystickHigh)
    joystickValue = joystickHigh;
  else if (joystickValue < joystickLow)
    joystickValue = joystickLow;

  return joystickValue;
}

