#pragma once
#include "ResponseParser.h"

// Progress parser implementation
class ProgressResponseParser : public ResponseParser {
    std::string data;
public:
    explicit ProgressResponseParser();
    ParseResult parse(const std::string& str) override;
};