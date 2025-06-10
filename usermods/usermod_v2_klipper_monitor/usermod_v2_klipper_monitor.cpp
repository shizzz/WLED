#include "usermod_v2_klipper_monitor.h"

    const char KlipperMonitor::_name[] PROGMEM = "Klipper Monitor";

    void KlipperMonitor::setup()
    {
        _initDone = true;
    }

    void KlipperMonitor::loop()
    {
        setActivePreset(userVar0);

        if (!_enabled || _state != IDLE) { return; }
        if (millis() - lastCheck >= checkInterval * 1000)
        { 
            update(); 
            lastCheck = millis();
        }
    }

    void KlipperMonitor::setActivePreset(uint8_t preset)
    {       
        preset = preset - 1;
        if (preset < 0 || preset > maxPresetNumber)
        {
            _active = false;
            return;
        }
        _activePreset = _presetSettings[preset];
        _url = "/printer/objects/query?";
        _url += _activePreset.entity;
        _active = true;
    }

    void KlipperMonitor::handleOverlayDraw()
    {
        if (!_enabled || !_active) return;
        
        auto painter = createPainter(_activePreset.effect);
        painter->paint(_activePreset, _parseResult);
    }

    void KlipperMonitor::addToConfig(JsonObject &root) {
        JsonObject top = root.createNestedObject(FPSTR(_name));
        top[F("enabled")] = _enabled;
        top[F("selected")] = userVar0;
        top[F("host")] = _host;
        top[F("port")] = _port;
        top[F("API Key")] = _apiKey;

        for (int i = 0; i < PRESET_COUNT; i++)
        {
            JsonObject modeJson = top.createNestedObject(_presetSettings[i].name);
            modeJson[F("entity")] = _presetSettings[i].entity;
            modeJson[F("effect")] = _presetSettings[i].effect;
            modeJson[F("start pixel")] = _presetSettings[i].startPixel;
            modeJson[F("end pixel")] = _presetSettings[i].endPixel;
            modeJson[F("clean stripe")] = _presetSettings[i].cleanStripe;

            JsonObject colorJson = modeJson.createNestedObject(F("color"));
            colorJson[F("red")] = _presetSettings[i].color.red;
            colorJson[F("green")] = _presetSettings[i].color.green;
            colorJson[F("blue")] = _presetSettings[i].color.blue;
        }
    }
    
    void KlipperMonitor::appendConfigData() {
        int effectsCount = sizeof(EffectStrings) / sizeof(EffectStrings[0]);
        oappend(F("dd=addDropdown('Klipper Monitor','selected');"));
        oappend(F("addOption(dd,'None',0);"));
        for (int i = 0; i < PRESET_COUNT; i++)
        {
            oappend(F("addOption(dd,'")); oappend(_presetSettings[i].name); oappend(F("',")); oappend(i + 1); oappend(F(");"));
        } 
        for (int i = 0; i < PRESET_COUNT; i++)
        {
            oappend(F("td=addDropdown('Klipper Monitor:")); oappend(_presetSettings[i].name); oappend(F("','effect');"));
            for (int effect = 0; effect < effectsCount; effect++)
            {
                oappend(F("addOption(td,'")); oappend(EffectStrings[effect]); oappend(F("',")); oappend(effect); oappend(F(");"));
            }  
        } 
    }

    bool KlipperMonitor::readFromConfig(JsonObject &root) {
        JsonObject top = root[FPSTR(_name)];
        bool configComplete = !top.isNull();

        configComplete &= getJsonValue(top[F("enabled")], _enabled);
        configComplete &= getJsonValue(top[F("selected")], userVar0);
        configComplete &= getJsonValue(top[F("host")], _host);
        configComplete &= getJsonValue(top[F("port")], _port, 80);
        configComplete &= getJsonValue(top[F("API Key")], _apiKey);
            
        uint8_t hw = strip.getBrightness();

        for (int i = 0; i < PRESET_COUNT; i++)
        {
            JsonObject jsonSetting = top[_presetSettings[i].name];
            _presetSettings[i].color = {};
            configComplete &= getJsonValue(jsonSetting[F("effect")], _presetSettings[i].effect, NORMAL);
            configComplete &= getJsonValue(jsonSetting[F("clean stripe")], _presetSettings[i].cleanStripe, false);
            configComplete &= getJsonValue(jsonSetting[F("entity")], _presetSettings[i].entity);
            configComplete &= getJsonValue(jsonSetting[F("start pixel")], _presetSettings[i].startPixel, 1);
            configComplete &= getJsonValue(jsonSetting[F("end pixel")], _presetSettings[i].endPixel);
            configComplete &= getJsonValue(jsonSetting[F("red")], _presetSettings[i].color.red, 255);
            configComplete &= getJsonValue(jsonSetting[F("green")], _presetSettings[i].color.green, 0);
            configComplete &= getJsonValue(jsonSetting[F("blue")], _presetSettings[i].color.blue, 0);
            
            _presetSettings[i].maxBrightness = hw;
            _presetSettings[i].startPixel = checkPixelSetting(_presetSettings[i].startPixel);
            _presetSettings[i].endPixel = checkPixelSetting(_presetSettings[i].endPixel);
            _presetSettings[i].color.red = checkColorSetting(_presetSettings[i].color.red);
            _presetSettings[i].color.green = checkColorSetting(_presetSettings[i].color.green);
            _presetSettings[i].color.blue = checkColorSetting(_presetSettings[i].color.blue);
            if (_presetSettings[i].color.red == 0 && _presetSettings[i].color.green == 0 && _presetSettings[i].color.blue == 0)
            {
                _presetSettings[i].useExistingColor;
            }
        }

        if (_host == "0.0.0.0" || _host.isEmpty() || _host == "null" || _host == nullptr)
        {
            _enabled = false;
        }
        
        return configComplete;
    }

    void KlipperMonitor::clientStop() {
        changeState(IDLE);
        if (client != nullptr) {
            delete client;
        }
        //delete client;
        client->stop();
        client = nullptr;
    }

    void KlipperMonitor::update() {
        // Extra Inactivity check to see if AsyncCLient hangs
        if (client != nullptr && ( millis() - lastActivityTime > inactivityTimeout ) ) {
            DEBUG_PRINTLN(F("Inactivity detected, deleting client."));
            clientStop();
        }
        if (client != nullptr && client->connected()) {
            DEBUG_PRINTLN(F("We are still connected, do nothing"));
            return;
        }

        changeState(AWAIT_ASYNC);

        if (client != nullptr) {
            DEBUG_PRINTLN(F("Delete previous instances"));
            clientStop();
        }

        DEBUG_PRINTLN(F("Creating new AsyncClient instance."));
        client = new AsyncClient();

        if(client) {
            client->onData([](void *arg, AsyncClient *c, void *data, size_t len) {
                DEBUG_PRINTLN(F("Data received."));
                // Cast arg back to the usermod class instance
                KlipperMonitor *instance = (KlipperMonitor *)arg;
                instance->changeState(READING);
                instance->lastActivityTime = millis(); // Update lastactivity time when data is received
                // Convertert to Safe-String
                char *strData = new char[len + 1];
                strncpy(strData, (char*)data, len);
                strData[len] = '\0';
                String responseData = String(strData);
                //String responseData = String((char *)data);
                // Make sure its zero-terminated String
                //responseData[len] = '\0';
                delete[] strData; // Do not forget to remove this one
                instance->parseResponse(responseData);
                instance->changeState(IDLE);
            }, this);
            client->onDisconnect([](void *arg, AsyncClient *c) {
                DEBUG_PRINTLN(F("Disconnected."));
                //Set the class-own client pointer to nullptr if its the current client
                KlipperMonitor *instance = static_cast<KlipperMonitor*>(arg);
                instance->changeState(IDLE);
                if (instance->client == c) {
                    instance->clientStop();
                }
            }, this);
            client->onTimeout([](void *arg, AsyncClient *c, uint32_t time) {
                DEBUG_PRINTLN(F("Timeout"));
                //Set the class-own client pointer to nullptr if its the current client
                KlipperMonitor *instance = static_cast<KlipperMonitor*>(arg);
                instance->changeState(IDLE);
                if (instance->client == c) {
                    instance->clientStop();
                }
            }, this);
            client->onError([](void *arg, AsyncClient *c, int8_t error) {
                DEBUG_PRINTLN("Connection error occurred!");
                DEBUG_PRINT("Error code: ");
                DEBUG_PRINTLN(error);
                //Set the class-own client pointer to nullptr if its the current client
                KlipperMonitor *instance = static_cast<KlipperMonitor*>(arg);
                instance->changeState(IDLE);
                if (instance->client == c) {
                    instance->clientStop();
                }
                // Do not remove client here, it is maintained by AsyncClient
            }, this);
            client->onConnect([](void *arg, AsyncClient *c) {
                // Cast arg back to the usermod class instance
                KlipperMonitor *instance = (KlipperMonitor *)arg;
                instance->changeState(CONNECTING);
                instance->onClientConnect(c);  // Call a method on the instance when the client connects
            }, this);
            client->setAckTimeout(ackTimeout); // Just some safety measures because we do not want any memory fillup
            client->setRxTimeout(rxTimeout);
            DEBUG_PRINT(F("Connecting to: "));
            DEBUG_PRINT(_host);
            DEBUG_PRINT(F(" via port "));
            DEBUG_PRINTLN(_port);
            // Update lastActivityTime just before sending the request
            lastActivityTime = millis();
            //Try to connect
            if (!client->connect(_host.c_str(), _port)) {
                DEBUG_PRINTLN(F("Failed to initiate connection."));
                // Connection failed, so cleanup
                clientStop();
            } else {
                // Connection successfull, wait for callbacks to go on.
                DEBUG_PRINTLN(F("Connection initiated, awaiting response..."));
            }
        } else {
            DEBUG_PRINTLN(F("Failed to create AsyncClient instance."));
        }
    }

    void KlipperMonitor::parseResponse(String response)
    {
        DEBUG_PRINTLN(F("Received response for handleResponse."));

        // Get a Bufferlock, we can not use doc
        if (!requestJSONBufferLock(lockId)) {
            DEBUG_PRINT(F("ERROR: Can not request JSON Buffer Lock, number: "));
            DEBUG_PRINTLN(lockId);
            releaseJSONBufferLock(); // Just release in any case, maybe there was already a buffer lock
            return;
        }

        DEBUG_PRINTLN("Response: ");
        DEBUG_PRINTLN(response.c_str());

        // Search for two linebreaks between headers and content
        int bodyPos = response.indexOf("\r\n\r\n");
        if (bodyPos > 0) {
            String jsonStr = response.substring(bodyPos + 4); // +4 Skip the two CRLFs
            jsonStr.trim();

            // Check for valid JSON, otherwise we brick the program runtime
            if (jsonStr[0] == '{' || jsonStr[0] == '[') {
                auto parser = createParser(_activePreset.type);
                _parseResult = parser->parse(jsonStr.c_str(), _activePreset.entity.c_str());
            } else {
                DEBUG_PRINTLN(F("Invalid JSON response"));
            }
        } else {
            DEBUG_PRINTLN(F("No body found in the response"));
        }

        // Release the BufferLock again
        releaseJSONBufferLock();
    }
    
    // This function is called from the checkUrl function when the connection is establised
    // We request the data here
    void KlipperMonitor::onClientConnect(AsyncClient *c) {
        DEBUG_PRINT(F("Client connected: "));
        DEBUG_PRINTLN(c->connected() ? F("Yes") : F("No"));
        changeState(SENDING);

        if (c->connected()) {
            String request = "GET " + _url + " HTTP/1.1\r\n"
                            "Host: " + _host + "\r\n"
                            "Connection: close\r\n"
                            "Accept: application/json\r\n"
                            "Accept-Encoding: identity\r\n" // No compression
                            "User-Agent: ESP32 HTTP Client\r\n\r\n"; // Optional: User-Agent and end with a double rnrn !
            DEBUG_PRINT(request.c_str());
            auto bytesSent  = c->write(request.c_str());
            if (bytesSent  == 0) {
                // Connection could not be made
                DEBUG_PRINT(F("Failed to send HTTP request."));
            } else {
                DEBUG_PRINT(F("Request sent successfully, bytes sent: "));
                DEBUG_PRINTLN(bytesSent );
            }
        }
    }

    uint8_t KlipperMonitor::checkColorSetting(uint8_t color)
    {
        if (color < 0)
        {
            return 0;
        }
        if (color > 255)
        {
            return 255;
        }
        return color;
    }

    unsigned int KlipperMonitor::checkPixelSetting(unsigned int pixel)
    {
        if (pixel < 0) {
            return 0;
        } 

        return pixel;
    }


    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void KlipperMonitor::addToJsonState(JsonObject& root)
    {
      if (!_initDone || !_enabled) return;  // prevent crash on boot applyPreset()

      JsonObject usermod = root[FPSTR(_name)];
      if (usermod.isNull()) usermod = root.createNestedObject(FPSTR(_name));
      usermod["user0"] = userVar0;
    }


    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void KlipperMonitor::readFromJsonState(JsonObject& root)
    {
      if (!_initDone) return;

      JsonObject usermod = root[FPSTR(_name)];
      if (!usermod.isNull()) {
        userVar0 = usermod["user0"] | userVar0;
      }
    }