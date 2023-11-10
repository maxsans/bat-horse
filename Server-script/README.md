# Server-script

This Python script is a server created with Flask that aims to receive files from a Raspberry Pi after data recording by a user. These files are stored on a server (NAS) that hosts the interns' websites. Subsequently, users can access and manage files on the server from the Python application at `/computer_app/app.exe`.

## Usage

1. Add files to the server:
   - You can add files using the `/file` route with a POST request. Files must be included in the request with the appropriate filename. The server checks if the file has been received correctly and stores it in the specified directory.

   !! The Raspberry Pi does not have the function to send files to the server

2. Get the list of files:
   - You can obtain the list of all files stored on the server using the `/files` route with a GET request. The server will return the list of file names as a JSON response.

3. Get a specific file:
   - To retrieve a specific file, use the `/file/<file_name>` route with a GET request. The server will return the specified file as a download.

4. Delete a file:
   - You can delete a file using the `/delete/<file_name>` route with a POST request. If the file exists, the server will delete it and return a response indicating that the file has been removed. If the file does not exist, the server will return a message indicating that the file was not found.

## Installation and Execution

To run localy this server, you need to have Flask and other dependencies installed. You can install them using the following command:

```shell
pip -r requirements.txt
```

Once Flask is installed, you can run the server by executing the Python script. The server will listen on port 3005 by default. You can change the port by modifying the following line

```python
app.run(host='127.0.0.1', port=3005, debug=DEBUG)
```

To run the server :

```shell
python app.py
```


# Deployment on the Server
For the next deployment, the Docker files are already configured, so there should be no need to modify them.

However, if additional dependencies are added, don't forget to include them in the `requirements.txt` file.

If folder management is modified on the server, make sure to update the access path in the Docker Compose file on the following line:

`device: /home/server/bathorse/datas_online`

This part of the access path concerns the location of the data on the server. Ensure that the path is correctly configured so the server can access the data without any issues.

