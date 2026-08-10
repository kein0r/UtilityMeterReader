# Setup

- `sudo apt install rtl-sdr`
- `python -m venv SmartMeterVEnv`
- `source SmartMeterVEnv/bin/activate`
- `pip install numpy`
- `pip install pyrtlsdr`
- `pip install pyrtlsdrlib`

# Run

- `source SmartMeterVEnv/bin/activate`
- `python sdr_meter_capture.py > capture.txt`

## In case of errors

- `sudo rmmod dvb_usb_rtl28xxu`
