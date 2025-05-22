#include <WString.h>
#include "wled.h"
#include "include/parser/ResponseParser.h"
#include "MonitorMode.h"

class KlipperMonitor : public Usermod {
private:
    // Constants
    enum RequestState { IDLE, CONNECTING, SENDING, AWAIT_ASYNC, READING };

    // Settings
    bool _enabled = false;
    uint8_t _direction = 0;
    String _ip = F("0.0.0.0");
    uint16_t _port = 80; //7125
    String _apiKey = "";

    WiFiClient wifiClient;
    AsyncClient *client = nullptr;

    // State params
    RequestState _state = IDLE;
    String _url;
    Mode _mode = NONE;
    unsigned long lastRequestTime = 0;

    // Results
    String lastResponse;
    String response;
    float progress = 0.0f;

    // Methods
    void parseResponse(String response);
    void handleOverlayDraw();
    void update();
    void stop();

    // Configuration
    void setMode(Mode new_mode) {
        _mode = new_mode;
        switch (new_mode) {
            case PROGRESS:
                _url = "GET /printer/objects/query?virtual_sdcard HTTP/1.1";
                break;
        }
    }
public:
  // Base overloads
  void setup();
  void loop();
  bool readFromConfig(JsonObject& root);
  void addToConfig(JsonObject& root);
  uint16_t getId() { return USERMOD_ID_KLIPPER_MONITOR; }
  inline void enable(bool enable) { _enabled = enable; }
  inline bool isEnabled() { return _enabled; }

  virtual ~KlipperMonitor() { 
    // Remove the cached client if needed
    if (client) {
      client->onDisconnect(nullptr);
      client->onError(nullptr);
      client->onTimeout(nullptr);
      client->onData(nullptr);
      client->onConnect(nullptr);
      // Now it is safe to delete the client.
      delete client; // This is safe even if client is nullptr.
      client = nullptr;
    }
  }
};

static KlipperMonitor usermod_v2_klipper_monitor;
REGISTER_USERMOD(usermod_v2_klipper_monitor);