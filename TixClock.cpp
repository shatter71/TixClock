#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <time.h>
#include "FastLED.h"
#include <Button.h>

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    2
#define LED_TYPE    WS2812B
#define COLOR_ORDER RGB
#define NUM_LEDS    27
//#define BRIGHTNESS  255
#define NIGHT_MODE        1     //Set night mode where 0 is off and 1 is on (Night mode turns the clock off at night)
#define WEEKEND_MODE      0     //Set weekend mode where 0 is off and 1 is on (Weekend mode turns the clock off on the weekends)
#define NIGHT_OFF_TIME    22    //Time that clock turns off when night mode is active
#define NIGHT_ON_TIME     7     //Time that clock turns on when night mode is active
#define MILITARY_MODE     0     //Set 24HR mode where hour is shown as 1-23 rather than 1-12 where 0 = 1-12 mode and 1 = 1-23 mode
int NIGHT_CHK = 0;              //Night model toggle
int WEEKEND_CHK = 0;            //Weekend mode toggle
#define PIN_BUTTON_HR1 14      //Set to GPIO PIN 14 
#define PIN_BUTTON_HR2 12       //Set to GPIO PIN 12 
#define PIN_BUTTON_MN1 13       //Set to GPIO PIN 13
#define PIN_BUTTON_MN2 15       //Set to GPIO PIN 15
//#define PIN_BUTTON_BRIGHT 5     //Set to GPIO PIN 5
#define DST             1       //DST adjustment toggle (set 0 if you don't want auto adjust, 1 if you do)

