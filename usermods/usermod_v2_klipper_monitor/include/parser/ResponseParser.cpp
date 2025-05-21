#include "ResponseParser.h"
#include "ProgressResponseParser.h"
#include <stdexcept>

// Factory function implementation
std::unique_ptr<ResponseParser> createParser(Mode mode) {
    switch (mode) {
        case PROGRESS:
            return std::make_unique<ProgressResponseParser>();
        default:
            return std::make_unique<ProgressResponseParser>();
    }
}