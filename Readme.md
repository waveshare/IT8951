# IT8951 Fork

This project forks the IT8951 library from Waveshare.

The code is adapted to create a straighforward C library for drawing
on the Waveshare E-Paper screens, with Python wrappers for easy use.

## Status

The project is in its initial stage and under active development.

## Usage
A WebViewer is included which allows you to view a dynamic webpage.
For example (see [Sunrise](https://github.com/rmehyde/sunrise)):

`sudo python3 web.py --refresh 20 --url http://127.0.0.1:5000/sunrise/app?stopId=A44N&updateInterval=15&renderInterval=5&timeRound=30&timeOffset=10`

The C library calls run in their own thread and release the GIL, so you will likely need to kill the process manually.