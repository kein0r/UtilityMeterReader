# Utility Meter Reader

Raspberry RP2040 and RFM69 based utility meter reader based on Adafruit STEMMA QT board https://www.adafruit.com/product/5712 with simple spring antenna https://www.adafruit.com/product/4269.
Center frequency: 912.38MHz
Baudrate: 32768

## Protocols

According to (https://en.wikipedia.org/wiki/Encoder_receiver_transmitter)
> ERT is an OOK modulated radio signal which is transmitted in the unlicensed 900-920 MHz band

### Neptune R900

Meter sends consumption messages roughly every 50s on 911.0815 to 919.0769MHz and can be read by this project.

Reference: [Neptune R900](https://fcc.io/P2SR900M)

## Itron

Seems like the Itron meter come in two versions. A version referred to as ERT and one marked as 
OpenWay. While the first sends data on 902.25 to 927.75 and can potentially be read by this project the 
second one seems to use encrypted ZigBee for which a special device is needed.

Reference: [Itron C2SOD](https://fcc.io/SK9AMI-3)
