#include "usermod_v2_klipper_monitor.h"
    void KlipperMonitor::setup() {}
    void KlipperMonitor::loop()
    {
        setMode(PROGRESS);

        if (!_enabled || _mode == NONE || _state != IDLE) { return; }
        if (millis() - lastCheck >= checkInterval * 1000) { update(); }
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
        top[F("Host")] = _host;
        top[F("Port")] = _port;
        top[F("API Key")] = _apiKey;
        top[F("Direction")] = 0;
    }

    bool KlipperMonitor::readFromConfig(JsonObject &root) {
        JsonObject top = root[F("Klipper Monitor")];
        if (top.isNull()) return false;

        _enabled = top[F("Enabled")].as<bool>() | false;
        _host = top[F("Host")].as<String>();
        _port = top[F("Port")].as<uint16_t>();
        _apiKey = top[F("API Key")].as<String>();
        _direction = top[F("Direction")] | 0;

        if (_host == "0.0.0.0" || _host.isEmpty() || _host == "null" || _host == nullptr)
        {
            _enabled = false;
        }
        
        return true;
    }

    void KlipperMonitor::clientStop() {
        _state = IDLE;
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
            // Do nothing, Client is still connected
            return;
        }

        if (client != nullptr) {
            // Delete previous client instance if exists, just to prevent any memory leaks
            DEBUG_PRINTLN(F("Delete previous instances"));
            clientStop();
        }

        DEBUG_PRINTLN(F("Creating new AsyncClient instance."));
        client = new AsyncClient();
        changeState(AWAIT_ASYNC);
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

        // Search for two linebreaks between headers and content
        int bodyPos = response.indexOf("\r\n\r\n");
        if (bodyPos > 0) {
            String jsonStr = response.substring(bodyPos + 4); // +4 Skip the two CRLFs
            jsonStr.trim();

            DEBUG_PRINTLN("Response: ");
            DEBUG_PRINTLN(jsonStr);

            // Check for valid JSON, otherwise we brick the program runtime
            if (jsonStr[0] == '{' || jsonStr[0] == '[') {
                auto parser = createParser(_mode);
                auto result = parser->parse(response.c_str());
                switch (_mode) {
                    case PROGRESS:
                        progress = std::get<float>(result);
                        DEBUG_PRINTF("Parsed progress %.2f%%\r\n", progress);
                        break;
                }
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
