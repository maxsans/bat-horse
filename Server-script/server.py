import os
# Server
from flask_cors import CORS
from flask import Flask, jsonify, send_file, request

# configuration
DEBUG = True

# instantiate the app
app = Flask(__name__, static_folder='static')
app.config.from_object(__name__)

# enable CORS
CORS(app, resources={r'/*': {'origins': '*', 'methods': 'POST', 'headers': 'Content-Type'}})

# Folder to save files
folder_path = "./datas_online/"


# Add data file
@app.route('/file', methods=['POST'])
def addFile():
    # Check if file in request
    if 'file' not in request.files:
        return 'No file send'

    uploaded_file = request.files['file']
    # Check name
    if uploaded_file.filename == '':
        return 'No file name'
    # If file OK, save it
    if uploaded_file:
        uploaded_file.save(os.path.join([folder_path], uploaded_file.filename))
        return 'File saved !'


# Get all files
@app.route('/files', methods=['GET'])
def getAllFiles():

    # Listing all files in directory
    data = os.listdir(folder_path)
    # Return list of files
    return jsonify(data), 200 


# Get one file
@app.route('/file/<path:filename>', methods=['GET'])
def getOneFile(filename):

    # File
    file_path = folder_path+filename
    # return a file
    return send_file(file_path, as_attachment=True)


# Delete a file
@app.route('/delete/<path:filename>', methods=['POST'])
def deleteFile(filename):

    directory_path = folder_path+filename
    # if file exist, delete it
    if os.path.exists(directory_path):
        os.remove(directory_path) 
        msg = (f"{filename} has been deleted.")
        return jsonify(msg), 200 
    else:
        msg = (f"{filename} does not exist.")
        return jsonify(msg), 400


if __name__ == '__main__':
    # Start server for server, or in local
    if os.environ.get('prod'):
        # IP for docker image
        app.run(host='0.0.0.0', port=3005, debug=DEBUG)
    else:
        # IP local
        app.run(host='127.0.0.1',port=3005,debug=DEBUG)