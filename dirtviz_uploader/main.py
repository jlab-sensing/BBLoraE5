#!/usr/bin/env python

from argparse import ArgumentParser
import csv
from time import sleep
from pprint import pprint
import os

import requests
from serial import SerialTimeoutException
from yaml import load
try:
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader

from .rocketlogger import RocketLogger
from .teros12 import Teros12
from .lora import Lora
from .http import HTTP

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
    if config["method"] == "http":
        if (args.verbose > 1):
            print("Sending data as http request")

        uploader = HTTP()
    elif config["method"] == "none":
        pass
    else:
        error = f"{config['method']} upload method not supported"
        raise NotImplementedError(error)

    # Create csv files
    if config["backup"]:
        if (args.verbose > 1):
            print("Creating CSV files and writting headers")

        csvfiles = {
            config["cell1"]["name"]: {
                "rocketlogger": {},
                "teros12": {},
            },
            config["cell2"]["name"]: {
                "rocketlogger": {},
                "teros12": {},
            },
        }

        # Cells
        for cell in csvfiles.keys():
            # Loop over datatype
            for dtype in ["rocketlogger", "teros12"]:
                filepath = f"{cell}_{dtype}.csv"

                # Append path
                if ("backup_folder" in config):
                    fullpath = os.path.join(config["backup_folder"],
                                            filepath)

                # Check for existance of file
                file_exists = os.path.isfile(fullpath)

                # Open file
                fd = open(fullpath, "a")

                # Create CSV writter
                if dtype == "rocketlogger":
                    fn = ["ts", "v", "i"]
                elif dtype == "teros12":
                    fn = ["ts", "raw_vwc", "vwc", "temp", "ec"]

                csvDictWriter = csv.DictWriter(fd, fieldnames=fn, extrasaction="ignore")
                # Write headers if file did not initially exist
                if not file_exists:
                    csvDictWriter.writeheader()
                    

                csvfiles[cell][dtype]["fd"] = fd
                csvfiles[cell][dtype]["csv"] = csvDictWriter


    # Buffer to store iterations for measurements
    buf = []
    row_num = -1 # This is for marking where the server went down and how much data needs to be uploaded once it's back uo
    # -1 when not in use, the total number of surpassed rows when not
    
    cur_row = 1 # Counts how many rows have been added to the csv, not perfect, but at a second of upload time it will still last 68 years


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
            
        # Account for a possible error in posting process
        if row_num != -1: # If previous upload failed
            with open(fullpath) as prev_stored_data: 
                csv_reader = list(csv.reader(prev_stored_data)) # Open the csv from the stored row number
                rows = list(csv_reader)
                for row in rows[row_num:]: # Should iterate from current row marker to the end
                    buf.append(row) # Append every row from the marker onwards to the lists
                    
            print("Previous upload failed")
            

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
                "cell": config["cell2"]["name"],
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
            print(d)
            if (args.verbose > 0):
                print(f"Internal Data: {d}")

            # Write to file
            if config["backup"]:
                csvfiles[d["cell"]][d["type"]]["csv"].writerow(d)
                csvfiles[d["cell"]][d["type"]]["fd"].flush()

            if config["method"] != "none":
                if (args.verbose > 1):
                    print(f"Uploading via {config['method']}")

            # Upload data
            if config["method"] == "http":
                # Explicitly state json keys

                # Formtated dict for jsonification
                f = {
                    "cell": str(d["cell"]),
                    "ts": int(d["ts"]),
                }

                if d["type"] == "rocketlogger":
                    f["logger"] = str(config["name"])
                    f["v"] = float(d["v"])
                    f["i"] = float(d["i"])

                    endpoint = config["http"]["rl_endpoint"]

                elif d["type"] == "teros12":
                    f["vwc"] = float(d["vwc"])
                    f["raw_vwc"] = float(d["raw_vwc"])
                    f["temp"] = float(d["temp"])
                    f["ec"] = int(d["ec"])

                    endpoint = config["http"]["teros_endpoint"]

                # Send post request
                r = requests.post(endpoint, json=f)
                print(r.status_code)

                #if (args.verbose > 2):
                    #print(r)

                # Check status code
                if (r.status_code != 200): # if a failure check to see if there is a row_num stored
                    if (row_num == -1):
                        row_num = cur_row # 
                else: # If succses reset row_num
                    row_num = -1
                    
                print(r.status_code)
                
                cur_row += 1 # Iterate number of rows stored
                print("Clearing buffer")
                buf.clear()
        # Sleep for a given number of seconds
        if (args.verbose > 1):
            print(f"Sleeping for {config['interval']} seconds")
        #sleep(config["interval"])
        sleep(4)


if __name__ == "__main__":
    cli()
