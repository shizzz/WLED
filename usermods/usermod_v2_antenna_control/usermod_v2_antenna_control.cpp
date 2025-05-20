#include <wled.h>

class antenna_control : public Usermod
{
private:
  static const char _name[];
  static const char _enabled[];
  
  bool enabled = false;
  uint8_t rf_pin = 3;
  uint8_t antenna_pin = 14;
  //External antenna disabled
  bool rf_high = true;
  bool antenna_high = false;

public:
  void setup()
  {
    if (enabled) {
        pinMode(rf_pin, OUTPUT);
        pinMode(antenna_pin, OUTPUT);
    }
  }

  void connected()
  {
  }

  void loop()
  {
    if (enabled) {
        digitalWrite(rf_pin, rf_high ? HIGH : LOW);
        digitalWrite(antenna_pin, antenna_high ? HIGH : LOW);
    }
  }

  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject(F("Antenna Control"));
    top[F("Enabled")] = enabled;
    top[F("RF PIN")] = rf_pin;
    top[F("RF HIGH")] = rf_high;
    top[F("Antenna PIN")] = antenna_pin;
    top[F("Antenna HIGH")] = antenna_high;
  }

  bool readFromConfig(JsonObject &root)
  {
    // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
    // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

    JsonObject top = root[F("Antenna Control")];
    bool configComplete = !top.isNull();
    configComplete &= getJsonValue(top[F("Enabled")], enabled);
    configComplete &= getJsonValue(top[F("RF PIN")], rf_pin);
    configComplete &= getJsonValue(top[F("RF HIGH")], rf_high);
    configComplete &= getJsonValue(top[F("Antenna PIN")], antenna_pin);
    configComplete &= getJsonValue(top[F("Antenna HIGH")], antenna_high);
    return configComplete;
  }
};

const char antenna_control::_name[] PROGMEM = "Antenna_Control";
const char antenna_control::_enabled[] PROGMEM = "enabled";

static antenna_control usermod_v2_antenna_control;
REGISTER_USERMOD(usermod_v2_antenna_control);