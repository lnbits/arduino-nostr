/*
  Nostr.h - Arduino library for Nostr.
  Created by Black Coffee <bc@omg.lol>, March 29th 2023
  Released under MIT License
*/
#include "Arduino.h"
#include "NostrEvent.h"
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
#include <esp_wifi.h>

NostrEvent::NostrEvent() {

}

/**
 * @brief Write debug data to the Serial output
 * 
 * @param title 
 * @param message 
 */
void NostrEvent::_logToSerialWithTitle(String title, String message) {
    if(_isLoggingEnabled) {
        Serial.println(title);
        Serial.println(message);
        Serial.println("-------");
    }
}

/**
 * @brief Enable or disable verbose logging?
 * 
 * @param loggingEnabled 
 */
void NostrEvent::setLogging(bool loggingEnabled) {
  _isLoggingEnabled = loggingEnabled;
}

/**
 * @brief Get a serialised string for a nostr note
 * 
 * @param privateKeyHex 
 * @param pubKeyHex 
 * @param timestamp 
 * @param content 
 * @return String 
 */
String NostrEvent::getNote(char const *privateKeyHex, char const *pubKeyHex, unsigned long timestamp, String content) {
    StaticJsonDocument<200> doc;
    JsonArray data = doc.createNestedArray("data");
    data.add(0);
    data.add(pubKeyHex);
    data.add(timestamp);
    data.add(1);
    data.add(doc.createNestedArray("tags"));
    data.add(content);

    // stringify event to message var
    String message;
    serializeJson(doc["data"], message);
    _logToSerialWithTitle("message is: ", String(message));

    // sha256 of message converted to hex, assign to msghash
    byte hash[64] = { 0 }; // hash
    int hashLen = 0;

    // Get the sha256 hash of the message
    hashLen = sha256(message, hash);
    String msgHash = toHex(hash, hashLen);
    _logToSerialWithTitle("SHA-256: ", msgHash);

    // Create the private key object
    int byteSize =  32;
    byte privateKeyBytes[byteSize];
    fromHex(privateKeyHex, privateKeyBytes, byteSize);
    PrivateKey privateKey(privateKeyBytes);

    // Generate the schnorr sig of the messageHash
    byte messageBytes[byteSize];
    fromHex(msgHash, messageBytes, byteSize);
    SchnorrSignature signature = privateKey.schnorr_sign(messageBytes);
    String signatureHex = String(signature);
    _logToSerialWithTitle("Schnorr sig is: ", signatureHex);

    // Device the public key and verify the schnorr sig is valid
    PublicKey pub = privateKey.publicKey();

    if(pub.schnorr_verify(signature, messageBytes)) {
        Serial.println("All good, signature is valid");
    } else {
        Serial.println("Something went wrong, signature is invalid");
    }

    // Generate the JSON object ready for broadcasting
    DynamicJsonDocument fullEvent(1024);
    fullEvent["id"] = msgHash;
    fullEvent["pubkey"] = pubKeyHex;
    fullEvent["created_at"] = timestamp;
    fullEvent["kind"] = 1;
    fullEvent["tags"] = doc.createNestedArray("test");
    fullEvent["content"] = content;
    fullEvent["sig"] = signature;

    // Serialize the array to JSON
    String json;
    serializeJson(fullEvent, json);

    String serialisedEventData = "[\"EVENT\"," + json + "]";
    // Print the JSON to the serial monitor
    _logToSerialWithTitle("Event JSON", serialisedEventData);
    return serialisedEventData;
}

