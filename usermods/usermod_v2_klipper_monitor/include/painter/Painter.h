#pragma once
#include <memory>
#include <variant>
#include "../MonitorTypes.h"

// Abstract base class for response parsers
class Painter {
public:
    virtual ~Painter() = default;
    virtual void paint(const PresetSettings& settings, const ParseResult& parseResult) = 0;

    static void doPaintLinear(const PresetSettings& settings, unsigned int n, bool toPaint);
};

// Progress parser implementation
class NormalPainter : public Painter {
public:
    explicit NormalPainter();
    void paint(const PresetSettings& settings, const ParseResult& parseResult) override;
};

// Reversed parser implementation
class ReversedPainter : public Painter {
public:
    explicit ReversedPainter();
    void paint(const PresetSettings& settings, const ParseResult& parseResult) override;
};

// Center parser implementation
class CenterPainter : public Painter {
public:
    explicit CenterPainter();
    void paint(const PresetSettings& settings, const ParseResult& parseResult) override;
};

// Center parser implementation
class BrightressPainter : public Painter {
public:
    explicit BrightressPainter();
    void paint(const PresetSettings& settings, const ParseResult& parseResult) override;
};

// Factory function declaration
std::unique_ptr<Painter> createPainter(Effect effect);