#include "Settings.h"
#include "CommHandler.h"
#include "Utils.h"

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    commHandler.init();
}

void loop() {
    commHandler.run();
}
