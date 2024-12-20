# ESP-NOW-Project
# ESP-NOW-Based Master-Slave Communication

This project demonstrates a master-slave communication model using ESP-NOW protocol on ESP32 microcontrollers. The master device communicates with multiple slave devices to achieve synchronized wireless control, such as toggling LEDs or handling touch inputs.

## Features

- **Master Discovery**: Slaves broadcast their availability, and the master acknowledges connections.
- **P2P Communication**: Messages between master and slaves are sent directly without broadcasting.
- **Dynamic Slave Management**: Slaves dynamically register to the master.
- **LED Control**: Slaves toggle LEDs based on master commands.

## How It Works

1. **Initialization**: 
   - Each device initializes ESP-NOW and sets up its role (master or slave).
   - The master broadcasts its availability periodically.
   
2. **Slave Registration**:
   - Slaves send ADDME requests with their MAC address.
   - Master adds the slave to its list and communicates directly.

3. **Touch Input**:
   - If a slave detects a touch, it sends a TOUCHED message to the master.
   - The master toggles its LED and sends the updated state (SETLED) to all slaves.

4. **LED Synchronization**:
   - Master and slaves synchronize their LED states in real-time.

## Getting Started

### Prerequisites

- ESP32 boards
- Arduino IDE or PlatformIO
- Basic understanding of ESP-NOW communication

### Installation

1. Clone the repository:
   `ash
   git clone https://github.com/username/repository.git


