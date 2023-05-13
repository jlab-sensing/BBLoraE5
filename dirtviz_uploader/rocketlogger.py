import json
import os
import shutil
import subprocess

import numpy as np
import zmq


class Rocketlogger:
    """Interface with Rocketlogger measurement device.
    """

    # Default values for rocketlogger
    _ROCKETLOGGER_BINARY = "rocketlogger"
    _DEFAULT_SAMPLE_RATE = 1000
    _DEFAULT_SAMPLE_COUNT = 5 * _DEFAULT_SAMPLE_RATE
    _DEFAULT_FILE_COMMENT = "RocketLogger system test"
    _DEFAULT_FILE_SIZE = 100 * 10**6

    DATA_SOCKET = "tcp://127.0.0.1:8277"

    DT_TIMESTAMP = np.dtype(
        [
            ("realtime_sec", "<M8[s]"),
            ("realtime_ns", "<m8[ns]"),
            ("monotonic_sec", "<M8[s]"),
            ("monotonic_ns", "<m8[ns]"),
        ]
    )
    
    def __init__(self):
        """Start logging on RocketLogger daemon and open connection to ZeroMQ
        socket.
        """

        # Find binary
        binary = self.getBinary()

        # Stop previous logging
        subprocess.run([binary, "stop"], check=True)

        # Start logging with defined config
        config = {
            "channel": ["V1,V2,I1L,I1H,I2L,I2H"],
            "rate": 1000,
            "update": 1,
            "output": "data.rld",
            "format": "rld",
            "size": "??",
            "comment": "",
            "digital": False,
            "ambient": True,
            "aggregate": "downsample",
            "high-range": [],
            "web": True,
        }
        args = self.configToCliArguments(config)
        subprocess.run([binary, "start"] + args, check=True)


        # Connect to socket
        self.context = zmq.Context()

        self.socket = self.context.socket(zmq.SUB)
        self.socket.connect(self.DATA_SOCKET) 
        self.socket.subscribe("")


    def getBinary(self) -> os.path:
        """
        Get the absolute path of the installed RocketLogger CLI binary.

        Returns
        -------
        os.path
            Full path to RocketLogger binary
        """
        binary = shutil.which(self._ROCKETLOGGER_BINARY)

        if not os.path.exists(binary):
            raise FileNotFoundError(f"Could not find RocketLogger CLI binary! [{binary}]")
        return os.path.abspath(binary)


    def configToCliArguments(config : dict) -> list:
        """
        Get CLI arguments for configuration.

        Parameters
        ----------
        config : dict
            Configuration to process.

        Returns
        -------
        list
            List of CLI arguments. 
        """
        if not isinstance(config, dict):
            raise TypeError("Expected dict for config")

        args = []
        for key, value in config.items():
            if value == None:
                args.append(f"--{key}")
                continue

            if isinstance(value, list):
                value = ",".join(value)
            args.append(f"--{key}={value}")

        return args


    def measure(self) -> dict:
        """Reads most recent measurement from Rocketlogger V1, V2, I1, and I2
        channels.

        Returns
        -------
        dict
            Key value pairs of measurements with the following keys ["V1", "I1",
            "V2", "I2"].
        """

        message = self.socket.recv_multipart()

        # 1. JSON channel metadata
        meta = json.loads(message[0])
        print(f"data received: {meta}")

        # 2. Data block timestamps
        time = np.frombuffer(message[1], dtype=self.DT_TIMESTAMP)
        print(f"time: {time}")

        channel_index = 2
        # 3. non-binary channels in order of channel metadata
        for channel in meta["channels"]:
            if channel["unit"] == "binary":
                continue

            data = np.frombuffer(message[channel_index], dtype="<i4")
            print(channel["name"], data)

            channel_index += 1

        # 4. binary channel bitmaps (if any listed in metadata)
        binary = np.frombuffer(message[channel_index], dtype="<u4")
        print(f"binary: {binary}")