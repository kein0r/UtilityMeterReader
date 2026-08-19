#include <RFM69.h>

RFM69::RFM69(arduino::MbedSPI* spi, uint8_t slaveSelectPin, uint8_t interruptPin, uint8_t *buffer):
    _slaveSelectPin(slaveSelectPin),
    _SPI(spi),
    _buffer(buffer)
{
}

bool RFM69::init(float frequency, uint16_t bitrate)
{
    bool retValue = false;
    uint8_t regValue;

    pinMode(_slaveSelectPin, OUTPUT);
    digitalWrite(_slaveSelectPin, HIGH);
    _SPI->begin();

    regValue = readRegister(RFM69_REGVERSION);
    /* Check if we can read version number and if it matches expected value */
    if (regValue == RFM69_REGVERSION_DEFAULT)
    {
        /* Place RFM69 in standby and restore default just in case */
        writeRegister(RFM69_REGOPMODE, RFM69_REGOPMODE_MODE_STBY);
        /* Enable packet mode with OOK */
        writeRegister(RFM69_REGDATAMODUL, RFM69_REGDATAMODUL_DATAMODE_PACKETMODE | RFM69_REGDATAMODUL_MODULATIONTYPE_OOK | RFM69_REGDATAMODUL_MODULATIONSHAPING_OOKNOSHAPING);
        setBitrate(bitrate);
        setFrequency(frequency);
        /* Set highest possible sample rate for now */
        setSampleRate(RFM69_REGRXBW_RXBWMANT_16, RFM69_REGRXBW_RXBWEXP_0);
        /* Enable Fixed Length Packet Format */
        writeRegister(RFM69_REGSYNCCONFIG, RFM69_REGSYNCCONFIG_SYNCON_OFF | RFM69_REGSYNCCONFIG_SYNCSIZE(0));
        writeRegister(RFM69_REGPACKETCONFIG1, RFM69_REGPACKETCONFIG1_PACKETFORMAT_FIXEDLENGTH | RFM69_REGPACKETCONFIG1_DCFREE_NONE | RFM69_REGPACKETCONFIG1_CRCON_OFF | RFM69_REGPACKETCONFIG1_CRCAUTOCLEAROFF_NOCLEAR | RFM69_REGPACKETCONFIG1_ADDRESSFILTERING_NONE);
        writeRegister(RFM69_REGPAYLOADLENGTH, 21);
        retValue = true;
    }
    return retValue;   
}

void RFM69::setFrequency(float centerFrequency)
{
    /* f_step = 32Mhz/2^19, RegFrf = f_rf/f_step = */
    uint32_t frf = (centerFrequency * 1000000.0) / RFM69_FSTEP;
    writeRegister(RFM69_REGFRFMSB, (frf >> 16) & 0xff);
    writeRegister(RFM69_REGFRFMID, (frf >> 8) & 0xff);
    writeRegister(RFM69_REGFRFLSB, frf & 0xff);
}

void RFM69::setBitrate(uint16_t bitrate)
{
    uint16_t regValue;
    regValue = 32000000 / bitrate;
    writeRegister(RFM69_REGBITRATEMSB, (regValue >> 8) & 0xff);
    regValue = bitrate & 0xff;
    writeRegister(RFM69_REGBITRATELSB, regValue  & 0xff);
}

void RFM69::setSampleRate(uint8_t RxBwMant, uint8_t RxBwExp)
{
    uint8_t regValue;
    regValue = RFM69_REGRXBW_DCCFREQ_4PERCENT | RxBwMant | RxBwExp;
    writeRegister(RFM69_REGRXBW, regValue);
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
    Serial.print("Write 0x");
    Serial.print(reg, HEX);
    Serial.print(" : 0b");
    Serial.println(value, BIN);
    digitalWrite(_slaveSelectPin, LOW);
    _SPI->transfer(reg | RFM69_REGWRITEMASK);
    _SPI->transfer(value);
    digitalWrite(_slaveSelectPin, HIGH);
}

uint8_t RFM69::readRegister(uint8_t reg)
{
    uint8_t regValue;

    digitalWrite(_slaveSelectPin, LOW);
    _SPI->transfer(reg);
    regValue = _SPI->transfer(0x00);
    digitalWrite(_slaveSelectPin, HIGH);
    return regValue;
}

void RFM69::printRegister(uint8_t reg)
{
    uint8_t regValue;
    regValue = readRegister(reg);
    Serial.print("Reg: 0x");
    Serial.print(reg, HEX);
    Serial.print(" Val: 0x");
    Serial.print(regValue, HEX);
    Serial.print(" -> 0b");
    Serial.println(regValue, BIN);
}
