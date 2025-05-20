#include "wled.h"
#include "include/KlipperMonitor.h"

class KlipperWatchdog : public Usermod {
private:
    KlipperMonitor monitor;
    bool _enabled = false;
    uint8_t _direction = 0;
    String _ip = F("0.0.0.0");
    uint16_t _port = 80; //7125
    String _apiKey = "";

public:
    void setup()
    {
    }

    void connected()
    {
    }

    void loop()
    {
        monitor.setMode(monitor.PROGRESS);
        monitor.update();
    }

    void handleOverlayDraw()
    {
        if (!monitor.isEnabled()) return;
        
        float progress = monitor.getProgress();
        DEBUG_PRINTF("Monitor set progress %.2f%%\r\n", progress);
        uint32_t color = strip.getSegment(0).colors[1];
        int total = strip.getLengthTotal();
        
        switch (monitor.getDirection()) {
            case 1: // reversed
                for (int i = 0; i < total * progress / 100; i++) {
                    strip.setPixelColor(total - 1 - i, color);
                }
                break;
                
            case 2: // center
                for (int i = 0; i < (total / 2) * progress / 100; i++) {
                    strip.setPixelColor((total / 2) + i, color);
                    strip.setPixelColor((total / 2) - 1 - i, color);
                }
                break;
                
            default: // normal
                for (int i = 0; i < total; i++) {
                    if (i < total * progress / 100)
                    {
                        strip.setPixelColor(i, 255, 0, 0, 150);
                    } else {
                        strip.setPixelColor(i, 0, 0, 0, 0);
                    }
                }
        }
    }

    void addToConfig(JsonObject &root) {
        JsonObject top = root.createNestedObject(F("Klipper Monitor"));
        top[F("Enabled")] = _enabled;
        top[F("IP")] = _ip;
        top[F("Port")] = _port;
        top[F("API Key")] = _apiKey;
        top[F("Direction")] = 0;
    }

    bool readFromConfig(JsonObject &root) override {
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
};

static KlipperWatchdog usermod_v2_klipper_monitor;
REGISTER_USERMOD(usermod_v2_klipper_monitor);