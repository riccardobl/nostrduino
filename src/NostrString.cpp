#ifndef NOSTR_STRING
#define NOSTR_STRING 1

#include <string>
#ifdef ARDUINO
#    include <Arduino.h>
#    include <Bitcoin.h>
#    define NostrString String
long NostrString_toInt(const  String& str) { return str.toInt(); }
String NostrString_fromChars(char* chars) { return String(chars); }
bool NostrString_startsWith(const  String& str, const char* prefix) {
    return str.startsWith(prefix);
}
String NostrString_substring(const  String& str, int start, int end) {
    return str.substring(start, end);
}
String NostrString_substring(const  String& str, int start) {
    return str.substring(start);
}

 
String NostrString_intToString(long int i) { return String(i); }
int NostrString_indexOf(const  String& str1, const char*  str2) {
    return str1.indexOf(str2);
}
void NostrString_hexToBytes(const String& hex, uint8_t* array, size_t arraySize) {
    fromHex(hex, array, arraySize);
}
String NostrString_bytesToHex(uint8_t* array, size_t arraySize) {
    return toHex(array, arraySize);
}

long int NostrString_length(const  String& str) { return str.length(); }

const char* NostrString_toChars(const  String& str) { return str.c_str(); }

String NostrString_base64ToHex(const  String& b64) { return base64ToHex(b64); }

String NostrString_trim(const String& s) { 
    String ss=s;
    ss.trim(); 
    for(int i=0;i<ss.length();i++){
        if(ss[i]<32){
            ss.remove(i);
        }
    }
    return ss;     
}
#else
#    define NostrString std::string
long NostrString_toInt(const  std::string &str) { return std::stol(str); }
std::string NostrString_fromChars(char *chars) { return std::string(chars); }
bool NostrString_startsWith(const  std::string &str, const char *prefix) {
    return str.find(prefix) == 0;
}

std::string NostrString_substring(const  std::string &str, int start, int end) {
    return str.substr(start, end);
}

std::string NostrString_substring(const  std::string &str, int start) {
    return str.substr(start);
}

std::string NostrString_intToString(long int i) { return std::to_string(i); }

int NostrString_indexOf(const  std::string &str1, const char* str2) {
    return str1.find(str2);
}

void NostrString_hexToBytes(const  std::string &hex, uint8_t *array,
                            size_t arraySize) {
    const char *chex = hex.c_str();
    fromHex(chex, strlen(chex), array, arraySize);
}

std::string NostrString_bytesToHex(uint8_t *array, size_t arraySize) {
    if (array == NULL) {
        return std::string();
    }
    size_t outputSize = arraySize * 2 + 1;
    char *output = (char *)malloc(outputSize);
    if (output == NULL) {
        return std::string();
    }

    toHex(array, arraySize, output, outputSize);

    std::string result(output);

    memzero(output, outputSize);
    free(output);
    return result;
}
long int NostrString_length(const  std::string &str) { return str.length(); }

const char *NostrString_toChars(const  std::string &str) { return str.c_str(); }

std::string NostrString_base64ToHex(const  std::string& b64) {
    return base64ToHex(b64);
}

std::string NostrString_trim(const std::string &s) {
    std::string ss = s;
    ss.erase(std::remove_if(ss.begin(), ss.end(),
                            [](unsigned char c) {
                                return std::isspace(c) || c < 32 || c > 126;
                            }),
             ss.end());
    return ss;
}
#endif

#endif