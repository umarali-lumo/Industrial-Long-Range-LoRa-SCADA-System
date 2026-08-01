# Hardware Guide: Industrial LoRa SCADA

This document details the hardware components and wiring for both the Base Station (TX) and the Field Node (RX).

## 🛠️ Bill of Materials (BOM)

| Component | Specification | Quantity | Purpose |
| :--- | :--- | :--- | :--- |
| **Microcontroller** | ESP32-WROOM-32 | 2 | TX Base and RX Node |
| **LoRa Module** | SX1278 (Ra-02) 433MHz | 2 | Wireless Communication |
| **Relay Module** | 4-Channel Opto-isolated | 1 | Field Actuation (RX Side) |
| **Antenna** | 433MHz Spring/SMA | 2 | RF Signal |
| **Power Supply** | 5V DC / 2A | 2 | System Power |

---

## 🔌 Wiring Specifications

Both the Transmitter and Receiver share the same SPI mapping for the LoRa module.

### 1. ESP32 to SX1278 (SPI)
| ESP32 Pin | SX1278 Pin | Function |
| :--- | :--- | :--- |
| **GPIO 5** | NSS | Chip Select |
| **GPIO 14** | RST | Reset |
| **GPIO 26** | DIO0 | Interrupt |
| **GPIO 18** | SCK | SPI Clock |
| **GPIO 19** | MISO | SPI Master In |
| **GPIO 23** | MOSI | SPI Master Out |
| **3.3V** | VCC | Power (Max 3.6V) |
| **GND** | GND | Common Ground |

### 2. Field Node (RX) to Relay Module
The relays are configured as **active-low**.
| ESP32 Pin | Relay Channel | Function |
| :--- | :--- | :--- |
| **GPIO 25** | Relay 1 | Actuator 1 |
| **GPIO 33** | Relay 2 | Actuator 2 |
| **GPIO 32** | Relay 3 | Actuator 3 |
| **GPIO 27** | Relay 4 | Actuator 4 |

---

## ⚡ Power Considerations
- **SX1278 Voltage:** The LoRa module **must** be powered by 3.3V. Applying 5V will damage the transceiver.
- **Relay Power:** The relay module often requires a separate 5V supply for the coils. Ensure the JD-VCC jumper is handled correctly to maintain optical isolation.
- **Antenna:** Never operate the LoRa module without an antenna connected, as this can damage the output stage.
