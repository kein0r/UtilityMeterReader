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
 * - Only RX mode supported
 * - Application needs to provide big enough buffer and receive needs to be called often enough to not 
 *   loose/miss data
 * 
 */
#define RFM69_REGWRITEMASK 0x80

/* 3.2.3. PLL Architecture: F_Step = F_Xosc/2^19 */
#define RFM69_FXOSC 32000000.0
#define RFM69_FSTEP float(RFM69_FXOSC / 524288)

/* 6.2. Common Configuration Registers */
#define RFM69_REGFIFO                         0x00
#define RFM69_REGOPMODE                       0x01
#define RFM69_REGOPMODE_MODE_SLEEP            (0x0 << 2)
#define RFM69_REGOPMODE_MODE_STBY             (0x1 << 2)
#define RFM69_REGOPMODE_MODE_FS               (0x2 << 2)
#define RFM69_REGOPMODE_MODE_TX               (0x3 << 2)
#define RFM69_REGOPMODE_MODE_RX               (0x4 << 2)
#define RFM69_REGOPMODE_MODE_MASK             (0x7 << 2)

#define RFM69_REGDATAMODUL                                             0x02
#define RFM69_REGDATAMODUL_DATAMODE_PACKETMODE                         (0x0 << 5)
#define RFM69_REGDATAMODUL_DATAMODE_CONTINUOUSMODEWPACKETSYNCHRONIZER  (0x2 << 5)
#define RFM69_REGDATAMODUL_DATAMODE_CONTINUOUSMODEWOPACKETSYNCHRONIZER (0x3 << 5)
#define RFM69_REGDATAMODUL_MODULATIONTYPE_FSK                          (0x0 << 3)
#define RFM69_REGDATAMODUL_MODULATIONTYPE_OOK                          (0x1 << 3)
#define RFM69_REGDATAMODUL_MODULATIONSHAPING_OOKNOSHAPING              (0x0 << 0)
#define RFM69_REGDATAMODUL_MODULATIONSHAPING_OOKFILTERINGBR            (0x1 << 0)
#define RFM69_REGDATAMODUL_MODULATIONSHAPING_OOKFILTERING2BR           (0x2 << 0)

#define RFM69_REGBITRATEMSB    0x03
#define RFM69_REGBITRATELSB    0x04
#define RFM69_REGFDEVMSB       0x05
#define RFM69_REGFDEVLSB       0x06

#define RFM69_REGFRFMSB 0x07
#define RFM69_REGFRFMID 0x08
#define RFM69_REGFRFLSB 0x09

#define RFM69_REGLISTEN1 0x0d
#define RFM69_REGLISTEN1_LISTENRESOLIDLE_64us                (0x1 << 6)
#define RFM69_REGLISTEN1_LISTENRESOLIDLE_4ms2                (0x2 << 6)
#define RFM69_REGLISTEN1_LISTENRESOLIDLE_262ms               (0x3 << 6)
#define RFM69_REGLISTEN1_LISTENRESOLRX_64us                  (0x1 << 4)
#define RFM69_REGLISTEN1_LISTENRESOLRX_4ms2                  (0x2 << 4)
#define RFM69_REGLISTEN1_LISTENRESOLRX_262ms                 (0x3 << 4)
#define RFM69_REGLISTEN1_LISTENCRITERIA_RSSI                 (0x0 << 3)
#define RFM69_REGLISTEN1_LISTENCRITERIA_RSSIANDSYNCADDRESS   (0x1 << 3)
#define RFM69_REGLISTEN1_LISTENEND_STAYRXMODE                (0x0 << 1)
#define RFM69_REGLISTEN1_LISTENEND_STAYRXMODEUNTILPAYLOADRDY (0x1 << 1)
#define RFM69_REGLISTEN1_LISTENEND_STAYRXMODETHENIDLE        (0x3 << 1)

#define RFM69_REGLISTEN2 0x0e
#define RFM69_REGLISTEN3 0x0f

#define RFM69_REGVERSION         0x10
#define RFM69_REGVERSION_DEFAULT 0x24

/* 6.3. Transmitter Registers */

/* 6.4. Receiver Registers */
#define RFM69_REGRXBW 0x19
#define RFM69_REGRXBW_DCCFREQ_16P    (0x0 << 5)
#define RFM69_REGRXBW_DCCFREQ_8P     (0x1 << 5)
#define RFM69_REGRXBW_DCCFREQ_4P     (0x2 << 5)
#define RFM69_REGRXBW_DCCFREQ_2P     (0x3 << 5)
#define RFM69_REGRXBW_DCCFREQ_1P     (0x4 << 5)
#define RFM69_REGRXBW_DCCFREQ_0P5    (0x5 << 5)
#define RFM69_REGRXBW_DCCFREQ_0P25   (0x6 << 5)
#define RFM69_REGRXBW_DCCFREQ_0P125  (0x7 << 5)
#define RFM69_REGRXBW_RXBWMANT_16    (0x0 << 3)
#define RFM69_REGRXBW_RXBWMANT_20    (0x1 << 3)
#define RFM69_REGRXBW_RXBWMANT_24    (0x2 << 3)
#define RFM69_REGRXBW_RXBWEXP_0      (0x0 << 0)
#define RFM69_REGRXBW_RXBWEXP_1      (0x1 << 0)
#define RFM69_REGRXBW_RXBWEXP_2      (0x2 << 0)
#define RFM69_REGRXBW_RXBWEXP_3      (0x3 << 0)
#define RFM69_REGRXBW_RXBWEXP_4      (0x4 << 0)
#define RFM69_REGRXBW_RXBWEXP_5      (0x5 << 0)
#define RFM69_REGRXBW_RXBWEXP_6      (0x6 << 0)
#define RFM69_REGRXBW_RXBWEXP_7      (0x7 << 0)

