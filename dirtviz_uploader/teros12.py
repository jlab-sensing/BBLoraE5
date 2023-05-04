class Teros12:
    """Interface with TEROS-12 soil moisture sensor connected through an
    Arduino.
    """

    def __init__(self, port, baud=9600):
        """Opens serial connection to Arduino.

        Parameters
        ----------- 
        port : str
            Serial port of Arduino.
        baud : int, optional
            Baud rate of serial connection to Arduino.
        """

        pass


    def __del__(self):
        """Close serial connection with Arduino.
        """

        pass


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