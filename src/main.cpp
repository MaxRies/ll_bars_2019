#define VERSION 3

#include <Arduino.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Pattern.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <LedFunctions.h>
#include <Protocol.h>

// LED DEFINES
#define FOREGROUND_NUM_LEDS 90
#define BACKGROUND_NUM_LEDS 90
#define NUM_LEDS FOREGROUND_NUM_LEDS + BACKGROUND_NUM_LEDS
#define LED_PIN 12
#define ONBOARDLED 2 // 1, 17, 21, 22 nicht.

// BUTTON DEFINES
#define BUTTONPIN 0
#define LONGPUSH 2
#define SHORTPUSH 1
#define UNDECIDED 0
#define LONGPUSHTIME 3000
#define PUSHTHRESHOLD 200

// WIFI DEFINES
#define K95WIFI
#define WIFI_WAIT 15
const int ticker_port = 1103;

// DEBUG
#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf(__VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif

/* LED VARIABLES */
int group;
int position;
CRGB front_foreground;
CRGB front_background;
CRGB back_foreground;
CRGB back_background;
CRGB leds[NUM_LEDS];
CRGB *fg_leds = &leds[BACKGROUND_NUM_LEDS];
CRGB *bg_leds = &leds[0];
Pattern pattern(leds, NUM_LEDS);

/* CHRISTOPH PATTERN VARIABLES */
IPAddress remoteIP;
unsigned int remotePort = 1103;
unsigned int packetSize = 0;
unsigned int commandCode = 48;
#define MAX_PACKET_SIZE 32
#define NUM_FRONT_PATTERNS 6
#define NUM_BACK_PATTERNS 7
#define NUM_STROBE_PATTERNS 4

char recvBuffer[MAX_PACKET_SIZE];

WiFiUDP Udp;

//subscribeMessage subMesg;
synchronisingMessage syncMesg;
//settingMessage setMesg;
//pushingMessage pushMesg;
//statusingMessage statMesg;

long millisSinceSync;
long lastSync;
long millisSinceBeat;
long lastBeat;
long millisSinceRequest;
long lastRequest;
long now;
long lastSave;

/* ESP VARIABLES */
bool configMode = false;
bool wifiMode = false;
char chipName[40];

#ifdef BRAINWIFI
const char *ssid = "llbrain2";
const char *password = "lichtundlaessig!";
const char *mqtt_server = "192.168.0.2";
#endif

#ifdef NIKOWIFI
const char *ssid = "FRITZ!Box 7430 HC";
const char *password = "72180323197714590513";
const char *mqtt_server = "192.168.178.31";
#endif

#ifdef K95WIFI
const char *ssid = "p0rnStream247";
const char *password = "Party_0815";
const char *mqtt_server = "192.168.178.23";
#endif

WiFiClient espClient;
PubSubClient client(espClient);

/* LED FUNCTIONS */
void flash(int times, CRGB color)
{
  for (int i = 0; i < times; i++)
  {
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
    delay(250);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(250);
  }
}

void flashOnboard(int n)
{
  for (size_t i = 0; i < n; i++)
  {
    digitalWrite(ONBOARDLED, LOW);
    delay(100);
    digitalWrite(ONBOARDLED, HIGH);
    delay(100);
  }
}

/* SETUP FUNCTIONS */
void setupChip()
{
  pinMode(BUTTONPIN, INPUT_PULLUP);
  pinMode(ONBOARDLED, OUTPUT);
  int32_t chipInteger = ESP.getChipId();
  sprintf(chipName, "bar_%08X_v%i", chipInteger, VERSION);
}

