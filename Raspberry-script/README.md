# Web app 3D mocap vizualisation

The aim of this project is to create a 3D visualization of a horse. The sensors used to capture a horse's movements are available at this link: https://github.com/maxsans/bat-horse.

The sensors send the motion capture data to an udp server (Port 5555). The backend then sends the data to the front end via the web socket (Port 8080). The frontend provides a 3D visualization of all the sensors attached to the horse.

This Python script runs on a Raspberry Pi and uses Twisted for UDP data transmission. It receives data from sensors and allows for data recording and direct retransmission. Additionally, it offers a web interface to control recording and check sensor status.

## Prerequisites

Make sure you have the following dependencies installed on your Raspberry Pi:

- Python
- Twisted
- Additional Python libraries: struct, json, datetime, subprocess, os, time, shutil

## Usage

1. Run the script on your Raspberry Pi to start the UDP server.

2. Access the web interface by connecting to your Raspberry Pi's IP address on port 8080 in a web browser.

3. Use the web interface to start and stop data recording, update direct retransmission settings, and check sensor statuses.

## Features

- **UDP Data Reception:** The script listens for UDP data from sensors and processes it.

- **Data Recording:** Toggle data recording on and off. Recorded data is saved to JSON files with timestamps.

- **Direct Retransmission:** If enabled, the script can retransmit received data over UDP to another destination.

- **Web Interface:** Access a web interface to control the script, check the status of connected sensors, and update retransmission settings.

## Web Interface Endpoints

- `/start_record`: Start recording sensor data.
- `/stop_record`: Stop recording sensor data and copy recorded data to a USB drive.
- `/get_status_record`: Get the status of data recording.
- `/get_sensor_status`: Get the status of connected sensors.
- `/update_status_direct_retransmission`: Update direct retransmission status.
- `/get_status_direct_retransmission`: Get the status of direct retransmission.

## Directory Structure

- The recorded data is saved in the `./datas` directory.
- The web interface files are stored in the `tmp/` directory.
