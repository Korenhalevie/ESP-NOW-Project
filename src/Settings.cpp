#include "Settings.h"

// Global variables
bool isMaster = false;
bool hasMaster = false;
bool isConnectedToMaster = false;
bool ledState = false;
bool isBlinking = false;
unsigned long lastBlinkTime = 0;
unsigned long lastBroadcastTime = 0;
unsigned long lastMasterSeen = 0;

struct_message incomingMessage;
struct_message outgoingMessage;
esp_now_peer_info_t peerInfo;
uint8_t selfMac[6];
uint8_t masterMac[6];
