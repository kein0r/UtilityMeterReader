#!/usr/bin/env python3
"""
sdr_meter_capture.py – Raw OOK bit-stream capture for 900 MHz utility meters.

Receives I/Q samples from an RTL-SDR dongle, demodulates the OOK signal, and
prints a raw binary bit-stream to stdout.  No protocol decoding is performed;
the output is intended for inspection or piping into a separate decoder.

Supported meters (same RF parameters):
  • Itron CL200 electric meter – ERT SCM / IDM messages at 912.38 MHz
  • Neptune R900 water meter   – R900 messages at 912.38 MHz

Protocol background:
  Both meters use OOK (On-Off Keying) at 32,768 bps in the unlicensed 900–920 MHz
  band.  An OOK signal is simply carrier ON (bit=1) or carrier OFF (bit=0), so
  demodulation reduces to: compute signal magnitude → apply threshold → resample.

Requirements:
  pip install pyrtlsdr numpy

Usage:
  python3 sdr_meter_capture.py                 # defaults: 912.38 MHz, auto gain
  python3 sdr_meter_capture.py --gain 30       # fixed gain in dB
  python3 sdr_meter_capture.py --freq 912600000  # Itron SCM alternate frequency
  python3 sdr_meter_capture.py --hex           # print bytes as hex instead of bits

Output format (default):
  One line of '0'/'1' characters per received block (64 bits per line).
  Status/info messages are written to stderr so stdout can be piped cleanly.
"""

import argparse
import signal
import sys

import numpy as np

# ── Try importing pyrtlsdr and give a friendly error if missing ───────────────
try:
    from rtlsdr import RtlSdr
except ImportError:
    sys.exit(
        "[ERROR] pyrtlsdr not found.\n"
        "Install it with:  pip install pyrtlsdr\n"
        "You also need librtlsdr installed on your system:\n"
        "  sudo apt install librtlsdr-dev     (Debian/Ubuntu)\n"
        "  sudo dnf install rtl-sdr-devel     (Fedora)"
    )

# ─────────────────────────────────────────────────────────────────────────────
# Radio parameters
# ─────────────────────────────────────────────────────────────────────────────

# Both Itron CL200 and Neptune R900 use 912.38 MHz (the R900 and IDM messages).
# The Itron CL200 SCM message also appears at 912.60 MHz.
DEFAULT_FREQ_HZ = 912_380_000   # 912.38 MHz

# Sample rate chosen to match rtlamr (https://github.com/bemasher/rtlamr).
# At this rate we get exactly 72 samples per bit (2 359 296 / 32 768 = 72),
# which makes decimation trivial and avoids fractional resampling.
DEFAULT_RATE_HZ = 2_359_296     # ~2.36 Msps

# Meter bit rate: all ERT-family meters broadcast at 32 768 bps.
BIT_RATE_BPS = 32_768

# Number of I/Q samples per bit symbol at the default sample rate.
SAMPLES_PER_BIT = DEFAULT_RATE_HZ // BIT_RATE_BPS  # = 72

# Block size fed to the callback; 16 384 I/Q pairs ≈ 5 ms of audio @ 2.36 Msps.
# Larger blocks reduce callback overhead; smaller blocks reduce latency.
BLOCK_SIZE = 16_384

# OOK threshold as a fraction of the peak magnitude within each block.
# A value of 0.5 means: anything above half the block's peak is a '1'.
# Increase toward 0.7 if silence looks like noise; decrease toward 0.3
# if weak signals are clipped to all-zeros.
DEFAULT_THRESHOLD_FRACTION = 0.5


# ─────────────────────────────────────────────────────────────────────────────
# DSP helpers
# ─────────────────────────────────────────────────────────────────────────────

def compute_magnitude(samples: np.ndarray) -> np.ndarray:
    """
    Compute the instantaneous signal envelope from complex I/Q samples.

    RTL-SDR delivers 8-bit I and Q interleaved; pyrtlsdr converts them to
    complex64 floats automatically.  The magnitude |I + jQ| gives the OOK
    amplitude: near-zero when the carrier is off, non-zero when it is on.
    """
    return np.abs(samples)


def ook_threshold(mag: np.ndarray, threshold: float) -> np.ndarray:
    """
    Convert a magnitude trace to a raw binary sequence (one value per sample).

    Parameters
    ----------
    mag       : Signal magnitude array (float).
    threshold : Amplitude value above which a sample is considered a '1'.

    Returns
    -------
    np.ndarray of dtype uint8 with values in {0, 1}.
    """
    return (mag >= threshold).astype(np.uint8)


def decimate_majority_vote(bits_per_sample: np.ndarray, spb: int) -> np.ndarray:
    """
    Down-sample from one-value-per-sample to one-value-per-bit.

    Groups `spb` consecutive samples into a single bit by majority vote:
    if more than half of the samples in a window are '1', the bit is '1'.
    Trailing samples that don't fill a complete window are discarded.

    Parameters
    ----------
    bits_per_sample : Binary array at sample rate (len = N * spb + remainder).
    spb             : Samples per bit (SAMPLES_PER_BIT).

    Returns
    -------
    np.ndarray of dtype uint8, length = len(bits_per_sample) // spb.
    """
    n_bits = len(bits_per_sample) // spb
    # Reshape so each row is one bit-period, then vote across columns
    windows = bits_per_sample[:n_bits * spb].reshape(n_bits, spb)
    return (windows.mean(axis=1) >= 0.5).astype(np.uint8)


