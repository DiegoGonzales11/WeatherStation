#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#include <Wire.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>

#include <ThreeWire.h>
#include <RtcDS1302.h>

#include "Image.h"
#include "enum.h"

#include <Fonts/Custom/DialogInput_bold_8.h>

// refresh values
uint8_t day_refresh = 0;
bool day_count = false;
uint8_t minute_refresh = 0;
bool minute_count = false;
float temp_refresh = 0.0;
bool temp_count = false;
float humidity_refresh = 0.0;
bool hum_count = false;
float altitude_refresh = 0.0;
bool alti_count = false;
float pressure_refresh = 0.0;
bool press_count = false;

void i2c_scanner(void);
void screen_welcome(void);
void screen_time(RtcDateTime &now);
void screen_temp(float temperature);
void screen_humidity(float humidity);
void screen_altitute(float altitude);
void screen_pressure(float pressure);

// tft
TFT_eSPI tft = TFT_eSPI();

// dht
#define DHT_PIN 4
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

// bmp
Adafruit_BMP085 bmp;

// rtc
#define CLK 13
#define DAT 12
#define RST 14

ThreeWire myWire(DAT, CLK, RST); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
#define countof(a) (sizeof(a) / sizeof(a[0]))
void printDateTime(const RtcDateTime &dt);

// led
#define LED_HOT 25
#define LED_COOL 26

// buzzer
#define BUZZER 27
uint8_t clock_sound = 1;

void setup()
{
  Serial.begin(115200);

  // tft init
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  int h = 160, w = 128, row, col, buffidx = 0;
  for (row = 0; row < h; row++)
  {
    for (col = 0; col < w; col++)
    {
      tft.drawPixel(col, row, pgm_read_word(cat + buffidx));
      buffidx++;
    }
  }

  // bmp180
  if (!bmp.begin())
  {
    Serial.println("Could not find a valid BMP085/BMP180 sensor, check wiring!");
    while (1)
    {
    }
  }

  // led
  pinMode(LED_HOT, OUTPUT);
  digitalWrite(LED_HOT, LOW);
  pinMode(LED_COOL, OUTPUT);
  digitalWrite(LED_COOL, LOW);

  // buzzer
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  for (int i = 0; i < 10; i++)
  {
    digitalWrite(BUZZER, LOW);
    delay(50);
    digitalWrite(BUZZER, HIGH);
    delay(50);
  }
  digitalWrite(BUZZER, LOW);

  // sensors init
  dht.begin();

  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.print(" - ");
  Serial.println(__TIME__);

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }

  // clear tft
  delay(1000);

  tft.fillScreen(TFT_BLACK);
  /*
  int time = millis();
  tft.fillRect(0, 0, 128, 38, ST7735_BLUE);
  time = millis() - time;
  Serial.printf("**%d**\n", time);
  */

  // screen
  uint8_t x_offset = 3, y_offset = 110;
  h = y_offset + 40;
  w = x_offset + 44;
  buffidx = 0;
  for (row = y_offset; row < h; row++)
  {
    for (col = x_offset; col < w; col++)
    {
      tft.drawPixel(col, row, pgm_read_word(icon_mountain + buffidx));
      buffidx++;
    }
  }

  RtcDateTime now_ref = Rtc.GetDateTime();
  day_refresh = now_ref.DayOfWeek();
  minute_refresh = now_ref.Minute();
  temp_refresh = dht.readTemperature();
  humidity_refresh = dht.readHumidity();
  altitude_refresh = bmp.readAltitude();
}

void loop()
{

  RtcDateTime now = Rtc.GetDateTime();

  printDateTime(now);
  Serial.print("\n");

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float pressure = bmp.readPressure();
  // float temperature = bmp.readTemperature();
  float altitude = bmp.readAltitude();

  screen_time(now);
  screen_temp(temperature);
  screen_humidity(humidity);
  screen_altitute(altitude);
  screen_pressure(pressure);

  if (temperature >= 24)
  {
    digitalWrite(LED_HOT, HIGH);
    digitalWrite(LED_COOL, LOW);
  }
  if (temperature < 24)
  {
    digitalWrite(LED_HOT, LOW);
    digitalWrite(LED_COOL, HIGH);
  }

  // buzzer bip

  if (now.Hour() > clock_sound)
  {
    clock_sound = now.Hour();
    for (int i = 0; i < 3; i++)
    {
      digitalWrite(BUZZER, LOW);
      delay(50);
      digitalWrite(BUZZER, HIGH);
      delay(50);
    }
    digitalWrite(BUZZER, LOW);
  }
  if (clock_sound > 23)
  {
    clock_sound = 1;
  }
}
void screen_time(RtcDateTime &now)
{

  if (now.DayOfWeek() != day_refresh || day_count == false)
  {
    tft.fillRect(0, 0, 128, 12, TFT_BLACK);
    uint8_t space = 23;
    // tft.setFreeFont(&FreeMono9pt7b);
    tft.setFreeFont(&DialogInput_bold_8);
    tft.setTextSize(1);
    tft.setCursor(space, 10);
    tft.print(days[now.DayOfWeek()]);
    tft.print(",");
    tft.setCursor(space + 23, 10);
    tft.print(now.Day());
    tft.setCursor(space + 38, 10);
    tft.print(months[now.Month()]);
    tft.setCursor(space + 58, 10);
    tft.print(now.Year());
    day_count = true;
  }

  if (now.Minute() != minute_refresh || minute_count == false)
  {
    minute_refresh = now.Minute();
    minute_count = true;

    tft.fillRect(0, 13, 128, 30, TFT_BLACK);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextSize(1);
    uint8_t hour_aux = now.Hour();

    if (hour_aux > 12)
    {
      hour_aux -= 12;
    }

    if (hour_aux == 0)
    {
      hour_aux = 12;
    }

    if (hour_aux < 11)
    {
      tft.setCursor(31, 40);
    }
    else
    {
      tft.setCursor(21, 40);
    }

    tft.print(hour_aux);

    tft.print(":");
    if (now.Minute() < 10)
    {
      tft.print("0");
      tft.print(now.Minute());
    }
    else
    {
      tft.print(now.Minute());
    }
  }

  uint8_t x_offset = 80, y_offset = 115;
  int h = y_offset + 40, w = x_offset + 44, row, col, buffidx = 0;
  if (now.Hour() < 18 && now.Hour() > 5)
  {
    for (row = y_offset; row < h; row++)
    {
      for (col = x_offset; col < w; col++)
      {
        tft.drawPixel(col, row, pgm_read_word(icon_sun + buffidx));
        buffidx++;
      }
    }
  }
  else
  {
    for (row = y_offset; row < h; row++)
    {
      for (col = x_offset; col < w; col++)
      {
        tft.drawPixel(col, row, pgm_read_word(icon_moon + buffidx));
        buffidx++;
      }
    }
  }
}

