# -*- coding: utf-8 -*-
"""
Created on Wed Sep 27 10:25:12 2023

@author: Steve

"""


import http.server
import socketserver

# Specify the port you want to use
PORT = 8000
# Global counter for POST requests
post_request_count = 0

# Define a custom request handler that responds to both GET and POST requests
class MyHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(b"Hello, World!")
        else:
            self.send_response(404)
            self.send_header("Content-type", "text/plain")
            self.end_headers()
            self.wfile.write(b"404 Not Found")

    def do_POST(self):
        global post_request_count
        post_request_count += 1

        if post_request_count <= 2 or post_request_count <= 8:
            # Respond with a 200 OK for the first 5 POST requests
            self.send_response(200)
            self.send_header("Content-type", "text/plain")
            self.end_headers()
            self.wfile.write(b"POST request received and processed.")
        else:
            # Respond with a 502 Bad Gateway for subsequent POST requests
            self.send_response(502)
            self.send_header("Content-type", "text/plain")
            self.end_headers()
            self.wfile.write(b"502 Bad Gateway - Too many POST requests.")
# Create the HTTP server
with socketserver.TCPServer(("127.0.0.1", PORT), MyHandler) as httpd:
    print(f"Serving at port {PORT}")
    # Start the server
    httpd.serve_forever()