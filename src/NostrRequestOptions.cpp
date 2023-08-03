#include "NostrRequestOptions.h"

/**
 * @brief Constructs a new NostrRequestOptions object with default values.
 *
 * Initializes the NostrRequestOptions object with default values for all its members.
 * This constructor takes no arguments.
 *
 * @param ids Pointer to an array of ids, defaults to nullptr.
 * @param ids_count Number of ids in the ids array, defaults to 0.
 * @param authors Pointer to an array of authors, defaults to nullptr.
 * @param authors_count Number of authors in the authors array, defaults to 0.
 * @param kinds Pointer to an array of kinds, defaults to nullptr.
 * @param kinds_count Number of kinds in the kinds array, defaults to 0.
 * @param e Pointer to an array of events, defaults to nullptr.
 * @param e_count Number of events in the e array, defaults to 0.
 * @param p Pointer to an array of pointers, defaults to nullptr.
 * @param p_count Number of pointers in the p array, defaults to 0.
 * @param since The starting timestamp for the request, defaults to -1.
 * @param until The ending timestamp for the request, defaults to -1.
 * @param limit The maximum number of results to return, defaults to -1.
 */
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