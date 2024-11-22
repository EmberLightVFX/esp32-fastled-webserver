/*
   ESP32 FastLED WebServer: https://github.com/jasoncoon/esp32-fastled-webserver
   Copyright (C) 2017 Jason Coon

   Built upon the amazing FastLED work of Daniel Garcia and Mark Kriegsman:
   https://github.com/FastLED/FastLED

   ESP32 support provided by the hard work of Sam Guyer:
   https://github.com/samguyer/FastLED

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
  Modified for https://github.com/me-no-dev/ESPAsyncWebServer
  https://github.com/philbowles/ESPAsyncWebServer/issues/
  https://github.com/CDFER/Captive-Portal-ESP32/blob/main/src/main.cpp
*/
#include <SPIFFS.h>
#include "esp_eap_client.h"
#include "ap.h"
#include "secrets.h"
#include "led/field.h"
#include "led/fields.h"

const int led = 5;

void initNetwork(bool ap_only = true)
{
  // Connect to Wi-Fi network with SSID and password
  Serial.println("Starting AP (Access Point)");

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  unsigned long startAttemptTime = millis(); // Record the start time

  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    if (millis() - startAttemptTime >= 5000)
    { // Check if 5 seconds have elapsed
      Serial.println("Failed to connect to WiFi. Continuing...");
      // break; // Exit the loop
      startAttemptTime = millis();
    }
    delay(100); // Wait 0.1 second
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected to WiFi network");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  Serial.println("Starting AP...");
  if (!WiFi.softAP(ap_ssid, ap_password))
  {
    Serial.println("Failed to create AP");
  }
  else
  {
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }

  webServer.begin();
  Serial.println("HTTP server started");
}

void setupWeb()
{

  webServer.on("/all", HTTP_GET, [](AsyncWebServerRequest *request)
               {
    digitalWrite(led, 0);
    String json = getFieldsJson(fields, fieldCount);
     request->send(200, "text/json", json);
    digitalWrite(led, 1); });

  webServer.on("/fieldValue", HTTP_GET, [](AsyncWebServerRequest *request)
               {
    digitalWrite(led, 0);
    String name =  request->getParam("name")->value();
    String value = getFieldValue(name, fields, fieldCount);
     request->send(200, "text/json", value);
    digitalWrite(led, 1); });

  webServer.on("/fieldValue", HTTP_POST, [](AsyncWebServerRequest *request)
               {
    digitalWrite(led, 0);
    String name =  request->getParam("name")->value();
    String value =  request->getParam("value")->value();
    String newValue;
    // Particularly like this since we cahnged some 
    if(request->hasParam("r") && request->hasParam("g") && request->hasParam("b")){
      String r = request->getParam("r")->value();
      String g = request->getParam("g")->value();
      String b = request->getParam("b")->value();
      String comb = r + "," + g + "," + b; //reuased named somewhere else asmoge
      newValue = setFieldValue(name, value, fields, fieldCount, comb);
    }
    else{
      newValue = setFieldValue(name, value, fields, fieldCount);
    }
    
    request->send(200, "text/json", newValue);
    digitalWrite(led, 1); });

  // Root
  webServer.serveStatic("", SPIFFS, "/index.htm", "max-age=86400");
  webServer.serveStatic("/", SPIFFS, "/index.htm", "max-age=86400");
  webServer.serveStatic("/index.htm", SPIFFS, "/index.htm", "max-age=86400");
  webServer.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico", "max-age=86400");

  // CSS
  webServer.serveStatic("/css/bootstrap.min.css", SPIFFS, "/css/bootstrap.min.css", "max-age=86400");
  webServer.serveStatic("/css/jquery.minicolors.min.css", SPIFFS, "/css/jquery.minicolors.min.css", "max-age=86400");
  webServer.serveStatic("/css/styles.css", SPIFFS, "/css/styles.css", "max-age=86400");

  // JS
  webServer.serveStatic("/js/app.js", SPIFFS, "/js/app.js", "max-age=86400");
  webServer.serveStatic("/js/bootstrap.min.js", SPIFFS, "/js/bootstrap.min.js", "max-age=86400");
  webServer.serveStatic("/js/jquery-3.1.1.min.js", SPIFFS, "/js/jquery-3.1.1.min.js", "max-age=86400");
  webServer.serveStatic("/js/jquery.minicolors.min.js", SPIFFS, "/js/jquery.minicolors.min.js", "max-age=86400");

  // Fonts
  webServer.serveStatic("/fonts/glyphicons-halflings-regular.ttf", SPIFFS, "/fonts/glyphicons.ttf", "max-age=86400");

  // Images

  webServer.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->redirect("http://logout.net"); }); // windows 11 captive portal workaround
  webServer.on("/wpad.dat", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(404); }); // Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)
  webServer.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->redirect("/"); }); // android captive portal redirect
  webServer.on("/redirect", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->redirect("/"); }); // microsoft redirect
  webServer.on("hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->redirect("/"); }); // apple call home
  webServer.on("netcts.cdn-apple.com", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(200); }); // apple call home
  webServer.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->redirect("/"); }); // apple call home
  webServer.on("/netcts.cdn-apple.com", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(200); }); // apple call home
  webServer.on("/canonical.html", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->redirect("/"); }); // firefox captive portal call home
  webServer.on("/success.txt", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(200); }); // firefox captive portal call home
  webServer.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->redirect("/"); }); // windows call home
  webServer.onNotFound([](AsyncWebServerRequest *request)
                       {
        request->redirect("/index.htm");
        Serial.print("onnotfound ");
        Serial.print(request->host()); // This gives some insight into whatever was being requested on the serial monitor
        Serial.print(" ");
        Serial.print(request->url());
        Serial.print(" sent redirect to / \n"); });
}

void webLoop()
{
}