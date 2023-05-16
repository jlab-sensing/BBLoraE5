from serial import Serial


class Teros12(Serial):
    """Interface with TEROS-12 soil moisture sensor connected through an
    Arduino.
    """

    def parse(self, raw : str) -> tuple:
        """Parses TEROS12 data into dictionary.

        Parameters
        ----------
        raw : str
            Raw readings.

        Returns
        -------
        tuple
            Key value pairs for sensor ID and measurements in format (id, meas).
            meas is a dictionary with keys, "vwc", "t", and "ec".
        """

        values = raw.split('+')
        
        data = {
            "sensorID": values[0],
            "vwc": values[1],
            "temp": values[2],
            "ec": values[3]
        }

        return data


    def measure(self) -> dict:
        """Take measurement from TEROS-12.
        
        Returns
        -------
        dict
            Key value pair of measurements from each soil moisture senor. The
            keys are the sensorId from each individually connected TEROS-12
            sensor.
        """

        # Measurement dictionary
        meas = []

        # Send measure command
        # NOTE check line endings to match Arduino implementation
        self.write(b'MEAS')

        while True:
            # Read serial port
            raw = self.readline()
            raw = raw.strip()

            # Check for end signal    
            if (raw == "END"):
                break

            # Read single measurement
            single = self.parse(raw)

            meas.append(single)

        return meas