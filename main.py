#!/usr/bin/env python

from argparse import ArgumentParser

if __name__ == "__main__":
    parser = ArgumentParser(description="Remotely upload MFC data to Dirtviz")
    parser.add_argument(
        "-c", "--config",
        default="/etc/dirtviz/conifg.yaml",
        help="Path to config file"
    )

    args = parser.parse_args()

    print(args)
