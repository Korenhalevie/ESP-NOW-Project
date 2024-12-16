#include "CommHandler.h"
#include <vector>

// Object of the CommHandler class
CommHandler commHandler;

// List of slave MAC addresses
std::vector<MacAddress> slaveMacs;

void CommHandler::init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_read_mac(_selfMac, ESP_MAC_WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register callback function to handle received data
    esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *incomingData, int len) {
        commHandler.onDataRecv(mac, incomingData, len);
    });

    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    memcpy(_peerInfo.peer_addr, broadcastAddress, 6);
    _peerInfo.channel = 1;
    _peerInfo.encrypt = false;
    esp_now_add_peer(&_peerInfo);

    // Looking for a master
    discoverMaster();

    // Define Master or Slave
    if (!_hasMaster) {
        _isMaster = true;
        isBlinking = false; // Master is not blinking
        ledState = true;    // Master LED is ON
        digitalWrite(ledPin, HIGH);
        Serial.println("No master found. I am the master.");
    } else {
        _isMaster = false;
        isBlinking = true;  // Slave blinks LED
        Serial.println("Master found. I am a slave.");
    }
}

void CommHandler::onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    memcpy(&_incomingMessage, incomingData, sizeof(_incomingMessage));
    String msg = String(_incomingMessage.message);

    Serial.println("Received message: " + msg + " from MAC: " + macToString(mac));

    // To receive the RSSI value of the received packet
    int rssi = WiFi.RSSI();

    if (msg == "I_AM_MASTER") {
        memcpy(_masterMac, mac, 6);
        _hasMaster = true;
        lastMasterSeen = millis();

        if (!_isConnectedToMaster) {
            _isMaster = false;
            _isConnectedToMaster = true;
            isBlinking = false;
            Serial.println("Master found. I am a slave.");
        }
    } else if (msg == "TOUCHED") {
        if (_isMaster) {
            toggleLED();
            String ledMessage = "SETLED=" + String(ledState ? 1 : 0);
            sendMessageToAllSlaves(ledMessage.c_str());
            _touchedOnce = true;
            Serial.println("TOUCHED received. LED toggled and state sent to all slaves.");
        }
    } else if (msg.startsWith("SETLED=")) {
        int ledValue = msg.substring(7).toInt();
        isBlinking = false;
        ledState = (ledValue == 1);
        digitalWrite(ledPin, ledState ? HIGH : LOW);
        Serial.println(String("LED state set to ") + (ledState ? "ON" : "OFF"));
    } else if (msg.startsWith("ADDME=")) {
        String macString = msg.substring(6);
                uint8_t newSlaveMac[6];
                if (parseMacAddress(macString, newSlaveMac)) {
                    if (rssi >= -60) {  // check if signal is stronger than -60 dbm
                        auto it = std::find_if(slaveMacs.begin(), slaveMacs.end(), [&](const MacAddress& slave) {
                            return memcmp(slave.addr, newSlaveMac, 6) == 0;
                        });
                        if (it == slaveMacs.end()) {
                            MacAddress newSlave;
                            memcpy(newSlave.addr, newSlaveMac, 6);
                            slaveMacs.push_back(newSlave);
                            Serial.println("New slave added (strong signal): " + macString);
                        } else {
                            Serial.println("Slave already exists: " + macString);
                        }
                    } else {
                        Serial.println("Slave rejected due to weak signal: " + macString);
                    }
                }
            }
        }

void CommHandler::sendBroadcastMessage(const char *message) {
    strcpy(_outgoingMessage.message, message);
    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_send(broadcastAddress, (uint8_t *)&_outgoingMessage, sizeof(_outgoingMessage));
}

void CommHandler::sendMessageToMaster(const char *message) {
    if (!esp_now_is_peer_exist(_masterMac)) {
        memcpy(_peerInfo.peer_addr, _masterMac, 6);
        _peerInfo.channel = 1;
        _peerInfo.encrypt = false;
        esp_now_add_peer(&_peerInfo);
    }
    strcpy(_outgoingMessage.message, message);
    esp_now_send(_masterMac, (uint8_t *)&_outgoingMessage, sizeof(_outgoingMessage));
}

