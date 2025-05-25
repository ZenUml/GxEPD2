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

// Current configuration: 7.5" 3-color e-paper display
#define GxEPD2_DISPLAY_CLASS GxEPD2_3C
#define GxEPD2_DRIVER_CLASS GxEPD2_750c_Z08 // GDEW075Z08  800x480, EK79655 (GD7965), (WFT0583CZ61)

// For even better font quality, consider using U8G2 fonts:
// Uncomment the following lines and modify the drawing functions:
// #include <U8g2_for_Adafruit_GFX.h>
// U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
// In setup(): u8g2Fonts.begin(display);
// Use fonts like: u8g2_font_helvR14_tf, u8g2_font_helvB18_tf
// See: https://github.com/olikraus/u8g2/wiki/fntlistall

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
  // Draw date with larger font
  display.setFont(&FreeSans12pt7b);
  display.setTextColor(GxEPD_RED);
  
  const char* dateText = "Today - Monday, December 16, 2024";
  int16_t tbx, tby; 
  uint16_t tbw, tbh;
  display.getTextBounds(dateText, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t dateX = (display.width() - tbw) / 2;
  uint16_t dateY = 30;
  
  display.setCursor(dateX, dateY);
  display.print(dateText);
  
  // Draw title with even larger font
  display.setFont(&FreeSans18pt7b);
  const char* titleText = "Engineering Manager Calendar";
  display.getTextBounds(titleText, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t titleX = (display.width() - tbw) / 2;
  uint16_t titleY = dateY + 45;
  
  display.setCursor(titleX, titleY);
  display.print(titleText);
  
  // Draw separator line
  display.drawLine(50, titleY + 20, display.width() - 50, titleY + 20, GxEPD_BLACK);
}

void drawCalendarItems()
{
  uint16_t startY = 120; // Start below header (adjusted for larger header)
  uint16_t itemHeight = 75; // Increased space between items
  uint16_t leftMargin = 30;
  uint16_t rightMargin = 30;
  
  for (int i = 0; i < numItems; i++) {
    uint16_t currentY = startY + (i * itemHeight);
    
    // Draw time (red, left aligned) with larger font
    display.setFont(&FreeMonoBold12pt7b);
    display.setTextColor(GxEPD_RED);
    display.setCursor(leftMargin, currentY);
    display.print(calendarItems[i].time);
    
    // Draw title (black, indented) with readable font
    display.setFont(&FreeSans12pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(leftMargin + 130, currentY);
    display.print(calendarItems[i].title);
    
    // Draw duration (black, right aligned)
    display.setFont(&FreeMonoBold12pt7b);
    int16_t tbx, tby; 
    uint16_t tbw, tbh;
    display.getTextBounds(calendarItems[i].duration, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t durationX = display.width() - rightMargin - tbw;
    display.setCursor(durationX, currentY);
    display.print(calendarItems[i].duration);
    
    // Draw subtle separator line between items (except last one)
    if (i < numItems - 1) {
      uint16_t lineY = currentY + 30;
      display.drawLine(leftMargin + 130, lineY, display.width() - rightMargin, lineY, GxEPD_BLACK);
    }
  }
}
