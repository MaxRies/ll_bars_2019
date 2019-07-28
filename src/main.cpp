#include <Arduino.h>
#include <FastLED.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Pattern.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// LED DEFINES
#define FOREGROUND_NUM_LEDS 100
#define BACKGROUND_NUM_LEDS 100
#define ALL_NUM_LEDS FOREGROUND_NUM_LEDS+BACKGROUND_NUM_LEDS
#define LED_PIN 6

// BUTTON DEFINES
#define PUSHBUTTON D3
#define LONGPUSH 2
#define SHORTPUSH 1
#define UNDECIDED 0
#define LONGPUSHTIME 3000
#define PUSHTHRESHOLD 200

// WIFI DEFINES
#define K95WIFI
#define DEBUG
#define WIFI_WAIT 15

/* LED VARIABLES */
int group;
int position;
CRGB front_foreground;
CRGB front_background;
CRGB back_foreground;
CRGB back_background;
CRGB leds[FOREGROUND_NUM_LEDS+BACKGROUND_NUM_LEDS];
CRGB * fg_leds = &leds[BACKGROUND_NUM_LEDS];
CRGB * bg_leds = &leds[0];
Pattern pattern(leds, ALL_NUM_LEDS);

/* PATTERN COMBINATIONS */


/* ESP VARIABLES */
bool configMode = false;
bool wifiMode = false;
char chipName[30];


#ifdef BRAINWIFI
const char* ssid = "llbrain2";
const char* password = "lichtundlaessig!";
const char* mqtt_server = "192.168.0.2";
#endif

#ifdef NIKOWIFI
const char* ssid = "FRITZ!Box 7430 HC";
const char* password = "72180323197714590513";
const char* mqtt_server = "192.168.178.31";
#endif

#ifdef K95WIFI
const char* ssid = "p0rnStream247";
const char* password = "Party_0815";
const char* mqtt_server = "192.168.178.23";
#endif

WiFiClient espClient;
PubSubClient client(espClient);

/* LED FUNCTIONS */




/* SETUP FUNCTIONS */
void setupChip() {
  pinMode(PUSHBUTTON, INPUT_PULLUP);
  int32_t chipInteger = ESP.getChipId();
  sprintf(chipName, "esp%08X", chipInteger);
}

void setupLEDs() {
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, FOREGROUND_NUM_LEDS+BACKGROUND_NUM_LEDS);
}

void setupOTA() {
 ArduinoOTA.setHostname(chipName);
 ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    fill_solid(leds, ALL_NUM_LEDS, CRGB::Green);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int percent = progress / (total / 100);
    Serial.printf("Progress: %u%%\r", percent);
    fg_leds[percent] = CRGB::Azure;
    FastLED.show();
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    tries++;
    if (tries > WIFI_WAIT) {
      wifiMode = false;
      break;
    }
  }
  wifiMode = true;

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.println(topic);
  Serial.println((char*) payload);
  Serial.println();
  if(strstr(topic, "group") != NULL) {
    if (strstr(topic, "set") != NULL) {
        if (configMode) {
        char value[20];
        strncpy(value, (char*)payload, length);
        group = atoi(value);
        Serial.printf("Group Set: %u\n", group);
      }
      else {
        Serial.println("Not in config mode!");
      }
    }
  }
  else if(strstr(topic, "position") != NULL) {
    if (strstr(topic, "set") != NULL) {
      if (configMode) {
        char value[20];
        strncpy(value, (char*)payload, length);
        position = atoi(value);
        Serial.printf("Position Set: %u\n", position);
      }
      else {
        Serial.println("Not in config mode!");
      }
    }
  }
  else if(strstr(topic, "mode") != NULL) {
    Serial.println("Config mode Message");
    char value[50];
    strncpy(value, (char*)payload, length);
    if(strstr(value, "config") != NULL) {
      configMode = true;
      Serial.println("Set mode to config mode");
    }
    else if(strstr(value, "normal") != NULL) {
      configMode = false;
      Serial.println("Set mode to normal mode");     
  }
}
}

void setupMQTT() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.connect(chipName);
  Serial.println("Connecting to MQTT Broker...");
  while (!client.connected()) {
    yield();
  }

  char topic0[100];
  sprintf(topic0, "bars/hello");
  client.publish(topic0, chipName);
  Serial.println("Subscribed to:");
  Serial.println(topic0);

  char topic1[100];
  sprintf(topic1, "bars/%s/position/set", chipName);
  client.subscribe(topic1);
  Serial.println("Subscribed to:");
  Serial.println(topic1);

  char topic2[100];
  sprintf(topic2, "bars/%s/group/set", chipName);
  client.subscribe(topic2);
  Serial.println("Subscribed to:");
  Serial.println(topic2);

  client.subscribe("brain/mode");
  Serial.println("MQTT Setup DONE!");
}

int checkButton() {
  static bool buttonPushed = false;
  static long buttonPressStart;
  int result;

  if (digitalRead(PUSHBUTTON) == LOW) {
    if (!buttonPushed) {
      Serial.println("button pressed."); 
      buttonPushed = true;
      buttonPressStart = millis();
      delay(50);
    }
    result = UNDECIDED; 
  }
  else {
    if (buttonPushed) {
      Serial.println("button released.");
      long timeHeld = millis() - buttonPressStart;
      Serial.print("Held for: ");
      Serial.println(timeHeld);
      buttonPushed = false;
      delay(50);
      if (timeHeld > LONGPUSHTIME) {
        result = LONGPUSH;
        Serial.println("result = longpush");
      }
      else if (timeHeld > PUSHTHRESHOLD) {
        result = SHORTPUSH;
        Serial.println("result = shortpush");
      }
      else {
        result = UNDECIDED;
      }
    }
    else {
      result = UNDECIDED;
    }
  }


  if (result == SHORTPUSH) {
    Serial.println("publish shortpush");
    char topic[100];
    sprintf(topic, "bars/%s/button", chipName);
    client.publish(topic, "short");
  }
  else if (result == LONGPUSH)
  {
    Serial.println("publish longpush");
    char topic[100];
    sprintf(topic, "bars/%s/button", chipName);
    client.publish(topic, "long");    
  }
  return result;
}

void setup() {
  Serial.begin(115200);
  setupChip();
  setupLEDs();
  setup_wifi();
  
  if (wifiMode) {
    setupMQTT();
    setupOTA();
    
  }
  else {
  }
}

void loop() {
  if (wifiMode) {
    checkButton();
    client.loop();

    // to implement
    ArduinoOTA.handle();
    //handleLEDs();
  }
  else {
    // noConnection show mode
  }
}