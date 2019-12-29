#include <EEPROM.h>
#include "nvm.h"

void read_EEPROM();
void write_EEPROM();

char ssid[64] = "";
char password[64] = "";

/* Load WLAN credentials from EEPROM */
void read_EEPROM()
{
    EEPROM.begin(512);
    EEPROM.get(0, ssid);
    EEPROM.get(0 + sizeof(ssid), password);
    EEPROM.end();
}

/* Store WLAN credentials to EEPROM */
void write_EEPROM()
{
    EEPROM.begin(512);
    EEPROM.put(0, ssid);
    EEPROM.put(0 + sizeof(ssid), password);
    EEPROM.commit();
    EEPROM.end();
}