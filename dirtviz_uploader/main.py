#!/usr/bin/env python

import pdb

from argparse import ArgumentParser
from time import sleep
from pprint import pprint

from yaml import load, dump
try:
    from yaml import CLoader as Loader, CDumper as Dumper
except ImportError:
    from yaml import Loader, Dumper

from .rocketlogger import Rocketlogger
from .teros12 import Teros12
from .lora import Lora

def cli():
    """CLI Interface"""

    parser = ArgumentParser(description="Remotely upload MFC data to Dirtviz")
    parser.add_argument(
        "-v", "--verbose",
        action="count",
        default=0
    )
    parser.add_argument(
        "-c", "--config",
        required=True,
        help="Path to config file"
    )

    args = parser.parse_args()

    # Read config file
    with open(args.config) as s:
        config = load(s, Loader=Loader)

    if (args.verbose > 0):
        pprint(config)

    # Create Rocketlogger
    rl = Rocketlogger()

    # Create TEROS-12    
    if ("teros" in config): 
        t12 = Teros12()

    # Create upload method
    if (config["method"] == "lora"):
        uploader = Lora()
    else:
        error = f"{config['method']} upload method not supported"
        raise NotImplementedError(error)

    # Initialize transmit buffer
    buf = []

    # Loop forever
    while True:
        # Add Rocketlogger data to buffer
        for d in rl.measure():
            buf.append(d)

        # Add TEROS-12 data to buffer
        if ("teros" in config):
            for d in t12.measure():
                buf.append()

        # Send everything in buffer
        for d in buf:
            uploader.send(d)

        # Clear buffer after transmit
        buf.clear()

        # Sleep for a given number of seconds    
        sleep(config["interval"])


if __name__ == "__main__":
    cli()