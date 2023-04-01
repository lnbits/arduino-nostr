/*
  NostrEvent.h - Arduino library for Nostr.
  Created by Black Coffee <bc@omg.lol>, March 29th 2023
  Released under MIT License
*/
#ifndef NostrEvent_h
#define NostrEvent_h

#include "Arduino.h"
#include "ArduinoJson.h"
#include <iostream>
#include <base64.h>
#include <aes.h>

#include "Bitcoin.h"
#include "Hash.h"

#include <stdint.h>
#include <stdlib.h>
#include <esp_system.h>
#include <esp_random.h>

class NostrEvent
{
  public:
    NostrEvent();
    String getNote(char const *privateKeyHex, char const *pubKeyHex, unsigned long timestamp, String content);
    String getEncryptedDm(char const *privateKeyHex, char const *pubKeyHex, char const *recipientPubKeyHex, unsigned long timestamp, String content);
    String decryptDm(const char *privateKeyHex, String serialisedJson);
    void setLogging(bool loggingEnabled);
  private:
    bool _isLoggingEnabled;
    String _decryptData(byte key[32], byte iv[16], String messageHex);
    void _logToSerialWithTitle(String title, String message);
    void _stringToByteArray(const char* input, int padding_diff, byte* output);
    String _encryptData(byte key[32], byte iv[16], String msg);
    String _getSerialisedEncryptedDmObject(const char *pubKeyHex, const char *recipientPubKeyHex, String msgHash, int timestamp, String encryptedMessageWithIv, String schnorrSig);
    String _getSerialisedEncryptedDmArray(char const *pubKeyHex, char const *recipientPubKeyHex, int timestamp, String encryptedMessageWithIv);
};

#endif