#!/usr/bin/env python

import unittest
from unittest import TestCase

from .teros12 import Teros12

TEROS12_PORT = "/dev/ttyUSB0"

class TestTeros12(TestCase):

	def test_init(self):
		t12 = Teros12(TEROS12_PORT)

		# Check if serial port is open
		self.assertTrue(t12.is_open)

		t12.close()

	def test_measure(self):
		with Teros12(TEROS12_PORT, 9600) as t12:
			data = t12.measure()

			# TODO Put measurement assertions her 


if __name__ == "__main__":
	unittest.main()