// Display Library example for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: the e-paper panels require 3.3V supply AND data lines!
//
// Display Library based on Demo Example from Good Display: https://www.good-display.com/companyfile/32/
//
// Author: Jean-Marc Zingg
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2

// GxEPD2_WS_ESP32_Driver : GxEPD2_Example variant for Universal e-Paper Raw Panel Driver Board, ESP32 WiFi / Bluetooth Wireless
// https://www.waveshare.com/product/e-paper-esp32-driver-board.htm

// Supporting Arduino Forum Topics (closed, read only):
// Good Display ePaper for Arduino: https://forum.arduino.cc/t/good-display-epaper-for-arduino/419657
// Waveshare e-paper displays with SPI: https://forum.arduino.cc/t/waveshare-e-paper-displays-with-spi/467865
//
// Add new topics in https://forum.arduino.cc/c/using-arduino/displays/23 for new questions and issues

// mapping of Waveshare ESP32 Driver Board
// BUSY -> 25, RST -> 26, DC -> 27, CS-> 15, CLK -> 13, DIN -> 14

// NOTE: this board uses "unusual" SPI pins and requires re-mapping of HW SPI to these pins in SPIClass
//       this example shows how this can be done easily, updated for use with HSPI
//
// The Wavehare ESP32 Driver Board uses uncommon SPI pins for the FPC connector. It uses HSPI pins, but SCK and MOSI are swapped.
// To use HW SPI with the ESP32 Driver Board, HW SPI pins need be re-mapped in any case. Can be done using either HSPI or VSPI.
// Other SPI clients can either be connected to the same SPI bus as the e-paper, or to the other HW SPI bus, or through SW SPI.
// The logical configuration would be to use the e-paper connection on HSPI with re-mapped pins, and use VSPI for other SPI clients.
// VSPI with standard VSPI pins is used by the global SPI instance of the Arduino IDE ESP32 package.

// uncomment next line to use HSPI for EPD (and VSPI for SD), e.g. with Waveshare ESP32 Driver Board
#define USE_HSPI_FOR_EPD

// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <U8g2_for_Adafruit_GFX.h>

// Current configuration: 7.5" 3-color e-paper display
#define GxEPD2_DISPLAY_CLASS GxEPD2_3C
#define GxEPD2_DRIVER_CLASS GxEPD2_750c_Z08 // GDEW075Z08  800x480, EK79655 (GD7965), (WFT0583CZ61)

// U8G2 fonts for better text quality
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

// Helper macros for display configuration
#define GxEPD2_3C_IS_GxEPD2_3C true
#define IS_GxEPD(c, x) (c##x)
#define IS_GxEPD2_3C(x) IS_GxEPD(GxEPD2_3C_IS_, x)

#if defined(ESP32)
#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/ 5, /*DC=*/ 19, /*RST=*/ 16, /*BUSY=*/ 17));
#endif

#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
SPIClass hspi(HSPI);
#endif

// Calendar item structure
struct CalendarItem {
  const char* time;
  const char* title;
  const char* duration;
};

// Sample calendar items for a software engineering manager
CalendarItem calendarItems[] = {
  {"9:00 AM", "Sprint Planning Meeting", "2h"},
  {"11:30 AM", "1:1 with Sarah (Senior Dev)", "30m"},
  {"1:00 PM", "Architecture Review", "1.5h"},
  {"3:00 PM", "Team Standup", "15m"},
  {"4:00 PM", "Code Review Session", "1h"}
};

const int numItems = sizeof(calendarItems) / sizeof(CalendarItem);

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Setup starting...");
  
  // *** special handling for Waveshare ESP32 Driver board *** //
#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
  Serial.println("Configuring HSPI for EPD...");
  hspi.begin(18, -1, 23, 5); // remap hspi for EPD (swap pins)
  display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
#endif
  // *** end of special handling for Waveshare ESP32 Driver board *** //
  
  display.init(115200);
  
  // Initialize U8G2 fonts
  u8g2Fonts.begin(display);
  
  // Display calendar
  displayCalendar();
  
  Serial.println("Setup done");
}

void loop()
{
  // Nothing to do in loop
}

void displayCalendar()
{
  Serial.println("Displaying Engineering Manager Calendar...");
  
  // Set rotation to portrait (0 degrees)
  display.setRotation(0);
  
  // Use full window mode
  display.setFullWindow();
  
  // Use paged drawing
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE); // Set background to white
    
    // Draw header
    drawHeader();
    
    // Draw calendar items
    drawCalendarItems();
  }
  while (display.nextPage());
  
  Serial.println("Calendar displayed");
}

void drawHeader()
{
  // Configure U8G2 font settings
  u8g2Fonts.setFontMode(1);                 // transparent mode
  u8g2Fonts.setFontDirection(0);            // left to right
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  
  // Draw date with U8G2 font
  u8g2Fonts.setForegroundColor(GxEPD_RED);
  u8g2Fonts.setFont(u8g2_font_helvR14_tf);  // Helvetica Regular 14pt
  
  const char* dateText = "Today - Monday, December 16, 2024";
  int16_t tw = u8g2Fonts.getUTF8Width(dateText);
  uint16_t dateX = (display.width() - tw) / 2;
  uint16_t dateY = 35;
  
  u8g2Fonts.setCursor(dateX, dateY);
  u8g2Fonts.print(dateText);
  
  // Draw title with larger U8G2 font
  u8g2Fonts.setFont(u8g2_font_helvB18_tf);  // Helvetica Bold 18pt
  const char* titleText = "Engineering Manager Calendar";
  tw = u8g2Fonts.getUTF8Width(titleText);
  uint16_t titleX = (display.width() - tw) / 2;
  uint16_t titleY = dateY + 50;
  
  u8g2Fonts.setCursor(titleX, titleY);
  u8g2Fonts.print(titleText);
  
  // Draw separator line
  display.drawLine(50, titleY + 25, display.width() - 50, titleY + 25, GxEPD_BLACK);
}

void drawCalendarItems()
{
  uint16_t startY = 130; // Start below header (adjusted for U8G2 fonts)
  uint16_t itemHeight = 75; // Space between items
  uint16_t leftMargin = 30;
  uint16_t rightMargin = 30;
  
  for (int i = 0; i < numItems; i++) {
    uint16_t currentY = startY + (i * itemHeight);
    
    // Draw time (red, left aligned) with U8G2 monospace font
    u8g2Fonts.setForegroundColor(GxEPD_RED);
    u8g2Fonts.setFont(u8g2_font_profont15_mr);  // Professional monospace font
    u8g2Fonts.setCursor(leftMargin, currentY);
    u8g2Fonts.print(calendarItems[i].time);
    
    // Draw title (black, indented) with U8G2 readable font
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setFont(u8g2_font_helvR14_tf);  // Helvetica Regular 14pt
    u8g2Fonts.setCursor(leftMargin + 130, currentY);
    u8g2Fonts.print(calendarItems[i].title);
    
    // Draw duration (black, right aligned) with U8G2 monospace font
    u8g2Fonts.setFont(u8g2_font_profont15_mr);  // Professional monospace font
    int16_t tw = u8g2Fonts.getUTF8Width(calendarItems[i].duration);
    uint16_t durationX = display.width() - rightMargin - tw;
    u8g2Fonts.setCursor(durationX, currentY);
    u8g2Fonts.print(calendarItems[i].duration);
    
    // Draw subtle separator line between items (except last one)
    if (i < numItems - 1) {
      uint16_t lineY = currentY + 30;
      display.drawLine(leftMargin + 130, lineY, display.width() - rightMargin, lineY, GxEPD_BLACK);
    }
  }
}
