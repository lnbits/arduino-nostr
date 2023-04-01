/*
  NostrRelayManager.h - Arduino library for Nostr.
  Created by Black Coffee <bc@omg.lol>, March 29th 2023
  Released under MIT License
*/
#ifndef NostrRelayManager_h
#define NostrRelayManager_h

#include <WebSocketsClient.h>
#include <map>
#include <functional>

#include "NostrQueueProcessor.h"

class NostrRelayManager
{
  public:
    using EventCallbackFn = std::function<void(const std::string&, const char*)>;
    using EventCallbackMap = std::map<std::string, EventCallbackFn>;

    NostrRelayManager();

    void subscribe(String subscriptionJson);
    void setEventCallback(const std::string& key, EventCallbackFn callback);
    void performEventAction(const std::string& key, const char *payload);

    void enqueueMessage(const char item[NostrQueueProcessor::MAX_ITEM_SIZE]);

    void setRelays(const char *const new_messages[], int size);
    void printRelay(int index) const;
    bool isConnected;
    void connect(std::function<void(WStype_t, uint8_t*, size_t)> callback = nullptr);
    void disconnect();
    void broadcastEvent(String serialisedEventJson);
    void broadcastEvents();
    void loop();
    String getNewSubscriptionId();
  private:
    EventCallbackMap m_callbacks;

    NostrQueueProcessor m_queue;

    const char *const *relays;
    int relay_count;
    WebSocketsClient _webSocketClients[4];
    void _webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
};

#endif