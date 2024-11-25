#include <GxEPD2_BW.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;
sensors_event_t humidity, temp;
#include<Wire.h>
#define I2C_ADDRESS 0x48
#include "driver/periph_ctrl.h"
int GPIO_reason;
#include "esp_sleep.h"


// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 1

#define sleeptimeSecs 60
#define maxArray 375
#define controlpin 10

RTC_DATA_ATTR float array1[maxArray];
RTC_DATA_ATTR float array2[maxArray];
RTC_DATA_ATTR float array3[maxArray];
RTC_DATA_ATTR float array4[maxArray];

  float t, h, pres;

 RTC_DATA_ATTR   int firstrun = 100;
 RTC_DATA_ATTR   int page = 0;
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
#define BUTTON_PIN_BITMASK(GPIO) (1ULL << GPIO)
GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(GxEPD2_213_BN(/*CS=5*/ SS, /*DC=*/ 21, /*RES=*/ 20, /*BUSY=*/ 3)); // DEPG0213BN 122x250, SSD1680



float vBat;

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
      pinMode(0, INPUT_PULLUP );
      pinMode(controlpin, INPUT);

      //delay(10000);
      //rtc_gpio_isolate(gpio_num_t(SDA));
      //rtc_gpio_isolate(gpio_num_t(SCL));
      //periph_module_disable(PERIPH_I2C0_MODULE);  
      //digitalWrite(SDA, 0);
      //digitalWrite(SCL, 0);
      uint64_t bitmask = BUTTON_PIN_BITMASK(GPIO_NUM_1) | BUTTON_PIN_BITMASK(GPIO_NUM_2) | BUTTON_PIN_BITMASK(GPIO_NUM_3) | BUTTON_PIN_BITMASK(GPIO_NUM_4) | BUTTON_PIN_BITMASK(GPIO_NUM_5);
   //   esp_deep_sleep_enable_gpio_wakeup(1 << 0, ESP_GPIO_WAKEUP_GPIO_LOW);
    //  esp_deep_sleep_enable_gpio_wakeup(1 << 1, ESP_GPIO_WAKEUP_GPIO_LOW);
   //   esp_deep_sleep_enable_gpio_wakeup(1 << 2, ESP_GPIO_WAKEUP_GPIO_LOW);
   //   esp_deep_sleep_enable_gpio_wakeup(1 << 3, ESP_GPIO_WAKEUP_GPIO_LOW);
      esp_deep_sleep_enable_gpio_wakeup(bitmask, ESP_GPIO_WAKEUP_GPIO_LOW);
      esp_sleep_enable_timer_wakeup(sleeptimeSecs * 1000000ULL);
      delay(1);
      esp_deep_sleep_start();
      //esp_light_sleep_start();
      delay(1000);
}

void wipeScreen(){

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
}

double mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void doTempDisplay() {
    // Recalculate min and max values
    float minVal = array1[maxArray - readingCount];
    float maxVal = array1[maxArray - readingCount];

    for (int i = maxArray - readingCount + 1; i < maxArray; i++) {
        if ((array1[i] < minVal) && (array1[i] > 0)) {
            minVal = array1[i];
        }
        if (array1[i] > maxVal) {
            maxVal = array1[i];
        }
    }

    // Calculate scaling factors
    float yScale = 121.0 / (maxVal - minVal);
    float xStep = 250.0 / (readingCount - 1);

    wipeScreen();
    
    
    do {
        display.fillRect(0,0,display.width(),display.height(),GxEPD_WHITE);
        display.setCursor(0, 9);
        display.print(maxVal, 3);
        display.setCursor(0, 122);
        display.print(minVal, 3);
        display.setCursor(150, 122);
        display.print("vBat: ");
        display.print(vBat, 4);
        display.setCursor(125, 9);
        display.print("Temp: ");
        display.print(t, 3);
        display.print("c");
        
        for (int i = maxArray - readingCount; i < (maxArray - 1); i++) {
            int x0 = (i - (maxArray - readingCount)) * xStep;
            int y0 = 121 - ((array1[i] - minVal) * yScale);
            int x1 = (i + 1 - (maxArray - readingCount)) * xStep;
            int y1 = 121 - ((array1[i + 1] - minVal) * yScale);
            if (array1[i] > 0) {
                display.drawLine(x0, y0, x1, y1, GxEPD_BLACK);
            }
        }
    } while (display.nextPage());

    display.setFullWindow();
    gotosleep();
}

