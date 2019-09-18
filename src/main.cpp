#define VERSION 12

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
#define FOREGROUND_NUM_LEDS 74 //74
#define BACKGROUND_NUM_LEDS 74 //74
#define NUM_LEDS 148           //148
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
#define BRAINWIFI
#define WIFI_WAIT 30
#define MQTT_WAIT 5
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
int noWifiShowPattern = 1;
bool autoShowOn = false;

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
long now;
long lastSave;
long lastShowTime = 0;

/* ESP VARIABLES */
bool configMode = false;
bool configured = true;
bool wifiMode = false;
long startupMillis = 0;
char chipName[40];

bool findMeFlash = false;

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

/* TIME FUNCTIONS */
void setTimes()
{
  now = millis();
  pattern.setNow(now);
  millisSinceSync = now - lastSync;
  millisSinceBeat = now - lastBeat;
  pattern.setMillisSinceBeat((double)millisSinceBeat);
  pattern.setStrobeTime(millis() - pattern.getStrobeStart());
}

/* LED FUNCTIONS */
void flash(int times, CRGB color)
{
  for (int i = 0; i < times; i++)
  {
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show(100);
    delay(250);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show(100);
    delay(250);
  }
}

void flashLoop(CRGB color)
{
  static long lastBlink = millis();
  static bool ledOn = false;

  if (now - lastBlink > 500)
  {
    DEBUG_MSG(".");
    lastBlink = now;
    if (!ledOn)
    {
      fill_solid(leds, NUM_LEDS, color);
      FastLED.show(100);
      ledOn = true;
    }
    else
    {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show(100);
      ledOn = false;
    }
  }
}

void flashOnboard(int n)
{
  for (int i = 0; i < n; i++)
  {
    digitalWrite(ONBOARDLED, LOW);
    delay(100);
    digitalWrite(ONBOARDLED, HIGH);
    delay(100);
  }
}

void nextShow()
{
  noWifiShowPattern++;
  if (noWifiShowPattern > 167)
  {
    noWifiShowPattern = 0;
  }
  pattern.patternChooser(noWifiShowPattern);
}

void selectShow(int show)
{
  if (show > 167)
  {
    show = 167;
  }
  else if (show < 0)
  {
    show = 0;
  }
  pattern.patternChooser(show);
}

void autoShow()
{
  static long lastChange = 0;
  if (autoShowOn)
  {
    if (lastChange - now > 300000)
    {
      nextShow();
      lastChange = now;
    }
  }
}

void toggleAutoShow()
{
  if (autoShowOn)
  {
    autoShowOn = false;
    flash(1, CRGB::Red);
  }
  else
  {
    autoShowOn = true;
    flash(1, CRGB::Green);
  }
}

void DEBUG_MQTT(const char *message)
{
  char topic1[100];
  sprintf(topic1, "LLBars/%s/debug", chipName);
  client.publish(topic1, message);
}

void setGroup(int newGroup)
{
  char topic0[50];
  sprintf(topic0, "LLBars/groups/%i/maxPos", group);
  client.unsubscribe(topic0);

  pattern.setGroup(newGroup);
  sprintf(topic0, "LLBars/groups/%i/maxPos", newGroup);
  client.subscribe(topic0);

  group = newGroup;
}

void setPosition(int newPosition)
{
  pattern.setPosition(newPosition);
  position = newPosition;
}

/* SETUP FUNCTIONS */
void setupChip()
{
  pinMode(BUTTONPIN, INPUT_PULLUP);
  pinMode(ONBOARDLED, OUTPUT);
  int32_t chipInteger = ESP.getChipId();
  sprintf(chipName, "bar_%08X_v%i", chipInteger, VERSION);
  DEBUG_MSG("CHIP NAME: %s \n", chipName);
}

void setupLEDs()
{
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  flash(3, CRGB::Red);
  //pattern.getValues();
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
  now = 0;
}

