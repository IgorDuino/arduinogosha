#include <Arduino.h>
#include "GyverMotor.h"
#include <microLED.h>

#define LED_PIN 12

#define HOLL_1_1_PIN 38
#define HOLL_1_2_PIN 39
#define HOLL_2_1_PIN 40
#define HOLL_2_2_PIN 41
#define HOLL_3_1_PIN 42
#define HOLL_3_2_PIN 43
#define HOLL_4_1_PIN 44
#define HOLL_4_2_PIN 45

#define HOLL_LEFT_1_PIN 24
#define HOLL_LEFT_2_PIN 25
#define HOLL_RIGHT_1_PIN 26
#define HOLL_RIGHT_2_PIN 27

#define LEFT_RIGHT_POT_PIN A0
#define BATTERY_PIN A3

String strData = "";
boolean recievedFlag;

long int timer_led;
long int timer_check_battery;

int battery_check_period = 10000;

bool rainbow = 0;

int r = 255;
int b = 0;
int g = 0;

int holl_1_1, holl_1_2, holl_2_1, holl_2_2, holl_3_1, holl_3_2, holl_4_1, holl_4_2;
int holl_left_1, holl_left_2, holl_right_1, holl_right_2;
int battery_val;
float right_ratio = 0.6648;

float body_x, body_y, head_x, head_y;

GMotor motorL(DRIVER3WIRE, 2, 3, 4, LOW);
GMotor motorR(DRIVER3WIRE, 5, 6, 7, HIGH);

GMotor motorHead1(DRIVER3WIRE, 30, 31, 8, HIGH);
GMotor motorHead2(DRIVER3WIRE, 32, 33, 9, HIGH);
GMotor motorHead3(DRIVER3WIRE, 34, 35, 10, HIGH);
GMotor motorHead4(DRIVER3WIRE, 36, 37, 11, HIGH);

microLED<1, LED_PIN, MLED_NO_CLOCK, LED_WS2818, ORDER_GRB, CLI_AVER> led;

void set_led(int rq, int gq, int bq)
{
  led.set(0, mRGB(rq, gq, bq));
  led.show();
}

