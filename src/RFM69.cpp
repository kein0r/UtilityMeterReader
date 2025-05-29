#include <RFM69.h>

RFM69::RFM69(uint8_t slaveSelectPin, uint8_t interruptPin, uint8_t *buffer):
    _slaveSelectPin(slaveSelectPin),
    _buffer(buffer)
{
}

bool RFM69::init(uint16_t baudrate)
{
    bool retValue = false;
    uint8_t regValue;

    pinMode(_slaveSelectPin, OUTPUT);
    digitalWrite(_slaveSelectPin, HIGH);
    SPI.begin();

    regValue = readRegister(RFM69_REGVERSION);
    /* Check if we can read version number and if it matches expected value */
    if (regValue == RFM69_REGVERSION_DEFAULT)
    {
        /* Place RFM69 in standby and restore default just in case */
        writeRegister(RFM69_REGOPMODE, RFM69_REGOPMODE_MODE_STBY);
        /* Enable packet mode with OOK */
        writeRegister(RFM69_REGDATAMODUL, RFM69_REGDATAMODUL_MODULATIONSHAPING_OOKNOSHAPING | RFM69_REGDATAMODUL_MODULATIONTYPE_OOK | RFM69_REGDATAMODUL_DATAMODE_PACKETMODE);
        setBaudrate(baudrate);
        /* "Unlimited length packet format is selected when bit PacketFormat is set to 0 and PayloadLength is 
         * set to 0 ... This mode is a replacement for the legacy buffered mode in RF63/RF64 transceivers" */
        writeRegister(RFM69_REGPACKETCONFIG1, RFM69_REGPACKETCONFIG1_ADDRESSFILTERING_NONE | RFM69_REGPACKETCONFIG1_CRCAUTOCLEAROFF_NOCLEAR | RFM69_REGPACKETCONFIG1_CRCON_OFF | RFM69_REGPACKETCONFIG1_DCFREE_NONE | RFM69_REGPACKETCONFIG1_PACKETFORMAT_VARIABLELENGTH );
        writeRegister(RFM69_REGPAYLOADLENGTH, 0x0);
        retValue = true;
    }
    return retValue;   
}

void RFM69::setBaudrate(uint16_t baudrate)
{
    uint8_t regValue;
    regValue = (baudrate >> 8) & 0xff;
    writeRegister(RFM69_REGBITRATEMSB, regValue);
    regValue = baudrate & 0xff;
    writeRegister(RFM69_REGBITRATELSB, regValue);
}

void RFM69::setPreamble(uint8_t *preamble, uint8_t length)
{
    writeRegister(RFM69_REGPREAMBLEMSB, 0x0);
    writeRegister(RFM69_REGPREAMBLELSB, 0x0);
}

void RFM69::setMode(uint8_t mode)
{
    uint8_t modeRegValue;
    modeRegValue = readRegister(RFM69_REGOPMODE);
    modeRegValue = (modeRegValue & ~RFM69_REGOPMODE_MODE_MASK) | mode;
    writeRegister(RFM69_REGOPMODE, modeRegValue);
}

void RFM69::writeRegister(uint8_t reg, uint8_t value)
{
    digitalWrite(_slaveSelectPin, LOW);
    SPI.transfer(reg);
    SPI.transfer(value);
    digitalWrite(_slaveSelectPin, HIGH);
}

uint8_t RFM69::readRegister(uint8_t reg)
{
    uint8_t regValue;
    digitalWrite(_slaveSelectPin, LOW);
    SPI.transfer(reg);
    regValue = SPI.transfer(0x00);
    digitalWrite(_slaveSelectPin, HIGH);
    return regValue;
}

void RFM69::printRegister(uint8_t reg)
{
    uint8_t regValue;
    regValue = readRegister(reg);
    Serial.print("Reg: ");
    Serial.print(reg);
    Serial.print(" Val: ");
    Serial.println(regValue);
}
