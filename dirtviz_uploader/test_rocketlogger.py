#!/usr/bin/env python

from .rocketlogger import RocketLogger


if __name__ == "__main__":
    rl = RocketLogger()
    
    while True:
        data = rl.measure() 
        print(data)