int HR1_COLOR = 1;  //red
int HR2_COLOR = 5;   //green
int MN1_COLOR = 4;   //blue
int MN2_COLOR = 6;   //purple
int COLOR_CHK = 0;
int DST_MODE = 0;
int BRIGHTNESS = 255;
int TIME_CHK = 0;
int TIME_CHK_DST = 0;
int SHFL_CHK = 0;
int hr10[3] = {0, 0, 0};
int hr11[3] = {1, 0, 0};
int hr12[3] = {1, 1, 0};
int hr20[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int hr21[10] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int hr22[10] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
int hr23[10] = {1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
int hr24[10] = {1, 1, 1, 1, 0, 0, 0, 0, 0, 0};
int hr25[10] = {1, 1, 1, 1, 1, 0, 0, 0, 0, 0};
int hr26[10] = {1, 1, 1, 1, 1, 1, 0, 0, 0, 0};
int hr27[10] = {1, 1, 1, 1, 1, 1, 1, 0, 0, 0};
int hr28[10] = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0};
int hr29[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 0};
int mn10[6] = {0, 0, 0, 0, 0, 0};
int mn11[6] = {1, 0, 0, 0, 0, 0};
int mn12[6] = {1, 1, 0, 0, 0, 0};
int mn13[6] = {1, 1, 1, 0, 0, 0};
int mn14[6] = {1, 1, 1, 1, 0, 0};
int mn15[6] = {1, 1, 1, 1, 1, 0};
int mn20[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int mn21[10] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int mn22[10] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
int mn23[10] = {1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
int mn24[10] = {1, 1, 1, 1, 0, 0, 0, 0, 0, 0};
int mn25[10] = {1, 1, 1, 1, 1, 0, 0, 0, 0, 0};
int mn26[10] = {1, 1, 1, 1, 1, 1, 0, 0, 0, 0};
int mn27[10] = {1, 1, 1, 1, 1, 1, 1, 0, 0, 0};
int mn28[10] = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0};
int mn29[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 0};
int c = 0;

Button BUTTON_HR1 = Button(PIN_BUTTON_HR1,BUTTON_PULLUP_INTERNAL); //Setup button to adjust color of hour tens place LEDs
Button BUTTON_HR2 = Button(PIN_BUTTON_HR2,BUTTON_PULLUP_INTERNAL); //Setup button to adjust color of hour ones place LEDs
Button BUTTON_MN1 = Button(PIN_BUTTON_MN1,BUTTON_PULLUP_INTERNAL); //Setup button to adjust color of minute tens place LEDs
Button BUTTON_MN2 = Button(PIN_BUTTON_MN2,BUTTON_PULLUP_INTERNAL); //Setup button to adjust color of minute ones place LEDs

CRGB leds[NUM_LEDS];

//Default HOUR, MIN, SEC color values in RGB when powering up
int hr1r = 255;
int hr1g = 0;
int hr1b = 0;
int hr2r = 0;
int hr2g = 255;
int hr2b = 0;
int mn1r = 0;
int mn1g = 0;
int mn1b = 255;
int mn2r = 255;
int mn2g = 0;
int mn2b = 255;

const char* ESP_HOST_NAME = "esp-" + ESP.getFlashChipId();
//Your Wifi info
const char* ssid    = "SSID";
const char* password = "PASSWORD";

//Your time zone
int timezone = -8 * 3600; //UTC offset * 3600
int dst = 0;
   
WiFiClient wifiClient;

void connectWifi() 
{
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  connectWifi();
  Serial.println();
  Serial.println("\n\nNext Loop-Step: " + String(millis()) + ":");
  configTime(timezone, 0, "pool.ntp.org","time.nist.gov");
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS)
    .setCorrection(TypicalLEDStrip)
    .setDither(BRIGHTNESS < 255);
  FastLED.setBrightness(BRIGHTNESS);
}

/* Arrange the N elements of ARRAY in random order.
   Only effective if N is much smaller than RAND_MAX;
   if this may not be the case, use a better random
   number generator. */
void shuffle(int *array, size_t n)
{
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

/*------------------------------------Hour LEDs------------------------------------*/
void hour0()
{
    for (int c = 0; 2; c++)
    {
    leds[hr10[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr20[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour1()
{
    for (int c = 0; 2; c++)
    {
    leds[hr10[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (c = 0; 9; c++)
    {
    leds[hr21[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour2()
{
    for (int c = 0; 2; c++)
    {
    leds[hr10[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (c = 0; 9; c++)
    {
    leds[hr22[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour3()
{
    for (int c = 0; 2; c++)
    {
    leds[hr10[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr23[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour4()
{
    for (int c = 0; 2; c++)
    {
    leds[hr10[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr24[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour5()
{
    for (int c = 0; 2; c++)
    {
    leds[hr10[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr25[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour6()
{
    for (int c = 0; 2; c++)
    {
    leds[hr10[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr26[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour7()
{
    for (int c = 0; 2; c++)
    {
    leds[hr10[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr27[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour8()
{
    for (int c = 0; 2; c++)
    {
    leds[hr10[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr28[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour9()
{
    for (int c = 0; 2; c++)
    {
    leds[hr10[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr29[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour10()
{
    for (int c = 0; 2; c++)
    {
    leds[hr11[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr20[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour11()
{
    for (int c = 0; 2; c++)
    {
    leds[hr11[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr21[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour12()
{
    for (int c = 0; 2; c++)
    {
    leds[hr11[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr22[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour13()
{
    for (int c = 0; 2; c++)
    {
    leds[hr11[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr23[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour14()
{
    for (int c = 0; 2; c++)
    {
    leds[hr11[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr24[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour15()
{
    for (int c = 0; 2; c++)
    {
    leds[hr11[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr25[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour16()
{
    for (int c = 0; 2; c++)
    {
    leds[hr11[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr26[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour17()
{
    for (int c = 0; 2; c++)
    {
    leds[hr11[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr27[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour18()
{
    for (int c = 0; 2; c++)
    {
    leds[hr11[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr28[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour19()
{
    for (int c = 0; 2; c++)
    {
    leds[hr11[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr29[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour20()
{
    for (int c = 0; 2; c++)
    {
    leds[hr12[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr20[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour21()
{
    for (int c = 0; 2; c++)
    {
    leds[hr12[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr21[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour22()
{
    for (int c = 0; 2; c++)
    {
    leds[hr12[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr22[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}
void hour23()
{
    for (int c = 0; 2; c++)
    {
    leds[hr12[c]] = CRGB(hr1r,hr1g,hr1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[hr23[c]] = CRGB(hr2r,hr2g,hr2b);
    }
}

/*----------------------------Minutes LEDs----------------------------*/
void min0()
{
    for (int c = 0; 2; c++)
    {
    leds[mn10[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn20[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min1()
{
    for (int c = 0; 2; c++)
    {
    leds[mn10[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn20[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min2()
{
    for (int c = 0; 2; c++)
    {
    leds[mn10[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn22[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min3()
{
    for (int c = 0; 2; c++)
    {
    leds[mn10[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn23[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min4()
{
    for (int c = 0; 2; c++)
    {
    leds[mn10[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn24[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min5()
{
    for (int c = 0; 2; c++)
    {
    leds[mn10[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn25[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min6()
{
    for (int c = 0; 2; c++)
    {
    leds[mn10[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn26[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min7()
{
    for (int c = 0; 2; c++)
    {
    leds[mn10[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn27[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min8()
{
    for (int c = 0; 2; c++)
    {
    leds[mn10[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn28[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min9()
{
    for (int c = 0; 2; c++)
    {
    leds[mn10[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn29[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min10()
{
    for (int c = 0; 2; c++)
    {
    leds[mn11[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn20[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min11()
{
    for (int c = 0; 2; c++)
    {
    leds[mn11[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn21[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min12()
{
    for (int c = 0; 2; c++)
    {
    leds[mn11[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn22[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min13()
{
    for (int c = 0; 2; c++)
    {
    leds[mn11[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn23[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min14()
{
    for (int c = 0; 2; c++)
    {
    leds[mn11[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn24[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min15()
{
    for (int c = 0; 2; c++)
    {
    leds[mn11[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn25[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min16()
{
    for (int c = 0; 2; c++)
    {
    leds[mn11[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn26[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min17()
{
    for (int c = 0; 2; c++)
    {
    leds[mn11[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn27[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min18()
{
    for (int c = 0; 2; c++)
    {
    leds[mn11[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn28[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min19()
{
    for (int c = 0; 2; c++)
    {
    leds[mn11[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn29[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min20()
{
    for (int c = 0; 2; c++)
    {
    leds[mn12[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn20[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min21()
{
    for (int c = 0; 2; c++)
    {
    leds[mn12[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn21[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min22()
{
    for (int c = 0; 2; c++)
    {
    leds[mn12[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn22[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min23()
{
    for (int c = 0; 2; c++)
    {
    leds[mn12[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn23[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min24()
{
    for (int c = 0; 2; c++)
    {
    leds[mn12[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn24[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min25()
{
    for (int c = 0; 2; c++)
    {
    leds[mn12[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn25[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min26()
{
    for (int c = 0; 2; c++)
    {
    leds[mn12[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn26[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min27()
{
    for (int c = 0; 2; c++)
    {
    leds[mn12[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn27[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min28()
{
    for (int c = 0; 2; c++)
    {
    leds[mn12[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn28[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min29()
{
    for (int c = 0; 2; c++)
    {
    leds[mn12[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn29[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min30()
{
    for (int c = 0; 2; c++)
    {
    leds[mn13[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn20[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min31()
{
    for (int c = 0; 2; c++)
    {
    leds[mn13[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn21[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min32()
{
    for (int c = 0; 2; c++)
    {
    leds[mn13[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn22[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min33()
{
    for (int c = 0; 2; c++)
    {
    leds[mn13[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn23[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min34()
{
    for (int c = 0; 2; c++)
    {
    leds[mn13[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn24[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min35()
{
    for (int c = 0; 2; c++)
    {
    leds[mn13[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn25[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min36()
{
    for (int c = 0; 2; c++)
    {
    leds[mn13[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn26[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min37()
{
    for (int c = 0; 2; c++)
    {
    leds[mn13[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn27[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min38()
{
    for (int c = 0; 2; c++)
    {
    leds[mn13[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn28[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min39()
{
    for (int c = 0; 2; c++)
    {
    leds[mn13[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn29[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min40()
{
    for (int c = 0; 2; c++)
    {
    leds[mn14[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn20[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min41()
{
    for (int c = 0; 2; c++)
    {
    leds[mn14[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn21[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min42()
{
    for (int c = 0; 2; c++)
    {
    leds[mn14[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn22[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min43()
{
    for (int c = 0; 2; c++)
    {
    leds[mn14[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn23[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min44()
{
    for (int c = 0; 2; c++)
    {
    leds[mn14[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn24[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min45()
{
    for (int c = 0; 2; c++)
    {
    leds[mn14[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn25[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min46()
{
    for (int c = 0; 2; c++)
    {
    leds[mn14[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn26[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min47()
{
    for (int c = 0; 2; c++)
    {
    leds[mn14[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn27[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min48()
{
    for (int c = 0; 2; c++)
    {
    leds[mn14[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn28[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min49()
{
    for (int c = 0; 2; c++)
    {
    leds[mn14[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn29[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min50()
{
    for (int c = 0; 2; c++)
    {
    leds[mn15[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn20[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min51()
{
    for (int c = 0; 2; c++)
    {
    leds[mn15[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn21[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min52()
{
    for (int c = 0; 2; c++)
    {
    leds[mn15[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn22[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min53()
{
    for (int c = 0; 2; c++)
    {
    leds[mn15[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn23[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min54()
{
    for (int c = 0; 2; c++)
    {
    leds[mn15[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn24[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min55()
{
    for (int c = 0; 2; c++)
    {
    leds[mn15[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn25[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min56()
{
    for (int c = 0; 2; c++)
    {
    leds[mn15[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn26[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min57()
{
    for (int c = 0; 2; c++)
    {
    leds[mn15[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn27[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min58()
{
    for (int c = 0; 2; c++)
    {
    leds[mn15[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn28[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}
void min59()
{
    for (int c = 0; 2; c++)
    {
    leds[mn15[c]] = CRGB(mn1r,mn1g,mn1b);
    }
    for (int c = 0; 9; c++)
    {
    leds[mn29[c]] = CRGB(mn2r,mn2g,mn2b);
    }
}

void loop()
{
    time_t now = time(nullptr);
    struct tm* p_tm = localtime(&now);
    Serial.print("-------------------------------------------------\n");
    Serial.print("Date & Time : ");
    Serial.print(p_tm->tm_mday);
    Serial.print("/");
    Serial.print(p_tm->tm_mon + 1);
    Serial.print("/");
    Serial.print(p_tm->tm_year + 1900);
    Serial.print(" DST ");
    Serial.print(DST);
    Serial.print(" DST_MODE ");
    Serial.print(DST_MODE);
    Serial.print(" ");
    int hour=p_tm->tm_hour;
    int month=p_tm->tm_mon;
    int day=p_tm->tm_mday;
    int minute=p_tm->tm_min;
    int second=p_tm->tm_sec;
    int weekday=p_tm->tm_wday; //day of the week, range 0 to 6

    if (DST == 1) { 
      if (DST_MODE == 0) {
        hour = hour;
      }
      else if (DST_MODE == 1) {
        hour = hour + 1;
      }
    }

if (DST == 1) {
  if (month + 1 > 3 && month + 1 < 11)
      {
       DST_MODE = 1;
      }
  else if (month + 1 == 3)
      {
       if (day > 14)
           {
            DST_MODE = 1;
           } 
       else if (day < 8)
           {
            DST_MODE = 0; 
           }
       else if (day == 8)
             {
              if (weekday == 1)
                {
                  if (hour > 1)
                   {
                    DST_MODE = 1;
                   }
                }
             }   
       else if (day == 9)
             {
              if (weekday == 1)
                {
                  if (hour > 1)
                   {
                    DST_MODE = 1;
                   }
                }
             }   
        else if (day == 10)
             {
              if (weekday == 1)
                {
                  if (hour > 1)
                   {
                    DST_MODE = 1;
                   }
                }
             }   
        else if (day == 11)
             {
              if (weekday == 1)
                {
                  if (hour > 1)
                   {
                    DST_MODE = 1;
                   }
                }
             }   
        else if (day == 12)
             {
              if (weekday == 1)
                {
                  if (hour > 1)
                   {
                    DST_MODE = 1;
                   }
                }
             }   
        else if (day == 13)
             {
              if (weekday == 1)
                {
                  if (hour > 1)
                   {
                    DST_MODE = 1;
                   }
                }
             }  
        else if (day == 14)
             {
              if (weekday == 1)
                {
                  if (hour > 1)
                   {
                    DST_MODE = 1;
                   }                 
                }
             }   
         else
             {
              DST_MODE = 0;
             }
      } 
  else if (month + 1 == 11)
      {
       if (day > 6)
           {
            DST_MODE = 0;
           } 
       else if (day == 1)
             {
              if (weekday == 1)
                {
                  if (hour > 1)
                   {
                    DST_MODE = 0;
                   }
                }
             }   
       else if (day == 2)
             {
              if (weekday == 1)
                {
                  if (hour > 1)
                   {
                    DST_MODE = 0;
                   }
                }
             }   
        else if (day == 3)
             {
              if (weekday == 1)
                {
                  if (hour > 1)
                   {
                    DST_MODE = 0;
                   }
                }
             }   
        else if (day == 4)
             {
              if (weekday == 1)
                {
                  if (hour > 1)
                   {
                    DST_MODE = 0;
                   }
                }
             }   
        else if (day == 5)
             {
              if (weekday == 1)
                {
                  if (hour > 1)
                   {
                    DST_MODE = 0;
                   }
                }
             }   
        else if (day == 6)
             {
              if (weekday == 1)
                {
                  if (hour > 1)
                   {
                    DST_MODE = 0;
                   }
                }
             }  
        else if (day == 7)
             {
              if (weekday == 1)
                {
                  if (hour > 1)
                   {
                    DST_MODE = 0;
                   }                 
                }
             }   
         else
             {
              DST_MODE = 1;
             }
      }
}
    Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.println(p_tm->tm_sec);

    // Adjust brightness on key press
    //if (BUTTON_DST.heldFor(100) && now > (TIME_CHK + 7)) {
    //if (BUTTON_BRIGHT.uniquePress()) {
    //  //Serial.println("Brightness adjusted!");
    //  BRIGHTNESS = BRIGHTNESS + 64;
    //  if (BRIGHTNESS > 255) {
    //    BRIGHTNESS = 0;
    //  }
    //  FastLED.setBrightness(BRIGHTNESS);
    //  TIME_CHK = now;
    //}

    // Check if we pressed the button to adjust the hour color...
    // Repeat the following loop for as long as we hold this button.
    if (BUTTON_HR1.uniquePress()) {
      //Serial.println(HR1_COLOR);
      HR1_COLOR = HR1_COLOR + 1;
      COLOR_CHK = 1;
      if (HR1_COLOR > 6) {
        HR1_COLOR = 1;
      }
    }
    
    if (BUTTON_HR2.uniquePress()) {
      //Serial.println(HR2_COLOR);
      HR2_COLOR = HR2_COLOR + 1;
      COLOR_CHK = 1;
      if (HR2_COLOR > 6) {
        HR2_COLOR = 1;
      }
    }
    if (BUTTON_MN1.uniquePress()) {
      //Serial.println(MN1_COLOR);
      MN1_COLOR = MN1_COLOR + 1;
      COLOR_CHK = 1;
      if (MN1_COLOR > 6) {
        MN1_COLOR = 1;
      }
    }
    if (BUTTON_MN2.uniquePress()) {
      //Serial.println(MN2_COLOR);
      MN2_COLOR = MN2_COLOR + 1;
      COLOR_CHK = 1;
      if (MN2_COLOR > 6) {
        MN2_COLOR = 1;
      }
    }
    
    if (COLOR_CHK == 1){
      COLOR_CHK = 0;
      if (HR1_COLOR == 1) { //red
        hr1r = 255;
        hr1g = 0;
        hr1b = 0;
        //Serial.println("RED");
      }
      if (HR1_COLOR == 2) { //cyan
        hr1r = 0;
        hr1g = 255;
        hr1b = 255;
        //Serial.println("YELLOW");
      }
      if (HR1_COLOR == 3) { //white
        hr1r = 255;
        hr1g = 255;
        hr1b = 255;
        //Serial.println("WHITE");
      }
      if (HR1_COLOR == 4) { //blue
        hr1r = 0;
        hr1g = 0;
        hr1b = 255;
        //Serial.println("BLUE");
      }
      if (HR1_COLOR == 5) { //green
        hr1r = 0;
        hr1g = 255;
        hr1b = 0;
        //Serial.println("GREEN");
      }
      if (HR1_COLOR == 6) { //purple
        hr1r = 255;
        hr1g = 0;
        hr1b = 255;
        //Serial.println("PURPLE");
      }
      
      if (HR2_COLOR == 1) { //red
        hr2r = 255;
        hr2g = 0;
        hr2b = 0;
        //Serial.println("RED");
      }
      if (HR2_COLOR == 2) { //cyan
        hr2r = 0;
        hr2g = 255;
        hr2b = 255;
        //Serial.println("YELLOW");
      }
      if (HR2_COLOR == 3) { //white
        hr2r = 255;
        hr2g = 255;
        hr2b = 255;
        //Serial.println("WHITE");
      }
      if (HR2_COLOR == 4) { //blue
        hr2r = 0;
        hr2g = 0;
        hr2b = 255;
        //Serial.println("BLUE");
      }
      if (HR2_COLOR == 5) { //green
        hr2r = 0;
        hr2g = 255;
        hr2b = 0;
        //Serial.println("GREEN");
      }
      if (HR2_COLOR == 6) { //purple
        hr2r = 255;
        hr2g = 0;
        hr2b = 255;
        //Serial.println("PURPLE");
      }
      if (MN1_COLOR == 1) { //red
        mn1r = 255;
        mn1g = 0;
        mn1b = 0;
        //Serial.println("RED");
      }
      if (MN1_COLOR == 2) { //cyan
        mn1r = 0;
        mn1g = 255;
        mn1b = 255;
        //Serial.println("YELLOW");
      }
      if (MN1_COLOR == 3) { //white
        mn1r = 255;
        mn1g = 255;
        mn1b = 255;
        //Serial.println("WHITE");
      }
      if (MN1_COLOR == 4) { //blue
        mn1r = 0;
        mn1g = 0;
        mn1b = 255;
        //Serial.println("BLUE");
      }
      if (MN1_COLOR == 5) { //green
        mn1r = 0;
        mn1g = 255;
        mn1b = 0;
        //Serial.println("GREEN");
      }
      if (MN1_COLOR == 6) { //purple
        mn1r = 255;
        mn1g = 0;
        mn1b = 255;
        //Serial.println("PURPLE");
      }
      if (MN2_COLOR == 1) { //red
        mn2r = 255;
        mn2g = 0;
        mn2b = 0;
        //Serial.println("RED");
      }
      if (MN2_COLOR == 2) { //cyan
        mn2r = 0;
        mn2g = 255;
        mn2b = 255;
        //Serial.println("YELLOW");
      }
      if (MN2_COLOR == 3) { //white
        mn2r = 255;
        mn2g = 255;
        mn2b = 255;
        //Serial.println("WHITE");
      }
      if (MN2_COLOR == 4) { //blue
        mn2r = 0;
        mn2g = 0;
        mn2b = 255;
        //Serial.println("BLUE");
      }
      if (MN2_COLOR == 5) { //green
        mn2r = 0;
        mn2g = 255;
        mn2b = 0;
        //Serial.println("GREEN");
      }
      if (MN2_COLOR == 6) { //purple
        mn2r = 255;
        mn2g = 0;
        mn2b = 255;
        //Serial.println("PURPLE");
      }
            
    }

    if (NIGHT_MODE==1)
    {
      if (hour==NIGHT_OFF_TIME && NIGHT_CHK==0)
      {
        NIGHT_CHK = 1;
      }
      if (hour==NIGHT_ON_TIME && NIGHT_CHK==1)
      {
        NIGHT_CHK = 0;
      }
    }

    if (WEEKEND_MODE == 1)
    {
      if (weekday == 0 || weekday == 6)
      {
        WEEKEND_CHK = 1;
      }
      if (weekday != 0 && weekday != 6)
      {
        WEEKEND_CHK = 0;
      }
    }

    if (NIGHT_CHK == 1 || WEEKEND_CHK == 1)
    {
      FastLED.setBrightness(0);
    }
    else
    {
      FastLED.setBrightness(BRIGHTNESS);
    }

    if (NIGHT_CHK == 0 && WEEKEND_CHK == 0)
    {
    if (MILITARY_MODE == 0)
    {
      if(hour>12)
      {
        hour=hour-12;
      }
    }

    // Update the LED layout every four seconds
    //if((SHFL_CHK==0) && (second==0) || (second==4) || (second==8) || (second==12) || (second==16) || (second==20) || (second==24) || (second==28) || (second==32) || (second==36) || (second==40) || (second==44) || (second==48) || (second==52) || (second==56))
    if((SHFL_CHK==0) && (second % 4==0))
    {
        if(hour>10 && hour<20){
            shuffle(hr11, 3);
        }
        else if(hour>19){
            shuffle(hr12, 3);
        }
        if(hour==1 || hour==21){
            shuffle(hr21, 9);
        }
        else if(hour==2 || hour==22){
            shuffle(hr22, 9);
        }
        else if(hour==3 || hour==23){
            shuffle(hr23, 9);
        }
        else if(hour==4 || hour==24){
            shuffle(hr24, 9);
        }
        else if(hour==5){
            shuffle(hr25, 9);
        }
        else if(hour==6){
            shuffle(hr26, 9);
        }
        else if(hour==7){
            shuffle(hr27, 9);
        }
        else if(hour==8){
            shuffle(hr28, 9);
        }
        else if(hour==9){
            shuffle(hr29, 9);
        }
        if(minute>9 && minute<20){
            shuffle(mn11, 6);
        }
        else if(minute>19 && minute<30){
            shuffle(mn12, 6);
        }
        else if(minute>29 && minute<40){
            shuffle(mn13, 6);
        }
        else if(minute>39 && minute<50){
            shuffle(mn14, 6);
        }
        else if(minute>49 && minute<60){
            shuffle(mn15, 6);
        }
        if(minute==1 || minute==11 || minute==21 || minute==31 || minute==41 || minute==51){
            shuffle(mn21, 9);
        }
        else if(minute==2 || minute==12 || minute==22 || minute==32 || minute==42 || minute==52){
            shuffle(mn22, 9);
        }
        else if(minute==3 || minute==13 || minute==23 || minute==33 || minute==43 || minute==53){
            shuffle(mn23, 9);
        }
        else if(minute==4 || minute==14 || minute==24 || minute==34 || minute==44 || minute==54){
            shuffle(mn24, 9);
        }
        else if(minute==5 || minute==15 || minute==25 || minute==35 || minute==45 || minute==55){
            shuffle(mn25, 9);
        }
        else if(minute==6 || minute==16 || minute==26 || minute==36 || minute==46 || minute==56){
            shuffle(mn26, 9);
        }
        else if(minute==7 || minute==17 || minute==27 || minute==37 || minute==47 || minute==57){
            shuffle(mn27, 9);
        }
        else if(minute==8 || minute==18 || minute==28 || minute==38 || minute==48 || minute==58){
            shuffle(mn28, 9);
        }
        else if(minute==9 || minute==19 || minute==29 || minute==39 || minute==49 || minute==59){
            shuffle(mn29, 9);
        }
        SHFL_CHK = 1;
    }

    if((SHFL_CHK==1) && (second==1) || (second==5) || (second==9) || (second==13) || (second==17) || (second==21) || (second==25) || (second==29) || (second==33) || (second==37) || (second==41) || (second==45) || (second==49) || (second==53) || (second==57))
    {
      SHFL_CHK = 0;
    }

    if(hour==1)
    {
      hour1();
    }
    if(hour==2)
    {
      hour2();
    }
    if(hour==3)
    {
      hour3();
    }
    if(hour==4)
    {
      hour4();
    }
    if(hour==5)
    {
      hour5();
    }
    if(hour==6)
    {
      hour6();
    }
    if(hour==7)
    {
      hour7();
    }
    if(hour==8)
    {
      hour8();
    }
    if(hour==9)
    {
      hour9();
    }
    if(hour==10)
    {
      hour10();
    }
    if(hour==11)
    {
      hour11();
    }
    if(hour==12)
    {
      hour12();
    }
    if (MILITARY_MODE == 1)
    {
        if(hour==12)
      {
        hour12();
      }
        if(hour==13)
      {
        hour13();
      }
        if(hour==14)
      {
        hour14();
      }
        if(hour==15)
      {
        hour15();
      }
        if(hour==16)
      {
        hour16();
      }
        if(hour==17)
      {
        hour17();
      }
        if(hour==18)
      {
        hour18();
      }
        if(hour==19)
      {
        hour19();
      }
        if(hour==20)
      {
        hour20();
      }
        if(hour==21)
      {
        hour21();
      }
        if(hour==22)
      {
        hour22();
      }
        if(hour==23)
      {
        hour23();
      }
        if(hour==0 || hour==24)
      {
        hour0();
      }
    }

    if(minute==0)
    {
      min0();
    }
    if(minute==1)
    {
      min1();
    }
    if(minute==2)
    {
      min2();
    }
    if(minute==3)
    {
      min3();
    }
    if(minute==4)
    {
      min4();
    }
    if(minute==5)
    {
      min5();
    }
    if(minute==6)
    {
      min6();
    }
    if(minute==7)
    {
      min7();
    }
    if(minute==8)
    {
      min8();
    }
    if(minute==9)
    {
      min9();
    }
    if(minute==10)
    {
      min10();
    }
    if(minute==11)
    {
      min11();
    }
    if(minute==12)
    {
      min12();
    }
    if(minute==13)
    {
      min13();
    }
    if(minute==14)
    {
      min14();
    }
    if(minute==15)
    {
      min15();
    }
    if(minute==16)
    {
      min16();
    }
    if(minute==17)
    {
      min17();
    }
    if(minute==18)
    {
      min18();
    }
    if(minute==19)
    {
      min19();
    }
    if(minute==20)
    {
      min20();
    }
    if(minute==21)
    {
      min21();
    }
    if(minute==22)
    {
      min22();
    }
    if(minute==23)
    {
      min23();
    }
    if(minute==24)
    {
      min24();
    }
    if(minute==25)
    {
      min25();
    }
    if(minute==26)
    {
      min26();
    }
    if(minute==27)
    {
      min27();
    }
    if(minute==28)
    {
      min28();
    }
    if(minute==29)
    {
      min29();
    }
    if(minute==30)
    {
      min30();
    }
    if(minute==31)
    {
      min31();
    }
    if(minute==32)
    {
      min32();
    }
    if(minute==33)
    {
      min33();
    }
    if(minute==34)
    {
      min34();
    }
    if(minute==35)
    {
      min35();
    }
    if(minute==36)
    {
      min36();
    }
    if(minute==37)
    {
      min37();
    }
    if(minute==38)
    {
      min38();
    }
    if(minute==39)
    {
      min39();
    }
    if(minute==40)
    {
      min40();
    }
    if(minute==41)
    {
      min41();
    }
    if(minute==42)
    {
      min42();
    }
    if(minute==43)
    {
      min43();
    }
    if(minute==44)
    {
      min44();
    }
    if(minute==45)
    {
      min45();
    }
    if(minute==46)
    {
      min46();
    }
    if(minute==47)
    {
      min47();
    }
    if(minute==48)
    {
      min48();
    }
    if(minute==49)
    {
      min49();
    }
    if(minute==50)
    {
      min50();
    }
    if(minute==51)
    {
      min51();
    }
    if(minute==52)
    {
      min52();
    }
    if(minute==53)
    {
      min53();
    }
    if(minute==54)
    {
      min54();
    }
    if(minute==55)
    {
      min55();
    }
    if(minute==56)
    {
      min56();
    }
    if(minute==57)
    {
      min57();
    }
    if(minute==58)
    {
      min58();
    }
    if(minute==59)
    {
      min59();
    }
    if(minute==60)
    {
      min0();
    }
 
    FastLED.show();
    FastLED.clear();
    FastLED.show();  
    }
}