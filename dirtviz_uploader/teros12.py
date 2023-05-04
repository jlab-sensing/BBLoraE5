class Teros12:
    """Interface with TEROS-12 soil moisture sensor connected through an
    Arduino
    """

    def __init__(self, port, baud=9600):
        """Initialize serial connection to Arduino

        Parameters
        ----------- 
        port : str
            Serial port of Arduino    
        baud : int, optional
            Baud rate of serial connection to Arduino
        """

        pass


    def __del__(self):
        """Close serial connection with Arduino"""

        pass


    def measure(self) -> list:
        """Take measurement from TEROS-12
        
        Returns
        -------
        list
            List of measurements in the following format 
        """

        pass