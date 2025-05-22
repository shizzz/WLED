#include "usermod_v2_klipper_monitor.h"
    void KlipperMonitor::setup() {}
    void KlipperMonitor::loop()
    {
        setMode(PROGRESS);

        if (_enabled && _mode != NONE)
        {
            update();
        }
    }

    void KlipperMonitor::handleOverlayDraw()
    {
        if (!_enabled) return;
        
        DEBUG_PRINTF("Monitor set progress %.2f%%\r\n", progress * 100);
        uint32_t color = strip.getSegment(0).colors[1];
        int total = strip.getLengthTotal();
        
        switch (_direction) {
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

    void KlipperMonitor::addToConfig(JsonObject &root) {
        JsonObject top = root.createNestedObject(F("Klipper Monitor"));
        top[F("Enabled")] = _enabled;
        top[F("IP")] = _ip;
        top[F("Port")] = _port;
        top[F("API Key")] = _apiKey;
        top[F("Direction")] = 0;
    }

    bool KlipperMonitor::readFromConfig(JsonObject &root) {
        JsonObject top = root[F("Klipper Monitor")];
        if (top.isNull()) return false;

        _enabled = top[F("Enabled")].as<bool>() | false;
        _ip = top[F("IP")].as<String>();
        _port = top[F("Port")].as<uint16_t>();
        _apiKey = top[F("API Key")].as<String>();
        _direction = top[F("Direction")] | 0;

        if (_ip == "0.0.0.0")
        {
            _enabled = false;
        }
        
        return true;
    }

    void KlipperMonitor::stop() {
        _state = IDLE;
        wifiClient.stop();
    }

    void KlipperMonitor::update() {
        switch (_state) {
            case IDLE:
                if (millis() - lastRequestTime >= 5000) {
                    _state = CONNECTING;
                    lastResponse = "";
                }
                break;
                
            case CONNECTING:
                wifiClient.setTimeout(10000);
                if (wifiClient.connect(_ip.c_str(), _port)) {
                    _state = SENDING;
                }
                break;
                
            case SENDING: {
                // Send HTTP request
                wifiClient.println(_url);
                wifiClient.print(F("Host: ")); wifiClient.println(_ip);
                wifiClient.println(F("Connection: close"));
                if (wifiClient.println() == 0) {
                    stop();
                    return;
                } else {
                    _state = READING;
                }
                break;
            }
                
            case READING:
                // Skip HTTP headers
                char endOfHeaders[] = "\r\n\r\n";
                if (!wifiClient.find(endOfHeaders)) {
                    stop();
                    return;
                }

                while (wifiClient.available()) {
                    response += wifiClient.readStringUntil('\r');
                }
                
                if (!wifiClient.connected()) {
                    parseResponse(response);
                    stop();
                    lastRequestTime = millis();
                    lastResponse = response;
                    response = "";
                }
                break;
        }
    }

    void KlipperMonitor::parseResponse(String response)
    {
        auto parser = createParser(_mode);
        auto result = parser->parse(response.c_str());
        switch (_mode) {
            case PROGRESS:
                progress = std::get<float>(result);
                DEBUG_PRINTF("Parsed progress %.2f%%\r\n", progress);
                break;
        }
    }