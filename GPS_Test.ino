/*
  Reconfigures the left Grove connector as a UART named SerialGrove
  Display data from the GPS to the TFT display
  Left side Grove connector shares pins with I2C1 of 40 pin connector.
  Do not use the default wire (I2C) library as these pins are now a UART.
*/

#include <wiring_private.h>
#include <TFT_eSPI.h>
#include <TinyGPS++.h>

#define SerialGrove_BAUD 9600
#define TFT_HEIGHT 240  // Landscape size
#define TFT_WIDTH 320
#define TFT_ROTATION 3 // Landscape USB connector at bottom
#define FONT 4 // builtin font 28 pixels high
#define TEXT_HEIGHT 28
#define FOOTER_Y (TFT_HEIGHT - TEXT_HEIGHT)// y location of the top of the footer zone

TinyGPSPlus gps;
TFT_eSPI tft = TFT_eSPI();
unsigned long charProcessed = 0;

// reassign sercom3 as a UART using the I2C pins on the left Grove connector
static Uart SerialGrove(&sercom3, PIN_WIRE_SCL, PIN_WIRE_SDA, SERCOM_RX_PAD_1, UART_TX_PAD_0);

void SERCOM3_0_Handler()
{
  SerialGrove.IrqHandler();
}

void SERCOM3_1_Handler()
{
  SerialGrove.IrqHandler();
}

void SERCOM3_2_Handler()
{
  SerialGrove.IrqHandler();
}

void SERCOM3_3_Handler()
{
  SerialGrove.IrqHandler();
}

void setup()
{
  tft.begin();
  tft.setRotation(3);
  tft.fillRect(0,0,320,240,TFT_BLACK);
  tft.setTextFont(FONT);
  printFooter("Waiting for GPS");
  
  // Serial.begin(115200);
  // while (!Serial) ;
  // Serial.println("Serial started");
  SerialGrove.begin(SerialGrove_BAUD);
   while (!SerialGrove);
  // Serial.println("SerialGrove Started");

  
  pinPeripheral(PIN_WIRE_SCL, PIO_SERCOM_ALT);
  pinPeripheral(PIN_WIRE_SDA, PIO_SERCOM_ALT);
  // Serial.println("Pin maps set");
}


void loop()
{
  if(gps.charsProcessed() <= charProcessed)
  {
    printFooter("Waiting for GPS");
  } 
  else
  {
    String textToPrint;
    char valueToPrint[100];
    
    // Print Date and Time
    if (gps.date.isValid() && gps.time.isValid())
    {
      sprintf(valueToPrint, "%d/%d/%d", gps.date.month(), gps.date.day(), gps.date.year());
      textToPrint = valueToPrint;
      sprintf(valueToPrint, "%d:%d:%d", gps.time.hour(), gps.time.minute(), gps.time.second());
      textToPrint = textToPrint + " " + valueToPrint + " GMT";
      printLine(textToPrint, 0);
    }
    else
    {
      printLine("NO DATE/TIME DATA", 0);
    }

    // Print LAT 
    if (gps.location.isValid())
    {
      textToPrint  = "LAT= ";
      sprintf(valueToPrint, "%0.6f", gps.location.lat());
      printLine(textToPrint + valueToPrint, 1);
    }
    else
    {
      printLine("LAT= NO FIX", 1);
    }
    

    // Print LONG
    if (gps.location.isValid())
    {
      textToPrint = "LONG= ";
      sprintf(valueToPrint, "%0.6f", gps.location.lng());
      printLine(textToPrint + valueToPrint, 2);
    }
    else
    {
      printLine("LONG= NO FIX", 2); ;
    }
    

     // Print Altitude
    if (gps.altitude.isValid())
    {
      textToPrint = "Altitude= ";
      sprintf(valueToPrint, "%0.0f", gps.altitude.feet());
      printLine(textToPrint + valueToPrint + " feet", 3);
    }
    else
    {
      printLine("Altitude= NO FIX", 3);
 
    }

    // Print HDOP
    if (gps.hdop.isValid())
    {
      textToPrint = "HDOP= ";
      double hdop = gps.hdop.hdop();
      sprintf(valueToPrint, "%0.3f", hdop);
      textToPrint = textToPrint + valueToPrint;
     
      if(hdop < 2)
      {
         textToPrint = textToPrint + " (Excellent)";
      }
      else if(hdop < 5)
      {
        textToPrint = textToPrint + " (Good)";
      }
      else if(hdop < 10)
      {
        textToPrint = textToPrint + " (Moderate)";
      }
      else if(hdop < 20)
      {
        textToPrint = textToPrint + " (Fair)";
      }
      else
      {
        textToPrint = textToPrint + " (Poor)";
      }

      printLine(textToPrint, 4);
    }
    else
    {
      printLine("HDOP= NO DATA", 4);
    }
     
    // Print Satellites
    if (gps.hdop.isValid())
    {
      textToPrint = "Satellites= ";
      sprintf(valueToPrint, "%d", gps.satellites.value());
      printLine(textToPrint + valueToPrint, 5);
    }
    else
    {
      printLine("Satellites= NO DATA", 5);
    }
    
    PrintStats();
  }
  
smartDelay(1000);
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (SerialGrove.available())
      gps.encode(SerialGrove.read());
  } while (millis() - start < ms);
}

void printFooter(String textToPrint)
{
  tft.fillRect(0,FOOTER_Y, TFT_WIDTH, TEXT_HEIGHT, TFT_BLUE);
  tft.setCursor(0,FOOTER_Y);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.print(textToPrint);
}

void PrintStats(){
  String footerText;
  footerText = gps.passedChecksum();
  footerText = footerText + " lines processed";
  printFooter(footerText);

}

void printLine(String textToPrint, uint8_t lineNum)
{
  uint8_t locY = lineNum * TEXT_HEIGHT;
  tft.fillRect(0,locY, TFT_WIDTH, TEXT_HEIGHT, TFT_BLACK);
  tft.setCursor(0,locY);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print(textToPrint);
  
}