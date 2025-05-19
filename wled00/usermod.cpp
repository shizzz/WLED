#include "wled.h"
/*
 * This v1 usermod file allows you to add own functionality to WLED more easily
 * See: https://github.com/wled-dev/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in const.h)
 * If you just need 8 bytes, use 2551-2559 (you do not need to increase EEPSIZE)
 *
 * Consider the v2 usermod API if you need a more advanced feature set!
 */

//Use userVar0 and userVar1 (API calls &U0=,&U1=, uint16_t)
/*
 * Physical IO
 */
#define PIN_UP_RELAY 14
#define PIN_DN_RELAY 3

//gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{
  pinMode(PIN_UP_RELAY, OUTPUT);
  pinMode(PIN_DN_RELAY, OUTPUT);
  userVar0 = 1;
}

//gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{

}

void handleRelay()
{
  //up and down relays
  if (userVar0 == 1) {
      digitalWrite(PIN_UP_RELAY, HIGH);
      digitalWrite(PIN_DN_RELAY, LOW);
  } else {
      digitalWrite(PIN_UP_RELAY, LOW);
      digitalWrite(PIN_DN_RELAY, HIGH);
  }
}

//loop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop()
{
  handleRelay();
}