void screen_temp(float temperature)
{
  uint8_t x_offset = 3, y_offset = 45;
  int h = y_offset + 30, w = x_offset + 33, row, col, buffidx = 0;

  if (temperature != temp_refresh || temp_count == false)
  {
    tft.fillRect(0, 44, 128, 32, TFT_BLACK);

    temp_count = true;
    temp_refresh = temperature;
    // tft.setFont(&Org_01);
    tft.setFreeFont(&FreeSansBold12pt7b);
    tft.setTextSize(1);
    tft.setCursor(37, 68);
    tft.print(temperature, 1);
    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.setCursor(tft.getCursorX() + 3, 60);
    tft.print("o");
    tft.setTextSize(1);
    tft.setCursor(tft.getCursorX(), 68);
    tft.print("C");

    if (temperature < 24)
    {
      for (row = y_offset; row < h; row++)
      {
        for (col = x_offset; col < w; col++)
        {
          tft.drawPixel(col, row, pgm_read_word(icon_temp_cool + buffidx));
          buffidx++;
        }
      }
    }
    else
    {
      for (row = y_offset; row < h; row++)
      {
        for (col = x_offset; col < w; col++)
        {
          tft.drawPixel(col, row, pgm_read_word(icon_temp_hot + buffidx));
          buffidx++;
        }
      }
    }
  }
}
void screen_humidity(float humidity)
{
  uint8_t x_offset = 80, y_offset = 78;
  int h = y_offset + 30, w = x_offset + 33, row, col, buffidx = 0;
  for (row = y_offset; row < h; row++)
  {
    for (col = x_offset; col < w; col++)
    {
      tft.drawPixel(col, row, pgm_read_word(icon_humity + buffidx));
      buffidx++;
    }
  }

  if (humidity != humidity_refresh || hum_count == false)
  {
    hum_count = true;
    humidity_refresh = humidity;

    tft.fillRect(0, 77, 80, 32, TFT_BLACK);
    // tft.setFont(&Org_01);
    tft.setFreeFont(&FreeSansBold12pt7b);
    tft.setTextSize(1);
    tft.setCursor(23, 100);
    tft.print((int)humidity);
    tft.print(" %");
  }
}
void screen_altitute(float altitude)
{

  if ((int)altitude != (int)altitude_refresh || alti_count == false)
  {
    alti_count = true;
    altitude_refresh = altitude;

    tft.fillRect(0, 140, 65, 20, TFT_BLACK);
    // tft.setFont(&Org_01);
    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.setTextSize(1);
    tft.setCursor(10, 155);
    tft.print((int)altitude);
    tft.setFreeFont(&DialogInput_bold_8);
    tft.print(" m");
  }
}
void screen_pressure(float pressure)
{
  if (pressure != pressure_refresh || press_count == false)
  {
    press_count = true;
    pressure_refresh = pressure;

    tft.fillRect(29, 127, 40, 10, TFT_BLACK);
    tft.setFreeFont(&DialogInput_bold_8);
    // tft.setFont(&Org_01);
    tft.setTextSize(1);
    tft.setCursor(31, 135);
    tft.print((int)pressure / 100);
    tft.print(" hPa");    
  }
}

void i2c_scanner(void)
{
  Wire.begin();

  uint8_t error;
  printf("Scannig...\n");
  printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
  printf("00:         ");
  for (int i = 3; i < 0x78; i++)
  {
    Wire.beginTransmission(i);
    error = Wire.endTransmission();

    if (i % 16 == 0)
    {
      printf("\n%.2x:", i);
    }
    if (error == 0)
    {
      printf(" %.2x", i);
    }
    else
    {
      printf(" --");
    }
  }
}

void printDateTime(const RtcDateTime &dt)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second());
  Serial.print(datestring);
}