void setupLEDs()
{
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void setupBeatListener()
{
  Udp.begin(ticker_port);
  for (int i = 0; i < MAX_PACKET_SIZE; i++)
  {
    recvBuffer[i] = '0';
  }
  millisSinceSync = 0;
  lastSync = 0;
  millisSinceBeat = 0;
  lastBeat = 0;
  millisSinceRequest = 0;
  lastRequest = 0;
  now = 0;
}

void setupOTA()
{
  ArduinoOTA.setHostname(chipName);
  ArduinoOTA.onStart([]() {
    DEBUG_MSG("Start updating ");
    fill_solid(leds, NUM_LEDS, CRGB::Yellow);
    FastLED.show();
  });
  ArduinoOTA.onEnd([]() {
    DEBUG_MSG("\nEnd");
    flash(3, CRGB::Azure);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int percent = progress / (total / 100);
    if ((percent < NUM_LEDS) && (percent > 0))
    {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      leds[percent] = CRGB::Green;
      FastLED.show();
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG_MSG("Error[%u]: \n", error);
  });
  ArduinoOTA.begin();
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  DEBUG_MSG();
  DEBUG_MSG("Connecting to %s", ssid);

  WiFi.begin(ssid, password);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    DEBUG_MSG(".");
    tries++;
    if (tries > WIFI_WAIT)
    {
      wifiMode = false;
      break;
    }
  }
  wifiMode = true;

  DEBUG_MSG("");
  DEBUG_MSG("WiFi connected");
  DEBUG_MSG("IP address: ");
  DEBUG_MSG(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  DEBUG_MSG();
  DEBUG_MSG(topic);
  DEBUG_MSG((char *)payload);
  DEBUG_MSG();
  if (strstr(topic, "group") != NULL)
  {
    if (strstr(topic, "set") != NULL)
    {
      if (configMode)
      {
        char value[20];
        strncpy(value, (char *)payload, length);
        group = atoi(value);
        DEBUG_MSG("Group Set: %u\n", group);
      }
      else
      {
        DEBUG_MSG("Not in config mode!");
      }
    }
  }
  else if (strstr(topic, "position") != NULL)
  {
    if (strstr(topic, "set") != NULL)
    {
      if (configMode)
      {
        char value[20];
        strncpy(value, (char *)payload, length);
        position = atoi(value);
        DEBUG_MSG("Position Set: %u\n", position);
      }
      else
      {
        DEBUG_MSG("Not in config mode!");
      }
    }
  }
  else if (strstr(topic, "mode") != NULL)
  {
    DEBUG_MSG("Config mode Message");
    char value[50];
    strncpy(value, (char *)payload, length);
    if (strstr(value, "config") != NULL)
    {
      configMode = true;
      DEBUG_MSG("Set mode to config mode");
    }
    else if (strstr(value, "normal") != NULL)
    {
      configMode = false;
      DEBUG_MSG("Set mode to normal mode");
    }
  }
  else if (strstr(topic, "Pattern") != NULL)
  {
    
  }
  else if (strstr(topic, "Dimm") != NULL)
  {
    
  }
  else if (strstr(topic, "Color") != NULL)
  {
    
  }
  else if (strstr(topic, "Speed") != NULL)
  {
    
  }
}

void setupMQTT()
{
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.connect(chipName);
  DEBUG_MSG("Connecting to MQTT Broker...");
  while (!client.connected())
  {
    yield();
  }

  char topic0[100];
  sprintf(topic0, "bars/hello");
  client.publish(topic0, chipName);
  DEBUG_MSG("Subscribed to:");
  DEBUG_MSG(topic0);

  char topic1[100];
  sprintf(topic1, "bars/%s/position/set", chipName);
  client.subscribe(topic1);
  DEBUG_MSG("Subscribed to:");
  DEBUG_MSG(topic1);

  char topic2[100];
  sprintf(topic2, "bars/%s/group/set", chipName);
  client.subscribe(topic2);
  DEBUG_MSG("Subscribed to:");
  DEBUG_MSG(topic2);

  client.subscribe("bars/Dimm");
  DEBUG_MSG("Subscribed to: bars/Dimm\n");

  client.subscribe("bars/Pattern");
  DEBUG_MSG("Subscribed to: bars/Pattern\n");

  client.subscribe("bars/Color");
  DEBUG_MSG("Subscribed to: bars/Color\n");

  client.subscribe("bars/Speed");
  DEBUG_MSG("Subscribed to: bars/Speed\n");

  client.subscribe("brain/mode");
  DEBUG_MSG("MQTT Setup DONE!");
}

int checkButton()
{
  static bool buttonPushed = false;
  static long buttonPressStart;
  int result;

  if (digitalRead(PUSHBUTTON) == LOW)
  {
    if (!buttonPushed)
    {
      DEBUG_MSG("button pressed.");
      buttonPushed = true;
      buttonPressStart = millis();
      delay(50);
    }
    result = UNDECIDED;
  }
  else
  {
    if (buttonPushed)
    {
      DEBUG_MSG("button released.");
      long timeHeld = millis() - buttonPressStart;
      DEBUG_MSG("Held for: %i ", timeHeld);
      buttonPushed = false;
      delay(50);
      if (timeHeld > LONGPUSHTIME)
      {
        result = LONGPUSH;
        DEBUG_MSG("result = longpush");
      }
      else if (timeHeld > PUSHTHRESHOLD)
      {
        result = SHORTPUSH;
        DEBUG_MSG("result = shortpush");
      }
      else
      {
        result = UNDECIDED;
      }
    }
    else
    {
      result = UNDECIDED;
    }
  }

  if (result == SHORTPUSH)
  {
    if (wifiMode)
    {
      DEBUG_MSG("publish shortpush");
      char topic[100];
      sprintf(topic, "bars/%s/button", chipName);
      client.publish(topic, "short");
    }
  }
  else if (result == LONGPUSH)
  {
    DEBUG_MSG("publish longpush");
    char topic[100];
    sprintf(topic, "bars/%s/button", chipName);
    client.publish(topic, "long");
  }
  return result;
}

void setTimes()
{
  now = millis();
  millisSinceSync = now - lastSync;
  millisSinceBeat = now - lastBeat;
  millisSinceRequest = now - lastRequest;
  pattern.setMillisSinceBeat((double)millisSinceBeat);
  pattern.setStrobeTime(millis() - pattern.getStrobeStart());
}

void reactToMusic()
{
  setTimes();
  packetSize = Udp.parsePacket();
  remoteIP = Udp.remoteIP();

  //Lese das Packet und reagiere
  if (packetSize > 0)
  {
    if (packetSize <= MAX_PACKET_SIZE)
    {

      Udp.read(recvBuffer, packetSize);
      //Serial.print(recvBuffer);
      //Checke ob Synchronisierungspacket
      if (synchronising(recvBuffer, packetSize))
      {
        syncMesg.create(recvBuffer, packetSize);
        if (syncMesg.direction == '0')
        {
          pattern.setBeatPeriodMillis((double)syncMesg.beat_period_millis);
          pattern.setBeatDistinctiveness((double)syncMesg.beat_distinctivness);
          pattern.setMillisSinceBeat(0);
          millisSinceBeat = 0;
          lastBeat = millis();
        }
      }
    }
  }
  pattern.baseChoser();
  pattern.frontChoser();
  pattern.strobeChoser();
  FastLED.setCorrection(TypicalSMD5050);
  FastLED.show((uint8_t)pattern.getDimVal());
}

void setup()
{
#ifdef DEBUG_ESP_PORT
  Serial.begin(115200);
#endif
  setupChip();
  setupLEDs();
  setup_wifi();

  if (wifiMode)
  {
    setupMQTT();
    setupOTA();
    setupBeatListener();
    flash(5, CRGB::Green);
  }
  else
  {
    flash(5, CRGB::Fuchsia);
  }
}

void loop()
{
  if (wifiMode)
  {
    checkButton();
    client.loop();

    // to implement
    ArduinoOTA.handle();
    reactToMusic();
  }
  else
  {
    // noConnection show mode
  }
}