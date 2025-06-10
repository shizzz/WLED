#include <WString.h>
#include "wled.h"
#include "include/parser/ResponseParser.h"
#include "include/painter/Painter.h"
#include "include/MonitorTypes.h"

#ifndef PRESETS
  #define PRESETS { { "PROGRESS", PROGRESS, "virtual_sdcard", NORMAL }, { "EXTRUDER", HEATER, "extruder", NORMAL }, { "HEATER_BED", HEATER, "heater_bed", NORMAL } }
#endif
#ifndef PRESET_COUNT
  #define PRESET_COUNT 3
#endif

class KlipperMonitor : public Usermod {
private:
    // Define constants
    static const char _name[];
    enum RequestState { IDLE, CONNECTING, SENDING, AWAIT_ASYNC, READING };
    static const uint8_t lockId = USERMOD_ID_KLIPPER_MONITOR;
    static const int16_t ackTimeout = 9000;  // ACK timeout in milliseconds when doing the URL request
    static const uint16_t rxTimeout = 9000;  // RX timeout in milliseconds when doing the URL request
    static const unsigned long inactivityTimeout = 30000; // When the AsyncClient is inactive (hanging) for this many milliseconds, we kill it

    // Settings
    bool _enabled = false;
    bool _active = false;
    Effect _direction = NORMAL;
    String _host = F("0.0.0.0");
    uint16_t _port = 80; //7125
    String _apiKey = "";

    AsyncClient *client = nullptr;

    // State params
    bool _initDone = false;
    RequestState _state = IDLE;
    String _url;
    uint16_t checkInterval = 5;
    size_t maxPresetNumber = PRESET_COUNT - 1;
    unsigned long lastRequestTime = 0;
    unsigned long lastCheck = 0;          // Timestamp of last check
    unsigned long lastActivityTime = 0;   // Time of last activity of AsyncClient
    PresetSettings _presetSettings[PRESET_COUNT] = PRESETS;
    PresetSettings _activePreset;

    // Results
    ParseResult _parseResult;

    // Methods
    void onClientConnect(AsyncClient *c);
    void parseResponse(String response);
    void handleOverlayDraw();
    void update();
    void clientStop();
    void changeState(RequestState state) { _state = state; };
    static uint8_t checkColorSetting(uint8_t color);
    static unsigned int checkPixelSetting(unsigned int pixel);

    // Configuration
    void setActivePreset(uint8_t preset);
public:
  // Base overloads
  void setup() override;
  void loop() override;
  bool readFromConfig(JsonObject& root) override;
  void addToConfig(JsonObject& root) override;
  void appendConfigData() override;
  void addToJsonState(JsonObject& root) override;
  void readFromJsonState(JsonObject& root) override;
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