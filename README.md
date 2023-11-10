# Motion Capture System for Horses

This project focuses on developing a motion capture system for horses using accelerometers and gyroscopes. The goal is to capture horse movements and display them in a 3D model. The project addresses various challenges related to data collection, transfer, processing, and visualization.

## Project Overview

The project encompasses several components, each addressing specific aspects:

### Motion Sensor (https://github.com/maxsans/bat-horse/tree/Motion-sensor)

The motion sensor component deals with the development and integration of sensors, particularly the ESP32 and MPU6050 accelerometers and gyroscopes. These sensors are essential for capturing horse movements accurately.

### Raspberry Pi Script and Web Interface (https://github.com/maxsans/bat-horse/tree/Raspberry-script)

This part involves the creation of a script to manage data transfer from the motion sensors to a Raspberry Pi. A web interface is also designed to control data recording and visualization. The primary aim is to enable users to start and stop recordings easily.

### Cloud Server Script (https://github.com/maxsans/bat-horse/tree/Server-script)

The cloud server script manages the data received from the Raspberry Pi. It handles real-time data transmission, ensuring that the data is accessible from anywhere with an internet connection. Additionally, it allows the sharing of saved data files across the network.

### User Application (https://github.com/maxsans/bat-horse/tree/User-script)

The user application provides a user-friendly interface for interacting with the motion capture system. It includes buttons for managing data visualization, enabling and disabling live data transmission.

## Challenges Faced

Throughout the project, we faced a number of challenges, including

- Creating an efficient system for transferring data from motion sensors to the cloud.
- Ensuring real-time data visualisation for accurate monitoring.
- Managing connectivity issues and preventing data loss.
- Develop a user interface to facilitate control of the system.
- Test the system on humans to validate its functionality.

## Conclusion

This project represents an innovative solution for capturing horse movements, with potential applications in equine research and beyond. By addressing the challenges faced and developing specific components, we have made significant progress toward our goal.
