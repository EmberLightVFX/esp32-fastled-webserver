#include <Arduino.h>
#include <SPIFFS.h>
#include <FS.h>
#include <EEPROM.h>
#include <WiFi.h>
#include "led/leds.h"
#include "web/web.h"

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void setup()
{
  Serial.begin(460800);
  delay(300);
  // while (!Serial)
  //{
  //   ; // wait for serial port to connect. Needed for native USB port only
  // }

  Serial.println("Loading SPIFFS");
  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS Mount Failed");
  }
  else
  {
    Serial.println("SPIFFS Mounted!");
  }
  listDir(SPIFFS, "/", 1);

  ledSetup();

  initNetwork();
  setupWeb();
}

void loop()
{
  ledLoop();
  webLoop();
}