#include "KlipperMonitor.h"

void KlipperMonitor::begin(const String& ip, uint16_t port, const String& apiKey) {
    this->_ip = ip;
    this->_port = port;
    this->_apiKey = apiKey;
    stop();
}

void KlipperMonitor::stop() {
    _state = IDLE;
}

void KlipperMonitor::update() {
    if (!_enabled) return;
    if (_mode == -1) return;

    switch (_state) {
        case IDLE:
            if (millis() - lastRequestTime >= 5000) {
                _state = CONNECTING;
                lastResponse = "";
            }
            break;
            
        case CONNECTING:
            client.setTimeout(10000);
            if (client.connect(_ip.c_str(), _port)) {
                _state = SENDING;
            }
            break;
            
        case SENDING: {
            // Send HTTP request
            client.println(_url);
            client.print(F("Host: ")); client.println(_ip);
            client.println(F("Connection: close"));
            if (client.println() == 0) {
                client.stop();
                _state = CONNECTING;
                return;
            } else {
                _state = READING;
            }
            break;
        }
            
        case READING:
            // Skip HTTP headers
            char endOfHeaders[] = "\r\n\r\n";
            if (!client.find(endOfHeaders)) {
                client.stop();
                _state = CONNECTING;
                return;
            }

            while (client.available()) {
                response += client.readStringUntil('\r');
            }
            
            if (!client.connected()) {
                client.stop();
                parseResponse(response);
                _state = IDLE;
                lastRequestTime = millis();
                lastResponse = response;
                response = "";
            }
            break;
    }
}

void KlipperMonitor::parseResponse(String response)
{
    auto parser = createParser(_mode);
    auto result = parser->parse(response.c_str());
    switch (_mode) {
        case PROGRESS:
            progress = std::get<float>(result);
            DEBUG_PRINTF("Parsed progress %.2f%%\r\n", progress);
            break;
    }
}