class Rocketlogger:
    """Interface with Rocketlogger measurement device.
    """
    
    def __init__(self):
        """Start logging on Rocketlogger daemon and open connection to ZeroMQ
        socket.
        """

        pass

    def measure(self) -> dict:
        """Reads most recent measurement from Rocketlogger V1, V2, I1, and I2
        channels.

        Returns
        -------
        dict
            Key value pairs of measurements with the following keys ["V1", "I1",
            "V2", "I2"].
        """