void setupOTA()
{
  DEBUG_MSG("Setting up OTA...\n");
  ArduinoOTA.setHostname(chipName);
  ArduinoOTA.onStart([]() {
    DEBUG_MSG("Start updating ");
    fill_solid(leds, NUM_LEDS, CRGB::Yellow);
    FastLED.show(100);
  });
  ArduinoOTA.onEnd([]() {
    DEBUG_MSG("End \n");
    flash(3, CRGB::Azure);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int percent = progress / (total / 100);
    int ID = (int)74.0 * (percent / 100.0);
    if ((ID < NUM_LEDS) && (ID > 0))
    {

      fill_solid(leds, NUM_LEDS, CRGB::Black);
      leds[ID] = CRGB::Green;
      FastLED.show(100);
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show(100);
    delay(3000);
    DEBUG_MSG("Error[%u]: \n", error);
  });
  ArduinoOTA.begin();
  DEBUG_MSG("OTA READY! \n");
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  DEBUG_MSG("Connecting to %s", ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  wifiMode = true;
  long startConnection = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    setTimes();
    flashLoop(CRGB::Red);
    if (now - startConnection > (WIFI_WAIT * 1000))
    {
      wifiMode = false;
      break;
    }
    yield();
  }
  if (wifiMode)
  {
    flash(3, CRGB::ForestGreen);
    DEBUG_MSG("WiFi connected\n");
  }
  else
  {
    flash(3, CRGB::Fuchsia);
    DEBUG_MSG("NO WIFI FOUND\n");
  }
  // DEBUG_MSG("IP address: %u", WiFi.localIP().toString());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  DEBUG_MSG("topic: %s \n", topic);
  DEBUG_MSG("message: %s \n", (char *)payload);
  if (strstr(topic, "LLBars/groups") != NULL)
  {
    if (strstr(topic, "maxPos") != NULL)
    {
      char value[20];
      strncpy(value, (char *)payload, length);
      int maxPos = atoi(value);
      pattern.setMaxPositionNumber(maxPos);
      DEBUG_MSG("MaxPos Set: %i\n", group);
    }
    else if (strstr(topic, "maxGroups") != NULL)
    {
      char value[20];
      strncpy(value, (char *)payload, length);
      int maxGroup = atoi(value);
      pattern.setMaxGroupNumber(maxGroup);
      DEBUG_MSG("MaxGroup Set: %i\n", maxGroup);
    }
  }
  else if (strstr(topic, "group") != NULL)
  {
    if (strstr(topic, "set") != NULL)
    {
      // Always accept group or position number!
      char value[20];
      strncpy(value, (char *)payload, length);
      int newGroup = atoi(value);
      setGroup(newGroup);
      DEBUG_MSG("Group Set: %u\n", newGroup);
      configured = true;
    }
  }
  else if (strstr(topic, "position") != NULL)
  {
    if (strstr(topic, "set") != NULL)
    {
      // Always accept group or position number!
      char value[20];
      strncpy(value, (char *)payload, length);
      int newPosition = atoi(value);
      setPosition(newPosition);
      DEBUG_MSG("Position Set: %u\n", position);
      configured = true;
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
      configured = false;
      flash(3, CRGB::Yellow);
    }
    else if (strstr(value, "normal") != NULL)
    {
      configMode = false;
      DEBUG_MSG("Set mode to normal mode");
      configured = true;
      flash(3, CRGB::Blue);
    }
    else if (strstr(value, "reset") != NULL)
    {
      configMode = false;
      DEBUG_MSG("Reset group and position");
      group = 0;
      position = 0;
      pattern.setGroup(0);
      pattern.setPosition(0);
      pattern.setMaxGroupNumber(0);
      pattern.setMaxPositionNumber(0);
      flash(3, CRGB::HotPink);
    }
  }
  else if (strstr(topic, "Pattern") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    int patternNumber = map(rawNumber, 0, 65536, 0, 94);
    pattern.patternChooser(patternNumber);
    DEBUG_MSG("SET PATTERN NUMBER TO: %i \n", patternNumber);
  }
  else if (strstr(topic, "Dimm") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    int dimFactor = map(rawNumber, 0, 65536, 0, 255);
    pattern.setDimVal(dimFactor);
    DEBUG_MSG("SET DIM FACTOR TO: %i \n", dimFactor);
  }
  else if (strstr(topic, "Color") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    int colorNumber = map(rawNumber, 0, 65536, 0, 100);
    pattern.colorChooser(colorNumber);
    DEBUG_MSG("SET COLOR NUMBER TO: %i \n", colorNumber);
  }
  else if (strstr(topic, "Speed") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    int number = map(rawNumber, 0, 65536, 0, 28);
    pattern.speedChooser(number);
    DEBUG_MSG("SET SPEED TO: %i \n", number);
  }
  else if (strstr(topic, "basespeed") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    int number = map(rawNumber, 0, 65536, 0, 255);
    pattern.setNbaseSpeed(number);
    DEBUG_MSG("SET basespeed TO: %i \n", number);
    pattern.saveValues();
  }
  else if (strstr(topic, "frontspeed") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    int number = map(rawNumber, 0, 65536, 0, 255);
    pattern.setNfrontSpeed(number);
    DEBUG_MSG("SET frontspeed TO: %i \n", number);
    pattern.saveValues();
  }
  else if (strstr(topic, "basedim") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    int number = map(rawNumber, 0, 65536, 0, 255);
    pattern.setNbaseDim(number);
    DEBUG_MSG("SET basedim TO: %i \n", number);
    pattern.saveValues();
  }
  else if (strstr(topic, "frontdim") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    int number = map(rawNumber, 0, 65536, 0, 255);
    pattern.setNfrontDim(number);
    DEBUG_MSG("SET frontdim TO: %i \n", number);
    pattern.saveValues();
  }
  else if (strstr(topic, "strobespeed") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    int number = map(rawNumber, 0, 65536, 0, 255);
    pattern.setNstrobeSpeed(number);
    DEBUG_MSG("SET strobespeed TO: %i \n", number);
    pattern.saveValues();
  }
  else if (strstr(topic, "strobedim") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    int number = map(rawNumber, 0, 65536, 0, 255);
    pattern.setNstrobeDim(number);
    DEBUG_MSG("SET strobedim TO: %i \n", number);
    pattern.saveValues();
  }
  else if (strstr(topic, "showpattern") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    int number = map(rawNumber, 0, 65536, 0, 167);
    selectShow(number);
    DEBUG_MSG("SET showpattern TO: %i \n", number);
    char message[100];
    sprintf(message, "showpattern: %i", number);
    DEBUG_MQTT(message);
  }
  else if (strstr(topic, "activate") != NULL)
  {
    pattern.animationActive = !pattern.animationActive;
  }
  else if (strstr(topic, "LLBars/basepat") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    if (rawNumber < 0)
      rawNumber = 0;
    if (rawNumber > 9)
      rawNumber = 9;
    pattern.setNbasePattern(rawNumber);
    pattern.setSettings();
  }
  else if (strstr(topic, "LLBars/frontpat") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    if (rawNumber < 0)
      rawNumber = 0;
    if (rawNumber > 9)
      rawNumber = 9;
    pattern.setNfrontPattern(rawNumber);
  }
  else if (strstr(topic, "LLBars/strobepat") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    if (rawNumber < 0)
      rawNumber = 0;
    if (rawNumber > 4)
      rawNumber = 4;
    pattern.setNstrobePattern(rawNumber);
  }
  else if (strstr(topic, "flash") != NULL)
  {
    char value[20];
    strncpy(value, (char *)payload, length);
    int rawNumber = atoi(value);
    if (rawNumber == 1)
    {
      findMeFlash = true;
    }
    else
    {
      findMeFlash = false;
    }
  }
}

void setupMQTT(bool reconnect = false, int connTimes = 0)
{
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.connect(chipName);
  DEBUG_MSG("Connecting to MQTT Broker...");

  long startConnecting = millis();
  while (!client.connected())
  {
    setTimes();

    if (connTimes == 0)
    {
      flashLoop(CRGB::Blue);
    }
    else
    {
      flashLoop(CRGB::Orange);
    }

    if (millis() - startConnecting > (MQTT_WAIT * 1000))
    {
      if (reconnect)
      {
        if (connTimes < 2)
        {
          connTimes++;
          setupMQTT(true, connTimes);
        }
        else
        {
          ESP.restart();
        }
      }
      wifiMode = false;
      return;
    }
    yield();
  }

  char topic0[100];
  sprintf(topic0, "LLBars/hello");
  client.publish(topic0, chipName);

  char topic1[100];
  sprintf(topic1, "LLBars/%s/position/set", chipName);
  client.subscribe(topic1);
  DEBUG_MSG("Subscribed to:");
  DEBUG_MSG(topic1);

  char topic2[100];
  sprintf(topic2, "LLBars/%s/group/set", chipName);
  client.subscribe(topic2);
  DEBUG_MSG("Subscribed to:");
  DEBUG_MSG(topic2);

  char topic3[100];
  sprintf(topic3, "LLBars/groups/%i/maxPos", group);
  client.subscribe(topic3);
  DEBUG_MSG("Subscribed to:");
  DEBUG_MSG(topic3);

  client.subscribe("LLBars/groups/maxGroups");
  DEBUG_MSG("Subscribed to:");
  DEBUG_MSG(topic3);

  client.subscribe("LLBars/Dimm");
  DEBUG_MSG("Subscribed to: LLBars/Dimm\n");

  client.subscribe("LLBars/Pattern");
  DEBUG_MSG("Subscribed to: LLBars/Pattern\n");

  client.subscribe("LLBars/Color");
  DEBUG_MSG("Subscribed to: LLBars/Color\n");

  client.subscribe("LLBars/Speed");
  DEBUG_MSG("Subscribed to: LLBars/Speed\n");

  client.subscribe("LLBars/basespeed");
  DEBUG_MSG("Subscribed to: LLBars/basespeed\n");

  client.subscribe("LLBars/frontspeed");
  DEBUG_MSG("Subscribed to: LLBars/frontspeed\n");

  client.subscribe("LLBars/basedim");
  DEBUG_MSG("Subscribed to: LLBars/basedim\n");

  client.subscribe("LLBars/frontdim");
  DEBUG_MSG("Subscribed to: LLBars/frontdim\n");

  client.subscribe("LLBars/strobespeed");
  DEBUG_MSG("Subscribed to: LLBars/strobespeed\n");

  client.subscribe("LLBars/strobedim");
  DEBUG_MSG("Subscribed to: LLBars/strobedim\n");

  client.subscribe("LLBars/showpattern");
  DEBUG_MSG("Subscribed to: LLBars/showpattern\n");

  client.subscribe("LLBars/basepat");
  DEBUG_MSG("SUBSCRIBED TO LLBars/basepat");

  client.subscribe("LLBars/frontpat");
  DEBUG_MSG("SUBSCRIBED TO LLBars/frontpat");

  client.subscribe("LLBars/strobepat");
  DEBUG_MSG("SUBSCRIBED TO LLbars/strobepat");

  sprintf(topic1, "LLBars/%s/flash", chipName);
  client.subscribe(topic1);
  DEBUG_MSG("Subscribed to:");
  DEBUG_MSG(topic1);

  client.subscribe("activate");
  DEBUG_MSG("Subscribed to: activate\n");

  client.subscribe("brain/mode");
  DEBUG_MSG("MQTT Setup DONE!");
}

int checkButton()
{
  static bool buttonPushed = false;
  static long buttonPressStart;
  int result;

  if (digitalRead(BUTTONPIN) == LOW)
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
      if (configMode)
      {
        DEBUG_MSG("publish shortpush");
        char topic[100];
        sprintf(topic, "LLBars/%s/button", chipName);
        client.publish(topic, "short");
      }
      else
      {
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        fill_solid(leds + 4, position + 1, CRGB::Red);
        fill_solid(leds + 14, group + 1, CRGB::Blue);
        FastLED.show();
        delay(2000);
        //flash(3, CRGB::White);
      }
    }
    else
    {
      nextShow();
      DEBUG_MSG("NEXT SHOW");
    }
  }
  else if (result == LONGPUSH)
  {
    if (wifiMode)
    {
      if (configMode)
      {
        DEBUG_MSG("publish longpush");
        char topic[100];
        sprintf(topic, "LLBars/%s/button", chipName);
        client.publish(topic, "long");
      }
    }
    else
    {
      toggleAutoShow();
      DEBUG_MSG("AUTOSHOW TOGGLED");
    }
  }
  return result;
}

