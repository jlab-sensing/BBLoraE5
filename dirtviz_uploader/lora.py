from serial import Serial

class Lora(Serial):
    def send(self, data : str):
        """Send data over LoRa.

        Parameters
        ----------
        data : str
            Character array of data.
        """

        pass