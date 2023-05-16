#!/usr/bin/env python

import pdb

from argparse import ArgumentParser
import json
from time import sleep
from pprint import pprint

from yaml import load, dump
try:
    from yaml import CLoader as Loader, CDumper as Dumper
except ImportError:
    from yaml import Loader, Dumper

from .rocketlogger import RocketLogger
from .teros12 import Teros12
from .lora import Lora

def cli():
    """CLI Interface"""

    #
    # Argument Parser
    #
    
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


    #
    # Initializers
    #

    # Create RocketLogger
    rl = RocketLogger()

    # Create TEROS-12    
    if ("teros" in config): 
        t12 = Teros12()

        # Mapping of Sensor ID to name
        t12_map = {
            config["cell1"]["teros"]: config["cell1"]["name"],
            config["cell2"]["teros"]: config["cell2"]["name"],
        }

    # Create upload method
    if (config["method"] == "lora"):
        uploader = Lora()
    else:
        error = f"{config['method']} upload method not supported"
        raise NotImplementedError(error)

    # Initialize transmit buffer
    buf = []


    #
    # Main Loop
    #

    # Loop forever
    while True:
        # Add RocketLogger data to buffer
        for d in rl.measure():
            # Channel 1
            if config["name1"]:
                meas = {
                    "type": "rocketlogger",
                    "cell": config["cell1"]["name"],
                    "ts": d["ts"],
                    "v": d["V1"],
                    "i": d["I1"],
                }

                meas_json = json.dumps(meas)

                buf.append(meas_json)

            # Channel 2
            if config["name2"]:
                meas = {
                    "type": "rocketlogger",
                    "cell": config["cell1"]["name"],
                    "ts": d["ts"],
                    "v": d["V2"],
                    "i": d["I2"],
                }

                meas_json = json.dumps(meas)

                buf.append(meas_json)


        # Add TEROS-12 data to buffer
        if ("teros" in config):
            for d in t12.measure():
                meas = {
                    "type": "teros12",
                    "cell": t12_map[d["sensorID"]],
                    "vwc": d["vwc"],
                    "temp": d["temp"],
                    "ec": d["ec"],
                }

                meas_json = json.dumps(meas)

                buf.append(meas_json)


        # Send everything in buffer
        for d in buf:
            uploader.send(d)

        # Clear buffer after transmit
        buf.clear()

        # Sleep for a given number of seconds    
        sleep(config["interval"])


if __name__ == "__main__":
    cli()