#pragma once
#include <WiFiClient.h>
#include <WString.h>

class KlipperMonitor {
public:
    enum Mode { PROGRESS };

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

    void setMode(uint8_t new_mode) {
        mode = new_mode;
        switch (new_mode) {
            case PROGRESS:
                url = "GET /printer/objects/query?virtual_sdcard HTTP/1.1";
                break;
        }
    }

private:
    void parseResponse(String response);

    WiFiClient client;
    String ip;
    uint16_t port;
    String apiKey;
    String lastResponse;
    String response;
    String url;
    bool enabled = false;
    uint8_t direction = 0; // 0=normal, 1=reversed, 2=center
    uint8_t mode = -1;
    
    enum RequestState { IDLE, CONNECTING, SENDING, READING };
    RequestState state = IDLE;
    unsigned long lastRequestTime = 0;

    float progress = 0.0f;
};