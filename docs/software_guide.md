# Software & Cryptography Guide: LoRa SCADA

This document explains the cryptographic implementation and firmware logic of the secure LoRa SCADA system.

## 🔐 Cryptographic Architecture

The system implements a robust security stack to protect against eavesdropping, tampering, and replay attacks.

### 1. Confidentiality: AES-128-CTR
- **Mode:** Counter (CTR) mode is used, which turns the block cipher into a stream cipher.
- **Key:** A 128-bit static key is shared between nodes.
- **Initialization Vector (IV):** A 16-byte IV is used for every packet.
  - Bytes 0-11: Static Base IV.
  - Bytes 12-15: 4-byte monotonic sequence number.
  - **Result:** The keystream never repeats for the same key.

### 2. Integrity & Authenticity: HMAC-SHA256
- **Process:** An HMAC is computed over the `NodeID`, `SequenceNumber`, and `Ciphertext`.
- **Truncation:** The resulting 32-byte digest is truncated to 8 bytes to minimize LoRa airtime while maintaining sufficient security for industrial control.
- **Verification:** The receiver computes the HMAC locally and discards the packet if it doesn't match the received tag.

### 3. Anti-Replay Mechanism
- Each node tracks the `last_rx_sequence_number`.
- Incoming packets are only processed if `rxSeqNum > last_rx_sequence_number`.
- A special `CMD|RESET_SEQ` command allows for counter synchronization after a reboot.

---

## 🛰️ Firmware Logic

### Base Station (Transmitter)
- **Serial Parsing:** Listens for commands from the PyQt6 dashboard (e.g., `CMD|1|0|0|0`).
- **Encapsulation:** Encrypts the payload, attaches the HMAC, and broadcasts via LoRa.
- **Telemetry Mirroring:** Receives encrypted feedback from the field node and forwards it to the dashboard.

### Field Node (Receiver)
- **Authenticated Decryption:** Verifies the HMAC before attempting to decrypt.
- **Safety Interlocks:** (If implemented in firmware) ensures that conflicting relay states are rejected.
- **Feedback Loop:** Sends an encrypted confirmation packet back to the base station after every successful actuation.

---

## 📡 LoRa Configuration
- **Frequency:** 433 MHz
- **Spreading Factor (SF):** 7 (Default for balanced range/speed)
- **Bandwidth:** 125 kHz
- **Coding Rate:** 4/5
