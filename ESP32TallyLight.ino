#include <FastLED.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define DATA_PIN 13                                                                                                                                                                                                                                                                                                                         
#define NUM_LEDS 8
CRGB leds[NUM_LEDS];
const char* ssid = ".";
const char* password = "03032007";
const unsigned int localUDPPort = 8888;
WiFiUDP udp;
char packetBuffer[255];
enum TallyState {
  STATE_OFFLINE,
  STATE_STANDBY,
  STATE_PREVIEW,
  STATE_PROGRAM
};
TallyState currentState = STATE_OFFLINE;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starte LED Test...");
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  bootAnimation();
  initWiFi();
  udp.begin(localUDPPort);
  Serial.println("UDP Server gestartet auf Port " + String(localUDPPort));
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    currentState = STATE_OFFLINE;
    handleTallyState();
    reconnectWiFi();
    return;
  }

  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    String command = String(packetBuffer);
    command.trim();

    if (command == "CAM_PGM") {
      currentState = STATE_PROGRAM;
    }
    else if (command == "CAM_PRV") {
      currentState = STATE_PREVIEW;
    }
    else if (command == "CAM_OFF") {
      currentState = STATE_STANDBY;
    }

    Serial.println("Empfangen: " + command);
  }

  handleTallyState();
}

void initWiFi() {
  Serial.println("Verbinde mit WLAN: " + String(ssid));
  WiFi.begin(ssid, password);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    fill_solid(leds, NUM_LEDS, CRGB::Pink);
    FastLED.show();
    delay(100);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();

    counter++;
    if (counter > 40) {
      Serial.println("\nWlan-Verbindung fehlgeschlagen, starte neu...");
      ESP.restart();
    }

    Serial.println("\nWLAN-Verbindung hergestellt! IP: " + String(WiFi.localIP()));
  }
}

void reconnectWiFi() {
  Serial.println("WLAN-Verbindung verloren! Versuche Verbindung wiedergerzustellen...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }
}

void handleTallyState() {
  switch (currentState) {
    case STATE_OFFLINE:
      fill_solid(leds, NUM_LEDS, CRGB::Blue);
      break;
    case STATE_STANDBY:
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      break;
    case STATE_PREVIEW:
      fill_solid(leds, NUM_LEDS, CRGB::Green);
      break;
    case STATE_PROGRAM:
      fill_solid(leds, NUM_LEDS, CRGB::Red);
      break;
  }
  FastLED.show();
}

void bootAnimation() {
  for (int i = 0; i < NUM_LEDS - 1; i++) {
    leds[i] = CRGB::White;
    FastLED.show();
    delay(50);
    leds[i] = CRGB::Black;
  }
}