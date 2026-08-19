# RFM69HCW Confguration
Reference: [RFM69HCW ISM Transceiver Module v1.1](https://cdn.sparkfun.com/datasheets/Wireless/General/RFM69HCW-V1.1.pdf).
This is a summary or paste-bin of the information relevant for this project from the datasheet.

# Datasheet
Important Chapters in datasheet
- 3.4.12. OOK Demodulator
- 4.3. Listen Mode
- 5.1.2. Data Operation Modes
- 5.5.2.1. Fixed Length Packet Format

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

| Preamble <br> 0 to 65535 <br> bytes  | Sync Word <br> 0 to 8 bytes | Message <br> Up to 255 bytes| CRC <br> 2 bytes |
| --                                   | --                          | --                          | --               |
|                                      |                             | Payload (min 1 byte)        |                  |

Fixed length packet format is selected when bit PacketFormat is set to 0 and PayloadLength is set to any value greater
than 0.  
Downside of this is that only one type of meter can be read at any given point in time.

## Registers
Registers used. All non-mentioned bits default to 0

## Common Registers

| Name          | Address | Bits  | Name                  | Description                  |
| ----          | ------- | ----  | ----                  | -----------                  |
| RegOpMode     | 0x01    | 4-2   | Mode                  | 001 → Standby mode (STDBY) <br> 100 → Receiver mode (RX)|
| RegDataModul  | 0x02    | 6-5   | DataMode              | 00 → Packet mode             |
|               |         | 4-3   | ModulationType        | 01 → OOK                     |
|               |         | 1-0   | ModulationShaping     | 00 → no shaping (default) tbc |
| RegBitrateMsb | 0x03    | 6-5   | DataMode              | MSB of Bit Rate              |
| RegBitrateLsb | 0x04    | 6-5   | DataMode              | LSB of Bit Rate              |
| RegFrfMsb     | 0x07    | 7-0   | Fdev                  | MSB of the RF carrier frequency |
| RegFrfMid     | 0x08    | 7-0   | Fdev                  | Middle byte of the RF carrier frequency |
| RegFrfLsb     | 0x09    | 7-0   | Fdev                  | LSB of the RF carrier frequency |
| RegListen1    | 0x0d    | 7-6   | ListenResolIdle       | Resolution of Listen mode Idle time <br> 01 → 64 us |
|               |         | 5-4   | ListenResolRx         | Resolution of Listen mode Rx time <br> 11 → 262 ms |
|               |         | 2-1   | ListenEnd             | Action taken after acceptance of a packet in Listen mode <br> 01 → chip stays in Rx mode until PayloadReady or Timeout interrupt occurs. It then goes to the mode defined by Mode. Listen mode stops and must be disabled <br> 10 → chip stays in Rx mode until PayloadReady or Timeout interrupt occurs. Listen mode then resumes in Idle state. FIFO content is lost at next Rx wakeup. |
| RegListen2    | 0x0e    | 7-0   | ListenCoefIdle        | Duration of the Idle phase in Listen mode |
| RegListen3    | 0x0f    | 7-0   | ListenCoefRx          | Duration of the Rx phase in Listen mode |

## Receiver Registers
| Name          | Address | Bits  | Name                  | Description                  |
| ----          | ------- | ----  | ----                  | -----------                  |
| RegRxBw       | 0x19    | 7-5   | DccFreq               | Cut-off frequency of the DC offset canceller (DCC). Default 010 |
|               |         | 4-3   | RxBwMant              | Channel filter bandwidth control <br> 00 → RxBwMant = 16 |
|               |         | 2-0   | RxBwExp               | Channel filter bandwidth control exponent 00 (see below) |

## Packet Engine Registers
| Name          | Address | Bits | Name                  | Description                  |
| ----          | ------- | ---- | ----                  | -----------                  |
| RegSyncConfig | 0x2e    | 7    | SyncOn                | Enables the Sync word detection 1 |
|               |         | 5-3  | SyncSize              | Size of the Sync word (SyncSize + 1) bytes |
|               |         | 2-0  | SyncTol               | Number of tolerated bit errors in Sync word 000 |
| RegSyncValuex | 0x2f-36 | 7-0  | SyncValue             | n-th byte of Sync word |
| RegPacketConfig1 | 0x37 | 7    | PacketFormat          | Defines the packet format used <br> 0 → Fixed length |
|               |         | 6-5  | DcFree                | Defines DC-free encoding/decoding performed: <br> 00 → None (Off) (Default) |
|               |         | 4    | CrcOn                 | Enables CRC calculation/check (Tx/Rx):<br> 0 → Off |
|               |         | 3    | CrcAutoClearOff       | Defines the behavior of the packet handler when CRC check fails:<br> 1 → Do not clear FIFO. PayloadReady interrupt issued |
|               |         | 2-1  | AddressFiltering      | Enables CRC calculation/check (Tx/Rx):<br> Defines address based filtering in Rx: 00 → None (Off) |
| RegPayloadLength | 0x38 | 7-0  | PayloadLength         | If PacketFormat = 0 (fixed), payload length |

## Register values 

### RegDataModul
- RegDataModul: 0b00001000
  - Unused 0
  - DataMode: 00
  - ModulationType 01
  - Unused 0
  - ModulationShaping 00

### RegBitrate, RegBitrate, RegFrf
Bit rate = 32768, FXO = 32MHz RegBitrage = 32000000/32768 = 976,5625 (0x3d0)
- RegBitrateMsb: 0x03
- RegBitrateLsb: 0xd0

Target frequency: 912.38
- f_step = 32Mhz/2^19
- f_rf = f_step * RegFrf
- RegFrf = f_rf/f_step = f_rf * 2^19/32MHz = 14948433.92 = 0xE41851

### RegListen1, RegListen2, RegListen3
Power consumption is not a problem. Minimize idle time, maximize Rx time.
- RegListen1 0b01110010 = 0x72
  - 7-6: ListenResolIdle 01
  - 5-4: ListenResolRx 11
  - 3: ListenCriteria 0
  - 2-1: ListenEnd 01
  - 0: 0
- RegListen2 ListenCoefIdle  = 0x01. 64us * 1 = 64us
- RegListen3 ListenCoefRx  = 0xff. 0.0262s * 255 = 67s

### RegRxBw
Maximum sample rate for OOK (Table 14 Available RxBw Settings) is set to 250kHz
| RxBwMant <br> (binary/value) | RxBwExp <br> (decimal) | RxBw (kHz) OOK ModulationType=01 |
| ---------------------------- | ---------------------- | -------------------------------- |
| 00b / 16                     | 0                      | 250.0                            |

- RegRxBw: 0b01000000

# Initialization
1. Switch device to Stand-by Mode (RegOpMode: ListenOn = 0, Mode = 0x1)
3. Transition device to Listen Mode RegOpMode: ListenOn = 1
4. Set RegListen3:ListenEnd = 0x00 to enable continuous listening
5. Packet mode (recommended)

# To be checked
- idel time check for end of package
- "Timeout interrupt occurs"?

