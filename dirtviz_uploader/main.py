#!/usr/bin/env python

from argparse import ArgumentParser
from csv import DictWriter
import json
from time import time_ns, sleep
from pprint import pprint
import os

from serial import SerialTimeoutException
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
    if (args.verbose > 1):
        print("Opening ZMQ connection with RocketLogger")

    rl = RocketLogger()

    # Create TEROS-12
    if "teros" in config:
        if (args.verbose > 1):
            print("Opening serial connection with TEROS12 Arduino")

        t12 = Teros12(config["teros"]["port"], config["teros"]["baud"],
                      timeout=10)
        t12.coef = [float(c) for c in config["teros"]["calibration"]]


    # Create upload method
    if config["method"] == "lora":
        if (args.verbose > 1):
            print("Opening serial connection with LoRa module")

        uploader = Lora(config["lora"]["port"], config["lora"]["baud"])
    elif config["method"] == "none":
        pass
    else:
        error = f"{config['method']} upload method not supported"
        raise NotImplementedError(error)

    # Create csv files
    if config["backup"]:
        if (args.verbose > 1):
            print("Creating CSV files and writting headers")

        # Generate filenames
        start_time = time_ns()

        csvfiles = {}

        # Cells
        for cell in ["cell1", "cell2"]:
            # Loop over datatype
            for dtype in ["rocketlogger", "teros12"]:
                filepath = f"{config[cell]['name']}_"
                filepath += dtype

                # Append path
                if ("backup_folder" in config):
                    fullpath = os.path.join(config["backup_folder"],
                                            filepath)

                fd = open(fullpath, "w")

                if dtype == "rl":
                    fn = ["ts", "v", "i"]
                elif dtype == "teros":
                    fn = ["ts", "raw_vwc", "vwc", "temp", "ec"]

                csv = DictWriter(fd, fieldnames=fn, extrasaction="ignore")
                csv.writeheader()

                csvfiles[cell][dtype]["fd"] = fd
                csvfiles[cell][dtype]["cell"] = csv


    # Buffer to store iterations for measurements
    buf = []


    #
    # Main Loop
    #

    # Loop forever
    while True:
        if (args.verbose > 1):
            print("RocketLogger measurement")

        # Add RocketLogger data to buffer
        rl_d = rl.measure()

        if (args.verbose > 2):
            print(rl_d)

        # Channel 1
        if "cell1" in config:
            meas = {
                "type": "rocketlogger",
                "cell": config["cell1"]["name"],
                "ts": rl_d["ts"],
                "v": rl_d["V1"],
                "i": rl_d["I1"],
            }

            if (args.verbose > 2):
                print(f"cell1: {meas}")

            buf.append(meas)

        # Channel 2
        if "cell2" in config:
            meas = {
                "type": "rocketlogger",
                "cell": config["cell1"]["name"],
                "ts": rl_d["ts"],
                "v": rl_d["V2"],
                "i": rl_d["I2"],
            }

            if (args.verbose > 2):
                print(f"cell2: {meas}")

            buf.append(meas)


        # Add TEROS-12 data to buffer
        if ("teros" in config):
            if (args.verbose > 1):
                print("Reading TEROS12 Sensor")

            # If Teros12 measure fails then go to next loop
            try:
                t12_d = t12.measure()
            except (SerialTimeoutException, ValueError):
                print("Failed to read TEROS12, retrying...")
                buf.clear()
                continue

            # Loop over t12 data
            for d in t12_d:
                # Find cell names associated with sensorIDs

                for c in ["cell1", "cell2"]:
                    if config[c]["teros"] == d["sensorID"]:
                        meas = {
                            "ts": d["ts"],
                            "type": "teros12",
                            "cell": config[c]["name"],
                            "vwc": d["vwc"],
                            "raw_vwc": d["raw_vwc"],
                            "temp": d["temp"],
                            "ec": d["ec"],
                        }

                        if (args.verbose > 2):
                            print(meas)

                        buf.append(meas)


        if (args.verbose > 1):
            print("Writing to csv")

        # Send everything in buffer
        for d in buf:
            # Print everything in buffer
            if (args.verbose > 0):
                print(d)

            # Write to file
            if config["backup"]:
                csvfiles[d["cell"]][d["type"]]["csv"].write(d)
                csvfiles[d["cell"]][d["type"]]["fd"].flush()

            # Upload data
            if config["method"] != "none":
                if (args.verbose > 1):
                    print(f"Uploading via {config['method']}")

                dj = json.dumps(d)

                # Print uplodaed json
                if (args.verbose > 2):
                    print(dj)

                uploader.send(dj)

        # Clear buffer after transmit
        if (args.verbose > 1):
            print("Clearing buffer")
        buf.clear()

        # Sleep for a given number of seconds
        if (args.verbose > 1):
            print(f"Sleeping for {config['interval']} seconds")
        sleep(config["interval"])


if __name__ == "__main__":
    cli()
