#include "ProgressResponseParser.h"
#include <stdexcept>
#include "wled.h"

ProgressResponseParser::ProgressResponseParser() : data() {}

ParseResult ProgressResponseParser::parse(const std::string& str) {
    PSRAMDynamicJsonDocument jsonResponse(4096);
    DeserializationError error = deserializeJson(jsonResponse, str);
    return jsonResponse[F("result")][F("status")][F("virtual_sdcard")][F("progress")].as<float>();
}