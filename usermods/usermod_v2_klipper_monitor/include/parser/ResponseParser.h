#pragma once
#include <memory>
#include <variant>
#include <string>
#include "../MonitorMode.h"

// Result type for parsing
using ParseResult = std::variant<float, std::string>;

// Abstract base class for response parsers
class ResponseParser {
public:
    virtual ~ResponseParser() = default;
    virtual ParseResult parse(const std::string& str) = 0;
};

// Factory function declaration
std::unique_ptr<ResponseParser> createParser(Mode mode);