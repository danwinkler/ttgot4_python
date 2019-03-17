#include <Arduino.h>

#include <ArduinoJson.h>
#include <PacketSerial.h>

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

PacketSerial pSerial;

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
  pSerial.begin(115200);
  pSerial.setPacketHandler(&onPacketReceived);

  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
   
  tft.begin();

  //printDebugToSerial();
  
  tft.setRotation(0); // 0 = 0°  1 = 90°  2 = 180°  3 = 270°
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextWrap(true);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println("Test");

  Serial.println("End of setup");
}

void loop() {
  pSerial.update();
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

void onPacketReceived(const uint8_t* buffer, size_t size)
{
  pSerial.send(buffer, size);
  Serial.println("Packet Received");
  StaticJsonDocument<2048> doc;
  bool err = deserializeJson(doc, (const char*)buffer);

  if(err) {
    printDebug("Failed to read message");
    tft.println((const char*)buffer);
    return;
  }

  JsonArray commands = doc["commands"].as<JsonArray>();

  if( commands.isNull() ) {
    printDebug("Invalid message format");
    return;
  }

  for(JsonVariant command : commands) {
    int commandId = command["command"];
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
  const char* text = (*args)["text"];
  tft.println(text);
}

void command_setTextSize(JsonVariant* args) {
  int size = (*args)["size"];
  tft.setTextSize(size);  
}

void command_setCursor(JsonVariant* args) {
  int x = (*args)["x"];
  int y = (*args)["y"];
  tft.setCursor(x, y);
}

void command_setTextWrap(JsonVariant* args) {
  bool wrap = (*args)["wrap"];
  tft.setTextWrap(wrap);
}

void command_setTextColor(JsonVariant* args) {
  int color = (*args)["color"];
  tft.setTextColor(color);
}

void command_fillScreen(JsonVariant* args) {
  int color = (*args)["color"];
  tft.fillScreen(color);
}

void command_setRotation(JsonVariant* args) {
  int orientation = (*args)["orientation"];
  tft.setRotation(orientation);
}
