# -*- coding: utf-8 -*-
"""
Created on Wed Sep 27 10:25:12 2023

@author: Steve

"""


import http.server
import socketserver

# Specify the port you want to use
PORT = 8000

# Define a custom request handler that responds with "Hello, World!"
class MyHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        self.post_request_count = 0
        super().__init__(*args, **kwargs)
    
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "text/plain")
        self.end_headers()
        self.wfile.write(b"Hello, World!")
        
    def do_POST(self):
        self.post_request_count += 1

        if self.post_request_count <= 5:
            # Respond with a 200 OK for the first 5 POST requests
            self.send_response(200)
            self.send_header("Content-type", "text/plain")
            self.end_headers()
            self.wfile.write(b"POST request received and processed.")
        else:
            # Respond with a 502 Bad Gateway for subsequent POST requests
            self.send_header("Content-type", "text/plain")
            self.end_headers()
            self.wfile.write(b"502 Bad Gateway - Too many POST requests.")

# Create the HTTP server
with socketserver.TCPServer(("127.0.0.1", PORT), MyHandler) as httpd:
    print(f"Serving at port {PORT}")
    # Start the server
    httpd.serve_forever()