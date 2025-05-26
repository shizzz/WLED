#pragma once
#include <memory>
#include <variant>
#include <string>
#include "../MonitorTypes.h"

// Abstract base class for response parsers
class ResponseParser {
public:
    virtual ~ResponseParser() = default;
    virtual void parse(const std::string& data, PresetSettings* _preset) = 0;
};

// Progress parser implementation
class ProgressResponseParser : public ResponseParser {
public:
    explicit ProgressResponseParser();
    void parse(const std::string& data, PresetSettings* _preset) override;
};

// Progress parser implementation
class HeaterResponseParser : public ResponseParser {
public:
    explicit HeaterResponseParser();
    void parse(const std::string& data, PresetSettings* _preset) override;
};

// Factory function declaration
std::unique_ptr<ResponseParser> createParser(Type type);