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

// Current configuration: 7.5" 3-color e-paper display
#define GxEPD2_DISPLAY_CLASS GxEPD2_3C
#define GxEPD2_DRIVER_CLASS GxEPD2_750c_Z08 // GDEW075Z08  800x480, EK79655 (GD7965), (WFT0583CZ61)

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
  
  // Display Hello World
  helloWorld();
  
  Serial.println("Setup done");
}

void loop()
{
  // Nothing to do in loop
}

void helloWorld()
{
  Serial.println("Displaying Hello World...");
  
  const char text[] = "Hello World!";
  
  // Set rotation to landscape for better text display
  display.setRotation(1);
  
  // Select a suitable font
  display.setFont(&FreeMonoBold9pt7b);
  
  // Set text color to black
  display.setTextColor(GxEPD_BLACK);
  
  // Calculate text bounds for centering
  int16_t tbx, tby; 
  uint16_t tbw, tbh;
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
  
  // Center the text
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  
  // Use full window mode
  display.setFullWindow();
  
  // Use paged drawing
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE); // Set background to white
    display.setCursor(x, y);
    display.print(text);
  }
  while (display.nextPage());
  
  Serial.println("Hello World displayed");
}
