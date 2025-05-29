#ifndef RFM69_h
#define RFM69_h

#include <stdint.h>
#include <SPI.h>

/* Minimal implementation of RFM69 Radio library to read utility meter.
 * Existing implementation like RadioHead library (https://www.airspayce.com/mikem/arduino/RadioHead)
 * or LowPowerLab's RFM69 library (https://github.com/LowPowerLab/RFM69), even though very good, 
 * turned out to have too much overhead for what's needed and not support "Unlimited Length Packet 
 * Format".
 * Limitations:
 * - Only RX mode supported and only in "unlimited length packet format" mode (PacketFormat set to 0
 *   and PayloadLength set to 0
 * - Application needs to provide big enough buffer and receive needs to be called often enough to not 
 *   loose/miss data
 * 
 */
 
#define RFM69_REGFIFO                         0x00
#define RFM69_REGOPMODE                       0x01
#define RFM69_REGOPMODE_MODE_SLEEP            (0x0 << 2)
#define RFM69_REGOPMODE_MODE_STBY             (0x1 << 2)
#define RFM69_REGOPMODE_MODE_FS               (0x2 << 2)
#define RFM69_REGOPMODE_MODE_TX               (0x3 << 2)
#define RFM69_REGOPMODE_MODE_RX               (0x4 << 2)
#define RFM69_REGOPMODE_MODE_MASK             (0x7 << 2)

#define RFM69_REGDATAMODUL                                             0x02
#define RFM69_REGDATAMODUL_MODULATIONSHAPING_OOKNOSHAPING              (0x0 << 0)
#define RFM69_REGDATAMODUL_MODULATIONSHAPING_OOKFILTERINGBR            (0x1 << 0)
#define RFM69_REGDATAMODUL_MODULATIONSHAPING_OOKFILTERING2BR           (0x2 << 0)
#define RFM69_REGDATAMODUL_MODULATIONTYPE_FSK                          (0x0 << 3)
#define RFM69_REGDATAMODUL_MODULATIONTYPE_OOK                          (0x1 << 3)
#define RFM69_REGDATAMODUL_DATAMODE_PACKETMODE                         (0x0 << 5)
#define RFM69_REGDATAMODUL_DATAMODE_CONTINUOUSMODEWPACKETSYNCHRONIZER  (0x2 << 5)
#define RFM69_REGDATAMODUL_DATAMODE_CONTINUOUSMODEWOPACKETSYNCHRONIZER (0x3 << 5)

#define RFM69_REGBITRATEMSB    0x03
#define RFM69_REGBITRATELSB    0x04
#define RFM69_REGBITRATE_32768 0xd103

#define RFM69_REGVERSION         0x10
#define RFM69_REGVERSION_DEFAULT 0x24

#define RFM69_REGPREAMBLEMSB 0x02c
#define RFM69_REGPREAMBLELSB 0x02d
#define RFM69_REGSYNCCONFIG 0x0e
#define RFM69_REGSYNCCONFIG_SYNCSIZE_MASK (0x7 << 3)
#define RFM69_REGSYNCCONFIG_FIFOFILLCONDITION
#define RFM69_REGSYNCCONFIG_SYNCON_ON

class RFM69
{
public:
  RFM69(uint8_t slaveSelectPin = SS, uint8_t interruptPin = 2, uint8_t *buffer = nullptr);
  bool init(uint16_t baudrate = );
  void setFrequency(float centerFrequency);
  void setBaudrate(uint16_t baudrate);
  void setMode(uint8_t mode);
  void writeRegister (uint8_t reg, uint8_t value);
  uint8_t readRegister (uint8_t reg);
  void printRegister(uint8_t reg);
  
  /**
  Receive function to be called regularly by application to copy available data from RFM69 FIFO to
  buffer.
  \return Buffer ready and number of bytes in buffer
  */
  uint8_t receive();

private:
  const uint8_t _slaveSelectPin;

  /** Pointer to application provided buffer to copy fifo content to */
  const uint8_t *_buffer;
};

#endif /* #define RFM69_h */