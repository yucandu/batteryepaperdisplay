#include <GxEPD2_BW.h>
#include<ADS1115_WE.h> 
#include "Adafruit_SHT31.h"
#include<Wire.h>
#define I2C_ADDRESS 0x48

#include "driver/periph_ctrl.h"

Adafruit_SHT31 sht31 = Adafruit_SHT31();


ADS1115_WE adc = ADS1115_WE(I2C_ADDRESS); /* Use this for the 16-bit version */
// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 1

#define sleeptimeSecs 60
#define maxArray 1500
#define controlpin 10
#define controlpin2 0
RTC_DATA_ATTR float volts0[maxArray];

  float t, h;

 RTC_DATA_ATTR   int firstrun = 100;
RTC_DATA_ATTR float minVal = 3.9;
RTC_DATA_ATTR float maxVal = 4.2;
RTC_DATA_ATTR int readingCount = 0; // Counter for the number of readings

#include "bitmaps/Bitmaps128x250.h"
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/Roboto_Condensed_12.h>
#include <Fonts/FreeSerif12pt7b.h> 
#include <Fonts/Open_Sans_Condensed_Bold_54.h> 
#include <Fonts/DejaVu_Serif_Condensed_36.h>
#include <Fonts/DejaVu_Serif_Condensed_60.h>

GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(GxEPD2_213_BN(/*CS=5*/ SS, /*DC=*/ 1, /*RES=*/ 2, /*BUSY=*/ 3)); // DEPG0213BN 122x250, SSD1680



float newVal;

void gotosleep() {
      //WiFi.disconnect();
      display.hibernate();
      SPI.end();
      Wire.end();
      pinMode(SS, INPUT_PULLUP );
      pinMode(6, INPUT_PULLUP );
      pinMode(4, INPUT_PULLUP );
      pinMode(8, INPUT_PULLUP );
      pinMode(9, INPUT_PULLUP );
      pinMode(1, INPUT_PULLUP );
      pinMode(2, INPUT_PULLUP );
      pinMode(3, INPUT_PULLUP );
      pinMode(controlpin, INPUT);
      pinMode(controlpin2, INPUT);
      //delay(10000);
      //rtc_gpio_isolate(gpio_num_t(SDA));
      //rtc_gpio_isolate(gpio_num_t(SCL));
      //periph_module_disable(PERIPH_I2C0_MODULE);  
      //digitalWrite(SDA, 0);
      //digitalWrite(SCL, 0);
      esp_sleep_enable_timer_wakeup(sleeptimeSecs * 1000000ULL);
      delay(1);
      esp_deep_sleep_start();
      //esp_light_sleep_start();
      delay(1000);
}



void doDisplay() {
     //newVal = ads.computeVolts(ads.readADC_SingleEnded(0)) * 2.0;
    
    // Shift the previous data points
    for (int i = 0; i < (maxArray - 1); i++) {
        volts0[i] = volts0[i + 1];
    }
    volts0[(maxArray - 1)] = t;

    // Increase the reading count up to maxArray
    if (readingCount < maxArray) {
        readingCount++;
    }

    // Recalculate min and max values
    float minVal = volts0[maxArray - readingCount];
    float maxVal = volts0[maxArray - readingCount];

    for (int i = maxArray - readingCount + 1; i < maxArray; i++) {
        if ((volts0[i] < minVal) && (volts0[i] > 0)) {
            minVal = volts0[i];
        }
        if (volts0[i] > maxVal) {
            maxVal = volts0[i];
        }
    }

    // Calculate scaling factors
    float yScale = 121.0 / (maxVal - minVal);
    float xStep = 250.0 / (readingCount - 1);

    // Draw the line chart
   // display.firstPage();
  //  do {
   // display.fillScreen(GxEPD_WHITE);
  //  } while (display.nextPage());
    display.setPartialWindow(0, 0, display.width(), display.height());

    display.firstPage();
    do {
      display.fillRect(0,0,display.width(),display.height(),GxEPD_BLACK);
    } while (display.nextPage());
    delay(10);
    display.firstPage();
    do {
      display.fillRect(0,0,display.width(),display.height(),GxEPD_WHITE);
    } while (display.nextPage());
    
    
    display.firstPage();
    do {
        display.fillRect(0,0,display.width(),display.height(),GxEPD_WHITE);
        display.setCursor(0, 9);
        display.print(maxVal, 3);
        display.setCursor(0, 122);
        display.print(minVal, 3);
        display.setCursor(150, 122);
        display.print(">");
        display.print(newVal, 4);
        display.print("v, ");
        display.print(t, 3);
        display.print("c<");
        display.setCursor(125, 9);
        display.print("#");
        display.print(readingCount);
        
        for (int i = maxArray - readingCount; i < (maxArray - 1); i++) {
            int x0 = (i - (maxArray - readingCount)) * xStep;
            int y0 = 121 - ((volts0[i] - minVal) * yScale);
            int x1 = (i + 1 - (maxArray - readingCount)) * xStep;
            int y1 = 121 - ((volts0[i + 1] - minVal) * yScale);
            if (volts0[i] > 0) {
                display.drawLine(x0, y0, x1, y1, GxEPD_BLACK);
            }
        }
    } while (display.nextPage());

    display.setFullWindow();
}

float readChannel(ADS1115_MUX channel) {
  float voltage = 0.0;
  adc.setCompareChannels(channel);
  adc.startSingleMeasurement();
  while(adc.isBusy()){}
  voltage = adc.getResult_V(); // alternative: getResult_mV for Millivolt
  return voltage;
}


void setup()
{
  pinMode(controlpin, OUTPUT);
  pinMode(controlpin2, OUTPUT);
  digitalWrite(controlpin, HIGH);
  digitalWrite(controlpin2, HIGH);
  delay(10);
  Wire.begin();  
  sht31.begin(0x44);
  adc.init();
  adc.setVoltageRange_mV(ADS1115_RANGE_4096);
   //newVal = analogReadMilliVolts(0) / 500.0;
  newVal = readChannel(ADS1115_COMP_0_GND) * 2.0;
   t = sht31.readTemperature();
   h = sht31.readHumidity();
  delay(10);
  display.init(115200, false, 10, false); // void init(uint32_t serial_diag_bitrate, bool initial, uint16_t reset_duration = 10, bool pulldown_rst_mode = false)
        display.setRotation(1);
            display.setFont(&Roboto_Condensed_12);
            if (firstrun >= 100) {display.clearScreen();
            firstrun = 0;}
  display.setTextColor(GxEPD_BLACK);
  firstrun++;
  doDisplay();
  gotosleep();

}

void loop()
{
    //newVal = analogRead(0);

    doDisplay();
    gotosleep();
    // Add a delay to sample data at intervals (e.g., every minute)
    //delay(1000); // 1 minute delay, adjust as needed
}
