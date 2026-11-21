#!/usr/bin/env python3
"""Создаёт pkg/sce_sys/param.sfo (бинарный формат PARAM.SFO) для игры PS4.

Запуск: python3 tools/make_sfo.py
Параметры меняются ниже в словаре PARAMS.
"""
import struct
import sys
from pathlib import Path

PARAMS = {
    "APP_TYPE": ("int", 1),                 # 1 = полноценное приложение
    "APP_VER": ("str", "01.00"),
    "ATTRIBUTE": ("int", 0),
    "CATEGORY": ("str", "gd"),              # gd = игра
    "CONTENT_ID": ("str", "UP0001-TITO00001_00-BONERUN000000001"),
    "FORMAT": ("str", "obs"),
    "TITLE": ("str", "Bone Run"),
    "TITLE_ID": ("str", "TITO00001"),
    "VERSION": ("str", "01.00"),
}

OUT = Path(__file__).resolve().parent.parent / "pkg" / "sce_sys" / "param.sfo"

T_STR = 0x0204   # utf-8, с нулевым байтом в конце
T_INT = 0x0404   # 32-бит integer


def align4(n):
    return (n + 3) & ~3


def build(params):
    keys = sorted(params)
    n = len(keys)
    key_table_off = 20 + 16 * n

    key_blob = b""
    key_offsets = {}
    for k in keys:
        key_offsets[k] = len(key_blob)
        key_blob += k.encode("ascii") + b"\x00"
    key_blob += b"\x00" * (align4(len(key_blob)) - len(key_blob))

    data_table_off = key_table_off + len(key_blob)

    index = b""
    data = b""
    for k in keys:
        kind, value = params[k]
        if kind == "int":
            dtype, raw, max_len = T_INT, struct.pack("<I", value), 4
        else:
            raw = value.encode("utf-8") + b"\x00"
            dtype, max_len = T_STR, align4(len(raw))
            raw = raw + b"\x00" * (max_len - len(raw))
        off = len(data)
        data += raw
        index += struct.pack("<HHIII", key_offsets[k], dtype, len(raw) if kind != "int" else 4,
                             max_len, off)

    header = struct.pack("<4sIIII", b"\x00PSF", 0x0101, key_table_off, data_table_off, n)
    return header + index + key_blob + data


def parse(blob):
    magic, ver, kto, dto, n = struct.unpack_from("<4sIIII", blob, 0)
    assert magic == b"\x00PSF", "bad magic"
    out = {}
    for i in range(n):
        koff, dtype, dlen, _maxlen, doff = struct.unpack_from("<HHIII", blob, 20 + 16 * i)
        key_end = blob.index(b"\x00", kto + koff)
        key = blob[kto + koff:key_end].decode("ascii")
        raw = blob[dto + doff:dto + doff + dlen]
        if dtype == T_INT:
            out[key] = struct.unpack("<I", raw)[0]
        else:
            out[key] = raw.split(b"\x00", 1)[0].decode("utf-8")
    return out


def main():
    blob = build(PARAMS)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_bytes(blob)
    check = parse(blob)
    expected = {k: v for k, (_, v) in PARAMS.items()}
    if check != expected:
        print("Проверка не прошла:", check, file=sys.stderr)
        return 1
    print(f"Создан {OUT} ({len(blob)} байт), проверка пройдена")
    return 0


if __name__ == "__main__":
    sys.exit(main())