void doHumDisplay() {
    // Recalculate min and max values
    float minVal = array2[maxArray - readingCount];
    float maxVal = array2[maxArray - readingCount];

    for (int i = maxArray - readingCount + 1; i < maxArray; i++) {
        if ((array2[i] < minVal) && (array2[i] > 0)) {
            minVal = array2[i];
        }
        if (array2[i] > maxVal) {
            maxVal = array2[i];
        }
    }

    // Calculate scaling factors
    float yScale = 121.0 / (maxVal - minVal);
    float xStep = 250.0 / (readingCount - 1);

    wipeScreen();
    
    
    do {
        display.fillRect(0,0,display.width(),display.height(),GxEPD_WHITE);
        display.setCursor(0, 9);
        display.print(maxVal, 3);
        display.setCursor(0, 122);
        display.print(minVal, 3);
        display.setCursor(150, 122);
        display.print("vBat: ");
        display.print(vBat, 4);
        display.setCursor(125, 9);
        display.print("Hum: ");
        display.print(h, 3);
        display.print("%");
        
        for (int i = maxArray - readingCount; i < (maxArray - 1); i++) {
            int x0 = (i - (maxArray - readingCount)) * xStep;
            int y0 = 121 - ((array2[i] - minVal) * yScale);
            int x1 = (i + 1 - (maxArray - readingCount)) * xStep;
            int y1 = 121 - ((array2[i + 1] - minVal) * yScale);
            if (array2[i] > 0) {
                display.drawLine(x0, y0, x1, y1, GxEPD_BLACK);
            }
        }
    } while (display.nextPage());

    display.setFullWindow();
    gotosleep();
}

void doPresDisplay() {
    // Recalculate min and max values
    float minVal = array3[maxArray - readingCount];
    float maxVal = array3[maxArray - readingCount];

    for (int i = maxArray - readingCount + 1; i < maxArray; i++) {
        if ((array3[i] < minVal) && (array3[i] > 0)) {
            minVal = array3[i];
        }
        if (array3[i] > maxVal) {
            maxVal = array3[i];
        }
    }

    // Calculate scaling factors
    float yScale = 121.0 / (maxVal - minVal);
    float xStep = 250.0 / (readingCount - 1);

    wipeScreen();
    
    
    do {
        display.fillRect(0,0,display.width(),display.height(),GxEPD_WHITE);
        display.setCursor(0, 9);
        display.print(maxVal, 2);
        display.setCursor(0, 122);
        display.print(minVal, 2);
        display.setCursor(150, 122);
        display.print("vBat: ");
        display.print(vBat, 4);
        display.setCursor(125, 9);
        display.print("Pres: ");
        display.print(pres, 2);
        display.print("mb");
        
        for (int i = maxArray - readingCount; i < (maxArray - 1); i++) {
            int x0 = (i - (maxArray - readingCount)) * xStep;
            int y0 = 121 - ((array3[i] - minVal) * yScale);
            int x1 = (i + 1 - (maxArray - readingCount)) * xStep;
            int y1 = 121 - ((array3[i + 1] - minVal) * yScale);
            if (array3[i] > 0) {
                display.drawLine(x0, y0, x1, y1, GxEPD_BLACK);
            }
        }
    } while (display.nextPage());

    display.setFullWindow();
    gotosleep();
}

