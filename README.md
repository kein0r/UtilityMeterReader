# Utility Meter Reader

Raspberry RP2040 and RFM69 based utility meter reader based on Adafruit STEMMA QT board https://www.adafruit.com/product/5712 with simple spring antenna https://www.adafruit.com/product/4269.
Center frequency: 912.38MHz
Baudrate: 32768

## Protocols

According to (https://en.wikipedia.org/wiki/Encoder_receiver_transmitter)
> ERT is an OOK modulated radio signal which is transmitted in the unlicensed 900-920 MHz band
Similar de

### Neptune R900

https://github.com/merbanan/rtl_433/blob/master/src/devices/neptune_r900.c

## Itron

tbd

## Debug

### Utility Meter Reading Using SDR

To analyze the protocol and debuggin below two project are of great help

#### RTL_433

Documentation at [rtl_433](https://github.com/merbanan/rtl_433), install `apt install rtl-433`, run something like
```
rtl_433 -f 912380000 -s 2359296 -X "n=r900,m=OOK_PCM, s=30, l=30, r=320, preamble={8}0xaa"
```
to check if water meters in your area are broadcasting messages. Frequency and sample rate are taken from rtlamr project.