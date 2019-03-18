import sys
import json
import time
from collections import namedtuple

import requests

rotation_map = {
    0: 0,
    90: 1,
    180: 2,
    270: 3
}

class TTGOT4(object):
    def __init__(self, host):
        self.host = host
        self.commands = []

    def _callfn(self, id, command_args):
        # print("Function {} called with {}".format(id, command_args))
        self.commands.append({"cmd": id, "args": command_args})

    def send_commands(self):
        data = {"cmds": self.commands}
        raw_json = json.dumps(data)

        req = requests.post(
            "http://{}/command".format(self.host), data={"command_data": raw_json}
        )
        
        self.commands.clear()

    def println(self, text):
        self._callfn(0, {"t": text})

    def set_text_size(self, size):
        self._callfn(1, {"s": size})

    def set_cursor(self, x, y):
        self._callfn(2, {"x": x, "y": y})

    def set_text_wrap(self, wrap):
        self._callfn(3, {"w": wrap})
    
    def set_text_color(self, color, background_color=None):
        args = {"c": color}
        if background_color is not None:
            args['b'] = background_color
        self._callfn(4, args)

    def fill_screen(self, color):
        self._callfn(5, {"c": color})
    
    def set_rotation(self, rotation):
        self._callfn(6, {"o": rotation_map.get(rotation, 0)})

    @staticmethod
    def color(r, g, b):
        return ((r // 8) << 11) | ((g // 4) << 5) | (b // 8)


if __name__ == "__main__":
    green = TTGOT4.color(0, 255, 0)
    red = TTGOT4.color(255, 0, 0)

    ttgo = TTGOT4("10.0.0.135")

    while True:
        for r in [0, 90, 180, 270]:
            ttgo.set_rotation(r)
            ttgo.set_cursor(0, 0)
            ttgo.fill_screen(red)
            for i in range(5):
                ttgo.println("{}".format(i))
            ttgo.send_commands()
            time.sleep(.25)
