#!/usr/bin/env python3
"""Package only the verified Android ARM64 library and required metadata."""
import argparse
import json
from pathlib import Path
import struct
import zipfile

ROOT = Path(__file__).resolve().parents[1]

def package(build_dir, output):
    library = Path(build_dir) / "libMeleeReforged.android.arm64.so"
    data = library.read_bytes()
    if data[:6] != b"\x7fELF\x02\x01" or struct.unpack_from("<H", data, 18)[0] != 183:
        raise ValueError("Expected ELF64 little-endian AArch64 library")
    for name in ("Manifest.json", "Info.json", "mod.json"):
        json.loads((ROOT / name).read_text())
    path = Path(output)
    path.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as archive:
        for name in ("Manifest.json", "Info.json", "mod.json", "LICENSE"):
            archive.write(ROOT / name, name)
        archive.writestr("Resources/", b"")
        archive.writestr("Resources/lib/" + library.name, data)
        archive.write(ROOT / "assets/slash-study/sources.json", "LICENSES/slash-sources.json")
        archive.write(ROOT / "vendor/dobby/LICENSE", "LICENSES/Dobby-Apache-2.0.txt")
        for name in ("tefkernel/tef_api.h", "kernelloader/mod_core.h"):
            archive.write(ROOT / "vendor" / name, "LICENSES/" + Path(name).name + ".txt")
    print(path.resolve())

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()
    package(args.build_dir, args.output)
