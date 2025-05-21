#include "wled.h"
#include "include/KlipperMonitor.h"
#include "include/MonitorMode.h"

class KlipperWatchdog : public Usermod {
private:
    KlipperMonitor monitor;
    bool _enabled = false;
    uint8_t _direction = 0;
    String _ip = F("0.0.0.0");
    uint16_t _port = 80; //7125
    String _apiKey = "";

    AsyncClient *client = nullptr;  // Used very often, beware of closing and freeing

    void handleOverlayDraw();

public:
  void setup();
  void loop();
  bool readFromConfig(JsonObject& root);
  void addToConfig(JsonObject& root);
  virtual ~KlipperWatchdog() { 
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

static KlipperWatchdog usermod_v2_klipper_monitor;
REGISTER_USERMOD(usermod_v2_klipper_monitor);