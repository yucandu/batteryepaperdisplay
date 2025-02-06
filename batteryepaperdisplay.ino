#include <GxEPD2_BW.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <WiFi.h>
//#include <AsyncTCP.h>
//#include <ESPAsyncWebServer.h>
//#include <AsyncElegantOTA.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;
sensors_event_t humidity, temp;
#include<Wire.h>
#include<ADS1115_WE.h> 
#include<Wire.h>
#define I2C_ADDRESS 0x48

#include "driver/periph_ctrl.h"
int GPIO_reason;
#include "esp_sleep.h"


ADS1115_WE adc = ADS1115_WE(I2C_ADDRESS);

const char* ssid = "mikesnet";
const char* password = "springchicken";
// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 1

#define sleeptimeSecs 300
#define maxArray 501


RTC_DATA_ATTR float array1[maxArray];
RTC_DATA_ATTR float array2[maxArray];
RTC_DATA_ATTR float array3[maxArray];
RTC_DATA_ATTR float array4[maxArray];
RTC_DATA_ATTR float windspeed, windgust, fridgetemp, outtemp;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;  //Replace with your GMT offset (secs)
const int daylightOffset_sec = 0;   //Replace with your daylight offset (secs)
  float t, h, pres, barx;
  float v41_value, v42_value, v62_value;

 RTC_DATA_ATTR   int firstrun = 100;
 RTC_DATA_ATTR   int page = 2;
float abshum;
 float minVal = 3.9;
 float maxVal = 4.2;
RTC_DATA_ATTR int readingCount = 0; // Counter for the number of readings
int readingTime;

#include "bitmaps/Bitmaps128x250.h"
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/Roboto_Condensed_12.h>
#include <Fonts/FreeSerif12pt7b.h> 
#include <Fonts/Open_Sans_Condensed_Bold_54.h> 
#include <Fonts/DejaVu_Serif_Condensed_36.h>
#include <Fonts/DejaVu_Serif_Condensed_60.h>
#define BUTTON_PIN_BITMASK(GPIO) (1ULL << GPIO)
//GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(GxEPD2_213_BN(/*CS=5*/ SS, /*DC=*/ 21, /*RES=*/ 20, /*BUSY=*/ 3)); // DEPG0213BN 122x250, SSD1680
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(/*CS=5*/ SS, /*DC=*/ 21, /*RES=*/ 20, /*BUSY=*/ 10)); // GDEH0154D67 200x200, SSD1681

const char* blynkserver = "192.168.50.197:9443";
const char* bedroomauth = "8_-CN2rm4ki9P3i_NkPhxIbCiKd5RXhK";  //hubert
//const char* fridgeauth = "VnFlJdW3V0uZQaqslqPJi6WPA9LaG1Pk";

// Virtual Pins
const char* v41_pin = "V41";
const char* v62_pin = "V62";

float vBat;

float findLowestNonZero(float a, float b, float c) {
  // Initialize minimum to a very large value
  float minimum = 999;

  // Check each variable and update the minimum value
  if (a != 0.0 && a < minimum) {
    minimum = a;
  }
  if (b != 0.0 && b < minimum) {
    minimum = b;
  }
  if (c != 0.0 && c < minimum) {
    minimum = c;
  }

  return minimum;
}

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
      pinMode(5, INPUT_PULLUP );


      //delay(10000);
      //rtc_gpio_isolate(gpio_num_t(SDA));
      //rtc_gpio_isolate(gpio_num_t(SCL));
      //periph_module_disable(PERIPH_I2C0_MODULE);  
      //digitalWrite(SDA, 0);
      //digitalWrite(SCL, 0);
      uint64_t bitmask = BUTTON_PIN_BITMASK(GPIO_NUM_1) | BUTTON_PIN_BITMASK(GPIO_NUM_2) | BUTTON_PIN_BITMASK(GPIO_NUM_3) | BUTTON_PIN_BITMASK(GPIO_NUM_0) | BUTTON_PIN_BITMASK(GPIO_NUM_5);
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

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V41);
  Blynk.syncVirtual(V42);
  Blynk.syncVirtual(V62);
  Blynk.syncVirtual(V78);
  Blynk.syncVirtual(V79);
  Blynk.syncVirtual(V82);
}

