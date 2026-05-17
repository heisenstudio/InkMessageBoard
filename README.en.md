# InkMessageBoard E-Paper Message Board

[中文](README.md) | English

> This is an open source prototype for personal learning and discussion. It is not a polished commercial product, does not guarantee production stability, and should not be copied directly into mass production without validation.

InkMessageBoard is a message board prototype built with **ESP32-S3, an e-paper display, MQTT, and an Android app**. The basic flow is: the Android app publishes a message through MQTT; the ESP32-S3 connects to Wi-Fi and an MQTT broker, receives the message, and refreshes the e-paper display. The device can also report online state, battery level, RSSI, firmware version, and message handling state back to the app.

![Device photo](docs/images/cover.jpg)

## Project Images

| Front View | Side View |
| --- | --- |
| ![Front view](docs/images/device_front.jpg) | ![Side view](docs/images/device_side.jpg) |

| Android App | System Architecture |
| --- | --- |
| ![Android app](docs/images/app_cover.jpg) | ![System architecture](docs/images/system_arch.png) |

![MQTT message and status flow](docs/images/mqtt_flow.png)

## Working Flow

1. Enter a message in the Android app.
2. Publish the message through MQTT.
3. Connect the ESP32-S3 to Wi-Fi and the MQTT broker.
4. Subscribe the ESP32-S3 to message and command topics.
5. Refresh the e-paper display after a message is received.
6. Send simple status replies from the device through touch keys.
7. Show online state, battery level, RSSI, firmware version, and message states in the app.

EMQX Cloud was used during testing. Real broker addresses, usernames, and passwords are not included in this repository. Use your own MQTT test account when reproducing the project.

## Purpose

This repository is mainly a learning record and reference project. It is useful for studying:

- ESP32-S3 + Arduino/PlatformIO project organization
- Wi-Fi, MQTT, TLS connection, and reconnection logic
- E-paper refresh and Chinese bitmap font rendering
- A simple Android Compose interface
- MQTT interaction between an Android app and an embedded device
- Open hardware release materials such as schematics, PCB files, Gerber, BOM, and pick-and-place files

Current limitations:

- It is not a stable commercial product.
- It does not include a complete security design.
- It does not include a full production test flow.
- The Android and firmware code can still be cleaned up.
- Documentation, parameters, debugging notes, and reproduction details can still be improved.

## Repository Layout

```text
InkMessageBoard/
├── android/              # Android app project
├── firmware/esp32/       # ESP32-S3 PlatformIO firmware project
├── hardware/             # Hardware source files, schematics, PCB images, Gerber, BOM
├── docs/                 # Build guides, protocol notes, software and hardware docs
└── media/                # Video scripts and demo assets
```

## Documentation

- [Software overview](docs/software_overview.md)
- [Hardware overview](docs/hardware_overview.md)
- [MQTT protocol](docs/mqtt_protocol.md)
- [Build ESP32 firmware](docs/build_firmware.md)
- [Build Android app](docs/build_android.md)
- [Hardware connection](docs/hardware_connection.md)
- [Known issues](docs/known_issues.md)

## Open Source Materials

This repository includes:

- ESP32-S3 firmware source and PlatformIO configuration
- Android app source and Gradle configuration
- Hardware source projects for three boards
- Schematic PDFs
- PCB front/back images
- Gerber archives
- BOM and pick-and-place files
- Device photos, flow diagrams, and an app preview image

## Privacy And Credentials

Before publishing, the repository was cleaned up to:

- Remove Android `local.properties`
- Remove Android and ESP32 build caches
- Remove nested `.git` data from the firmware project
- Remove real EMQX Cloud host, username, and password values
- Use placeholders in sample configuration

If you fork or republish this project, check Wi-Fi credentials, MQTT credentials, screenshots, serial logs, and photo backgrounds again before making them public.

## License

Software code, documentation, images, and hardware design files in this repository are released under the MIT License. See [LICENSE](LICENSE).

The hardware files are also shared under the MIT License for learning, modification, manufacturing, and further improvement. Before fabricating or using the boards, verify the schematics, PCB files, BOM, Gerber files, and assembly method yourself.
