/*
  NostrRelayManager.h - Arduino library for Nostr.
  Created by Black Coffee <bc@omg.lol>, March 29th 2023
  Released under MIT License
*/
#include <NostrRelayManager.h>
#include <NostrEvent.h>
#include <WebSocketsClient.h>


NostrRelayManager::NostrRelayManager() {
}

void NostrRelayManager::setEventCallback(const std::string& key, EventCallbackFn callback) {
    m_callbacks[key] = callback;
}

void NostrRelayManager::performEventAction(const std::string& key, const char* payload) {
    auto it = m_callbacks.find(key);
    if (it != m_callbacks.end()) {
        it->second(key, payload); // pass in the argument to the callback function
    }
}

void NostrRelayManager::enqueueMessage(const char item[NostrQueueProcessor::MAX_ITEM_SIZE]) {
    m_queue.enqueue(item);
}

void NostrRelayManager::subscribe(String subscriptionJson) {
  broadcastEvent(subscriptionJson);
}

/**
 * @brief Set the relays to use
 * 
 * @param new_relays 
 * @param size 
 */
void NostrRelayManager::setRelays(const char *const new_relays[], int size) {
    relays = new_relays;
    relay_count = size;
}

/**
 * @brief Keep all relays active
 * 
 */
void NostrRelayManager::loop() {
  for (int i = 0; i < relay_count; i++) {
    _webSocketClients[i].loop();
  }
}

/**
 * @brief Broadcasts messages in m_queue
 * 
 */
void NostrRelayManager::broadcastEvents() {
  if (!m_queue.isEmpty() && isConnected) {
    Serial.println("Broadcasting events");
  }
  while (!m_queue.isEmpty() && isConnected) {
    const char *item = m_queue.dequeue();
    String message = "Processing item: " + String(item);
    Serial.println(message);
    broadcastEvent(String(item));
  }
}

/**
 * @brief Print the relay at index: index
 * 
 * @param index 
 */
void NostrRelayManager::printRelay(int index) const {
    if (index >= 0 && index < relay_count) {
        Serial.println("Relay is " + String(relays[index]));
    } else {
        Serial.println("Invalid index.");
    }
}

/**
 * @brief Connect to all specified relays
 * 
 * @param callback an optional callback to be called when a websocket event is received
 */
void NostrRelayManager::connect(std::function<void(WStype_t, uint8_t*, size_t)> callback) {
  // Initialize WebSocket connections
  for (int i = 0; i < relay_count; i++) {
    _webSocketClients[i].beginSSL(relays[i], 443);
    _webSocketClients[i].onEvent([this, callback](WStype_t type, uint8_t* payload, size_t length) {
      this->_webSocketEvent(type, payload, length);
      if (callback != nullptr) {
        callback(type, payload, length);
      }
    });
  }
  delay(500);
}

/**
 * @brief Disconnect from all websockets
 * 
 */
void NostrRelayManager::disconnect() {
  for (int i = 0; i < relay_count; i++) {
    _webSocketClients[i].disconnect();
    delay(1000);
  }
}

/**
 * @brief broadcast a serialised event object to all relays
 * 
 * @param serialisedEventJson 
 */
void NostrRelayManager::broadcastEvent(String serialisedEventJson) {
  for (int i = 0; i < relay_count; i++) {
    _webSocketClients[i].sendTXT(serialisedEventJson);
    delay(1000);
  }
}

/**
 * @brief Get a 64 character long string to be used when subscribing to events and updates
 * 
 * @return String 
 */
String NostrRelayManager::getNewSubscriptionId() {
  String chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  String result = "";
  int length = 64;
  for (int i = 0; i < length; i++) {
    int randomIndex = random(chars.length());
    result += chars[randomIndex];
  }
  return result;
}

/**
 * @brief The websocket event listener
 * 
 * @param type 
 * @param payload 
 * @param length 
 */
void NostrRelayManager::_webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected.\n");
    break;
    case WStype_CONNECTED:
    {
      Serial.printf("[WSc] Connected.\n");
      isConnected = true;
    }
    break;
    case WStype_TEXT:
    {
      Serial.printf("[WSc]: Received text: %s\n", payload);
      const char *payloadStr = (char *)payload;

      if(String(payloadStr).indexOf("\"OK\"") > 0) {
        performEventAction("ok", payloadStr);
      }
      else if(String(payloadStr).indexOf("\"EVENT\"") > 0 && String(payloadStr).indexOf("\"kind\":1") > 0) {
        performEventAction("nip01", payloadStr);
      }
      else if(String(payloadStr).indexOf("\"EVENT\"") > 0 && String(payloadStr).indexOf("\"kind\":4") > 0) {
        performEventAction("nip04", payloadStr);
      }
    }
    break;
    case WStype_ERROR:
      Serial.printf("[WSc] Error: %s\n", payload);
    default:
    break;
  }
}