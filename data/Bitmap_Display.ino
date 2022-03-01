#include <MCUFRIEND_kbv.h>

MCUFRIEND_kbv tft;

extern const uint16_t PROGMEM t030rs[1665];

#define BLACK 0x0000
#define WHITE 0xFFFF
#define RED   0xF800
#define BLUE  0x001F

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
    uint16_t ID = tft.readID();
    Serial.println(ID, HEX);
    tft.begin(ID);
    tft.fillScreen(WHITE);
    //tft.drawRGBBitmap(10, 10, t030rs, 45, 37);
    tft.setRotation(1);
}

void loop() 
{
    
    tft.drawRGBBitmap(10, 10, t030rs, 45, 37);
    delay(2000);
    tft.fillScreen(WHITE);
    tft.drawRGBBitmap(100, 100, t030rs, 45, 37);
    delay(2000);
    tft.fillScreen(WHITE);
    tft.drawRGBBitmap(200, 200, t030rs, 45, 37);
    delay(2000);
    tft.fillScreen(WHITE);
}
