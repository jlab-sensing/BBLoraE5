from time import time_ns

from serial import Serial


class Teros12(Serial):
    """Interface with TEROS-12 soil moisture sensor connected through an
    Arduino.
    """

    # Coefficients for polynomial fit
    coef = [1]

    def apply_poly_fit(self, raw_vwc : int, coef : list) -> float:
        """Applies polynomial fit raw VWC to transfer into a precentage

        Parameters
        ----------
        raw_vwc : int
            Raw VWC readings from TEROS12
        coef : lits
            Polynomial coefficients in decending order. The element coef[0]
            corresponds to the highest order term

        Returns
        -------
        float
            VWC as percentage
        """

        # Reverse coef order so lowest order is first
        coef.reverse()

        vwc = 0

        for i, c in enumerate(coef):
            vwc += c * (raw_vwc ** i)

        return vwc


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
            meas is a dictionary with keys, "raw_vwc", "vwc", "t", and "ec".
        """

        # Split and convert to ints
        values = [int(v) for v in raw.split('+')]

        data = {
            "sensorID": values[0],
            "raw_vwc": values[1],
            "vwc": self.apply_poly_fit(values[1], self.coef),
            "temp": values[2],
            "ec": values[3],
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
        self.write(b'MEAS\r\n')

        while True:
            # Read serial port
            raw = self.readline()
            raw = raw.strip()

            meas_str = raw.decode("utf-8")

            # Check for end signal
            if (meas_str == "END"):
                break

            # Read single measurement
            single = self.parse(meas_str)
            # Append timestamp
            single["ts"] = time_ns()

            meas.append(single)

        return meas