void rainbow_fn()
{
  static unsigned long timestamp = millis();
  static int e = 1;

  if (millis() - timestamp > 5)
  {
    timestamp = millis();

    if (e <= 255)
    {
      r++;
      b--;
    }
    if (e > 255 and e <= 510)
    {
      b++;
      g--;
    }
    if (e > 510)
    {
      g++;
      r--;
    }

    set_led(r, g, b);

    e++;
    if (e > 765)
      e = 1;
  }
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex and found <= index; i++)
  {
    if (data.charAt(i) == separator or i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void head_left_right(int val)
{
  if (val == 0)
  {
    motorHead1.setMode(STOP);
    motorHead3.setMode(STOP);
  }
  else
  {
    motorHead1.setMode(AUTO);
    motorHead3.setMode(AUTO);

    motorHead1.setSpeed(val / 2);
    motorHead3.setSpeed(val / -2);
  }
}

void head_up_down(int val)
{
  if (val == 0)
  {
    motorHead2.setMode(STOP);
  }
  else
  {
    motorHead2.setMode(AUTO);
    motorHead2.setSpeed(val / -2);
  }
}

void head_forward_back(int val)
{
  if (val == 0)
  {
    motorHead1.setMode(STOP);
    motorHead4.setMode(STOP);
  }
  else
  {
    motorHead1.setMode(AUTO);
    motorHead4.setMode(AUTO);

    motorHead1.setSpeed(val / 2);
    motorHead4.setSpeed(val / -2);
  }
}

void move_head(int val)
{
  if (val == 0)
  {
    motorHead1.setMode(STOP);
    motorHead4.setMode(STOP);

    motorHead2.setMode(STOP);
  }
  else
  {
    motorHead1.setMode(AUTO);
    motorHead4.setMode(AUTO);

    motorHead2.setMode(AUTO);

    motorHead1.setSpeed(val / 3);
    motorHead4.setSpeed(val / -3);

    motorHead2.setSpeed(val / -2);
  }
}

void read_holls()
{
  holl_1_1 = digitalRead(HOLL_1_1_PIN);
  holl_1_2 = digitalRead(HOLL_1_2_PIN);
  holl_2_1 = digitalRead(HOLL_2_1_PIN);
  holl_2_2 = digitalRead(HOLL_2_2_PIN);
  holl_3_1 = digitalRead(HOLL_3_1_PIN);
  holl_3_2 = digitalRead(HOLL_3_2_PIN);
  holl_4_1 = digitalRead(HOLL_4_1_PIN);
  holl_4_2 = digitalRead(HOLL_4_2_PIN);

  holl_left_1 = digitalRead(HOLL_LEFT_1_PIN);
  holl_left_2 = digitalRead(HOLL_LEFT_2_PIN);

  holl_right_1 = digitalRead(HOLL_RIGHT_1_PIN);
  holl_right_2 = digitalRead(HOLL_RIGHT_2_PIN);
}

void setup()
{
  Serial.begin(9600);

  pinMode(HOLL_1_1_PIN, INPUT);
  pinMode(HOLL_1_2_PIN, INPUT);
  pinMode(HOLL_2_1_PIN, INPUT);
  pinMode(HOLL_2_2_PIN, INPUT);
  pinMode(HOLL_3_1_PIN, INPUT);
  pinMode(HOLL_3_2_PIN, INPUT);
  pinMode(HOLL_4_1_PIN, INPUT);
  pinMode(HOLL_4_2_PIN, INPUT);

  pinMode(HOLL_RIGHT_1_PIN, INPUT);
  pinMode(HOLL_RIGHT_1_PIN, INPUT);
  pinMode(HOLL_LEFT_1_PIN, INPUT);
  pinMode(HOLL_LEFT_2_PIN, INPUT);

  pinMode(BATTERY_PIN, INPUT);
  pinMode(LEFT_RIGHT_POT_PIN, INPUT);

  led.setBrightness(255);

  motorL.setMode(AUTO);
  motorR.setMode(AUTO);

  motorHead1.setMode(AUTO);
  motorHead2.setMode(AUTO);
  motorHead3.setMode(AUTO);
  motorHead4.setMode(AUTO);

  motorL.setMinDuty(21);
  motorR.setMinDuty(21);

  Serial.println("Ready");

  set_led(255, 0, 0);
}

void loop()
{
  if (rainbow)
  {
    rainbow_fn();
  }

  if (millis() - timer_check_battery >= battery_check_period)
  {
    battery_val = analogRead(BATTERY_PIN);
    // battery_val = map(battery_val, 0, 1023, 0, 100);

    Serial.print("{\"to\": \"admin\", \"type\":\"bat\", \"data\": \"");
    Serial.print(battery_val);
    Serial.println("\"}");

    timer_check_battery = millis();
  }

  while (Serial.available() > 0)
  {
    strData += (char)Serial.read();
    recievedFlag = true;
    delay(2);
  }
  if (recievedFlag)
  {
    if (strData != "")
    {
      String state = getValue(strData, ';', 0);
      if (state == "m" or state == "mh")
      {
        read_holls();

        String holl_data_str = "";
        holl_data_str += String(holl_1_1) + "  " + String(holl_1_2) + " and ";
        holl_data_str += String(holl_2_1) + "  " + String(holl_2_2) + " and ";
        holl_data_str += String(holl_3_1) + "  " + String(holl_3_2) + " and ";
        holl_data_str += String(holl_4_1) + "  " + String(holl_4_2);

        Serial.print("{\"to\": \"admin\", \"type\":\"holl\", \"data\": \"");
        Serial.print(holl_data_str);
        Serial.println("\"}");

        body_x = getValue(strData, ';', 1).toFloat();
        body_y = getValue(strData, ';', 2).toFloat();

        body_y = body_y * 100;
        body_x = body_x * -100;

        body_x = map(body_x, -100, 100, -255, 255);
        body_y = map(body_y, -100, 100, -255, 255);

        int dutyL = body_y - body_x;
        int dutyR = body_y + body_x;

        motorL.setSpeed(dutyL * 0.35);
        motorR.setSpeed(dutyR * 0.35 * right_ratio);

        head_x = getValue(strData, ';', 3).toFloat();
        head_y = getValue(strData, ';', 4).toFloat();

        head_y = head_y * 100;
        head_x = head_x * -100;

        head_x = map(head_x, -100, 100, -255, 255);
        head_y = map(head_y, -100, 100, -255, 255);

        head_left_right(head_x);
      }
      else if (state == "l")
      {
        int ledRint = getValue(strData, ';', 1).toInt();
        int ledGint = getValue(strData, ';', 2).toInt();
        int ledBint = getValue(strData, ';', 3).toInt();

        if (ledRint == 666 and ledGint == 666 and ledBint == 666)
        {
          rainbow = true;
        }
        else
        {
          rainbow = false;
          set_led(ledRint, ledGint, ledBint);
        }
      }
      else if (state == "set")
      {
        String var_name = getValue(strData, ';', 1);
        if (var_name == "right_ratio")
        {
          right_ratio = getValue(strData, ';', 2).toFloat();
        }
      }
      if (state == "mh")
      {
        head_forward_back(head_y);
      }
      else if (state == "m")
      {
        head_up_down(head_y);
      }
    }
    strData = "";
    recievedFlag = false;
  }
}