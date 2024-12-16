#include "Utils.h"

bool isTouched() {
    return touchRead(touchPin) < 30;
}

String macToString(const uint8_t *mac) {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}

void toggleLED() {
    ledState = !ledState;
    digitalWrite(ledPin, ledState ? HIGH : LOW);
    Serial.println(String("LED toggled: ") + (ledState ? "ON" : "OFF"));
}
