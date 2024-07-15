#include <Arduino.h>
#include "WiFiClientSecure.h"
#include "time.h"
#include <NostrEvent.h>
#include <NostrRelayManager.h>
#include <TFT_eSPI.h>
#include <vector>

const char* ssid     = "<SSID>"; // wifi SSID here
const char* password = "<PASSWORD>"; // wifi password here

NostrEvent nostr;
NostrRelayManager nostrRelayManager;
NostrQueueProcessor nostrQueue;

bool hasSentEvent = false;

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

TFT_eSPI tft = TFT_eSPI(135, 240);

char const *nsecHex = "<SENDER PRIVATE KEY IN HEX FORMAT>"; // sender private key in hex e.g. bdd19cecdXXXXXXXXXXXXXXXXXXXXXXXXXX
char const *npubHex = "<SENDER PUBLIC KEY IN HEX FORMAT>"; // sender public key in hex e.g. d0bfc94bd4324f7df2a7601c4177209828047c4d3904d64009a3c67fb5d5e7ca
// BC
char const *testRecipientPubKeyHex = "<RECIPIENT PUB KEY IN HEX FORMAT>"; // e.g. // sender public key 683211bd155c7b764e4b99ba263a151d81209be7a566a2bb1971dc1bbd3b715e

void writeToDisplay(String text) {
    tft.setTextSize(2);
    tft.fillScreen(TFT_BLACK);
    tft.setTextWrap(true, true);
    tft.drawString(text, 5, 5);
}

unsigned long getUnixTimestamp() {
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

void okEvent(const std::string& key, const char* payload) {
    Serial.println("OK event");
    Serial.println("payload is: ");
    Serial.println(payload);
    writeToDisplay("OK event");
}

void nip01Event(const std::string& key, const char* payload) {
    Serial.println("NIP01 event");
    Serial.println("payload is: ");
    Serial.println(payload);
    writeToDisplay("NIP01");
    delay(1000);
    writeToDisplay(payload);
}

void nip04Event(const std::string& key, const char* payload) {
    Serial.println("NIP04 event");
    String dmMessage = nostr.decryptDm(nsecHex, payload);
    Serial.println("message is: ");
    Serial.println(dmMessage);
    writeToDisplay("NIP04");
    delay(1000);
    writeToDisplay(dmMessage);
}

void setup() {
  Serial.begin(115200);
  
  tft.init();
  tft.setRotation(1);
  writeToDisplay("OH HAI NOSTR!");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  long timestamp = getUnixTimestamp();

  std::vector<String> relays = {
    "relay.damus.io",
    "nostr.mom",
    "relay.nostr.bg"
  };
    
  nostr.setLogging(false);
  nostrRelayManager.setRelays(relays);
  nostrRelayManager.setMinRelaysAndTimeout(2,10000);

  // Set some event specific callbacks here
  nostrRelayManager.setEventCallback("ok", okEvent);
  nostrRelayManager.setEventCallback(1, nip01Event);
  nostrRelayManager.setEventCallback(4, nip04Event);

  nostrRelayManager.connect();

  String subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"authors\": [\"d0bfc94bd4324f7df2a7601c4177209828047c4d3904d64009a3c67fb5d5e7ca\"], \"kinds\": [1], \"limit\": 1}]";
  nostrRelayManager.enqueueMessage(subscriptionString.c_str());

  subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"#p\": [\"d0bfc94bd4324f7df2a7601c4177209828047c4d3904d64009a3c67fb5d5e7ca\"], \"kinds\": [4], \"limit\": 1}]";
  nostrRelayManager.enqueueMessage(subscriptionString.c_str());

  subscriptionString = nostr.getEncryptedDm(nsecHex, npubHex, testRecipientPubKeyHex, timestamp, "Running NIP04!");
  nostrRelayManager.enqueueMessage(subscriptionString.c_str());
}

void loop() {
  nostrRelayManager.loop();
  nostrRelayManager.broadcastEvents();
}