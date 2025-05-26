#include "ResponseParser.h"
#include <stdexcept>
#include "wled.h"

ProgressResponseParser::ProgressResponseParser() {}
HeaterResponseParser::HeaterResponseParser() {}

// Factory function implementation
std::unique_ptr<ResponseParser> createParser(Type type) {
    switch (type) {
        case PROGRESS:
            return std::make_unique<ProgressResponseParser>();
        case HEATER:
            return std::make_unique<HeaterResponseParser>();
        default:
            return std::make_unique<ProgressResponseParser>();
    }
}

void ProgressResponseParser::parse(const std::string& data, PresetSettings* _preset) {
    DEBUG_PRINTLN(F("Progress parser begin"));
    DEBUG_PRINTLN(data.c_str());

    PSRAMDynamicJsonDocument jsonResponse(4096);
    DeserializationError error = deserializeJson(jsonResponse, data);

    if (error) 
    { 
        DEBUG_PRINTLN(F("Json deserealization failed"));
        _preset->result = { 0, 0, 0 }; 
    }
    float progress = jsonResponse[F("result")][F("status")][_preset->entity.c_str()][F("progress")].as<float>();
    DEBUG_PRINTF("GOT %s state \r\n", entity.c_str());
    DEBUG_PRINTF("progress: %.3f%\r\n", progress);

    ParseResult result {
        progress,
        0,
        0
    };
    
    _preset->result = result;
}

void HeaterResponseParser::parse(const std::string& data, PresetSettings* _preset) {
    DEBUG_PRINTLN(F("Heater parser begin"));
    DEBUG_PRINTLN(data.c_str());
    PSRAMDynamicJsonDocument jsonResponse(4096);
    DeserializationError error = deserializeJson(jsonResponse, data);

    if (error) 
    { 
        DEBUG_PRINTLN(F("Json deserealization failed"));
        _preset->result = { 0, 0, 0 }; 
    }
    
    float state = jsonResponse[F("result")][F("status")][_preset->entity.c_str()][F("temperature")].as<float>();
    float target = jsonResponse[F("result")][F("status")][_preset->entity.c_str()][F("target")].as<float>();
    float progress;

    DEBUG_PRINTF("GOT %s state \r\n", _preset->entity.c_str());
    DEBUG_PRINTF("state: %.3f%\r\n", state);
    DEBUG_PRINTF("target: %.3f%\r\n", target);

    if (target < state)
    {
        progress = 1;
    } else {
        progress = state / target;
    }
    DEBUG_PRINTF("progress: %.3f%\r\n", progress);

    _preset->result = {
        progress,
        0,
        0
    };
}