BLYNK_WRITE(V41) {
  v41_value = param.asFloat();
}

BLYNK_WRITE(V42) {
  v42_value = param.asFloat();
}
BLYNK_WRITE(V62) {
  v62_value = param.asFloat();
}
BLYNK_WRITE(V78) {
  windspeed = param.asFloat();
}
BLYNK_WRITE(V79) {
  windgust = param.asFloat();
}
BLYNK_WRITE(V82) {
  fridgetemp = param.asFloat();
}

void startWifi(){

  //display.clearScreen();
  //display.setPartialWindow(0, 0, display.width(), display.height());
  //display.setCursor(0, 0);
  //display.firstPage();

  //do {
  //  display.print("Connecting...");
  //} while (display.nextPage());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);  
  WiFi.setTxPower (WIFI_POWER_8_5dBm);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() > 10000) { //display.print("!");
      WiFi.setTxPower(WIFI_POWER_8_5dBm);
    }
    if (millis() > 20000) {
        return;
      }
    //do {
      //display.print(".");
      // display.display(true);
    //} while (display.nextPage());
  }
  //wipeScreen();
  //display.setCursor(0, 0);
  //display.firstPage();
  //do {
    //display.print("Connected! to: ");
    //display.println(WiFi.localIP());
  //} while (display.nextPage());
   // display.print("RSSI: ");
  //  display.println(WiFi.RSSI());
  // display.display(true);
  // display.print("Connecting to blynk...");
  Blynk.config(bedroomauth, IPAddress(192, 168, 50, 197), 8080);
  Blynk.connect();
  while ((!Blynk.connected()) && (millis() < 20000)){
     // display.print(".");
     //  display.display(true);
       delay(500);}
  if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
            Blynk.virtualWrite(V111, t);
          if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
          Blynk.virtualWrite(V112, h);
          if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
          Blynk.virtualWrite(V113, pres);
          if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
          Blynk.virtualWrite(V114, abshum);
          if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
          Blynk.virtualWrite(V115, vBat);
          if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
          Blynk.virtualWrite(V115, vBat);
          if (WiFi.status() == WL_CONNECTED) {Blynk.run();}
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    time_t now = time(NULL);
  localtime_r(&now, &timeinfo);

  // Allocate a char array for the time string
  char timeString[10]; // "12:34 PM" is 8 chars + null terminator

  // Format the time string
 /* if (timeinfo.tm_min < 10) {
    snprintf(timeString, sizeof(timeString), "%d:0%d %s", timeinfo.tm_hour % 12 == 0 ? 12 : timeinfo.tm_hour % 12, timeinfo.tm_min, timeinfo.tm_hour < 12 ? "AM" : "PM");
  } else {
    snprintf(timeString, sizeof(timeString), "%d:%d %s", timeinfo.tm_hour % 12 == 0 ? 12 : timeinfo.tm_hour % 12, timeinfo.tm_min, timeinfo.tm_hour < 12 ? "AM" : "PM");
  }
    display.println(timeString);
    display.display(true);*/
}

void startWebserver(){

  //display.clearScreen();
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.setCursor(0, 0);
  display.firstPage();

  do {
    display.print("Connecting...");
  } while (display.nextPage());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);  
  WiFi.setTxPower (WIFI_POWER_8_5dBm);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() > 20000) { display.print("!");}
    if ((millis() > 30000)) {gotosleep();}
    //do {
      display.print(".");
       display.display(true);
    //} while (display.nextPage());
    delay(1000);
  }
  wipeScreen();
  display.setCursor(0, 0);
  display.firstPage();
  do {
    display.print("Connected! to: ");
    display.println(WiFi.localIP());
  } while (display.nextPage());
  ArduinoOTA.setHostname("epaperdisplay");
  ArduinoOTA.begin();
  display.println("ArduinoOTA started");
    display.print("RSSI: ");
    display.println(WiFi.RSSI());
   display.display(true);
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

    //readingTime = ((readingCount - 1) * sleeptimeSecs) / 60;

}

