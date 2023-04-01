/*
  NostrRelayManager.h - Arduino library for Nostr.
  Created by Black Coffee <bc@omg.lol>, March 29th 2023
  Released under MIT License
*/
#ifndef NostrRelayManager_h
#define NostrRelayManager_h

#include <WebSocketsClient.h>

class NostrRelayManager
{
  public:
    NostrRelayManager();
    void setRelays(const char *const new_messages[], int size);
    void printRelay(int index) const;
    bool isConnected;
    void connect(std::function<void(WStype_t, uint8_t*, size_t)> callback = nullptr);
    void disconnect();
    void broadcastEvent(String serialisedEventJson);
    void loop();
    String getNewSubscriptionId();
  private:
    const char *const *relays;
    int relay_count;
    WebSocketsClient _webSocketClients[4];
    void _webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
};

#endif