#pragma once
#include <WiFiClient.h>
#include <WString.h>

class KlipperMonitor {
public:
    enum Mode { NONE, PROGRESS };

    void begin(const String& ip, uint16_t port, const String& apiKey = "");
    void update();
    void stop();
    
    bool isRunning() const { return state != IDLE; }
    const String& getLastResponse() const { return lastResponse; }
    
    // Configuration
    void setEnabled(bool enabled) { this->enabled = enabled; }
    bool isEnabled() const { return enabled; }

    void setDirection(uint8_t dir) { direction = dir; }
    uint8_t getDirection() const { return direction; }

    float getProgress() { return progress; }

    void setMode(Mode new_mode) {
        mode = new_mode;
        switch (new_mode) {
            case PROGRESS:
                url = "GET /printer/objects/query?virtual_sdcard HTTP/1.1";
                break;
        }
    }

private:
    enum RequestState { IDLE, CONNECTING, SENDING, AWAIT_ASYNC, READING };
    void parseResponse(String response);

    WiFiClient client;

    // Base config
    bool enabled = false;
    uint8_t direction = 0; // 0=normal, 1=reversed, 2=center
    String ip;
    uint16_t port;
    String apiKey;

    // State params
    RequestState state = IDLE;
    String url;
    Mode mode = NONE;
    unsigned long lastRequestTime = 0;

    // Results
    String lastResponse;
    String response;
    float progress = 0.0f;
};