void reactToMusic()
{

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
          DEBUG_MSG("REAL BEAT \n");
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
  if (now - lastShowTime > 5)
  {
    lastShowTime = now;
    pattern.setLastShowTime(lastShowTime);
    FastLED.setCorrection(TypicalSMD5050);
    FastLED.show((uint8_t)pattern.getDimVal());
  }
}

void lightshow()
{

  setTimes();

  if (millisSinceBeat > 2000)
  {
    pattern.setBeatPeriodMillis((double)500.0);
    pattern.setMillisSinceBeat((double)0.0);
    millisSinceBeat = 0.0;
    lastBeat = now;
    DEBUG_MSG("FAKE BEAT\n");
  }

  pattern.baseChoser();

  pattern.frontChoser();

  pattern.strobeChoser();
  if (now - lastShowTime > 5)
  {
    lastShowTime = now;
    pattern.setLastShowTime(lastShowTime);
    FastLED.setCorrection(TypicalSMD5050);
    FastLED.show((uint8_t)pattern.getDimVal());
  }
}

void blinkLed()
{
  static long lastBlink = millis();
  static bool ledOn = false;
  if (now - lastBlink > 1000)
  {
    lastBlink = now;
    if (!ledOn)
    {
      digitalWrite(ONBOARDLED, HIGH);
      ledOn = true;
      //DEBUG_MSG("ON");
    }
    else
    {
      digitalWrite(ONBOARDLED, LOW);
      ledOn = false;
      //DEBUG_MSG("OFF");
    }
  }
}

