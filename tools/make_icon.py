#!/usr/bin/env python3
"""Создаёт pkg/sce_sys/icon0.png (512x512) для PKG. Без внешних библиотек."""
import struct
import zlib
from pathlib import Path

W = H = 512
OUT = Path(__file__).resolve().parent.parent / "pkg" / "sce_sys" / "icon0.png"


def chunk(tag, data):
    return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)


def main():
    raw = bytearray()
    for y in range(H):
        raw.append(0)  # фильтр: none
        for x in range(W):
            # тёмный фон, жёлтый круг в центре (как намёк на Тито и кость)
            dx, dy = x - W // 2, y - H // 2
            if dx * dx + dy * dy < 150 * 150:
                raw += bytes((0x11, 0x11, 0x11, 0xFF))
            else:
                raw += bytes((0xFF, 0xD1, 0x66, 0xFF))
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", W, H, 8, 6, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    png += chunk(b"IEND", b"")
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_bytes(png)
    print(f"Создан {OUT} ({len(png)} байт)")


if __name__ == "__main__":
    main()
