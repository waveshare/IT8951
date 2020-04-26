from device import Device

import numpy as np
from PIL import Image
from io import BytesIO

import time
import logging
import threading
import argparse

from selenium import webdriver
from selenium.webdriver.firefox.options import Options
from selenium.webdriver.common.keys import Keys

class WebViewer(object):

    def __init__(self, url, refresh_interval_seconds):
        self.url = url
        self.refresh_interval = refresh_interval_seconds

        self.device = Device()
        self.width, self.height = self.device.get_dimensions()
        print("Device Screen is %d W x %d H", self.width, self.height)
        self.total_pixels = self.width * self.height

        print("Setting up driver")
        self.driver = self._setup_driver()

        self.last_screen_bytes = None
        self.running = True
        print("Creating worker & starting")
        self.thread = self._create_and_start_worker()

    def _setup_driver(self):
        options = Options()
        options.headless = True
        driver = webdriver.Firefox(options=options)
        driver.set_window_size(self.height, self.width)
        driver.get(self.url)
        return driver

    def _get_grayscale_screenshot(self):
        screenbytes = self.driver.get_screenshot_as_png()
        image = (Image.open(BytesIO(screenbytes))
                 .transpose(Image.ROTATE_90)
                 .resize((self.width, self.height)))

        grayvals = np.asarray(image.convert('L'))
        reducedvals = np.clip(grayvals / 16, 0, 15).flatten().astype(int)
        data = np.left_shift(reducedvals, 4)
        return data

    def _create_and_start_worker(self):
        thread = threading.Thread(target=self.run_loop())
        thread.setDaemon(True)
        thread.start()
        return thread

    def refresh_view(self):
        image_data = self._get_grayscale_screenshot()
        if np.any(self.last_screen_bytes != image_data):
            self.last_screen_bytes = image_data
            self.device.display_image(image_data)

    def run_loop(self):
        while self.running:
            start_time = time.time()
            self.refresh_view()
            elapsed = time.time() - start_time
            print(f"Refreshed in {elapsed:.3f} sec")
            if elapsed < self.refresh_interval:
                time.sleep(self.refresh_interval - elapsed)

    def get_worker_thread(self):
        return self.thread

    def stop(self):
        self.running = False
        self.driver.quit()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--url")
    parser.add_argument("--refresh", type=float)
    args = parser.parse_args()
    print("creating web viewer")
    viewer = WebViewer(args.url, args.refresh)
    print("created, joining to thread")
    viewer.get_worker_thread().join()
    print("done")