#ifndef _NOSTR_STRING_H
#define _NOSTR_STRING_H
#include <vector>

#ifdef ARDUINO
#include <Arduino.h>
#define NostrString String
double NostrString_toFloat(const String &str);
long NostrString_toInt(const String &str);
String NostrString_fromChars(char *chars);
bool NostrString_startsWith(const String &str, const char *prefix);

String NostrString_substring(const String &str, int start, int end);
String NostrString_substring(const String &str, int start);
String NostrString_intToString(long int i);
int NostrString_indexOf(const String &str1, const char *str2);
void NostrString_hexToBytes(const String &hex, uint8_t *array, size_t arraySize);
String NostrString_bytesToHex(uint8_t *array, size_t arraySize);
String NostrString_urlDecode(const String &encoded);
long int NostrString_length(const String &str);
bool NostrString_equals(const String &str1, const String &str2);

const char *NostrString_toChars(const String &str);

String NostrString_base64ToHex(const String &b64);

String NostrString_trim(const String &s);
void NostrString_split(const String &str, const char separator, std::vector<String> &result);
String NostrString_fromUInt(unsigned long long i);
String NostrString_urlEncode(const String &str);
#else
#include <string>
#define NostrString std::string
double NostrString_toFloat(const std::string &str);
long NostrString_toInt(const std::string &str);
std::string NostrString_fromChars(char *chars);
bool NostrString_startsWith(const std::string &str, const char *prefix);

std::string NostrString_substring(const std::string &str, int start, int end);

std::string NostrString_substring(const std::string &str, int start);

std::string NostrString_intToString(long int i);

int NostrString_indexOf(const std::string &str1, const char *str2);

void NostrString_hexToBytes(const std::string &hex, uint8_t *array, size_t arraySize);

std::string NostrString_bytesToHex(uint8_t *array, size_t arraySize);
long int NostrString_length(const std::string &str);
const char *NostrString_toChars(const std::string &str);

std::string NostrString_base64ToHex(const std::string &b64);
std::string NostrString_trim(const std::string &s);
void NostrString_split(const std::string &str, const char separator, std::vector<std::string> &result);
std::string NostrString_urlDecode(const std::string &encoded);

bool NostrString_equals(const std::string &str1, const std::string &str2);
std::string NostrString_fromUInt(unsigned long long i);
std::string NostrString_urlEncode(const std::string &str);
#endif

#endif