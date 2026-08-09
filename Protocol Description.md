# Utility Meter RF Protocol Reference

Seems like all meters broadcast in the unlicensed 900–920 MHz band using OOK (On-Off Keying)
modulation at 32.768 bps on 912.38 MHz.  The protocol family is called ERT (Encoder 
Receiver Transmitter), originally defined by Itron.

---

## Common RF Parameters

| Parameter       | Value                  |
|-----------------|------------------------|
| Frequency       | 912.38 MHz             |
| Modulation      | OOK (PCM)              |
| Bit rate        | 32 768 bps             |
| Symbol duration | ~30.5 µs               |
| End-of-frame    | gap > 320 µs           |

---

## Neptune R900 (Water Meter)

### Frame Structure

| Section  | Raw on-air bytes          | Purpose                              |
|----------|---------------------------|--------------------------------------|
| Preamble | `0x55 0x55 0x55`           | Alternating 1/0 for clock recovery, zero based   |
| Sync     | `0xa9 0x66 0x69 0x65`  | Frame delimiter                      |
| Payload  | 21 encoded bytes (168 b)  | Base-6 encoded data (see below)      |

## Itron R900 Example
```
Raw bit stream (script output)
 └── Preamble: 010101010101010101010101...   (0x55 0x55 0x55)
 └── Sync:     10101001011001100110100101100101  (0xa9 0x66 0x69 0x65)
 └── Payload:  168 bits of 4b6b-encoded data  ← still needs decoding
                  └── 42 base-6 digits
                         └── 104 bits of actual meter data (ID, consumption, leak, …)
```

### Data Encoding — 4-chip Base-6 (4b6b)

Each raw payload byte contains two 4-bit chips.  Each chip maps to one base-6
digit.  Exactly 2 of the 4 bits are always `1`, making the code DC-balanced.
2 chips per byte, technically 0-35 per byte, however, only 0-31 is used to nicely fit
in 5 bits

| 4-bit chip | Base-6 digit |
|------------|-------------|
| `0011`     | 0           |
| `0101`     | 1           |
| `0110`     | 2           |
| `1100`     | 3           |
| `1010`     | 4           |
| `1001`     | 5           |

21 encoded bytes (168 bits) → 168/4 = 42 base-6 digits → 104 decoded bits of payload.

### Decoded Payload (104 bits)

| Field       | Bits | Description                                       |
|-------------|------|---------------------------------------------------|
| ID          | 32   | Meter serial number (little-endian)               |
| Unkn1       | 8    | Unknown                                           |
| Unkn2       | 3    | Unknown                                           |
| NoUse       | 3    | Days without use (0–6 bins)                       |
| BackFlow    | 2    | Backflow in last 35 days (0=none, 1=low, 2=high)  |
| Consumption | 24   | Cumulative reading, units of 1/10 gallon          |
| Unkn3       | 3    | Unknown                                           |
| Leak        | 3    | Days with leak detected (0–6 bins)                |
| LeakNow     | 2    | Leak in last 24 h (0=none, 1=low, 2=high)         |
| Extra       | 24   | Unknown extra data                                |

---

## Itron CL200 (Electric Meter) — ERT Protocol

The CL200 transmits two interleaved message types on the same frequency using
standard Manchester encoding (hardware-decodable).

---

### SCM — Standard Consumption Message (96 bits / 12 bytes)

Short, frequently transmitted message containing a single cumulative reading.
Also appears at **912.60 MHz**.

**Frame sync (preamble + sync word):** 21 bits = `111110010101001100000` (`0x1F2A60`),
Manchester-encoded on air.

| Field        | Bits | Description                               |
|--------------|------|-------------------------------------------|
| Frame Sync   | 21   | `0x1F2A60` — preamble + sync pattern      |
| ERT ID MSB   | 2    | Two most-significant bits of serial no.   |
| Reserved     | 1    | —                                         |
| Tamper Phy   | 2    | Physical tamper flags                     |
| ERT Type     | 4    | Commodity type (7 = electric, 8 = gas, …) |
| Tamper Enc   | 2    | Encoder tamper flags                      |
| Consumption  | 24   | Cumulative meter reading                  |
| ERT ID LSB   | 24   | 24 least-significant bits of serial no.   |
| Checksum     | 16   | BCH-16, poly `0x6F63`, over bytes 2–11    |

