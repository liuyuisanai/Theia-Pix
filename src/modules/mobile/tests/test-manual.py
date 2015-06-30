from base64 import b64decode
import sys
import time

import serial

def one_run(d):
    # # garbage
    # d.write(time.ctime())
    # d.write("\n")

    # HandShake with read_command() test
    d.write("\x05S")
    time.sleep(0.2)
    d.write("H")
    print(repr(d.read(3)))
    print(repr(d.read(4)))

    # Invalid command
    d.write("\x05xx")
    print(repr(d.read(3)))

    # Reveice Presets
    d.write("\x05PR")
    print(repr(d.read(3)))
    d.write("\x1eRR0123456789ab\x1Err0123456789ab\x04")
    print(repr(d.read(3)))

    d.write("\x05PR\x18")
    print(repr(d.read(4)))

    d.write("\x05PR")
    print(repr(d.read(3)))
    d.write("\x1eRR0123456789ab\xFF");
    print(repr(d.read(2)))

    # File Info
    d.write("\x05IF\x00")
    print(repr(d.read(3)))
    print(repr(d.read(6)))

    d.write("\x05IF\x01")
    print(repr(d.read(3)))
    print(repr(d.read(6)))

    # File Block
    d.write("\x05BF\x01\x00\x00\x00\x00")
    print(repr(d.read(3)))

    x = d.read(1024)
    b64 = x
    while x:
        print(repr(x))
        x = d.read(1024)
        b64 += x
    print("base64 frame ok: %s %s" %
            (b64.startswith('\x06BF\x02'), b64.endswith('\x03')))
    b64 = b64[4:-1]
    print(repr(b64))
    print("base64 length %d" % len(b64))
    print(repr(b64decode(b64)))

    d.write("\x05BF\x01\x00\x00\x10\x00")
    print(repr(d.read(3)))
    x = d.read(1024)
    while x:
        print(repr(x))
        x = d.read(1024)

    # BYE with read_command() test
    d.write("\x05E")
    time.sleep(0.2)
    d.write("Y")
    print(repr(d.read(3)))

    print(repr(d.read()))

if __name__ == '__main__':
    d = serial.Serial(sys.argv[1], 57600)
    d.timeout = 0.25

    forever = sys.argv[2:3] == ['--forever']

    one_run(d)
    while forever:
        time.sleep(1)
        one_run(d)
