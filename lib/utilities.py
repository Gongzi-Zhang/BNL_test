'''
useful utilities
'''

import sys
import logging

class customFormatter(logging.Formatter):
    grey = "\x1b[30;20m"
    green = "\x1b[32;20m"
    yellow = "\x1b[33;20m"
    red = "\x1b[31;20m"
    bold_red = "\x1b[31;1m"
    reset = "\x1b[0m"

    color = {
        logging.DEBUG: grey,
        logging.INFO: green,
        logging.WARNING: yellow,
        logging.ERROR: red,
        logging.CRITICAL: bold_red,
    }

    def __init__(self):
        self.FORMATS = {level: f'(%(filename)s:%(lineno)d) {self.color[level]} %(levelname)s{self.reset} - %(message)s' for level in self.color.keys()}

    def format(self, record):
        log_fmt = self.FORMATS.get(record.levelno)
        formatter = logging.Formatter(log_fmt)
        return formatter.format(record)

logger = logging.getLogger()
logger.setLevel(logging.INFO)
# logger.setLevel(logging.DEBUG)

handler = logging.StreamHandler(sys.stdout)
# formatter = logging.Formatter('%(levelname)s - %(message)s')
handler.setFormatter(customFormatter())
logger.addHandler(handler)
