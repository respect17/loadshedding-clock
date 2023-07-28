#ifndef STORAGE_H
#define STORAGE_H

#include "Arduino.h"
#include "LittleFS.h"
#include <ArduinoJson.h>
#include <Preferences.h>

void setupStorage();
void resetStorage();
String readString(char const *key);
void writeString(char const *key, String value);
int32_t readLong(char const *key);
void writeLong(char const *key, int32_t value);
void writeEvents(char* areaInfoJsonString);
DynamicJsonDocument readEvents();

#endif