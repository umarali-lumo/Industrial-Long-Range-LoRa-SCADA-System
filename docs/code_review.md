# Code Review & Engineering Quality Assessment: LoRa SCADA

This document provides a professional review of the secure LoRa SCADA system and recommendations for research-grade hardening.

## 📊 Quality Scores (Out of 10)

| Category | Score | Notes |
| :--- | :--- | :--- |
| **Readability** | 9/10 | Clear structure, excellent nomenclature, and modular design. |
| **Documentation** | 8/10 | Theoretical foundations are well-explained in the report. |
| **Code Quality** | 9/10 | Cryptographic implementation is correct and follows best practices. |
| **Engineering Design** | 9/10 | Strong multi-layer protection (Physical, Crypto, Safety). |
| **Research Value** | 10/10 | Implementation of AES-CTR + HMAC on LoRa is a significant contribution. |
| **Maintainability** | 8/10 | Shared configuration header would improve gain/key management. |

---

## 🔍 Detailed Findings

### 1. Cryptographic Implementation
**Strengths:**
- **CTR Mode:** Using AES-128 in CTR mode is ideal for LoRa as it avoids padding and keeps packet sizes minimal.
- **Truncated HMAC:** Truncating the SHA256 digest to 8 bytes is a clever engineering trade-off for low-bandwidth links.
- **Anti-Replay:** The sequence number check is correctly performed **after** HMAC verification but **before** decryption/execution.

**Suggested Improvements:**
- **Constant-Time Comparison:** Use a constant-time memory comparison for HMAC verification to prevent timing attacks.
- **Key Rotation:** For long-term deployments, implement a session-key agreement protocol (e.g., Diffie-Hellman) instead of static keys.

### 2. Firmware Architecture
**Strengths:**
- **Non-Blocking:** The use of `LoRa.parsePacket()` in the main loop ensures the ESP32 remains responsive.
- **Fail-Safe Defaults:** The `allOff()` function on boot is a critical industrial safety feature.

**Suggested Improvements:**
- **Watchdog Timer:** Enable the ESP32 hardware watchdog to recover from potential radio lockups.
- **CRC Validation:** While LoRa has hardware CRC, adding a software-level checksum to the plaintext would provide an extra layer of structural validation.

---

## 👨‍💼 Hiring Manager's Perspective

### **Senior Firmware Engineer**
> "The candidate demonstrates a deep understanding of secure embedded systems. Implementing authenticated encryption on a resource-constrained LoRa link requires balancing security with airtime, and this project handles it perfectly. The use of sequence numbers for anti-replay shows they understand real-world attack vectors."

### **Research Professor (IoT & Cybersecurity)**
> "This is an excellent example of applying modern cryptographic primitives to legacy-style SCADA problems. The theoretical framing of Chirp Spread Spectrum path loss alongside AES-CTR implementation makes this a standout research prototype."
