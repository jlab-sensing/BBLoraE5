#!/usr/bin/env python

from argparse import ArgumentParser
from pprint import pprint

from yaml import load, dump
try:
    from yaml import CLoader as Loader, CDumper as Dumper
except ImportError:
    from yaml import Loader, Dumper

if __name__ == "__main__":
    parser = ArgumentParser(description="Remotely upload MFC data to Dirtviz")
    parser.add_argument(
        "-v", "--verbose",
        action="count",
        default=0
    )
    parser.add_argument(
        "-c", "--config",
        default="/etc/dirtviz/config.yaml",
        help="Path to config file"
    )

    args = parser.parse_args()

    # Read config file
    with open(args.config) as s:
        data = load(s, Loader=Loader)

    if (args.verbose > 0):
        pprint(data)