String NostrEvent::decryptDm(const char *privateKeyHex, String serialisedJson) {
    // get the content
    StaticJsonDocument<2048> doc;
    deserializeJson(doc, serialisedJson);
    String serialisedTest;
    serializeJson(doc, serialisedTest);
    _logToSerialWithTitle("serialisedTest", serialisedTest);

    String content = doc[2]["content"];

    String encryptedMessage = content.substring(0, content.indexOf("?iv="));
    String encryptedMessageHex = base64ToHex(encryptedMessage);
    int encryptedMessageSize =  encryptedMessageHex.length() / 2;
    byte encryptedMessageBin[encryptedMessageSize];
    fromHex(encryptedMessageHex, encryptedMessageBin, encryptedMessageSize);
    _logToSerialWithTitle("encryptedMessage", encryptedMessage);
    _logToSerialWithTitle("encryptedMessageHex", encryptedMessageHex);

    String iv = content.substring(content.indexOf("?iv=") + 4);
    String ivHex = base64ToHex(iv);
    int ivSize =  16;
    byte ivBin[ivSize];
    fromHex(ivHex, ivBin, ivSize);
    _logToSerialWithTitle("iv", iv);
    _logToSerialWithTitle("ivHex", ivHex);

    int byteSize =  32;
    byte privateKeyBytes[byteSize];
    fromHex(privateKeyHex, privateKeyBytes, byteSize);
    PrivateKey privateKey(privateKeyBytes);

    String senderPubKeyHex = doc[2]["pubkey"];
    _logToSerialWithTitle("senderPubKeyHex", senderPubKeyHex);
    byte senderPublicKeyBin[64];
    fromHex("02" + String(senderPubKeyHex), senderPublicKeyBin, 64);
    PublicKey senderPublicKey(senderPublicKeyBin);
    _logToSerialWithTitle("senderPublicKey.toString() is", senderPublicKey.toString());

    byte sharedPointX[32];
    privateKey.ecdh(senderPublicKey, sharedPointX, false);
    String sharedPointXHex = toHex(sharedPointX, sizeof(sharedPointX));
    _logToSerialWithTitle("sharedPointXHex is", sharedPointXHex);

    String message = _decryptData(sharedPointX, ivBin, encryptedMessageHex);
    message.trim();

    _logToSerialWithTitle("message", message);

    return message;
}

String NostrEvent::_decryptData(byte key[32], byte iv[16], String messageHex) {
  int byteSize =  messageHex.length() / 2;
  byte messageBin[byteSize];
  fromHex(messageHex, messageBin, byteSize);

  AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key, iv);
  AES_CBC_decrypt_buffer(&ctx, messageBin, sizeof(messageBin));

  return String((char *)messageBin).substring(0, byteSize);
}

/**
 * @brief Get a serialised string for a nostr NIP04 encrypted DM
 * 
 * @param privateKeyHex 
 * @param pubKeyHex 
 * @param recipientPubKeyHex 
 * @param timestamp 
 * @param content 
 * @return String 
 */
String NostrEvent::getEncryptedDm(char const *privateKeyHex, char const *pubKeyHex, char const *recipientPubKeyHex, unsigned long timestamp, String content) {
    // Get shared point
    // Create the private key object
    int byteSize =  32;
    byte privateKeyBytes[byteSize];
    fromHex(privateKeyHex, privateKeyBytes, byteSize);
    PrivateKey privateKey(privateKeyBytes);

    byte publicKeyBin[64];
    fromHex("02" + String(recipientPubKeyHex), publicKeyBin, 64);
    PublicKey otherDhPublicKey(publicKeyBin);
    _logToSerialWithTitle("otherDhPublicKey.toString() is", otherDhPublicKey.toString());


    byte sharedPointX[32];
    privateKey.ecdh(otherDhPublicKey, sharedPointX, false);
    String sharedPointXHex = toHex(sharedPointX, sizeof(sharedPointX));
    _logToSerialWithTitle("sharedPointXHex is", sharedPointXHex);

    // Create the initialization vector
    esp_wifi_start();
    uint8_t iv[16];
    esp_random() % 256;
    for (int i = 0; i < sizeof(iv); i++) {
        iv[i] = esp_random() % 256;
    }

    String ivHex = toHex(iv, 16);
    _logToSerialWithTitle("ivHex is", ivHex);

    String ivBase64 = hexToBase64(ivHex);
    _logToSerialWithTitle("ivBase64 is", ivBase64);

    String encryptedMessageHex = _encryptData(sharedPointX, iv, content);
    _logToSerialWithTitle("encryptedMessage is", encryptedMessageHex);

    // divide the length of the hex string 2 to get the size of the byte array, since each byte consists of 2 hexadecimal characters.
    int encryptedMessageSize = encryptedMessageHex.length() / 2;
    uint8_t encryptedMessage[encryptedMessageSize];
    fromHex(encryptedMessageHex, encryptedMessage, encryptedMessageSize);

    String encryptedMessageBase64 = hexToBase64(encryptedMessageHex);
    _logToSerialWithTitle("encryptedMessageBase64 is", encryptedMessageBase64);

    encryptedMessageBase64 += "?iv=" + ivBase64;

    String message = _getSerialisedEncryptedDmArray(pubKeyHex, recipientPubKeyHex, timestamp, encryptedMessageBase64);

    byte hash[64] = { 0 }; // hash
    int hashLen = 0;

    // Get the sha256 hash of the message
    hashLen = sha256(message, hash);
    String msgHash = toHex(hash, hashLen);
    _logToSerialWithTitle("SHA-256:", msgHash);

    // Generate the schnorr sig of the messageHash
    SchnorrSignature signature = privateKey.schnorr_sign(hash);
    String signatureHex = String(signature);
    _logToSerialWithTitle("Schnorr sig is: ", signatureHex);

    String serialisedEventData = _getSerialisedEncryptedDmObject(pubKeyHex, recipientPubKeyHex, msgHash, timestamp, encryptedMessageBase64, signatureHex);
    _logToSerialWithTitle("serialisedEventData is", serialisedEventData);
    return serialisedEventData;
}

