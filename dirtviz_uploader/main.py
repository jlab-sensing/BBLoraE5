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

        rlroots = {
            config["cell1"]["name"]: f"{config['cell1']['name']}_rl",
            config["cell2"]["name"]: f"{config['cell2']['name']}_rl"
        }

        terosroots = {
            config["cell1"]["name"]: f"{config['cell1']['name']}_teros",
            config["cell2"]["name"]: f"{config['cell2']['name']}_teros"
        }


        rlpaths = {}
        terospaths = {}


        # Loop over roots and paths
        for root, path in zip([rlroots, terosroots], [rlpaths, terospaths]):
            # Loop over all elements in path
            for key, value in root.items():
                # The main format string
                filename = f"{start_time}_{value}.csv"

                # Append path
                if ("backup_folder" in config):
                    fullpath = os.path.join(config["backup_folder"], filename)
                else:
                    fullpath = filename

                # Store in path dictionaries
                path[key] = fullpath

        # Print formatted paths
        if args.verbose > 0:
            print("Filepaths for RocketLogger csv")
            for _, v in rlpaths.items():
                print(v)

            print("Filepaths for TEROS12 csv")
            for _, v in terospaths.items():
                print(v)


        # Open csv filestreams and write headers

        # Dictionary to hold csv writters for rl and teros. Both dictionaries
        # are mapped to cell names
        rl_csv = {}
        teros_csv = {}

        # headers for teros and rocketlogger data
        rl_fieldnames = ["ts", "v", "i"]
        teros_fieldnames = ["ts", "raw_vwc", "vwc", "temp", "ec"]

        for cell in ["cell1", "cell2"]:
            # Open cell1 csv
            if cell in config:
                name = config[cell]["name"]

                # Create directories if doesn't exist
                os.makedirs(os.path.dirname(rlpaths[name]), exist_ok=True)
                # Open FD
                rl_csv_fs = open(rlpaths[name], "w")
                # Initialize DictWriter
                rl_csv[name] = DictWriter(
                    rl_csv_fs,
                    fieldnames=rl_fieldnames,
                    extrasaction='ignore'
                )
                # Write header
                rl_csv[name].writeheader()

                # Open teros1 csv
                if "teros" in config[cell]:
                    # Create directories if doesn't exist
                    os.makedirs(os.path.dirname(terospaths[name]), exist_ok=True)
                    # Open FD
                    teros_csv_fs = open(terospaths[name], "w")
                    # Initialize DictWriter
                    teros_csv[name] = DictWriter(
                        teros_csv_fs,
                        fieldnames=teros_fieldnames,
                        extrasaction='ignore'
                    )
                    # Write header
                    teros_csv[name].writeheader()


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

            if config["backup"]:
                if d["type"] == "rocketlogger":
                    teros_csv[d["cell"]].writerow(d)
                    teros_csv[d["cell"]].flush()
                elif d["type"] == "teros12":
                    rl_csv[d["cell"]].writerow(d)
                    rl_csv[d["cell"]].flush()

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
