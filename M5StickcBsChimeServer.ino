#include <M5StickC.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


// WiFi Setting
const char *ssid = "WiFi-AP-SSID";
const char *password = "WiFi-AP-PASSWORD";
const IPAddress ip(192, 168, 1, 2);         //M5StickC BS Chime Server IP Address
const IPAddress subnet(255, 255, 255, 0);   //My home subnet mask
const IPAddress gateway(192,168, 1, 1);     //My home router
const IPAddress DNS(192, 168, 1, 1);        //My home router
const int chimePin = 33;                    //Chime A Contact Pin

// WebServer
AsyncWebServer server(80); // 80 port
// Websocket Server
WebSocketsServer webSocket = WebSocketsServer(81); // 81 port

// WebSocket Response(JSON Format)
const char RES_JSON[] PROGMEM = R"=====({"chime":%s})=====";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
          {
              IPAddress ip = webSocket.remoteIP(num);
              Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
              break;
          }
        case WStype_TEXT:
            USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
            break;
        case WStype_ERROR:      
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            break;
    }
}

void chime_call() {
  char payload[16];
  snprintf_P(payload, sizeof(payload), RES_JSON, "true");
  webSocket.broadcastTXT(payload, strlen(payload));
  Serial.println("Chime CALL!");
}

void setup() {
  M5.begin();
  Serial.begin(115200);
  pinMode(chimePin, INPUT_PULLUP);
  pinMode(M5_LED, OUTPUT);
  digitalWrite(M5_LED, HIGH);
  M5.Lcd.setRotation(1);
  M5.Axp.ScreenBreath(9);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setTextSize(4);
  M5.Lcd.print("START!");
  
  // SPIFFS Setup
  if(!SPIFFS.begin(true)){
    Serial.println("Error SPIFFS Setup");
    return;
  }

  // WiFi Setup
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet, DNS);
  delay(100);
  WiFi.begin(ssid, password);
  Serial.print("WiFi Connected...");
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.println("Connect!");

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  // ROOT HTML Response
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  
  // style.css Response
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // WebServer Start
  server.begin();

  // WebSocket server Start
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  M5.Lcd.fillScreen(BLACK);
}

void loop() {
  M5.update();
  ArduinoOTA.handle();
  webSocket.loop();
  if (digitalRead(chimePin) == LOW) {
    delay(500);
    if (digitalRead(chimePin) == LOW) {
      digitalWrite(M5_LED, LOW);
      M5.Lcd.setCursor(0, 20);
      M5.Lcd.print("CHIME!");
      chime_call();
      while (digitalRead(chimePin) == LOW) {
        delay(500);
      }
      digitalWrite(M5_LED, HIGH);
      M5.Lcd.fillScreen(BLACK);
    }
  }
}
