#ifndef NOSTR_REQUEST_OPTIONS_H
#define NOSTR_REQUEST_OPTIONS_H

#include <Arduino.h>
#include <ArduinoJson.h>

class NostrRequestOptions {
public:
  String* ids;
  int ids_count;

  String* authors;
  int authors_count;

  int* kinds;
  int kinds_count;

  String* e;
  int e_count;

  String* p;
  int p_count;

  long since;
  long until;
  int limit;

  String toJson() const;

  // Default constructor
  NostrRequestOptions();
};

#endif
