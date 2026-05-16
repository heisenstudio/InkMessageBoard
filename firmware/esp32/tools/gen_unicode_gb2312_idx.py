from pathlib import Path
import struct

OUT = Path("data/fonts/unicode_gb2312.idx")

def main():
    table = bytearray(65536 * 2)

    count = 0

    for cp in range(0x0000, 0x10000):
        ch = chr(cp)

        try:
            gb = ch.encode("gb2312")
        except UnicodeEncodeError:
            continue

        # ASCII 单字节不需要写入表，ESP32 端直接处理
        if len(gb) != 2:
            continue

        gb_value = (gb[0] << 8) | gb[1]

        # 只保留标准 GB2312 双字节范围
        if not (0xA1 <= gb[0] <= 0xF7 and 0xA1 <= gb[1] <= 0xFE):
            continue

        struct.pack_into("<H", table, cp * 2, gb_value)
        count += 1

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_bytes(table)

    print(f"generated: {OUT}")
    print(f"size: {len(table)} bytes")
    print(f"mapped chars: {count}")

if __name__ == "__main__":
    main()