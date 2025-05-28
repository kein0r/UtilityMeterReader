#include <Arduino.h>
#include <SPI.h>
#include <RH_RF69.h>

/**
 * RFM69 related definitions
*/
#define RFM69_FREQ 912.38
#define RFM69_CS   16
#define RFM69_INT  21
#define RFM69_RST  17
#define LED        LED_BUILTIN

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

/*
 * Interval Data Message (IDM) related stuff
*/
uint8_t IDMMessageSyncWord[] = { 0xaa, 0xaa };
#define IDM_PREAMBLELENGTH     (uint8_t)2
#define IDM_SYNCWORDLENGTH     (uint8_t)sizeof(IDMMessageSyncWord)

bool rxBufferValid = false;
uint8_t rxBufferIDM[0x5c];

void RF69ISR()
{ 
  // Get the interrupt cause
  ATOMIC_BLOCK_START;
  uint8_t irqflags2 = rf69.spiRead(RH_RF69_REG_28_IRQFLAGS2);
  if (irqflags2 & RH_RF69_IRQFLAGS2_PAYLOADREADY)
  {
    // A complete message has been received with (good) CRC
    //lastRSSI = -((int8_t)(rf69.spiRead(RH_RF69_REG_24_RSSIVALUE) >> 1));
    //lastCRCOk = rf69.spiRead((RH_RF69_REG_28_IRQFLAGS2 & RH_RF69_IRQFLAGS2_CRCOK) >> 1);
  
    rf69.setModeIdle();
    rf69.spiBurstRead(RH_RF69_REG_00_FIFO, rxBufferIDM, 0x5c-2);
    rf69.setModeRx();
    rxBufferValid = true;
  }
  ATOMIC_BLOCK_END;
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);
  Serial.begin(115200);

  // reset RF module before init
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  delay(5000);
  if (!rf69.init()) {
    Serial.println("RFM69 radio init failed");
    while (1);
  }
  Serial.println("RFM69 radio init OK!");

  if (!rf69.setFrequency(RFM69_FREQ)) {
    Serial.println("setFrequency failed");
  }
  rf69.setModemConfig(RH_RF69::OOK_Rb2_4Bw4_8);
  /* Change to fix-length, Manchester encoding and set to trigger ISR even if CRC does not match. */
  rf69.spiWrite(RH_RF69_REG_37_PACKETCONFIG1, (RH_RF69_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RH_RF69_PACKETCONFIG1_DCFREE_MANCHESTER  | RH_RF69_PACKETCONFIG1_CRCAUTOCLEAROFF | RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NONE));
  //rf69.spiWrite(RH_RF69_REG_38_PAYLOADLENGTH, 0x5c);  /* 0x5c minus preamble and CRC (each two bytes) */
  rf69.setPreambleLength(IDM_PREAMBLELENGTH);
  /* map interrupt to own function */
  attachInterrupt(digitalPinToInterrupt(RFM69_INT), RF69ISR, RISING);
  /* rf69.setSyncWords(IDMMessageSyncWord, IDM_SYNCWORDLENGTH); */
  rf69.setSyncWords(IDMMessageSyncWord, 0);
  rf69.setModeRx(); // Start receiving, we never switch out of this mode
} 

void loop() {
 if (rxBufferValid)
 {
    Serial.println();
    Serial.print(millis());
    Serial.print(": Rcvd: ");
    ATOMIC_BLOCK_START;
    for (int i=0; i<sizeof(rxBufferIDM); i++)
    {
      Serial.print(rxBufferIDM[i], HEX);
      Serial.print(" ");
    }
    rxBufferValid = false;
    ATOMIC_BLOCK_END;
 }
}