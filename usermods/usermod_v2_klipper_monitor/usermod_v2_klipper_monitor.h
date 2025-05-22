#include <WString.h>
#include "wled.h"
#include "include/parser/ResponseParser.h"
#include "MonitorMode.h"

class KlipperMonitor : public Usermod {
private:
    // Define constants
    enum RequestState { IDLE, CONNECTING, SENDING, AWAIT_ASYNC, READING };
    static const uint8_t lockId = USERMOD_ID_KLIPPER_MONITOR;
    static const int16_t ackTimeout = 9000;  // ACK timeout in milliseconds when doing the URL request
    static const uint16_t rxTimeout = 9000;  // RX timeout in milliseconds when doing the URL request
    static const unsigned long inactivityTimeout = 30000; // When the AsyncClient is inactive (hanging) for this many milliseconds, we kill it

    // Settings
    bool _enabled = false;
    uint8_t _direction = 0;
    String _host = F("0.0.0.0");
    uint16_t _port = 80; //7125
    String _apiKey = "";

    AsyncClient *client = nullptr;

    // State params
    RequestState _state = IDLE;
    String _url;
    Mode _mode = NONE;
    uint16_t checkInterval = 5;
    unsigned long lastRequestTime = 0;
    unsigned long lastCheck = 0;          // Timestamp of last check
    unsigned long lastActivityTime = 0;   // Time of last activity of AsyncClient

    // Results
    String response;
    float progress = 0.0f;

    // Methods
    void onClientConnect(AsyncClient *c);
    void parseResponse(String response);
    void handleOverlayDraw();
    void update();
    void clientStop();
    void changeState(RequestState state) { _state = state; };

    // Configuration
    void setMode(Mode new_mode) {
        _mode = new_mode;
        switch (new_mode) {
            case PROGRESS:
                _url = "/printer/objects/query?virtual_sdcard";
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