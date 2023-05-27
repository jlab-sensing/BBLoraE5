import requests

class HTTP:
    def __init__(self, hostname : str):
        """Constructor

        Parameters
        ----------
        hostname : str
            Hostname of server to post to
        """

        self.hostname = hostname


    def send(self, data : dict):
        """Send data as HTTP request

        Parameters
        ----------
        data : dict
            Dictionary of values

        Returns
        -------
        requests.Response
            Request response
        """

        r = requests.post(self.hostname, json=data)

        return r
