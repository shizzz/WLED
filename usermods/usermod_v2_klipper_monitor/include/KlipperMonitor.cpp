#include "KlipperMonitor.h"
#include "wled.h"
#include <WString.h>

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
    PSRAMDynamicJsonDocument jsonResponse(4096);
    DeserializationError error = deserializeJson(jsonResponse, response);

    DEBUG_PRINTF("Parse response mode: %d\r\n", mode);
    DEBUG_PRINTF("Parse response raw: %s\r\n", response.c_str());
    if (!error)
    {
        switch (mode) {
            case PROGRESS:
                DEBUG_PRINTF("Parse response json result\r\n");
                progress = jsonResponse[F("result")][F("status")][F("virtual_sdcard")][F("progress")].as<float>() * 100;
                DEBUG_PRINTF("Parsed response: %.2f%\r\n");
                break;
        }
    }
}