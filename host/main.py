import sys
import json
import time
from collections import namedtuple

import serial
import sliplib
from cobs import cobs


class TTGOT4(object):
    def __init__(self, port):
        self.serial = serial.Serial(port, 115200, bytesize=8, parity='N', stopbits=1, timeout=1)
        self.driver = sliplib.Driver()

    def _callfn(self, id, command_args):
        print("Function {} called with {}".format(id, command_args))
        data = {"commands": [{"command": id, "args": command_args}]}
        raw_json = json.dumps(data).encode("ascii")
        #encoded = self.driver.send(raw_json)
        encoded = cobs.encode(raw_json)
        self.serial.write(encoded)
        self.serial.flushOutput()

    def println(self, text):
        self._callfn(0, {"text": text})

    def fillScreen(self, color):
        self._callfn(5, {"color": color})

    def print_from_serial(self):
        while self.serial.in_waiting > 0:
            decoded = ''.join([chr(b) for b in self.serial.read()])
            sys.stdout.write(decoded)

    def close(self):
        self.serial.close()

    @staticmethod
    def color(r, g, b):
        return ((r // 8) << 11) | ((g // 4) << 5) | (b // 8)


if __name__ == "__main__":
    green = TTGOT4.color(0, 255, 0)
    red = TTGOT4.color(255, 0, 0)

    ttgo = TTGOT4("COM6")
    try:
        while True:
            ttgo.println("frompy")
            for i in range(10):
                ttgo.print_from_serial()
                time.sleep(.1)
            
    finally:
        ttgo.close()