/**
 * @brief Convert a string to a byte array
 * 
 * @param input 
 * @param padding_diff 
 * @param output 
 */
void NostrEvent::_stringToByteArray(const char* input, int padding_diff, byte* output) {
    int i = 0;
    // remove end-of-string char
    while (input[i] != '\0') {
        output[i] = input[i];
        i++;
    }

    // pad between 1 and 16 bytes
    for (int j = 0; j < padding_diff; j++) {
        output[i + j] = padding_diff;
    }
}

/**
 * @brief AES CBC Encrpyt some data using a keys byte array and an initialisation vector
 * 
 * @param key 
 * @param iv 
 * @param msg 
 * @return String 
 */
String NostrEvent::_encryptData(byte key[32], byte iv[16], String msg) {
    // message has to be padded at the end so it is a multiple of 16
    int padding_diff = msg.length() % 16 == 0 ? 16 : 16 - (msg.length() % 16);

    int byteSize = msg.length() + padding_diff;
    byte messageBin[byteSize];
    _stringToByteArray(msg.c_str(), padding_diff, messageBin);

    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);

    AES_CBC_encrypt_buffer(&ctx, messageBin, sizeof(messageBin));

    return toHex(messageBin, sizeof(messageBin));
}

/**
 * @brief
 * 
 * @param pubKeyHex 
 * @param recipientPubKeyHex 
 * @param msgHash 
 * @param timestamp 
 * @param encryptedMessageWithIv 
 * @param schnorrSig 
 * @return String 
 */
String NostrEvent::_getSerialisedEncryptedDmObject(const char *pubKeyHex, const char *recipientPubKeyHex, String msgHash, int timestamp, String encryptedMessageWithIv, String schnorrSig) {
    // compute the required size
    const size_t CAPACITY = JSON_ARRAY_SIZE(6);
    // allocate the memory for the document
    StaticJsonDocument<1000> tagsDoc;
    // parse a JSON array
    String serialisedTagsArray = "[[\"p\",\"" + String(recipientPubKeyHex) + "\"]]";
    _logToSerialWithTitle("serialisedTagsArray is: ", serialisedTagsArray);
    deserializeJson(tagsDoc, serialisedTagsArray);

    // Generate the JSON object ready for broadcasting
    DynamicJsonDocument fullEvent(2000);
    fullEvent["id"] = msgHash;
    fullEvent["pubkey"] = pubKeyHex;
    fullEvent["created_at"] = timestamp;
    fullEvent["kind"] = 4;
    fullEvent["tags"] = tagsDoc;
    fullEvent["content"] = encryptedMessageWithIv;
    fullEvent["sig"] = schnorrSig;

    // Serialize the array to JSON
    String serialisedObject;
    serializeJson(fullEvent, serialisedObject);

    String serialisedEventObject = "[\"EVENT\"," + serialisedObject + "]";
    return serialisedEventObject;
}

/**
 * @brief Get a NIP04 message as an array that can be hashed to create a message ID
 * 
 * @param pubKeyHex 
 * @param recipientPubKeyHex 
 * @param timestamp 
 * @param encryptedMessageWithIv 
 * @return String 
 */
String NostrEvent::_getSerialisedEncryptedDmArray(char const *pubKeyHex, char const *recipientPubKeyHex, int timestamp, String encryptedMessageWithIv) {
    // compute the required size
    const size_t CAPACITY = JSON_ARRAY_SIZE(6);
    // allocate the memory for the document
    StaticJsonDocument<1000> tagsDoc;
    // parse a JSON array
    String serialisedTagsArray = "[[\"p\",\"" + String(recipientPubKeyHex) + "\"]]";
    _logToSerialWithTitle("serialisedTagsArray is: ", serialisedTagsArray);
    deserializeJson(tagsDoc, serialisedTagsArray);

    StaticJsonDocument<2000> doc;

    JsonArray data = doc.createNestedArray("data");

    data.add(0);
    data.add(pubKeyHex);
    data.add(timestamp);
    data.add(4);
    data.add(tagsDoc);
    data.add(encryptedMessageWithIv);

    // :61 stringify event to message var
    String message;
    serializeJson(doc["data"], message);
    _logToSerialWithTitle("message is: ", String(message));

    doc.clear();
    return message;
}