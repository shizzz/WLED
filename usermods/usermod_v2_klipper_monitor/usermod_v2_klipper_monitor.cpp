#include "usermod_v2_klipper_monitor.h"

    void KlipperWatchdog::loop()
    {
        monitor.setMode(PROGRESS);
        monitor.update();
    }

    void KlipperWatchdog::setup() {}

    void KlipperWatchdog::handleOverlayDraw()
    {
        if (!monitor.isEnabled()) return;
        
        float progress = monitor.getProgress();
        DEBUG_PRINTF("Monitor set progress %.2f%%\r\n", progress * 100);
        uint32_t color = strip.getSegment(0).colors[1];
        int total = strip.getLengthTotal();
        
        switch (monitor.getDirection()) {
            case 1: // reversed
                for (int i = 0; i < total * progress; i++) {
                    strip.setPixelColor(total - 1 - i, color);
                }
                break;
                
            case 2: // center
                for (int i = 0; i < (total / 2) * progress; i++) {
                    strip.setPixelColor((total / 2) + i, color);
                    strip.setPixelColor((total / 2) - 1 - i, color);
                }
                break;
                
            default: // normal
                for (int i = 0; i < total; i++) {
                    if (i >= total * progress)
                    {
                        strip.setPixelColor(i, 0, 0, 0, 0);
                    }
                }
        }
    }

    void KlipperWatchdog::addToConfig(JsonObject &root) {
        JsonObject top = root.createNestedObject(F("Klipper Monitor"));
        top[F("Enabled")] = _enabled;
        top[F("IP")] = _ip;
        top[F("Port")] = _port;
        top[F("API Key")] = _apiKey;
        top[F("Direction")] = 0;
    }

    bool KlipperWatchdog::readFromConfig(JsonObject &root) {
        JsonObject top = root[F("Klipper Monitor")];
        if (top.isNull()) return false;

        _enabled = top[F("Enabled")].as<bool>() | false;
        _ip = top[F("IP")].as<String>();
        _port = top[F("Port")].as<uint16_t>();
        _apiKey = top[F("API Key")].as<String>();
        _direction = top[F("Direction")] | 0;

        monitor.setDirection(_direction);

        if (_ip == "0.0.0.0")
        {
            monitor.setEnabled(false);
        } else {
            monitor.setEnabled(_enabled);
            monitor.begin(_ip, _port, _apiKey);
        }
        
        return true;
    }