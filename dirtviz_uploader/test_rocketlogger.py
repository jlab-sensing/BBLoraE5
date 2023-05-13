#!/usr/bin/env python

import unittest
from unittest import TestCase

from .rocketlogger import RocketLogger


class TestRocketLogger(unittest.TestCase):

	def test_init(self):
		rl = RocketLogger()

	def test_measure(self):
		rl = RocketLogger()

		data = rl.measure()

		# TODO Put measurement assertions here


if __name__ == "__main__":
	unittest.main()