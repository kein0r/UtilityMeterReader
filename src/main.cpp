#include <Arduino.h>
#include <SPI.h>
#include <RFM69.h>

/**
 * RFM69 related definitions
*/
#define RFM69_FREQ 912.38
#define RFM69_CS   16
#define RFM69_INT  21
#define RFM69_RST  17
#define LED        LED_BUILTIN

bool rxBufferValid = false;
uint8_t rxBufferIDM[0x5c];

// Singleton instance of the radio driver
RFM69 rfm69(RFM69_CS, RFM69_INT, rxBufferIDM);

/*
 * Interval Data Message (IDM) related stuff
*/
uint8_t IDMMessageSyncWord[] = { 0xaa, 0xaa };
#define IDM_PREAMBLELENGTH     (uint8_t)2
#define IDM_SYNCWORDLENGTH     (uint8_t)sizeof(IDMMessageSyncWord)

#if 0
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

#endif

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
  if (!rfm69.init(912.38, 32768)) {
    Serial.println("RFM69 radio init failed");
    while (1);
  }
  Serial.println("RFM69 radio init OK!");

  rfm69.setFrequency(RFM69_FREQ);
} 

void loop() {
 if (rxBufferValid)
 {
    Serial.println();
    Serial.print(millis());
    Serial.print(": Rcvd: ");
 }
 delay(5000);
 rfm69.printRegister(RFM69_REGOPMODE);
 rfm69.printRegister(RFM69_REGDATAMODUL);
 rfm69.printRegister(RFM69_REGBITRATEMSB);
 rfm69.printRegister(RFM69_REGBITRATELSB);
}