#pragma once
#include <memory>
#include <string>
#include "../MonitorTypes.h"

// Abstract base class for response parsers
class ResponseParser {
public:
    virtual ~ResponseParser() = default;
    virtual ParseResult parse(const std::string& data, const std::string& entity) = 0;
};

// Progress parser implementation
class ProgressResponseParser : public ResponseParser {
public:
    explicit ProgressResponseParser();
    ParseResult parse(const std::string& data, const std::string& entity) override;
};

// Progress parser implementation
class HeaterResponseParser : public ResponseParser {
public:
    explicit HeaterResponseParser();
    ParseResult parse(const std::string& data, const std::string& entity) override;
};

// Factory function declaration
std::unique_ptr<ResponseParser> createParser(Type type);