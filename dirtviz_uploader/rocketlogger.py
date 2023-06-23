import json
import os
import shutil
import subprocess
from time import sleep

import numpy as np
import zmq


class RocketLogger:
    """Interface with RocketLogger measurement device.
    """

    # Default values for rocketlogger
    _ROCKETLOGGER_BINARY = "rocketlogger"

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
        self.binary = self.getBinary()

        # Stop previous logging, no matter what
        subprocess.run([self.binary, "stop"])
        # Wait a second
        sleep(2)

        # Start logging with defined config
        config = {
            "channel": ["V1,V2,I1L,I1H,I2L,I2H"],
            "rate": 1000,
            "update": 1,
            "output": 0,
            "digital": False,
            "ambient": False,
            "web": True,
            "stream": True,
            "quiet": None,
        }
        args = self.configToCliArguments(config)
        self.rl_cli = subprocess.Popen([self.binary, "start"] + args)

        # Connect to socket
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.SUB)


    def __del__(self):
        """Destructor

        Closes the RocketLogger CLI interface
        """
        subprocess.run([self.binary, "stop"])


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


    def configToCliArguments(self, config : dict) -> list:
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


    def decode_time(self, msg, ns=False) -> int:
        """Decodes timestamp of measurement in unix epoch seconds by default
        
        Parameters
        ----------
        msg : bytes
            Binary measurement message
        ns : bool
            Flag to enable ns accuracy

        Returns
        -------
        int
            Timestamp of measurements
        """
        # Decode binary data
        time_arr = np.frombuffer(msg[1], dtype=self.DT_TIMESTAMP)

        # Datetime
        datetime = time_arr[0][0]

        # ns precision
        if ns:
            # Timedelta
            timedelta = time_arr[0][1]
            # Combine Datetime and Timedelta
            timestamp = datetime.astype('datetime64[ns]').astype(np.int64) + timedelta.astype(np.int64)

        # sec precision
        else:
            timestamp = datetime.astype(np.int64)

        return timestamp


    def decode_meas(self, msg) -> dict:
        """Decode measurement channels
        
        Parameters
        ----------
        msg : bytes
            Binary measurement message

        Returns
        -------
        dict
            Data dictionary with each of the keys as channels
        """

        data = {}

        meta = json.loads(msg[0])

        for ch_meta in meta["channels"]:
            data[ch_meta["name"]] = np.array
   
        # Channel metadata
        meta = json.loads(msg[0])
 
        # Store measurement data
        for ch_idx, ch_meta in enumerate(meta["channels"], start=2):
            # Stop at binary channels
            if ch_meta["unit"] == "binary":
                break

            # Convert binary to list of measurements
            meas_list = np.frombuffer(msg[ch_idx], dtype="<i4")
            # Adjust for units
            meas_adj_list = meas_list.astype(float) * ch_meta["scale"]
            # Store data
            data[ch_meta["name"]] = meas_adj_list

        # Store digital and valid channels which are all stored together
        # requiring special handling
        binary = np.frombuffer(msg[ch_idx], dtype="<u4")
        for ch_meta in meta["channels"][ch_idx-2:]:
            # Generate bitmask
            mask = 0x01 << ch_meta["bit"]
            # Store boolean
            data[ch_meta["name"]] = (binary & mask).astype(bool)

        # Apply valid to current channels
        for ch in [1,2]:
            # Hardcoded names of channels
            ch_name = f"I{ch}"
            ch_low_name = f"I{ch}L"
            ch_high_name = f"I{ch}H"
            ch_valid_name = f"I{ch}L_valid"

            valid_list = []

            for low, high, valid in zip(data[ch_low_name], data[ch_high_name], data[ch_valid_name]):
                if valid:
                    valid_list.append(low)
                else:
                    valid_list.append(high)

            data[ch_name] = np.array(valid_list)

            # Remove high low channels
            del data[ch_low_name]
            del data[ch_high_name]
            del data[ch_valid_name]
        return data
    

    def average(self, data : dict) -> dict:
        """Takes averages of array in each key

        Parameters
        ----------
        data : dict
            Dictionary with channel names keys and arrays of measurements

        Returns
        -------
        dict
            Same keys with averages    
        """

        for key, value in data.items():
            data[key] = np.mean(value)

        return data


    def measure(self) -> dict:
        """Reads most recent measurement from RocketLogger V1, V2, I1, and I2
        channels.

        Returns
        -------
        dict
            Key value pairs of measurements with the following keys ["V1", "I1",
            "V2", "I2"].
        """

        # Connect and read message
        self.socket.connect(self.DATA_SOCKET)
        self.socket.subscribe("")
        msg = self.socket.recv_multipart()

        ## Process Data

        # Measurement time
        ts = self.decode_time(msg)
        # Decode measurement into channel
        meas = self.decode_meas(msg)

        ## Aggregate data

        # Average data
        meas = self.average(meas)
        # Add timestamp
        meas["ts"] = ts

        ## Disconnect from socket when entering sleep mode
        self.socket.unsubscribe("")
        self.socket.disconnect(self.DATA_SOCKET)

        return meas