void setupChart() {
    display.setCursor(0, 0);
    display.print("<");
    display.print(maxVal, 3);
    
    // Adjusted for bottom of 200x200 display
    display.setCursor(0, 199);
    display.print("<");
    display.print(minVal, 3);

    // Adjusted for horizontal placement of the additional text
    display.setCursor(160, 199);
    display.print("<#");
    display.print(readingCount - 1, 0);
    display.print("*");
    display.print(sleeptimeSecs, 0);
    display.print("s>");

    // Adjusted rectangle and progress bar to fit within bounds
    display.drawRect(179, 192, 19, 7, GxEPD_BLACK); // Rectangle moved to fit fully within 200x200
    display.fillRect(179, 192, barx, 7, GxEPD_BLACK); // Progress bar inside the rectangle

    // Adjusted marker lines to stay within bounds
    display.drawLine(198, 193, 198, 198, GxEPD_BLACK); 
    display.drawLine(199, 193, 199, 198, GxEPD_BLACK);

    // Set the cursor for additional chart decorations
    display.setCursor(125, 0); 
}



double mapf(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void doTempDisplay() {
    // Recalculate min and max values
    minVal = array1[maxArray - readingCount];
    maxVal = array1[maxArray - readingCount];

    for (int i = maxArray - readingCount + 1; i < maxArray; i++) {
        if (array1[i] != 0) {  // Only consider non-zero values
            if (array1[i] < minVal) {
                minVal = array1[i];
            }
            if (array1[i] > maxVal) {
                maxVal = array1[i];
            }
        }
    }

    // Calculate scaling factors for full 200x200 display
    float yScale = 199.0 / (maxVal - minVal); // Adjusted for full vertical range
    float xStep = 200.0 / (readingCount - 1); // Adjusted for full horizontal range

    wipeScreen();

    do {
        display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);

        for (int i = maxArray - readingCount; i < (maxArray - 1); i++) {
            int x0 = (i - (maxArray - readingCount)) * xStep;
            int y0 = 199 - ((array1[i] - minVal) * yScale); // Full vertical range
            int x1 = (i + 1 - (maxArray - readingCount)) * xStep;
            int y1 = 199 - ((array1[i + 1] - minVal) * yScale); // Full vertical range

            // Only draw a line for valid (non-zero) values
            if (array1[i] != 0) {
                display.drawLine(x0, y0, x1, y1, GxEPD_BLACK);
            }
        }

        // Call setupChart to draw chart decorations
        setupChart();

        // Display temperature information at the bottom
        display.setCursor(10, 190); // Positioned near the bottom for readability
        display.print("[");
        display.print("Temp: ");
        display.print(array1[maxArray - 1], 3);
        display.print("c");
        display.print("]");
    } while (display.nextPage());

    display.setFullWindow();
    gotosleep();
}


void doHumDisplay() {
    // Recalculate min and max values
    minVal = array2[maxArray - readingCount];
    maxVal = array2[maxArray - readingCount];

    for (int i = maxArray - readingCount + 1; i < maxArray; i++) {
        if ((array2[i] < minVal) && (array2[i] > 0)) {
            minVal = array2[i];
        }
        if (array2[i] > maxVal) {
            maxVal = array2[i];
        }
    }

    // Adjust scaling factors
    float yScale = 199.0 / (maxVal - minVal); // Adjusted for full vertical range
    float xStep = 200.0 / (readingCount - 1); // Adjusted for full horizontal range

    wipeScreen();

    do {
        display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);

        for (int i = maxArray - readingCount; i < (maxArray - 1); i++) {
            int x0 = (i - (maxArray - readingCount)) * xStep;
            int y0 = 199 - ((array2[i] - minVal) * yScale); // Adjusted for vertical range
            int x1 = (i + 1 - (maxArray - readingCount)) * xStep;
            int y1 = 199 - ((array2[i + 1] - minVal) * yScale); // Adjusted for vertical range
            if (array2[i] > 0) {
                display.drawLine(x0, y0, x1, y1, GxEPD_BLACK);
            }
        }

        setupChart();
        display.print("[");
        display.print("Hum: ");
        display.print(array2[(maxArray - 1)], 2);
        display.print("g");
        display.print("]");
    } while (display.nextPage());

    display.setFullWindow();
    gotosleep();
}

void doWindDisplay() {




        
 wipeScreen();

  updateMain();
  gotosleep();
}

