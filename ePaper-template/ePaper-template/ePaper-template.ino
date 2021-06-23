/**
 * Program: ePaper-template
 * Purpose:
 *   A basic framework for starting any program.
 * Notes:
 *   1) Uses the ESP32 Dev Module board definition in the Arduino IDE.
 *   2) It's necessary to install the GxEPD and Adafruit_GFX libraries
 * @author: David Argles, d.argles@gmx.com: drawing heavily on code by Lewis he
 */

/* Program identification */ 
#define PROG    "ePaper-template"
#define VER     "1.0"
#define BUILD   "22jun2021 @15:06h"

// Define the board (used later)
#define LILYGO_T5_V213

// Other defines
#define ORIENTATION 1 // 1 and 3 are landscape

#include <boards.h>
#include <GxEPD.h>
#include <SD.h>
#include <FS.h>
#include <WiFi.h>

// Local includes
#include "flashscreen.h"
#include "speaker.h"

// Not sure where this file is hiding :/
#include <GxGDEH0213B73/GxGDEH0213B73.h>  // 2.13" b/w old panel

#include GxEPD_BitmapExamples

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

// Instantiate my local objects
flashscreen   flash;

// Not sure where this will be used
GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
// display will be the class we use to manage the ePaper screen
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

#if defined(_GxGDEW0213Z16_H_) || defined(_GxGDEW029Z10_H_) || defined(_GxGDEW027C44_H_) ||defined(_GxGDEW0154Z17_H_) || defined(_GxGDEW0154Z04_H_) || defined(_GxDEPG0290R_H_)
#define _HAS_COLOR_
#endif

bool setupSDCard(void)
{
#if defined(_HAS_SDCARD_) && !defined(_USE_SHARED_SPI_BUS_)
    SPIClass SDSPI(VSPI);
#endif

#if defined(_HAS_SDCARD_) && !defined(_USE_SHARED_SPI_BUS_)
    SDSPI.begin(SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI);
    return SD.begin(SDCARD_CS, SDSPI);
#elif defined(_HAS_SDCARD_)
    return SD.begin(SDCARD_CS);
#endif
}

void showSplashscreen(){
    bool rlst = setupSDCard();

    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);

#if defined(_HAS_COLOR_)
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_RED);
#else
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
#endif

#if defined(_HAS_SDCARD_)
    display.setRotation(ORIENTATION);
    display.setCursor(20, display.height() - 15);
    String sizeString = "SD:" + String(SD.cardSize() / 1024.0 / 1024.0 / 1024.0) + "G";
    display.println(rlst ? sizeString : "SD:N/A");

    int16_t x1, x2;
    uint16_t w, h;
    String str = GxEPD_BitmapExamplesQ;
    str = str.substring(2, str.lastIndexOf("/"));
    display.getTextBounds(str, 0, 0, &x1, &x2, &w, &h);
    display.setCursor(display.width() - w - 5, display.height() - 15);
    display.println(str);
#endif

    display.writeFastHLine(0, 0, 249, 0);
    display.writeFastHLine(0, 120, 249, 0);
    display.writeFastVLine(0, 0, 120, 0);
    display.writeFastVLine(249, 0, 120, 0);
    
    display.update();
    return;
}

void testWiFi()
{
    display.fillScreen(GxEPD_WHITE);
    //changeFont("FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
    display.setCursor(0,0);
    display.println("Scanning WiFi networks...");
    display.update();
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();

    display.println("Scan completed\n");
    display.fillScreen(GxEPD_WHITE);
    changeFont("FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
    display.setCursor(0,15);
    if (n == 0) {
        display.println("No networks found\n");
    } else {
        display.print(n);
        display.println(" networks found");

        
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            display.print(i + 1);
            display.print(": ");
            display.print(WiFi.SSID(i));
            display.print(" (");
            display.print(WiFi.RSSI(i));
            display.print(")");
            display.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
            delay(10);
        }
    }
    display.println("");
    display.update();
}

void setup()
{
    Serial.begin(115200);
    // Send program details to serial output
    flash.message(PROG, VER, BUILD);
    
    // Start up the SPI
    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    // Start up the ePaper display
    display.init();
    display.setTextColor(GxEPD_BLACK);
    showSplashscreen();
    delay(5000);
    
    Serial.println("\nWiFi setup...");

    testSpeaker();

    testWiFi();
    
    delay(10000);

    display.powerDown();

    Serial.println("\nPowering down...");

    esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_1)), ESP_EXT1_WAKEUP_ALL_LOW);

    esp_deep_sleep_start();
}

void loop(){
  // Nothing happens here, we deep sleep instead
}

void writeText()
{
    int i = 0;

    while (i < 4) {
        display.setRotation(i);
        showFont("FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
        //showFont("FreeMonoBold18pt7b", &FreeMonoBold18pt7b);
        //showFont("FreeMonoBold24pt7b", &FreeMonoBold24pt7b);
        i++;
    }
}

void changeFont(const char name[], const GFXfont *f){
    display.setFont(f);
}

void showFont(const char name[], const GFXfont *f)
{
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(f);
    display.setCursor(0, 0);
    display.println();
    display.println(name);
    display.println(" !\"#$%&'()*+,-./");
    display.println("0123456789:;<=>?");
    display.println("@ABCDEFGHIJKLMNO");
    display.println("PQRSTUVWXYZ[\\]^_");
    display.println("`abcdefghijklmno");
    display.println("pqrstuvwxyz{|}~ ");
    display.update();
    delay(5000);
}

void drawCornerTest()
{
    display.drawCornerTest();
    delay(5000);
    uint8_t rotation = display.getRotation();
    for (uint16_t r = 0; r < 4; r++) {
        display.setRotation(r);
        display.fillScreen(GxEPD_WHITE);
        display.fillRect(0, 0, 8, 8, GxEPD_BLACK);
        display.fillRect(display.width() - 18, 0, 16, 16, GxEPD_BLACK);
        display.fillRect(display.width() - 25, display.height() - 25, 24, 24, GxEPD_BLACK);
        display.fillRect(0, display.height() - 33, 32, 32, GxEPD_BLACK);
        display.update();
        delay(5000);
    }
    display.setRotation(rotation); // restore
}