void CommHandler::sendMessageToAllSlaves(const char *message) {
    strcpy(_outgoingMessage.message, message);

    for (const auto &slave : slaveMacs) {
        if (!esp_now_is_peer_exist(slave.toArray())) {
            memcpy(_peerInfo.peer_addr, slave.toArray(), 6);
            _peerInfo.channel = 1;
            _peerInfo.encrypt = false;
            esp_now_add_peer(&_peerInfo);
        }
        esp_now_send(slave.toArray(), (uint8_t *)&_outgoingMessage, sizeof(_outgoingMessage));
    }
}

void CommHandler::discoverMaster() {
    String macString = macToString(_selfMac);
    String message = "ADDME=" + macString;
    sendBroadcastMessage(message.c_str());

    unsigned long startTime = millis();
    while (millis() - startTime < masterTimeout) {
        if (_hasMaster) return;
        delay(100);
    }

    if (!_hasMaster) {
        _isMaster = true;
        memcpy(_masterMac, _selfMac, 6);
        isBlinking = false;  
        ledState = true;
        digitalWrite(ledPin, HIGH);
        Serial.println("No master found. I am the master.");
    }
}

void CommHandler::checkMasterTimeout() {
    if (millis() - lastMasterSeen > masterTimeout) {
        Serial.println("Master not found. Searching for new master...");
        _hasMaster = false;
        _isMaster = false;
        _isConnectedToMaster = false;
        discoverMaster();
    }
}

void CommHandler::run() {
    unsigned long currentTime = millis();

    // Touch logic
    if (isTouched()) {
        if (!_touchedOnce) {
            _touchedOnce = true;
            isBlinking = false;
            if (_isMaster) {
                ledState = false;
                digitalWrite(ledPin, LOW);
                Serial.println("Master LED stopped. Toggled mode activated.");
            } else {
                Serial.println("Slave LED stopped. Toggled mode activated.");
            }
        } else {
            toggleLED();
            String ledMessage = ledState ? "SETLED=1" : "SETLED=0";
            if (_isMaster) {
                sendMessageToAllSlaves(ledMessage.c_str());
                Serial.println("Master toggled LED and sent state to slaves.");
            } else {
                sendMessageToMaster("TOUCHED");
                Serial.println("TOUCHED sent to master.");
            }
        }
        delay(500);
    }

    // Blinking logic before first touching
    if (isBlinking && !_touchedOnce) {
        if (currentTime - lastBlinkTime > blinkInterval) {
            lastBlinkTime = currentTime;
            ledState = !ledState;
            digitalWrite(ledPin, ledState ? HIGH : LOW);
        }
    }

    // The master also broadcasts its status
    if (_isMaster && currentTime - lastBroadcastTime > broadcastInterval) {
        lastBroadcastTime = currentTime;
        sendBroadcastMessage("I_AM_MASTER");
        Serial.println("I am the master. Broadcasting status...");
    }

    // Checks if master is still alive
    if (!_isMaster && currentTime - lastMasterSeen > masterTimeout) {
        Serial.println("Master not found. Restarting setup...");
        _hasMaster = false;
        _isConnectedToMaster = false;
        discoverMaster();
    }
}

// פונקציה להמרת String של MAC למערך uint8_t
bool CommHandler::parseMacAddress(const String &macString, uint8_t *macArray) {
    if (macString.length() != 17) return false;

    int values[6];
    if (sscanf(macString.c_str(), "%x:%x:%x:%x:%x:%x", 
               &values[0], &values[1], &values[2], 
               &values[3], &values[4], &values[5]) == 6) {
        for (int i = 0; i < 6; ++i) {
            macArray[i] = (uint8_t)values[i];
        }
        return true;
    }
    return false;
}


