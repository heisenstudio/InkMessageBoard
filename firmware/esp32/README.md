# ESP32 Firmware

This is the ESP32-S3 firmware for the InkMessageBoard prototype.

The firmware connects to Wi-Fi, connects to an MQTT broker, receives messages from the Android app, and refreshes the e-paper display. It is shared as a learning/reference project, not as a polished production firmware.

## Main Features

- Wi-Fi connection and retry
- MQTT over TLS, tested with free EMQX Cloud
- Message, command, and status topics
- E-paper UI pages for boot, offline, idle, message, and reply state
- Basic battery/status reporting
- GB2312 font data for Chinese text rendering

## Build

Install PlatformIO, then build from this directory:

```bash
pio run
```

Flash:

```bash
pio run --target upload
```

Upload filesystem data if needed:

```bash
pio run --target uploadfs
```

## Configuration

Edit `src/config/AppConfig.h` before building.

The values in that file are placeholders. Replace them with your own Wi-Fi and MQTT information. Do not publish real Wi-Fi passwords, MQTT passwords, or private broker addresses.

## Dependencies

Third-party Arduino libraries are declared in `platformio.ini` through `lib_deps`. Build caches and downloaded libraries are intentionally not committed.

