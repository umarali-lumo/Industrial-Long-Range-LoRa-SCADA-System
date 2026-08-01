# System Architecture & Safety Guide: LoRa SCADA

The LoRa SCADA system is built on a hierarchical architecture that prioritizes deterministic safety and secure communication.

## 📐 System Architecture

### 1. The Supervisory Layer (PyQt6 Dashboard)
The HMI acts as the "brain" of the system, providing:
- **Threaded Communication:** A dedicated worker thread handles non-blocking serial I/O.
- **Visual Analytics:** Real-time RSSI waveforms and packet statistics.
- **Safety Configuration:** Users can define interlock groups and timed fallbacks through the UI.

### 2. The Communication Layer (LoRa)
- **CSS Modulation:** Uses Chirp Spread Spectrum to maintain links in high-interference industrial environments.
- **Authenticated Frames:** Every packet is a self-contained secure unit containing Node ID, SeqNo, HMAC, and Ciphertext.

### 3. The Actuation Layer (Field Node)
- **Opto-Isolation:** Protects the ESP32 from high-voltage spikes on the relay side.
- **Fail-Safe State:** All relays default to the HIGH (OFF) state upon initialization or security failure.

---

## 🛡️ Industrial Safety Models

### 1. Mutual-Exclusion Interlocks
The system prevents conflicting operations by enforcing a "Sum of States" rule. For example, if Relay 1 (Forward) and Relay 2 (Reverse) are in an interlock group, the dashboard will reject any command where both are `1`.
- **Logic:** $\sum r_i \leq 1$ for all $i$ in an interlocked set.

### 2. Timed Fallback (Auto-Shutdown)
Each channel can be assigned a timeout value ($T_k$).
- **Operation:** When a relay is energized, a timer starts.
- **Trigger:** If the timer reaches $T_k$, the dashboard automatically issues a `POWER OFF` command for that channel.
- **Purpose:** Prevents equipment damage in case of operator distraction or communication loss.

### 3. Emergency Stop (E-STOP)
A global broadcast mechanism that overrides all current states.
- **Payload:** `CMD|0|0|0|0`
- **Priority:** The E-STOP command is transmitted with zero delay and triggers a visual alarm on the HMI.