void doPresDisplay() {
    // Recalculate min and max values
    minVal = array3[maxArray - readingCount];
    maxVal = array3[maxArray - readingCount];

    for (int i = maxArray - readingCount + 1; i < maxArray; i++) {
        if ((array3[i] < minVal) && (array3[i] > 0)) {
            minVal = array3[i];
        }
        if (array3[i] > maxVal) {
            maxVal = array3[i];
        }
    }

    // Adjust scaling factors
    float yScale = 199.0 / (maxVal - minVal); // Adjusted for full vertical range
    float xStep = 200.0 / (readingCount - 1); // Adjusted for full horizontal range

    wipeScreen();

    do {
        display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);

        for (int i = maxArray - readingCount; i < (maxArray - 1); i++) {
            int x0 = (i - (maxArray - readingCount)) * xStep;
            int y0 = 199 - ((array3[i] - minVal) * yScale); // Adjusted for vertical range
            int x1 = (i + 1 - (maxArray - readingCount)) * xStep;
            int y1 = 199 - ((array3[i + 1] - minVal) * yScale); // Adjusted for vertical range
            if (array3[i] > 0) {
                display.drawLine(x0, y0, x1, y1, GxEPD_BLACK);
            }
        }

        setupChart();
        display.print("[");
        display.print("Pres: ");
        display.print(array3[(maxArray - 1)], 2);
        display.print("mb");
        display.print("]");
    } while (display.nextPage());

    display.setFullWindow();
    gotosleep();
}


void doBatDisplay() {
    // Recalculate min and max values
    minVal = array4[maxArray - readingCount];
    maxVal = array4[maxArray - readingCount];

    for (int i = maxArray - readingCount + 1; i < maxArray; i++) {
        if ((array4[i] < minVal) && (array4[i] > 0)) {
            minVal = array4[i];
        }
        if (array4[i] > maxVal) {
            maxVal = array4[i];
        }
    }

    // Adjust scaling factors
    float yScale = 199.0 / (maxVal - minVal); // Adjusted for full vertical range
    float xStep = 200.0 / (readingCount - 1); // Adjusted for full horizontal range

    wipeScreen();

    do {
        display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);

        for (int i = maxArray - readingCount; i < (maxArray - 1); i++) {
            int x0 = (i - (maxArray - readingCount)) * xStep;
            int y0 = 199 - ((array4[i] - minVal) * yScale); // Adjusted for vertical range
            int x1 = (i + 1 - (maxArray - readingCount)) * xStep;
            int y1 = 199 - ((array4[i + 1] - minVal) * yScale); // Adjusted for vertical range
            if (array4[i] > 0) {
                display.drawLine(x0, y0, x1, y1, GxEPD_BLACK);
            }
        }

        display.setCursor(0, 0);
        display.print("<");
        display.print(maxVal, 3);
        display.setCursor(0, 193); // Adjusted to stay 7px from the bottom
        display.print("<");
        display.print(minVal, 3);
        display.setCursor(120, 193); // Adjusted for new width
        display.print("<#");
        display.print(readingCount - 1, 0);
        display.print("*");
        display.print(sleeptimeSecs, 0);
        display.print("s>");
        display.setCursor(175, 193); // Adjusted for new width

        int batPct = mapf(vBat, 3.3, 4.15, 0, 100);
        display.setCursor(125, 0); // Same placement
        display.print("[vBat: ");
        display.print(vBat, 3);
        display.print("v/");
        display.print(batPct, 1);
        display.print("%]");
    } while (display.nextPage());

    display.setFullWindow();
    gotosleep();
}


float fetchBlynkValue(const char* vpin, const char* authToken) {
  WiFiClientSecure client;
  //client.setCACert(root_ca); // Set the certificate
  client.setInsecure(); 
  
  HTTPClient https;
  https.setReuse(true);
  String url = String("https://") + blynkserver + "/" + authToken + "/get/" + vpin;


  if (https.begin(client, url)) { // HTTPS connection
    int httpCode = https.GET();
    float value = NAN;

    if (httpCode == HTTP_CODE_OK) {
      String payload = https.getString();
      payload.replace("[", ""); // Remove brackets from the JSON
      payload.replace("]", "");
      payload.replace("\"", ""); // Remove brackets from the JSON
      payload.replace("\"", "");
      value = payload.toFloat();  
      if (isnan(value)) {return NAN;}

    } 
    https.end();
    return value;
  } else {
    gotosleep();
    return NAN;
  }
}