#define RFM69_REGAFCBW 0x1a

#define RFM69_REGOOKPEAK 0x1b
#define RFM69_REGOOKPEAK_OOKTHRESHTYPE_FIXED    (0x0 << 6)
#define RFM69_REGOOKPEAK_OOKTHRESHTYPE_PEAK     (0x1 << 6)
#define RFM69_REGOOKPEAK_OOKTHRESHTYPE_AVERAGE  (0x2 << 6)
#define RFM69_REGOOKPEAK_OOKPEAKTHESHSTEP_0dB5  (0x0 << 3)
#define RFM69_REGOOKPEAK_OOKPEAKTHESHSTEP_1dB0  (0x1 << 3)
#define RFM69_REGOOKPEAK_OOKPEAKTHESHSTEP_1dB5  (0x2 << 3)
#define RFM69_REGOOKPEAK_OOKPEAKTHESHSTEP_2dB0  (0x3 << 3)
#define RFM69_REGOOKPEAK_OOKPEAKTHESHSTEP_3dB0  (0x4 << 3)
#define RFM69_REGOOKPEAK_OOKPEAKTHESHSTEP_4dB0  (0x5 << 3)
#define RFM69_REGOOKPEAK_OOKPEAKTHESHSTEP_5dB0  (0x6 << 3)
#define RFM69_REGOOKPEAK_OOKPEAKTHESHSTEP_6dB0  (0x7 << 3)
#define RFM69_REGOOKPEAK_OOKPEAKTHRESHDEC_ONCEPERCHIP (0x0 < 0)

/* 6.6. Packet Engine Registers */
#define RFM69_REGSYNCCONFIG 0x2e
#define RFM69_REGSYNCCONFIG_SYNCON_OFF   (0x0 << 7)
#define RFM69_REGSYNCCONFIG_SYNCON_ON    (0x1 << 7)
#define RFM69_REGSYNCCONFIG_SYNCSIZE(a)  ((a & 0x7) << 3)


#define RFM69_REGSYNCVALUE1 0x2f
#define RFM69_REGSYNCVALUE2 0x30
#define RFM69_REGSYNCVALUE3 0x31
#define RFM69_REGSYNCVALUE4 0x32
#define RFM69_REGSYNCVALUE5 0x33
#define RFM69_REGSYNCVALUE6 0x34
#define RFM69_REGSYNCVALUE7 0x35
#define RFM69_REGSYNCVALUE8 0x36

#define RFM69_REGPACKETCONFIG1                                  0x37
#define RFM69_REGPACKETCONFIG1_PACKETFORMAT_FIXEDLENGTH         (0x0 << 7)
#define RFM69_REGPACKETCONFIG1_PACKETFORMAT_VARIABLELENGTH      (0x1 << 7)
#define RFM69_REGPACKETCONFIG1_DCFREE_NONE                      (0x0 << 5)
#define RFM69_REGPACKETCONFIG1_DCFREE_MANCHESTER                (0x1 << 5)
#define RFM69_REGPACKETCONFIG1_DCFREE_WHITENING                 (0x2 << 5)
#define RFM69_REGPACKETCONFIG1_CRCON_OFF                        (0x0 << 4)
#define RFM69_REGPACKETCONFIG1_CRCON_ON                         (0x1 << 4)
#define RFM69_REGPACKETCONFIG1_CRCAUTOCLEAROFF_CLEAR            (0x0 << 3)
#define RFM69_REGPACKETCONFIG1_CRCAUTOCLEAROFF_NOCLEAR          (0x1 << 3)
#define RFM69_REGPACKETCONFIG1_ADDRESSFILTERING_NONE            (0x0 << 1)
#define RFM69_REGPACKETCONFIG1_ADDRESSFILTERING_NODEADDRESS     (0x1 << 1)
#define RFM69_REGPACKETCONFIG1_ADDRESSFILTERING_NODEORBCADDRESS (0x2 << 1)

#define RFM69_REGPAYLOADLENGTH 0x38

/* 6.5. IRQ and Pin Mapping Registers */
#define RFM69_REGIRQFLAGS1 0x27
#define RFM69_REGIRQFLAGS2              0x28
#define RFM69_REGIRQFLAGS2_FIFOFULL     (0x1 << 7)
#define RFM69_REGIRQFLAGS2_FIFONOTEMPTY (0x1 << 6)
#define RFM69_REGIRQFLAGS2_FIFOLEVEL    (0x1 << 5)
#define RFM69_REGIRQFLAGS2_FIFOOVERRUN  (0x1 << 4)
#define RFM69_REGIRQFLAGS2_PACKETSENT   (0x1 << 3)
#define RFM69_REGIRQFLAGS2_PAYLOADREADY (0x1 << 2)
#define RFM69_REGIRQFLAGS2_CRCOK        (0x1 << 1)

#define RFM69_REGPAYLOADLENGTH 0x38

class RFM69
{
public:
  RFM69(arduino::MbedSPI* spi, uint8_t slaveSelectPin = SS, uint8_t interruptPin = 2, uint8_t *buffer = nullptr);
  bool init(float frequency = 912.38, uint16_t bitrate = 32768);
  void setFrequency(float centerFrequency);
  void setBitrate(uint16_t bitrate);
  void setBandwidth(uint8_t RxBwMant, uint8_t RxBwExp);
  void setMode(uint8_t mode);
  void setSyncWord(uint8_t *syncWord, uint8_t length);
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
  arduino::MbedSPI* _SPI;
  const uint8_t _slaveSelectPin;

  /** Pointer to application provided buffer to copy fifo content to */
  const uint8_t *_buffer;
};

#endif /* #define RFM69_h */