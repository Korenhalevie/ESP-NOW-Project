#ifndef SETTINGS_H
#define SETTINGS_H

#include <esp_now.h>
#include <WiFi.h>

// Pin definitions
const int ledPin = 2;
const int touchPin = 4;
const unsigned long blinkInterval = 500;         // Slave's LED blink interval
const unsigned long broadcastInterval = 2000;   // Master's broadcast interval
const unsigned long masterTimeout = 5000;       // Maximum time to wait for master

// Global variables
extern bool isMaster;
extern bool hasMaster;
extern bool isConnectedToMaster;
extern bool ledState;
extern bool isBlinking;
extern unsigned long lastBlinkTime;
extern unsigned long lastBroadcastTime;
extern unsigned long lastMasterSeen;

// Struct for messages
typedef struct struct_message {
    char message[100];
} struct_message;

// Struct to hold MAC address
struct MacAddress {
    uint8_t addr[6];

    // Function to convert MAC address to String
    const uint8_t* toArray() const {
        return addr;
    }
};


// משתנים גלובליים נוספים
extern struct_message incomingMessage, outgoingMessage;
extern esp_now_peer_info_t peerInfo;
extern uint8_t selfMac[6];
extern uint8_t masterMac[6];

#endif
