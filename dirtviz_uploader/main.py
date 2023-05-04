#!/usr/bin/env python

import pdb

from argparse import ArgumentParser
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
    """Processes CLI interface"""

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
        lora = Lora()
    else:
        error = f"{config['method']} upload method not supported"
        raise NotImplementedError(error)

if __name__ == "__main__":
    cli()