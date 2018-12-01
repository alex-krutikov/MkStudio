import datetime
import struct


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


def write_mmpm_bin(info, in_filename, out_filename):
    now = datetime.datetime.now().strftime('%Y%m%d%H%M%S')

    header = memoryview(bytearray(1024))
    header[0x00:0x0F] = '{:<15}'.format(info['module_name']).encode('latin1')
    header[0x10:0x1E] = now.encode('latin1')
    header[0x30:0x3F] = '{:<15}'.format(info['cpu_name']).encode('latin1')
    header[0x40:0x4E] = now.encode('latin1')
    header[0x50:0x54] = struct.pack('<L', info['flash_begin'])
    header[0x54:0x58] = struct.pack('<L', info['flash_end'])
    header[0x58:0x5C] = struct.pack('<L', info['sector_size'])

    with open(in_filename, 'rb') as f_bin:
        firmware_bin = f_bin.read()

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
    }
}


def main():
    # write_mmpm_bin(MODULES['PC100'], "Du100SAM3X_20.bin", "fw-mmpm.bin")
    # write_mmpm_bin(MODULES['MB100'], "Mb100_05_01_499K_100K.bin", "mmpm-MB100_05_01_499K_100K.bin")


if __name__ == '__main__':
    main()