---

# Example Capture

Capture using `rtl_433 -f 912380000 -s 2359999 -vvv -M bits > capture_rtl433.txt`
```
time      : 2026-08-08 18:40:50
model     : Neptune-R900 id        : 1540735174
unkn1     : 163          unkn2     : 4             nouse     : 0             backflow  : 0             consumption: 10207816     unkn3     : 0             leak      : 1             leaknow   : 0             extra     : c25d57
[106;96m[[30mpulse_slicer_pcm[96m][0m Neptune R900 flow meters
codes     : {239}aaaaaaab52ccd2cab2d8b5546ad39878d46ccb547586667546b8cd98d800 bits      : 1010 1010 1010 1010 1010 1010 1010 1011 0101 0010 1100 1100 1101 0010 1100 1010 1011 0010 1101 1000 1011 0101 0101 0100 0110 1010 1101 0011 1001 1000 0111 1000 1101 0100 0110 1100 1100 1011 0101 0100 0111 0101 1000 0110 0110 0110 0111 0101 0100 0110 1011 1000 1100 1101 1001 1000 1101 1000 0000 000
```

Capture using `sdr_meter_capture.py` > capture.txt`
```
0000000000000000000000000000000000010101010101010101010101010101
0110101001011001100110100101100101010110010110110001011010101010
1000110101011010011100110000111100011010100011011001100101101010
10001110100101110001101010001110101
1000111111001111111111111011110111011100000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000
```

## Decoding
Preamble is zero based and starts with 0x555555 followed by 0x55a9666965. Note above 
preamble 0xaaaaaa followed by 0xab52ccd2ca is just shifted by 1 and the last bit of 
0xab52ccd2ca is actually the first bit of the payload.

### Preamble
Nothing: `00000000000000000000000000000000000`

Preamble: 0x55555555 `01010101010101010101010101010101`

Sync word: 0xA9666965 `10101001011001100110100101100101`

Total: 64 bits

### Payload

30 bit: `010110010110110001011010101010`

64 bit: `1000110101011010011100110000111100011010100011011001100101101010`

35 bit: `10001110100101110001101010001110101`

39 bit: `100011111100111111111111101111011111000`

Total 168 bits

```
Bits:   0101 1001 0110 1100 0101 1010 1010 1010 0011 0101 0110 1001 1100 1100 0011 1100 (64 bits)
Base 6: 1    5    2    3    1    4    4    4    0    1    2    5    3    3    0    3
        6         15        10        28        1         17        21        3
5 bits: 00110     01111     01010     11100     00001     10001     10101     00011

Bits:   0110 1010 0011 0110 0110 0101 1010 1010 0011 1010 0101 1100 0110 1010 0011 1010 (64 bits)
Base 6: 2    4    0    2    2    1    4    4    0    4    1    3    2    4    0    4
        16        2         13        28        4         9         16        4
5 bits: 10000     00010     01101     11100     00100     01001     10000     00100
Bits:   1100 0111 1110 0111 1111 1111 1101 1110 1111 1000
Base 6: 3    X
```

```
00110011110101011100000011000110 1010 0011 100000 00 100110111100001000100110 00 0001 00
|                                     |    |         |                           |    └── Leak now
|                                     |    |         |                           └── Leak
|                                     |    |         └── Consumption
|                                     |    └── No use (3 bits only)
|                                     └── Meter Type
└── Meter ID
```
- Meter ID: 0x33D5C0C6 = 869646534
- Meter Type: 3
- No use: 0
- Consumption: 10207782
- Leak: 1
- Leak now: 0

---

## References

- [ERT Wikipedia article](https://en.wikipedia.org/wiki/Encoder_receiver_transmitter)
- [rtlamr protocol wiki](https://github.com/bemasher/rtlamr/wiki/Protocol)
- [rtl_433 Neptune R900 decoder](https://github.com/merbanan/rtl_433/blob/master/src/devices/neptune_r900.c)
