#!/usr/bin/env python

from argparse import ArgumentParser
from time import sleep

from yaml import load, dump
try:
    from yaml import CLoader as Loader, CDumper as Dumper
except ImportError:
    from yaml import Loader, Dumper

from .rocketlogger import RocketLogger

from .teros12 import Teros12


if __name__ == "__main__":
    #
    # Argument parser
    # 

    parser = ArgumentParser(description="Test utility to read RocketLogger measurements and print to terminal.")
    parser.add_argument(
        "-c", "--config",
        required=True,
        help="Path to config file"
    )

    args = parser.parse_args()
    
    # Read config file
    with open(args.config) as s:
        config = load(s, Loader=Loader)


    #
    # Read measurements
    #

    t12 = Teros12(config["teros"]["port"], config["teros"]["baud"])
    
    while True:
        data = t12.measure()
        
        print(data)
        
        sleep(5)