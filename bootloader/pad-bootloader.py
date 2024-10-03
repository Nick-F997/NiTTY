import sys

BOOTLOADER_SIZE = 0x8000
BOOTLOADER_FILE = "bootloader.bin"

def main() -> int:
    with open(BOOTLOADER_FILE, "rb") as bootloader_file:
        raw_file = bootloader_file.read()

    bytes_to_pad = BOOTLOADER_SIZE - len(raw_file)
    padding = bytes([0xff for _ in range(bytes_to_pad)])

    with open(BOOTLOADER_FILE, "wb") as bootloader_file:
        bootloader_file.write(raw_file + padding)
    return 0


if __name__ == "__main__":
    sys.exit(main())