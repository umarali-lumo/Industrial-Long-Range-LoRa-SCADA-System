#include <SPI.h>
#include <LoRa.h>
#include <Crypto.h>
#include <AES.h>
#include <CTR.h>
#include <SHA256.h>

#define R1 25
#define R2 33
#define R3 32
#define R4 27

const uint16_t NODE_ID = 3;
uint32_t last_rx_sequence_number = 0;
uint32_t tx_sequence_number = 1;

const uint8_t AES_KEY[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

const uint8_t BASE_IV[16] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

CTR<AES128> ctr;

void allOff() {
  digitalWrite(R1, HIGH);
  digitalWrite(R2, HIGH);
  digitalWrite(R3, HIGH);
  digitalWrite(R4, HIGH);
}

void setup() {
  Serial.begin(115200);

  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(R3, OUTPUT);
  pinMode(R4, OUTPUT);
  allOff();

  LoRa.setPins(5, 14, 26);
  if (!LoRa.begin(433E6)) {
    while (1);
  }
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize > 14) {
    uint16_t targetId = LoRa.read() | (LoRa.read() << 8);
    uint32_t rxSeqNum = (uint32_t)LoRa.read() | ((uint32_t)LoRa.read() << 8) |
                        ((uint32_t)LoRa.read() << 16) | ((uint32_t)LoRa.read() << 24);

    uint8_t receivedMac[8];
    for (int i = 0; i < 8; i++) receivedMac[i] = LoRa.read();

    int encLen = packetSize - 14;
    uint8_t encBuf[64];
    for (int i = 0; i < encLen; i++) encBuf[i] = LoRa.read();

    if (targetId != NODE_ID) return;

    // Verify Message Authentication FIRST
    SHA256 sha;
    sha.resetHMAC(AES_KEY, 16);
    sha.update((uint8_t*)&targetId, sizeof(targetId));
    sha.update((uint8_t*)&rxSeqNum, sizeof(rxSeqNum));
    sha.update(encBuf, encLen);
    uint8_t expectedHmac[32];
    sha.finalizeHMAC(AES_KEY, 16, expectedHmac, 32);

    if (memcmp(receivedMac, expectedHmac, 8) != 0) return;

    // Decrypt
    uint8_t iv[16];
    memcpy(iv, BASE_IV, 16);
    iv[12] = (rxSeqNum >> 24) & 0xFF;
    iv[13] = (rxSeqNum >> 16) & 0xFF;
    iv[14] = (rxSeqNum >> 8) & 0xFF;
    iv[15] = rxSeqNum & 0xFF;

    uint8_t decBuf[64];
    ctr.setKey(AES_KEY, 16);
    ctr.setIV(iv, 16);
    ctr.decrypt(decBuf, encBuf, encLen);
    decBuf[encLen] = '\0';

    String msg = String((char*)decBuf);
    msg.trim();

    // Reset sequence handler on explicit valid sync request
    if (msg == "CMD|RESET_SEQ") {
      last_rx_sequence_number = rxSeqNum;
      return;
    }

    // Check anti-replay sequence number for normal execution
    if (rxSeqNum <= last_rx_sequence_number) return; 
    last_rx_sequence_number = rxSeqNum;

    int r1, r2, r3, r4;
    int parsed = sscanf(msg.c_str(), "CMD|%d|%d|%d|%d", &r1, &r2, &r3, &r4);

    if (parsed == 4) {
      digitalWrite(R1, r1 ? LOW : HIGH);
      digitalWrite(R2, r2 ? LOW : HIGH);
      digitalWrite(R3, r3 ? LOW : HIGH);
      digitalWrite(R4, r4 ? LOW : HIGH);

      String feedback = "RELAYS:" + String(r1) + "," + String(r2) + "," + String(r3) + "," + String(r4);
      delay(50);
      
      uint8_t fbBytes[64];
      uint8_t encFbBytes[64];
      int fbLen = feedback.length();
      memcpy(fbBytes, feedback.c_str(), fbLen);

      uint8_t txIv[16];
      memcpy(txIv, BASE_IV, 16);
      txIv[12] = (tx_sequence_number >> 24) & 0xFF;
      txIv[13] = (tx_sequence_number >> 16) & 0xFF;
      txIv[14] = (tx_sequence_number >> 8) & 0xFF;
      txIv[15] = tx_sequence_number & 0xFF;

      ctr.setKey(AES_KEY, 16);
      ctr.setIV(txIv, 16);
      ctr.encrypt(encFbBytes, fbBytes, fbLen);

      SHA256 fbSha;
      fbSha.resetHMAC(AES_KEY, 16);
      fbSha.update((uint8_t*)&NODE_ID, sizeof(NODE_ID));
      fbSha.update((uint8_t*)&tx_sequence_number, sizeof(tx_sequence_number));
      fbSha.update(encFbBytes, fbLen);
      uint8_t fbHmac[32];
      fbSha.finalizeHMAC(AES_KEY, 16, fbHmac, 32);

      LoRa.beginPacket();
      LoRa.write((uint8_t)(NODE_ID & 0xFF));
      LoRa.write((uint8_t)((NODE_ID >> 8) & 0xFF));
      LoRa.write((uint8_t)(tx_sequence_number & 0xFF));
      LoRa.write((uint8_t)((tx_sequence_number >> 8) & 0xFF));
      LoRa.write((uint8_t)((tx_sequence_number >> 16) & 0xFF));
      LoRa.write((uint8_t)((tx_sequence_number >> 24) & 0xFF));

      for (int i = 0; i < 8; i++) LoRa.write(fbHmac[i]);
      for (int i = 0; i < fbLen; i++) LoRa.write(encFbBytes[i]);
      LoRa.endPacket();

      tx_sequence_number++;
    }
  }
}
