#pragma once
#include <WString.h>
#include "wled.h"
#include "include/parser/ResponseParser.h"
#include "MonitorMode.h"

class KlipperMonitor
{
public:
    void begin(const String& ip, uint16_t port, const String& apiKey = "");
    void update();
    void stop();
    
    bool isRunning() const { return _state != IDLE; }
    const String& getLastResponse() const { return lastResponse; }
    
    // Configuration
    void setEnabled(bool enabled) { this->_enabled = enabled; }
    bool isEnabled() const { return _enabled; }

    void setDirection(uint8_t dir) { _direction = dir; }
    uint8_t getDirection() const { return _direction; }

    float getProgress() { return progress; }

    void setMode(Mode new_mode) {
        _mode = new_mode;
        switch (new_mode) {
            case PROGRESS:
                _url = "GET /printer/objects/query?virtual_sdcard HTTP/1.1";
                break;
        }
    }

private:
    enum RequestState { IDLE, CONNECTING, SENDING, AWAIT_ASYNC, READING };
    void parseResponse(String response);

    WiFiClient wifiClient;
    AsyncClient *client = nullptr;

    // Base config
    bool _enabled = false;
    uint8_t _direction = 0; // 0=normal, 1=reversed, 2=center
    String _ip;
    uint16_t _port;
    String _apiKey;

    // State params
    RequestState _state = IDLE;
    String _url;
    Mode _mode = NONE;
    unsigned long lastRequestTime = 0;

    // Results
    String lastResponse;
    String response;
    float progress = 0.0f;
};