void takeSamples(){

     if (WiFi.status() == WL_CONNECTED) {
          Blynk.syncVirtual(V41);
          Blynk.syncVirtual(V62);
          Blynk.syncVirtual(V78);
          Blynk.syncVirtual(V79);
          Blynk.syncVirtual(V82);

          /*display.print("North temp: ");
          display.println(v41_value);
          //display.display(true);
          //float v62_value = fetchBlynkValue("V62", bedroomauth);
          display.print("Neo temp: ");
          display.println(v62_value);
          display.print("Joju temp: ");
          display.println(v42_value);
          //display.display(true);
          //windspeed = fetchBlynkValue("V78", bedroomauth);
          display.print("Wind speed: ");
          display.println(windspeed);
          //display.display(true);
          //windgust = fetchBlynkValue("V79", bedroomauth);
          display.print("Wind gust: ");
          display.println(windgust);
          //display.display(true);
          //fridgetemp = fetchBlynkValue("V1", fridgeauth);
          display.print("Fridge temp: ");
          display.println(fridgetemp);
          display.print("Time: ");
          display.print(millis());
          display.display(true);*/
          float min_value = findLowestNonZero(v41_value, v42_value, v62_value);





          //display.display(true);

          if (min_value != 999) {
            for (int i = 0; i < (maxArray - 1); i++) {
                array3[i] = array3[i + 1];
            }
            array3[(maxArray - 1)] = min_value;
          }
        }

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
        array2[(maxArray - 1)] = abshum;



        for (int i = 0; i < (maxArray - 1); i++) {
            array4[i] = array4[i + 1];
        }
        array4[(maxArray - 1)] = vBat;
}

void updateMain() {
    time_t now = time(NULL);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    // Allocate a char array for the time string
    char timeString[10]; // "12:34 PM" is 8 chars + null terminator

    // Format the time string
    if (timeinfo.tm_min < 10) {
        snprintf(timeString, sizeof(timeString), "%d:0%d %s", timeinfo.tm_hour % 12 == 0 ? 12 : timeinfo.tm_hour % 12, timeinfo.tm_min, timeinfo.tm_hour < 12 ? "AM" : "PM");
    } else {
        snprintf(timeString, sizeof(timeString), "%d:%d %s", timeinfo.tm_hour % 12 == 0 ? 12 : timeinfo.tm_hour % 12, timeinfo.tm_min, timeinfo.tm_hour < 12 ? "AM" : "PM");
    }

    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);

    // Draw crosshair lines to divide the screen into 4 quadrants
    display.drawLine(99, 0, 99, 200, GxEPD_BLACK);  // Vertical center line
    display.drawLine(100, 0, 100, 200, GxEPD_BLACK); // Vertical center line (thicker)
    display.drawLine(0, 99, 200, 99, GxEPD_BLACK);  // Horizontal center line
    display.drawLine(0, 100, 200, 100, GxEPD_BLACK); // Horizontal center line (thicker)

    // Quadrant 1: Top-left
    display.setTextSize(1); // Font size 2 (16px)
    display.setCursor(24, 2); // Adjusted to fit top-left quadrant
    display.print("Temp:");
    display.setCursor(6, 40); // Centered vertically in quadrant
    display.setTextSize(2); // Font size 3 (24px)
    float temptodraw = array3[(maxArray - 1)];
    if ((temptodraw > 0) && (temptodraw < 10)) { display.print(" "); }
    display.print(temptodraw, 1);
    display.print("c");

    // Quadrant 2: Top-right
    display.setTextSize(1); // Font size 2 (16px)
    display.setCursor(124, 2); // Adjusted to fit top-right quadrant
    display.print("Wind:");
    display.setCursor(130, 40); // Centered vertically in quadrant
    display.setTextSize(2); // Font size 3 (24px)
    display.print(windspeed, 0);
    display.print("kph");

    // Quadrant 3: Bottom-left
    display.setTextSize(1); // Font size 2 (16px)
    display.setCursor(24, 104); // Adjusted for bottom-left quadrant
    display.print("Fridge:");
    display.setCursor(20, 140); // Centered vertically in quadrant
    display.setTextSize(2); // Font size 3 (24px)
    display.print(fridgetemp, 1);
    display.print("c");

    // Quadrant 4: Bottom-right
    display.setTextSize(1); // Font size 2 (16px)
    display.setCursor(124, 104); // Adjusted for bottom-right quadrant
    display.print("Gust:");
    display.setCursor(130, 140); // Centered vertically in quadrant
    display.setTextSize(2); // Font size 3 (24px)
    display.print(windgust, 0);
    display.print("kph");

    // Display time string near the bottom-left corner
    display.setTextSize(1); // Font size 1 (8px)
    display.setCursor(0, 192); // 8px above the bottom
    display.print(timeString);

    // Battery status (bottom-right)
    int barx = mapf(vBat, 3.3, 4.15, 0, 19); // Map battery value to progress bar width
    if (barx > 19) { barx = 19; }
    display.drawRect(179, 192, 19, 7, GxEPD_BLACK); // Battery outline (adjusted for 200x200)
    display.fillRect(179, 192, barx, 7, GxEPD_BLACK); // Battery fill
    display.drawLine(198, 193, 198, 198, GxEPD_BLACK); // Tick marks on battery
    display.drawLine(199, 193, 199, 198, GxEPD_BLACK); // Tick marks on battery
    display.drawRect(0,0, display.width(), display.height(), GxEPD_BLACK);
    display.display(true);
}


