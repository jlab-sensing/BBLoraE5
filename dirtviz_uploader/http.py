import requests

class HTTP:
    def send(self, data : dict, url=None):
        """Send data as HTTP request

        Parameters
        ----------
        data : dict
            Dictionary of values
        url : str
            Endpoint url

        Returns
        -------
        requests.Response
            Request response
        """

        r = requests.post(url, json=data)

        return r
