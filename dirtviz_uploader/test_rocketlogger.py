#!/usr/bin/env python

import unittest
from unittest import TestCase

from .rocketlogger import RocketLogger


class TestRocketLogger(unittest.TestCase):

	def test_creation(self):
		rl = RocketLogger()

	def test_measure(self):
		rl = RocketLogger()

		data = rl.measure()


if __name__ == "__main__":
	unittest.main()