void connectionCheck()
{
  static long lastTryWifi = 0;
  static long lastTryMQTT = 0;
  //long now = millis();
  if (WiFi.status() != WL_CONNECTED)
  {
    if (now - lastTryWifi > 15000)
    {
      WiFi.reconnect();
    }
    lastTryWifi = now;
  }

  if (!client.connected())
  {
    if (now - lastTryMQTT > 15000)
    {
      setupMQTT(true);
    }
    lastTryMQTT = now;
  }
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
  startupMillis = millis();
}

void loop()
{
  setTimes();

  blinkLed();
  if (wifiMode)
  {
    checkButton();
    client.loop();
    ArduinoOTA.handle();

    if (configMode)
    {
      if (digitalRead(BUTTONPIN) == LOW)
      {
        flashLoop(CRGB::Tomato);
      }
      else
      {
        if (configured)
        {
          fill_solid(leds, NUM_LEDS, CRGB::Black);
          fill_solid(leds + 5, pattern.getPosition() + 1, CRGB::Red);
          fill_solid(leds + 15, pattern.getGroup() + 1, CRGB::Blue);
          if (now - lastShowTime > 5)
          {
            lastShowTime = now;
            FastLED.show();
          }
        }
        else
        {
          flashLoop(CRGB::Gold);
        }
      }
    }
    else
    {
      if (findMeFlash)
      {
        pattern.ballAFAP();
        if (now - lastShowTime > 5)
        {
          lastShowTime = now;
          FastLED.show();
        }
      }
      else
      {
        reactToMusic();
      }
    }
    //lightshow();
    connectionCheck();
  }
  else
  {
    // noConnection show mode
    checkButton();
    lightshow();
  }
}