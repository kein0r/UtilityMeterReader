# RFM69HCW Confguration
Reference: [RFM69HCW ISM Transceiver Module v1.1](https://cdn.sparkfun.com/datasheets/Wireless/General/RFM69HCW-V1.1.pdf).
This is a summary or paste-bin of the information relevant for this project from the datasheet.

# General Analysis and Description

## Listen Mode
RFM69HCW spends most of the time in Idle mode. Periodically the receiver is woken up and listens for an RF signal.
- The duration of the Idle phase is given by tListenIdle
- The time during which the receiver is on and waits for a signal is given by tListenRx
- Both are calculated as follows: tListenX = ListenCoefX ∗ ListenResolX


## Package Mode
Technically "5.5.2.1. Fixed Length Packet Format" should be usable configuring it to
- Preamble: 0x55
- Sync word: 0xA9666965, length 4
- No address bytes
- Payload length 21
- No CRC

Fixed length packet format is selected when bit PacketFormat is set to 0 and PayloadLength is set to any value greater
than 0.  
Downside of this is that only one type of meter can be read at any given point in time.

## Registers
Registers used. All non-mentioned bits default to 0

## Common Registers

| Name          | Address | Bits  | Name                  | Description                  |
| ----          | ------- | ----  | ----                  | -----------                  |
| RegOpMode     | 0x01    | 6     | ListenOn              | Enables listen mode, enabled while in stand-by |
|               |         | 4-2   | Mode                  | 001 → Standby mode (STDBY) <br> 100 → Receiver mode (RX)|
| RegDataModul  | 0x02    | 6-5   | DataMode              | 00 → Packet mode             |
|               |         | 4-3   | ModulationType        | 01 → OOK                     |
|               |         | 1-0   | ModulationShaping     | 00 → no shaping (default) tbc |
| RegFrfMsb     | 0x07    | 7-0   | Fdev                  | MSB of the RF carrier frequency |
| RegFrfMid     | 0x08    | 7-0   | Fdev                  | Middle byte of the RF carrier frequency |
| RegFrfLsb     | 0x09    | 7-0   | Fdev                  | LSB of the RF carrier frequency |
| RegListen1    | 0x0d    | 7-6   | ListenResolIdle       | Resolution of Listen mode Idle time <br> 10 → 4.1 ms (Default) |
|               |         | 5-4   | ListenResolRx         | Resolution of Listen mode Rx time <br> 01 → 64 us (Default) |
|               |         | 2-1   | ListenEnd             | Action taken after acceptance of a packet in Listen mode <br> 01 → chip stays in Rx mode until PayloadReady or Timeout interrupt occurs. It then goes to the mode defined by Mode. Listen mode stops and must be disabled <br> 10 → chip stays in Rx mode until PayloadReady or Timeout interrupt occurs. Listen mode then resumes in Idle state. FIFO content is lost at next Rx wakeup. |
| RegListen2    | 0x0e    | 7-0   | ListenCoefIdle        | Duration of the Idle phase in Listen mode |
| RegListen3    | 0x0f    | 7-0   | ListenCoefRx          | Duration of the Rx phase in Listen mode |

## Receiver Registers
| Name          | Address | Bits  | Name                  | Description                  |
| ----          | ------- | ----  | ----                  | -----------                  |

## Registers to be checked
| Name          | Address | Bits  | Name                  | Description                  |
| ----          | ------- | ----  | ----                  | -----------                  |
| RegBitrateMsb | 0x03    | 7-0   | BitRate               | MSB of Bit Rate (Chip Rate when Manchester encoding is enabled) |
| RegBitrateLsb | 0x04    | 7-0   | BitRate               | LSB of Bit Rate (Chip Rate when Manchester encoding is enabled) |
| RegFdevMsb    | 0x05    | 5-0   | DataMode              | MSB of the frequency deviation |
| RegFdevLsb    | 0x06    | 7-0   | DataMode              | LSB of the frequency deviation |

## Register values 
Frf = Fstep ⋅ Frf 23;0 (Default: 915 = 32MHz ⋅14,991,360) tbc

## RegListen1, RegListen2, RegListen3
Power consumption is not a problem. Minimize idle time, maximize Rx time.
- RegListen1 0b01110010 = 0x72
  - 7-6: ListenResolIdle 01
  - 5-4: ListenResolRx 11
  - 3: ListenCriteria 0
  - 2-1: ListenEnd 01
  - 0: 0
- RegListen2 ListenCoefIdle  = 0x01. 64us * 1 = 64us
- RegListen3 ListenCoefRx  = 0xff. 0.0262s * 255 = 67s


# Initialization
1. Switch device to Stand-by Mode (RegOpMode: ListenOn = 0, Mode = 0x1)
3. Transition device to Listen Mode RegOpMode: ListenOn = 1
4. Set RegListen3:ListenEnd = 0x00 to enable continuous listening
5. Packet mode (recommended)

# To be checked
- tListenIdle in Listen Mode?
- idel time check for end of package
- Bit rate for OOK RX RegBitrateMsb and RegBitrateLsb seems to only be for Manchester encoding?
- ListenEnd value!
- "Timeout interrupt occurs"?

# Datasheet
Important Chapters are
- 4.3. Listen Mode
- 5.1.2. Data Operation Modes
- 5.5.2.1. Fixed Length Packet Format
