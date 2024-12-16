#ifndef COMMHANDLER_H
#define COMMHANDLER_H

#include "Settings.h"
#include "Utils.h"

struct CommHandler {
    uint8_t _selfMac[6];
    uint8_t _masterMac[6];
    bool _isMaster = false;
    bool _hasMaster = false;
    bool _isConnectedToMaster = false;
    bool _touchedOnce = false;
    struct_message _incomingMessage, _outgoingMessage;
    esp_now_peer_info_t _peerInfo;

    void init();
    void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
    void sendBroadcastMessage(const char *message);
    void sendMessageToMaster(const char *message);
    void discoverMaster();
    void checkMasterTimeout();
    void sendMessageToAllSlaves(const char *message);
bool parseMacAddress(const String &macString, uint8_t *macArray);
    void run(); // function to be called in loop() and runs the logic
};

extern CommHandler commHandler;

#endif
