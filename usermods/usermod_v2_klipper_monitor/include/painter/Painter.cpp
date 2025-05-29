#include "Painter.h"
#include <stdexcept>
#include "wled.h"

NormalPainter::NormalPainter() {}
ReversedPainter::ReversedPainter() {}
CenterPainter::CenterPainter() {}
BrightressPainter::BrightressPainter() {}

// Factory function implementation
std::unique_ptr<Painter> createPainter(Effect effect) {
    switch (effect) {
        case NORMAL:
            return std::make_unique<NormalPainter>();
        case REVERSED:
            return std::make_unique<ReversedPainter>();
        case CENTER:
            return std::make_unique<CenterPainter>();
        case BRIGHTNESS:
            return std::make_unique<BrightressPainter>();
        default:
            return std::make_unique<NormalPainter>();
    }
}

void Painter::doPaintLinear(const PresetSettings& settings, unsigned int n, bool toPaint)
{
    if (settings.cleanStripe)
    {
        if (!toPaint)
        {
            strip.setPixelColor(n, 0, 0, 0, 0);
        } else {
            if (!settings.useExistingColor) {
                strip.setPixelColor(n, settings.color.red, settings.color.green, settings.color.blue);
            }
        }
    } else {
        if (toPaint)
        {
            strip.setPixelColor(n, settings.color.red, settings.color.green, settings.color.blue);
        }
    }
}

void NormalPainter::paint(const PresetSettings& settings, const ParseResult& parseResult) {
    unsigned int total = settings.endPixel - settings.startPixel + 1;
    bool toPaint = false;

    for (unsigned int i = settings.startPixel; i <= settings.endPixel; i++) {
        toPaint = i + settings.startPixel < total * parseResult.progress;
        doPaintLinear(settings, i, toPaint);
    }

    return;
}

void ReversedPainter::paint(const PresetSettings& settings, const ParseResult& parseResult) {
    unsigned int total = settings.endPixel - settings.startPixel + 1;
    bool toPaint = false;

    for (unsigned int i = settings.startPixel; i <= settings.endPixel; i++) {
        toPaint = i + settings.startPixel > total * parseResult.progress;
        doPaintLinear(settings, i, toPaint);
    }

    return;
}

void CenterPainter::paint(const PresetSettings& settings, const ParseResult& parseResult) {
    unsigned int total = settings.endPixel - settings.startPixel + 1;
    unsigned int pixelsToPaint = total * parseResult.progress;
    unsigned int centerPixel = settings.startPixel + total / 2;
    unsigned int borderLeft = centerPixel - pixelsToPaint;
    unsigned int borderRight = centerPixel - pixelsToPaint;
    bool toPaint = false;

    for (unsigned int i = settings.startPixel; i <= settings.endPixel; i++)
    {
        toPaint = (pixelsToPaint >= 1) && (i + settings.startPixel > borderLeft || i + settings.startPixel < borderRight);
        doPaintLinear(settings, i, toPaint);
    }

    return;
}

void BrightressPainter::paint(const PresetSettings& settings, const ParseResult& parseResult) {
    uint8_t newBrigtness = settings.maxBrightness * parseResult.progress;
    for (unsigned int i = settings.startPixel; i <= settings.endPixel; i++)
    {
        strip.setBrightness(newBrigtness, true);
    }

    return;
}