#include <Arduino.h>

#include <ArduinoJson.h>
#include <DNSServer.h>
#include <IotWebConf.h>
#include <WiFi.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS   27
#define TFT_DC   32
#define TFT_RST  5

#define BUTTON_CENTER  37
#define BUTTON_LEFT    38
#define BUTTON_RIGHT   39

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

const char* initialAP = "TTGO_T4";
const char* initialPassword = "ttgopass";

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(initialAP, &dnsServer, &server, initialPassword);

void printDebugToSerial() {
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
}

void setup() {
  Serial.begin(115200);

  // Turn backlight on
  // TODO: is this necessary?
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
   
  tft.begin();

  // Default tft settings:
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextWrap(true);
  tft.setCursor(0, 0);
  tft.setTextSize(2);

  //printDebugToSerial();
  
  tft.setRotation(0); // 0 = 0°  1 = 90°  2 = 180°  3 = 270°

  printDebug("Starting");
  
  // -- Initializing the configuration.
  iotWebConf.setWifiConnectionCallback(&wifiConnected);
  iotWebConf.init();
  
  // For testing (not neccessary using iotwebconf):
  //WiFi.enableSTA(true);
  //delay(10);
  //WiFi.begin("", "");
  //waitForConnected();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/command", handleCommand);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

  // For testing (not neccessary using iotwebconf):
  //server.begin();

}

void wifiConnected() {
  IPAddress ip = WiFi.localIP();
  char ipBuffer[16];
  sprintf(ipBuffer, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  printDebug(ipBuffer);
}

void waitForConnected()
{
  int i = 0;
  while(WiFi.status() != WL_CONNECTED) {
    i++;
    if(i % 10 == 0) {
      tft.fillScreen(ILI9341_BLACK);
      tft.setCursor(0, 0);
    }
    tft.print(".");
    delay(100);
  }
}

void loop() {
  iotWebConf.doLoop();

  // For Testing:
  //server.handleClient();
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>TTGO T4 WiFi Setup</title></head><body>";
  s += "Go to <a href='config'>configure page</a> to change settings.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void printDebug(const char* message) {
  Serial.println(message);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextWrap(true);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println(message);
}

void handleCommand() {
  char data[2048] = {};
  String dataStr = server.arg("command_data");
  dataStr.toCharArray(data, dataStr.length()+1);
  onCommandInfoReceived(data);
  server.send(200, "text/plain", "ok");
}

void onCommandInfoReceived(const char* buffer)
{
  StaticJsonDocument<2048> doc;
  bool err = deserializeJson(doc, (const char*)buffer);

  if(err) {
    printDebug("Failed to read message");
    tft.println((const char*)buffer);
    return;
  }

  JsonArray commands = doc["cmds"].as<JsonArray>();

  if( commands.isNull() ) {
    printDebug("Invalid message format");
    return;
  }

  for(JsonVariant command : commands) {
    int commandId = command["cmd"];
    JsonVariant args = command["args"];
    
    switch(commandId) {
      case 0: command_println(&args); break;
      case 1: command_setTextSize(&args); break;
      case 2: command_setCursor(&args); break;
      case 3: command_setTextWrap(&args); break;
      case 4: command_setTextColor(&args); break;
      case 5: command_fillScreen(&args); break;
      case 6: command_setRotation(&args); break;
    }
  }
}

/**
 * COMMANDS
 */

void command_println(JsonVariant* args) {
  const char* text = (*args)["t"];
  tft.println(text);
}

void command_setTextSize(JsonVariant* args) {
  int size = (*args)["s"];
  tft.setTextSize(size);  
}

void command_setCursor(JsonVariant* args) {
  int x = (*args)["x"];
  int y = (*args)["y"];
  tft.setCursor(x, y);
}

void command_setTextWrap(JsonVariant* args) {
  bool wrap = (*args)["w"];
  tft.setTextWrap(wrap);
}

void command_setTextColor(JsonVariant* args) {
  JsonObject* argsObject = (JsonObject*)args;
  int color = (*args)["c"];
  if( argsObject->containsKey("b") ) {
    int bgColor = (*args)["b"];
    tft.setTextColor(color, bgColor);
  } else {
    tft.setTextColor(color);
  }
  
}

void command_fillScreen(JsonVariant* args) {
  int color = (*args)["c"];
  tft.fillScreen(color);
}

void command_setRotation(JsonVariant* args) {
  int orientation = (*args)["o"];
  tft.setRotation(orientation);
}
