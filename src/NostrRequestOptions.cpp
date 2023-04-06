#include "NostrRequestOptions.h"

NostrRequestOptions::NostrRequestOptions()
    : ids(nullptr),
      ids_count(0),
      authors(nullptr),
      authors_count(0),
      kinds(nullptr),
      kinds_count(0),
      e(nullptr),
      e_count(0),
      p(nullptr),
      p_count(0),
      since(-1),
      until(-1),
      limit(-1) {}

/**
 * @brief serialise a NostrRequestOptions object to a JSON string
 * 
 * @return String 
 */
String NostrRequestOptions::toJson() const {
  StaticJsonDocument<1024> json;

  if (ids) {
    JsonArray idsArray = json.createNestedArray("ids");
    for (int i = 0; i < ids_count; i++) {
      idsArray.add(ids[i]);
    }
  }

  if (authors) {
    JsonArray authorsArray = json.createNestedArray("authors");
    for (int i = 0; i < authors_count; i++) {
      authorsArray.add(authors[i]);
    }
  }

  if (kinds) {
    JsonArray kindsArray = json.createNestedArray("kinds");
    for (int i = 0; i < kinds_count; i++) {
      kindsArray.add(kinds[i]);
    }
  }

  if (e) {
    JsonArray eArray = json.createNestedArray("#e");
    for (int i = 0; i < e_count; i++) {
      eArray.add(e[i]);
    }
  }

  if (p) {
    JsonArray pArray = json.createNestedArray("#p");
    for (int i = 0; i < p_count; i++) {
      pArray.add(p[i]);
    }
  }

  if (since != -1) {
    json["since"] = since;
  }
  if (until != -1) {
    json["until"] = until;
  }
  if (limit != -1) {
    json["limit"] = limit;
  }

  String jsonString;
  serializeJson(json, jsonString);

  return jsonString;
}