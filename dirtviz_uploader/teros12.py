from serial import Serial

class Teros12(Serial):
    """Interface with TEROS-12 soil moisture sensor connected through an
    Arduino.
    """

    def measure(self) -> dict:
        """Take measurement from TEROS-12.
        
        Returns
        -------
        dict
            Key value pair of measurements from each soil moisture senor. The
            keys are the sensorId from each individually connected TEROS-12
            sensor.
        """

        pass