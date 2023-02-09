# Copyright (C) 2013 Riverbank Computing Limited.
# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
from __future__ import annotations

"""Simple port of the corelib/threads/mandelbrot example from Qt v5.x, originating from PyQt"""
from argparse import ArgumentParser, RawTextHelpFormatter
import sys
import time
import concurrent.futures

DEFAULT_CENTER_X = -0.647011
DEFAULT_CENTER_Y = -0.0395159
DEFAULT_SCALE = 0.00403897

ZOOM_IN_FACTOR = 0.8
ZOOM_OUT_FACTOR = 1 / ZOOM_IN_FACTOR
SCROLL_STEP = 20

NUM_PASSES = 8
QUIT = False
POOL_SIZE = 4

INFO_KEY = 'info'


class Mandelbrot:
    def __init__(self):
        self._scale_factor = DEFAULT_SCALE
        self._center_x = DEFAULT_CENTER_X
        self._center_y = DEFAULT_CENTER_Y
        self.restart = False

    def calculate_stripe(self, image, y, max_iterations, LIMIT):
        scale_factor = self._scale_factor
        # half_height = self._half_height
        half_width = self._half_width
        centerX = self._center_x
        centerY = self._center_y
        LIMIT = LIMIT + 0
        max_iterations = max_iterations + 0

        ay = 1j * (centerY + (y * scale_factor))

        all_black = True
        for x in range(-half_width, half_width):
            c0 = centerX + (x * scale_factor) + ay
            c = c0
            num_iterations = 0

            while num_iterations < max_iterations:
                num_iterations += 1
                c = c * c + c0
                if abs(c) >= LIMIT:
                    break
                num_iterations += 1
                c = c * c + c0
                if abs(c) >= LIMIT:
                    break
                num_iterations += 1
                c = c * c + c0
                if abs(c) >= LIMIT:
                    break
                num_iterations += 1
                c = c * c + c0
                if abs(c) >= LIMIT:
                    break

            if num_iterations < max_iterations:
                # image.setPixel(x + half_width, 0,
                #                self.colormap[num_iterations % RenderThread.colormap_size])
                all_black = False
            else:
                pass
                # image.setPixel(x + half_width, 0, qRgb(0, 0, 0))
        return all_black, image

    def run(self):
        while True:
            half_width = 576
            half_height = 359

            curpass = 0

            self._half_width = half_width
            self._half_height = half_height

            while curpass < NUM_PASSES:
                start = time.perf_counter()
                max_iterations = (1 << (2 * curpass + 6)) + 32
                LIMIT = 4
                all_black = True

                with concurrent.futures.ThreadPoolExecutor(max_workers=POOL_SIZE) as executor:

                    future_to_y = {}
                    num_futures = 0

                    for y in range(-half_height, half_height):
                        # The idea to decouple multiple calculations:
                        # * Build individual 1-bit stripes.
                        # * Have a single collector that runs when all stripes are ready.
                        stripe = "stripe"
                        # stripe = QImage(QSize(resultSize.width(), 1), QImage.Format_RGB32)
                        future_to_y[executor.submit(self.calculate_stripe, stripe, y,
                                                    max_iterations, LIMIT)] = y
                        num_futures += 1

                    print(f"{num_futures=}")
                    for future in concurrent.futures.as_completed(future_to_y):
                        y = future_to_y[future]
                        try:
                            data = future.result()
                        except Exception as exc:
                            print('%r generated an exception: %s' % (y, exc))
                        black, stripe = data
                        all_black &= black
                        # for x in range(resultSize.width()):
                        #     image.setPixel(x, y + half_height, stripe.pixel(x, 0))

                if all_black and curpass == 0:
                    curpass = 4
                else:
                    if not self.restart:
                        end = time.perf_counter()
                        elapsed = round((end - start) * 1000)
                        unit = 'ms'
                        if elapsed > 2000:
                            elapsed /= 1000
                            unit = 's'
                        text = (f"Pass {curpass + 1}/{NUM_PASSES}, "
                                f"max iterations: {max_iterations}, time: {elapsed}{unit}")
                        # image.setText(INFO_KEY, text)
                        # self.rendered_image.emit(image, scale_factor)
                        print(text)
                    curpass += 1
            if QUIT:
                return


if __name__ == "__main__":
    parser = ArgumentParser(description='Mandelbrot Example',
                            formatter_class=RawTextHelpFormatter)
    parser.add_argument('--passes', '-p', type=int, help='Number of passes (1-8)')
    parser.add_argument('--poolsize', '-s', type=int, help='Size of thread pool')
    parser.add_argument('--quit', '-q', action="store_true", help='Quit after passes')
    options = parser.parse_args()
    if options.passes:
        NUM_PASSES = int(options.passes)
        if NUM_PASSES < 1 or NUM_PASSES > 8:
            print(f'Invalid value: {options.passes}')
            sys.exit(-1)
    if options.poolsize:
        POOL_SIZE = int(options.poolsize)
    if options.quit:
        QUIT = True
    m = Mandelbrot()
    m.run()