void doBatDisplay() {
    // Recalculate min and max values
    float minVal = array4[maxArray - readingCount];
    float maxVal = array4[maxArray - readingCount];

    for (int i = maxArray - readingCount + 1; i < maxArray; i++) {
        if ((array4[i] < minVal) && (array4[i] > 0)) {
            minVal = array4[i];
        }
        if (array4[i] > maxVal) {
            maxVal = array4[i];
        }
    }

    // Calculate scaling factors
    float yScale = 121.0 / (maxVal - minVal);
    float xStep = 250.0 / (readingCount - 1);

    wipeScreen();
    
    
    do {
        display.fillRect(0,0,display.width(),display.height(),GxEPD_WHITE);
        display.setCursor(0, 9);
        display.print(maxVal, 4);
        display.setCursor(0, 122);
        display.print(minVal, 4);
        display.setCursor(150, 122);
        int batPct = mapf(vBat, 3.6, 4.15, 0, 100);
        display.print("vPct: ");
        display.print(batPct, 1);
        display.print("%");
        display.setCursor(125, 9);
        display.print("vBat: ");
        display.print(vBat, 4);
        display.print("v");
        
        for (int i = maxArray - readingCount; i < (maxArray - 1); i++) {
            int x0 = (i - (maxArray - readingCount)) * xStep;
            int y0 = 121 - ((array4[i] - minVal) * yScale);
            int x1 = (i + 1 - (maxArray - readingCount)) * xStep;
            int y1 = 121 - ((array4[i + 1] - minVal) * yScale);
            if (array4[i] > 0) {
                display.drawLine(x0, y0, x1, y1, GxEPD_BLACK);
            }
        }
    } while (display.nextPage());

    display.setFullWindow();
    gotosleep();
}

void setup()
{
  pinMode(controlpin, OUTPUT);
  digitalWrite(controlpin, HIGH);
   GPIO_reason = log(esp_sleep_get_gpio_wakeup_status())/log(2);
  delay(10);
  Wire.begin();  

  aht.begin();
  bmp.begin();
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500);
  bmp.takeForcedMeasurement();
  vBat = analogReadMilliVolts(0) / 500.0;
  aht.getEvent(&humidity, &temp);
   t = temp.temperature;
   h = humidity.relative_humidity;
   pres = bmp.readPressure() / 100.0;
  delay(10);
  display.init(115200, false, 10, false); // void init(uint32_t serial_diag_bitrate, bool initial, uint16_t reset_duration = 10, bool pulldown_rst_mode = false)
  display.setRotation(1);
  display.setFont(&Roboto_Condensed_12);
            
  if (firstrun >= 100) {display.clearScreen();
  firstrun = 0;}
  firstrun++;

  display.setTextColor(GxEPD_BLACK);
  

  if (readingCount < maxArray) {
      readingCount++;
  }

  for (int i = 0; i < (maxArray - 1); i++) {
      array1[i] = array1[i + 1];
  }
  array1[(maxArray - 1)] = t;

  for (int i = 0; i < (maxArray - 1); i++) {
      array2[i] = array2[i + 1];
  }
  array2[(maxArray - 1)] = h;

  for (int i = 0; i < (maxArray - 1); i++) {
      array3[i] = array3[i + 1];
  }
  array3[(maxArray - 1)] = pres;

  for (int i = 0; i < (maxArray - 1); i++) {
      array4[i] = array4[i + 1];
  }
  array4[(maxArray - 1)] = vBat;

  if (GPIO_reason < 0) {
      switch (page){
        case 0: 
          doTempDisplay();
          break;
        case 1: 
          doTempDisplay();
          break;
        case 2: 
          doHumDisplay();
          break;
        case 3: 
          doPresDisplay();
          break;
        case 4: 
          doBatDisplay();
          break;
      }
    }
  switch (GPIO_reason) {
    case 1: 
      page = 1;
      doTempDisplay();
      break;
    case 2: 
      page = 2;
      doHumDisplay();
      break;
    case 3: 
      page = 3;
      doPresDisplay();
      break;
    case 4: 
      page = 4;
      doBatDisplay();
      break;
    case 5: 
      display.clearScreen();
      switch (page){
        case 0: 
          doTempDisplay();
          break;
        case 1: 
          doTempDisplay();
          break;
        case 2: 
          doHumDisplay();
          break;
        case 3: 
          doPresDisplay();
          break;
        case 4: 
          doBatDisplay();
          break;
      }
  }

  

}

void loop()
{
    //vBat = analogRead(0);

    doTempDisplay();
    gotosleep();
    // Add a delay to sample data at intervals (e.g., every minute)
    //delay(1000); // 1 minute delay, adjust as needed
}
