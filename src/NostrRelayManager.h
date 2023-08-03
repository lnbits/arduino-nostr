/*
  NostrRelayManager - Arduino library for Nostr.
  Created by Black Coffee <bc@omg.lol>, March 29th 2023
  Released under MIT License
*/
#ifndef NostrRelayManager_h
#define NostrRelayManager_h

#include <Arduino.h>
#include <WebSocketsClient.h>
#include <NostrQueueProcessor.h>
#include <NostrEvent.h>
#include "NostrRequestOptions.h"
#include <string>
#include <map>
#include <functional>
#include <vector>

class NostrRelayManager
{
  public:
    using EventCallbackFn = std::function<void(const std::string&, const char*)>;
    using EventCallbackMap = std::map<std::string, EventCallbackFn>;

    NostrRelayManager();

    void setMinRelaysAndTimeout(int minRelays, unsigned long minRelaysTimeout);

    void subscribe(String subscriptionJson);
    void requestEvents(const NostrRequestOptions* options);
    void setEventCallback(const std::string& key, EventCallbackFn callback);
    void setEventCallback(const int key, EventCallbackFn callback);

    void performEventAction(const std::string& key, const char *payload);

    void enqueueMessage(const char item[NostrQueueProcessor::MAX_ITEM_SIZE]);
    bool hasEnqueuedMessages();

    void setRelays(const std::vector<String>& new_relays);
    void printRelay(int index) const;
    void connect(std::function<void(WStype_t, uint8_t*, size_t)> callback = nullptr);
    void disconnect();
    void broadcastEvent(String serialisedEventJson);
    void broadcastEventToRelay(String serializedEventJson, String relayUrl);
    void broadcastEvents();
    void loop();
    String getNewSubscriptionId();
  private:
    EventCallbackMap m_callbacks;

    int minRelays;
    unsigned long minRelaysTimeout;
    unsigned long lastBroadcastAttempt;

    int connectedRelayCount();

    NostrQueueProcessor m_queue;

    std::vector<String> relays;
    int relay_count;
    int socketDisconnectionCount;
    WebSocketsClient _webSocketClients[4];
    void _webSocketEvent(WStype_t type, uint8_t* payload, size_t length, int relayIndex);
};

#endif