# ─────────────────────────────────────────────────────────────────────────────
# RTL-SDR callback
# ─────────────────────────────────────────────────────────────────────────────

def on_samples(samples: np.ndarray, context: dict) -> None:
    """
    Callback invoked by pyrtlsdr for every received block of I/Q samples.

    pyrtlsdr calls this as callback(samples, context) – two arguments only.

    Steps performed on each block:
      1. Compute signal magnitude.
      2. Derive an adaptive threshold (fraction of this block's peak).
      3. Threshold the magnitude trace to get a per-sample binary sequence.
      4. Decimate by majority vote to get one bit per symbol period.
      5. Print the resulting bits to stdout.

    The adaptive threshold recalculates each block, which helps it follow slow
    gain or signal-strength changes without manual tuning.
    """
    mag = compute_magnitude(samples)

    # Adaptive threshold: half of this block's peak magnitude.
    # Falls back to a very small value if the block is all noise (peak ≈ 0).
    peak = mag.max()
    if peak < 1e-6:
        return  # Silent block – nothing useful to print
    threshold = peak * context["threshold_fraction"]

    # Convert to per-sample bits, then decimate to per-bit stream
    raw_bits = ook_threshold(mag, threshold)
    bits = decimate_majority_vote(raw_bits, context["samples_per_bit"])

    # Skip blocks that carry no signal (all zeros = carrier off the whole block)
    if not bits.any():
        return

    if context["hex_output"]:
        # Pack 8 bits into bytes and print as hex (ignores trailing < 8 bits)
        n_bytes = len(bits) // 8
        if n_bytes:
            byte_vals = np.packbits(bits[:n_bytes * 8])
            print(byte_vals.tobytes().hex())
    else:
        # Print as a string of '0'/'1' characters, 64 bits per line
        bit_str = "".join(str(b) for b in bits)
        for offset in range(0, len(bit_str), 64):
            print(bit_str[offset:offset + 64])


# ─────────────────────────────────────────────────────────────────────────────
# Entry point
# ─────────────────────────────────────────────────────────────────────────────

def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Capture raw OOK bit-stream from 900 MHz utility meters "
            "(Itron ERT / Neptune R900) via an RTL-SDR dongle."
        ),
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "--freq", type=int, default=DEFAULT_FREQ_HZ,
        help="Center frequency in Hz. Use 912380000 for R900+IDM, 912600000 for SCM.",
    )
    parser.add_argument(
        "--rate", type=int, default=DEFAULT_RATE_HZ,
        help="RTL-SDR sample rate in Hz. Keep at default unless you know what you are doing.",
    )
    parser.add_argument(
        "--gain", default="auto",
        help="Tuner gain: 'auto' for AGC, or a numeric dB value (e.g. 30).",
    )
    parser.add_argument(
        "--device", type=int, default=0,
        help="RTL-SDR USB device index (use 0 if you have only one dongle).",
    )
    parser.add_argument(
        "--threshold", type=float, default=DEFAULT_THRESHOLD_FRACTION,
        metavar="FRAC",
        help=(
            "OOK decision threshold as a fraction (0.0–1.0) of each block's peak "
            "magnitude. Raise if noise looks like signal; lower for weak signals."
        ),
    )
    parser.add_argument(
        "--hex", dest="hex_output", action="store_true",
        help="Output bytes as hex instead of individual bits.",
    )
    args = parser.parse_args()

    # Recompute samples-per-bit for whatever sample rate the user chose
    samples_per_bit = args.rate // BIT_RATE_BPS

    # Bundle all per-callback state into a single dict (avoids globals)
    context = {
        "threshold_fraction": args.threshold,
        "samples_per_bit":    samples_per_bit,
        "hex_output":         args.hex_output,
    }

    sdr = RtlSdr(args.device)
    try:
        sdr.sample_rate = args.rate
        sdr.center_freq = args.freq
        # pyrtlsdr accepts 'auto' or a numeric dB string/int for gain
        sdr.gain = args.gain

        print(
            f"[INFO] SDR open – freq={args.freq/1e6:.3f} MHz  "
            f"rate={args.rate/1e6:.3f} Msps  gain={args.gain}  "
            f"spb={samples_per_bit}",
            file=sys.stderr,
        )
        print(
            "[INFO] Output: raw OOK bits (0/1).  "
            "Press Ctrl+C to stop.",
            file=sys.stderr,
        )

        # Install a SIGINT handler that calls cancel_read_async().
        # This tells the C library to stop its internal loop cleanly *before*
        # Python raises KeyboardInterrupt, avoiding the ctypes callback error.
        def _sigint_handler(sig, frame):
            sdr.cancel_read_async()

        signal.signal(signal.SIGINT, _sigint_handler)

        # read_samples_async blocks until cancel_read_async() is called or an
        # error occurs.  The callback is invoked with BLOCK_SIZE samples each time.
        sdr.read_samples_async(on_samples, num_samples=BLOCK_SIZE, context=context)

        print("\n[INFO] Capture stopped.", file=sys.stderr)
    finally:
        sdr.close()


if __name__ == "__main__":
    main()

