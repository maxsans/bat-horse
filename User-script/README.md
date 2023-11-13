# User-script

**Use `User_guide.pdf` to start using this tool**

## Preparing Files

1. Add the files created on the Raspberry Pi to the `/datas` folder within the application.

2. Launch the `Bewegungsfelder.exe` application located in the `/bewegungsfelder-app` folder. This application handles the 3D visualization of the data.

3. Next, run the 'app.exe' application.

## Modifying the Server Address

The server address is already preconfigured, but you can change it at the application's startup.

## How the Application Works

This Python application is designed to send previously recorded data from the Raspberry Pi to `Bewegungsfelder.exe`. A timestamp is recorded to ensure smooth visualization.

The application retrieves data files from the `datas` folder and sends them to port 5555 to `Bewegungsfelder.exe`.

The application's interface includes two options: `Fake Data` and `Start`.

- **Fake Data**: This option allows you to send fake data with the IDs of sensors present in the data. It enables you to set up the sensors on `Bewegungsfelder.exe` with their IDs. Once the positioning is correct in the visualization application, you can stop the fake data and press `Start` to send real data.

## Managing Files on the Server

The application's interface offers a feature to retrieve data from a server. However, the Raspberry Pi has not been implemented to send files to the server yet. For now, there is nothing to retrieve. Once files are available on the server, and the server's address is configured, the application will be able to retrieve the files and save them in the 'datas' folder.

## Application Features

The application offers several features to select local or remote files for visualization, initiate live visualization with fake data, and stop live visualization.

- **Choose a Local File**: You can select a local data file for visualization. If the file exists, you can load it by clicking the "Confirm" button.

- **Delete a Local File**: You have the option to delete a local file by selecting it in the list of local files and clicking the "Delete" button.

- **Choose an Online File**: You can select an online file from the remote server. If the file is available, you can load it by clicking the "Confirm" button.

- **Delete an Online File**: You can delete an online file by selecting it in the list of online files and clicking the "Delete" button.

- **Init with Fake Data**: You can initiate visualization with fake data by clicking the "Init with fake data" button.

- **Start Live Visualization**: You can start live data visualization by clicking the "Start" button.

- **Stop Live Visualization**: You can stop live data visualization by clicking the "Stop" button.

## Server Settings

The application uses a remote server to retrieve files online and perform live operations. You can modify the server's address by changing the `url` variable in the source code. Here's how to do it:

```python
# Server URL
url = "http://nimbus-idir.ie"  # Replace this URL with your server's address
```

## Start app for dev 

```
pip install -r requirements.txt
python app.py
```