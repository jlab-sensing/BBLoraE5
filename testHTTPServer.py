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
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "text/plain")
        self.end_headers()
        self.wfile.write(b"Hello, World!")
        
    def do_POST(self):
        self.send_response(200)
        self.send_header("Content-type", "text/plain")
        self.end_headers()
        self.wfile.write(b"POST request received and processed.")

# Create the HTTP server
with socketserver.TCPServer(("127.0.0.1", PORT), MyHandler) as httpd:
    print(f"Serving at port {PORT}")
    # Start the server
    httpd.serve_forever()