import time
from datetime import datetime

import cachetools
import requests
from plumbum import cli

from ttgot4 import TTGOT4

condition_to_emoji = {
    "Unknown": u"✨",
    "Cloudy": u"☁️",
    "Fog": u"🌫",
    "Heavy Rain": u"🌧",
    "Heavy Showers": u"🌧",
    "Heavy Snow": u"❄️",
    "Heavy Snow Showers": u"❄️",
    "Light Rain": u"🌦",
    "Light Showers": u"🌦",
    "Light Sleet": u"🌧",
    "Light Sleet Showers": u"🌧",
    "Light Snow": u"🌨",
    "Light Snow Showers": u"🌨",
    "Partly Cloudy": u"⛅️",
    "Clear": u"☀️",
    "Thundery Heavy Rain": u"🌩",
    "Thundery Showers": u"⛈",
    "Thundery Snow Showers": u"⛈",
    "Very Cloudy": u"☁️",
}


class WorkDisplay(cli.Application):
    background_color = TTGOT4.color(0x1A, 0x2A, 0x3A)
    text_light = TTGOT4.color(0xEE, 0xEE, 0xEE)

    weather_emoji_map = {v: k for k, v in condition_to_emoji.items()}

    @cachetools.cached(cachetools.TTLCache(10, 60))
    def get_weather(self):
        print("Fetching weather")
        weather_info = requests.get("http://wttr.in/?T&format=%l|%c|%t")
        text = weather_info.text

        location, condition, temp = text.split("|")

        # Remove degrees character
        temp = temp.replace("\u00b0", "")

        condition = self.weather_emoji_map[condition]

        return location, condition, temp

    def get_current_date_string(self, dt: datetime):
        return dt.strftime("%A, %b %d %Y")

    def get_current_time_string(self, dt: datetime):
        return dt.strftime("%I:%M:%S %p")

    def draw_display(self, d: TTGOT4):
        dt = datetime.now()

        date_display = self.get_current_date_string(dt)
        time_display = self.get_current_time_string(dt)
        location, condition, temp = self.get_weather()

        d.set_text_color(self.text_light, self.background_color)

        # Draw Date
        d.set_cursor(5, 5)
        d.set_text_size(2)
        d.println(date_display)

        # Draw Time
        d.set_cursor(5, 27)
        d.set_text_size(4)
        d.println(time_display)

        d.send_commands()

        # Draw Weather
        d.set_text_size(2)
        d.set_cursor(5, 60)
        d.println(location)
        d.set_cursor(5, 80)
        d.println("Weather: {} {}".format(condition, temp))

        d.send_commands()

    def main(self, hostname: str):
        d = TTGOT4(hostname)
        d.set_rotation(270)
        d.fill_screen(self.background_color)

        while True:
            self.draw_display(d)
            time.sleep(1)


if __name__ == "__main__":
    WorkDisplay.run()
