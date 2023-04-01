#include <Arduino.h>
#include "WiFiClientSecure.h"
#include "time.h"
#include "Button2.h"
#include <NostrEvent.h>
#include <NostrRelayManager.h>

#include <TFT_eSPI.h>

#define BUTTON_1            35
#define BUTTON_2            0

#define LOGGING_ENABLED     false

const char* ssid     = "wubwub"; // wifi SSID here
const char* password = "blob19750405blob"; // wifi password here

NostrEvent nostr;
NostrRelayManager nostrRelayManager;

bool hasSentEvent = false;

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

TFT_eSPI tft = TFT_eSPI(135, 240);

Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);

String serialisedEventData;

char const *nsecHex = "bdd19cecd942ed8964c2e0ddc92d5e09838d3a09ebb230d974868be00886704b";
char const *npubHex = "d0bfc94bd4324f7df2a7601c4177209828047c4d3904d64009a3c67fb5d5e7ca";
// BC
char const *testRecipientPubKeyHex = "683211bd155c7b764e4b99ba263a151d81209be7a566a2bb1971dc1bbd3b715e";

String buttonMessage = "No button was pressed";

void writeToDisplay(String text) {
    tft.setTextSize(2);
    tft.fillScreen(TFT_BLACK);
    tft.setTextWrap(true, true);
    tft.drawString(text, 5, 5);
}

unsigned long getUnixTimestamp(){
  time_t now;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return 0;
  } else {
    Serial.println("Got timestamp of " + String(now));
  }
  time(&now);
  return now;
}

void button_init()
{
    btn1.setPressedHandler([](Button2 & b) {
        long timestamp = getUnixTimestamp();
        buttonMessage = "Button 1 was pressed";
        // nostr.getNote(buttonMessage);
        serialisedEventData = nostr.getEncryptedDm(nsecHex, npubHex, testRecipientPubKeyHex, timestamp, buttonMessage);
        hasSentEvent = false;
        writeToDisplay(buttonMessage);
    });

    btn2.setPressedHandler([](Button2 & b) {
        long timestamp = getUnixTimestamp();
        buttonMessage = "Button 2 was pressed. This is a really long message to test something or other. Button 2 was pressed. This is a really long message to test something or other. Button 2 was pressed. This is a really long message to test something or other.";
        serialisedEventData = nostr.getEncryptedDm(nsecHex, npubHex, testRecipientPubKeyHex, timestamp, buttonMessage);
        hasSentEvent = false;
        writeToDisplay(buttonMessage);
    });
}

void customWebSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] CUSTOM Disconnected.\n");
    break;
    case WStype_CONNECTED:
    {
      Serial.printf("[WSc] CUSTOM Connected.\n");
    }
    break;
    case WStype_TEXT:
    {
      Serial.printf("[WSc]: CUSTOM Received text: %s\n", payload);
    }
    break;
    case WStype_ERROR:
      Serial.printf("[WSc] CUSTOM Error: %s\n", payload);
    default:
    break;
  }
}

void setup() {
  Serial.begin(115200);

  writeToDisplay("I AM BECOME NOSTR!");

  tft.init();
  tft.setRotation(1);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  writeToDisplay("WiFi Connected :)");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  long timestamp = getUnixTimestamp();

  //serialisedEventData = nostr.getEncryptedDm(nsecHex, npubHex, testRecipientPubKeyHex, timestamp, "Running NIP04!");
  serialisedEventData = nostr.getNote(nsecHex, npubHex, timestamp, "This is a public note!");

    const char *const relays[] = {
      "relay.damus.io",
      "nostr.mom",
      "relay.nostr.bg"
  };
    int relayCount = sizeof(relays) / sizeof(relays[0]);
    Serial.println("relayCount: " + String(relayCount));
    
    nostr.setLogging(false);
    nostrRelayManager.setRelays(relays, relayCount);
    nostrRelayManager.connect(customWebSocketEvent);
  
  button_init();
}

void button_loop()
{
    btn1.loop();
    btn2.loop();
}

void loop() {
  button_loop();

  nostrRelayManager.loop();

  // Send some data to the WebSocket server
  if(!hasSentEvent && nostrRelayManager.isConnected) {
    Serial.println("Sending data to web socket");
    nostrRelayManager.broadcastEvent(serialisedEventData);
    hasSentEvent = true;
  }
}
