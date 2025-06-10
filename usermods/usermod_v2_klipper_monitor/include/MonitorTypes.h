#pragma once
#include <WString.h>

enum Type 
{ 
    NONE,
    PROGRESS,
    HEATER,
    TOOL,
    STEPPE
};

static const char* TypeStrings[] = 
{
    "NONE",
    "PROGRESS",
    "HEATER",
    "TOOL",
    "STEPPER"
};

enum Effect 
{
    NORMAL,
    REVERSED,
    CENTER,
    BRIGHTNESS
};

static const char* EffectStrings[] = 
{
    "NORMAL",
    "REVERSED",
    "CENTER",
    "BRIGHTNESS"
};

typedef struct settings_t {
    String name;
    Type type;
    String entity;
    Effect effect;
    struct color_t {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    } color;
    bool useExistingColor;
    uint8_t maxBrightness;
    unsigned int startPixel;
    unsigned int endPixel;
    bool cleanStripe;
} PresetSettings;

typedef struct result_t {
    float progress;
    uint8_t x;
    uint8_t y;
} ParseResult;