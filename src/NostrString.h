#ifndef NOSTR_STRING
#define NOSTR_STRING 1

#include <string>
#ifdef ARDUINO
#    include <Arduino.h>
#    include <Bitcoin.h>
#    define NostrString String
long NostrString_toInt(const  String& str);
String NostrString_fromChars(char* chars);
bool NostrString_startsWith(const  String& str, const char* prefix);

String NostrString_substring(const  String& str, int start, int end);
String NostrString_substring(const  String& str, int start);
String NostrString_intToString(long int i);
int NostrString_indexOf(const  String& str1, const char* str2);
void NostrString_hexToBytes(const  String& hex, uint8_t* array,
                            size_t arraySize);
String NostrString_bytesToHex(uint8_t* array, size_t arraySize);

long int NostrString_length(const  String& str);

const char* NostrString_toChars(const  String& str);

String NostrString_base64ToHex(const  String& b64);

String NostrString_trim(const String& s);
#else
#    define NostrString std::string
long NostrString_toInt(const  std::string &str);
std::string NostrString_fromChars(char *chars);
bool NostrString_startsWith(const  std::string& str, const char* prefix);

std::string NostrString_substring(const  std::string& str, int start, int end);

std::string NostrString_substring(const  std::string &str, int start);

 
std::string NostrString_intToString(long int i);

int NostrString_indexOf(const  std::string &str1, const char* str2);

void NostrString_hexToBytes(const  std::string &hex, uint8_t *array, size_t arraySize);

std::string NostrString_bytesToHex(uint8_t *array, size_t arraySize);
long int NostrString_length(const  std::string& str);
const char *NostrString_toChars(const  std::string& str);

std::string NostrString_base64ToHex(const  std::string& b64);
std::string NostrString_trim(const std::string& s);
#endif

#endif