float readChannel(ADS1115_MUX channel) {
  float voltage = 0.0;
  adc.setCompareChannels(channel);
  adc.startSingleMeasurement();
  while(adc.isBusy()){delay(0);}
  voltage = adc.getResult_V(); // alternative: getResult_mV for Millivolt
  return voltage;
}

void setup()
{
  Wire.begin();  
  adc.init();
  adc.setVoltageRange_mV(ADS1115_RANGE_4096); 
  vBat = readChannel(ADS1115_COMP_0_GND) * 2.0;
  GPIO_reason = log(esp_sleep_get_gpio_wakeup_status())/log(2);
  

  aht.begin();
  bmp.begin();
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500);
  bmp.takeForcedMeasurement();
  
  aht.getEvent(&humidity, &temp);
   t = temp.temperature;
   h = humidity.relative_humidity;
   pres = bmp.readPressure() / 100.0;
    abshum = (6.112 * pow(2.71828, ((17.67 * temp.temperature)/(temp.temperature + 243.5))) * humidity.relative_humidity * 2.1674)/(273.15 + temp.temperature);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(50);

  display.init(115200, false, 10, false); // void init(uint32_t serial_diag_bitrate, bool initial, uint16_t reset_duration = 10, bool pulldown_rst_mode = false)
  display.setRotation(2);
  display.setTextSize(1);
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);

  pinMode(5, INPUT_PULLUP);



   
  delay(10);

            
  if (firstrun >= 100) {display.clearScreen();
   if (page == 2){
        wipeScreen();

   }
  firstrun = 0;}
  firstrun++;

  display.setTextColor(GxEPD_BLACK, GxEPD_WHITE);
  



  if (GPIO_reason < 0) {
    startWifi();
    takeSamples();
      switch (page){
        case 0: 
          doTempDisplay();
          break;
        case 1: 
          doTempDisplay();
          break;
        case 2: 
          doWindDisplay();
          break;
        case 3: 
          doHumDisplay();
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
    case 3: 
      page = 2;
        wipeScreen();
        doWindDisplay();
      break;
    case 2: 
      page = 3;
      doHumDisplay();
      break;
    case 5: 
      page = 4;
      doBatDisplay();
      break;
    case 0: 
    delay(50);
      while (!digitalRead(0))
        {
          delay(10);
          if (millis() > 2000) {
            startWebserver();
          return;}
        }
      startWifi();
      takeSamples();
      display.clearScreen();
      switch (page){
        case 0: 
          doTempDisplay();
          break;
        case 1: 
          doTempDisplay();
          break;
        case 2: 
          doWindDisplay();
          break;
        case 3: 
          doHumDisplay();
          break;
        case 4: 
          doBatDisplay();
          break;
      }
  }

  

}

void loop()
{
ArduinoOTA.handle();
if (!digitalRead(0)) {gotosleep();}
delay(250);
    // Add a delay to sample data at intervals (e.g., every minute)
    //delay(1000); // 1 minute delay, adjust as needed
}
