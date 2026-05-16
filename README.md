# InkMessageBoard

> Note: this is a personal learning prototype shared for discussion and improvement. It is not a mature commercial product and does not guarantee production-level stability.

An ESP32-S3 + e-paper + MQTT + Android App message board prototype.

This is a learning/reference project, not a polished product. The main message flow works: the Android app publishes a message through MQTT, the ESP32-S3 receives it, and the e-paper display refreshes with the new content.

The repository is shared for study, discussion, and further improvement. Chinese documentation is the primary documentation: [README.zh-CN.md](README.zh-CN.md).

![Device](docs/images/cover.jpg)

## Flow Diagrams

![Topic data flow](docs/images/system_arch.png)

![Message and status flow](docs/images/mqtt_flow.png)

## Contents

- `firmware/esp32/`: ESP32-S3 PlatformIO firmware
- `android/`: Android Compose app
- `hardware/`: hardware source files, schematic PDFs, PCB images, Gerber, BOM, and pick-and-place files
- `docs/`: software/hardware notes, build docs, MQTT protocol, and known issues
- `media/`: demo/video publishing notes

## Docs

- [Software overview](docs/software_overview.md)
- [Hardware overview](docs/hardware_overview.md)
- [MQTT protocol](docs/mqtt_protocol.md)
- [Build firmware](docs/build_firmware.md)
- [Build Android app](docs/build_android.md)
- [Known issues](docs/known_issues.md)

## Privacy

Real Wi-Fi and MQTT credentials are not included. The example configuration uses placeholders. The project was tested with free EMQX Cloud, but you should use your own broker and credentials.

## License

Software, documentation, media assets, and hardware design files are released under the MIT License unless otherwise noted. See [LICENSE](LICENSE).
