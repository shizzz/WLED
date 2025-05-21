#include "KlipperMonitor.h"

void KlipperMonitor::begin(const String& ip, uint16_t port, const String& apiKey) {
    this->ip = ip;
    this->port = port;
    this->apiKey = apiKey;
    stop();
}

void KlipperMonitor::stop() {
    if (client.connected()) {
        client.stop();
    }
    state = IDLE;
}

void KlipperMonitor::update() {
    if (!enabled) return;
    if (mode == -1) return;

    switch (state) {
        case IDLE:
            if (millis() - lastRequestTime >= 5000) {
                state = CONNECTING;
                lastResponse = "";
            }
            break;
            
        case CONNECTING:
            client.setTimeout(10000);
            if (client.connect(ip.c_str(), port)) {
                state = SENDING;
            }
            break;
            
        case SENDING: {
            // Send HTTP request
            client.println(url);
            client.print(F("Host: ")); client.println(ip);
            client.println(F("Connection: close"));
            if (client.println() == 0) {
                client.stop();
                state = CONNECTING;
                return;
            } else {
                state = READING;
            }
            break;
        }
            
        case READING:
            // Skip HTTP headers
            char endOfHeaders[] = "\r\n\r\n";
            if (!client.find(endOfHeaders)) {
                client.stop();
                state = CONNECTING;
                return;
            }

            while (client.available()) {
                response += client.readStringUntil('\r');
            }
            
            if (!client.connected()) {
                client.stop();
                parseResponse(response);
                state = IDLE;
                lastRequestTime = millis();
                lastResponse = response;
                response = "";
            }
            break;
    }
}

void KlipperMonitor::parseResponse(String response)
{
    auto parser = createParser(mode);
    auto result = parser->parse(response.c_str());
    switch (mode) {
        case PROGRESS:
            progress = std::get<float>(result);
            DEBUG_PRINTF("Parsed progress %.2f%%\r\n", progress);
            break;
    }
}