import datetime
import intelhex
import struct
import os


def calc_crc32table():
    CRCPOLY = 0xEDB88320
    crctable = []

    for i in range(256):
        r = i
        for j in range(8):
            if r & 1:
                r = (r >> 1) ^ CRCPOLY
            else:
                r >>= 1
        crctable.append(r)

    return crctable


CRCTABLE = calc_crc32table()


def get_crc32(data, init_value=0xFFFFFFFF):
    crc = init_value
    for d in data:
        index = (crc ^ d) & 0xFF
        crc = CRCTABLE[index] ^ (crc >> 8)
    return crc


def write_mmpm_from_ihex(info, in_filename, out_filename):
    now = datetime.datetime.now().strftime('%Y%m%d%H%M%S')

    header = memoryview(bytearray(1024))
    header[0x00:0x0F] = '{:<15}'.format(info['module_name']).encode('latin1')
    header[0x10:0x1E] = now.encode('latin1')
    header[0x30:0x3F] = '{:<15}'.format(info['cpu_name']).encode('latin1')
    header[0x40:0x4E] = now.encode('latin1')
    header[0x50:0x54] = struct.pack('<L', info['flash_begin'])
    header[0x54:0x58] = struct.pack('<L', info['flash_end'])
    header[0x58:0x5C] = struct.pack('<L', info['sector_size'])

    ih = intelhex.IntelHex()
    # ih.padding = 0x00 # change padding byte
    ih.fromfile(in_filename, format='hex')
    ih_minaddr = ih.minaddr()
    ih_maxaddr = ih.maxaddr()
    ih_size = ih_maxaddr - ih_minaddr + 1

    if (ih_minaddr != info['flash_begin']):
        print("Error: ihex start addr (0x{:X}),  flash start addr (0x{:X})"
              .format(ih_minaddr, info['flash_begin']))
        exit(1)

    if (ih_maxaddr >= info['flash_end']):
        print("Error: ihex end addr (0x{:X}) > flash end addr (0x{:X})"
              .format(ih_maxaddr, info['flash_end']))
        exit(1)

    firmware_bin = bytearray(ih_size)
    for p in range(ih_minaddr, ih_maxaddr + 1):
        firmware_bin[p - ih_minaddr] = ih[p]

    crc32 = get_crc32(header)
    crc32 = get_crc32(firmware_bin, crc32)

    header[0x20:0x24] = struct.pack('<L', crc32)

    with open(out_filename, 'wb') as f_out:
        f_out.write(header)
        f_out.write(firmware_bin)


MODULES = {
    "PC100": {
            "module_name": "PC100",
            "cpu_name": "AT91SAM3X8C",
            "flash_begin": 0x00084000,
            "flash_end": 0x000A0000,
            "sector_size": 256,
    },
    "MB100": {
            "module_name": "MB100",
            "cpu_name": "LPC2124        ",
            "flash_begin": 0x00002000,
            "flash_end": 0x0003A000,
            "sector_size": 8192,
    },
    "PC100-SamE70N20": {
            "module_name": "PC100SAME70N20",
            "cpu_name": "ATSAME70N20    ",
            "flash_begin": 0x00404000,
            "flash_end": 0x00440000,
            "sector_size": 512,
    }
}

FILES = [
    {
        "in_filename": "Pc100SAM3X.hex",
        "out_filename": "mmpm-Pc100SAM3X-{}.bin",
        "module": "PC100"
    },
    {
        "in_filename": "Pc100SAM3X_30.hex",
        "out_filename": "mmpm-Pc100SAM3X_30-{}.bin",
        "module": "PC100"
    },
    {
        "in_filename": "Pc100SAM3X-40_x32.hex",
        "out_filename": "mmpm-Pc100SAM3X_40-{}.bin",
        "module": "PC100"
    },
    {
        "in_filename": "Du100SAM3X_10_x32.hex",
        "out_filename": "mmpm-Du100SAM3X_10-{}.bin",
        "module": "PC100"
    },
    {
        "in_filename": "Du100SAM3X_20_x32.hex",
        "out_filename": "mmpm-Du100SAM3X_20-{}.bin",
        "module": "PC100"
    },
    {
        "in_filename": "Du102SAM3X_20_x32.hex",
        "out_filename": "mmpm-Du102SAM3X_20-{}.bin",
        "module": "PC100"
    },
    {
        "in_filename": "Pc100SamE70N20_40.hex",
        "out_filename": "mmpm-Pc100SamE70N20_40-{}.bin",
        "module": "PC100-SamE70N20"
    },
]


def main():
    for item in FILES:
        in_filename = item['in_filename']
        if os.path.isfile(in_filename):
            now = datetime.datetime.now().strftime('%Y%m%d')
            out_filename = item['out_filename'].format(now)

            print("ihex found: {}".format(in_filename))
            write_mmpm_from_ihex(MODULES[item['module']],
                                 in_filename, out_filename)
            print("mmpm written: {}".format(out_filename))

if __name__ == '__main__':
    main()
