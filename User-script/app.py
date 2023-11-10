# Interface
import tkinter as tk
from tkinter import messagebox
#   
import requests
import threading
import socket
import time
import os
import struct
from twisted.internet.protocol import DatagramProtocol
from twisted.internet import reactor



# Server URL 
url = "http://nimbus-idir.ie"   # Url for request
server_address = (url, 5550)    # Adress and port for direct link
stop = False    
direct_link = False


def get_ip_from_server(server_address, timeout=3):
    """
    This function is responsible for registering with a server and obtaining an IP address. It uses UDP communication
    to send registration and request messages to the specified server.

    :param server_address: A tuple containing the server's IP address and port.
    :param timeout: The maximum time to wait for a response from the server in seconds (default is 3 seconds).

    :return: True if the registration and IP request were successful, or False if a timeout occurred or no data was received.
    """
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client_socket.sendto(b"register", server_address)
    client_socket.settimeout(timeout)  # Set a timeout for receiving data
    while True:
        try:
            client_socket.sendto(b"get_ip", server_address)
            data, _ = client_socket.recvfrom(1024)
            if data:
                return True # data.decode('utf-8')
        except socket.timeout:
            # Handle timeout, for example, by retrying or returning None
            return False

def remove_from_relay():
    """
    This function is responsible for unregistering from a relay server, effectively terminating a data relay connection.
    It uses UDP communication to send an "unregister" message to the server.

    :return: None if the unregister request is successful. In case of a timeout or other errors, it may return None or
             perform error handling, as specified.
    """
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client_socket.settimeout(3)  # Set a timeout for receiving data
    try:        
        client_socket.sendto(b"unregister", server_address)
    except socket.timeout:
        # Handle timeout, for example, by retrying or returning None
        return

def start_receiving():
    """
    This function is used to start direct vizualisation
    """
    if get_ip_from_server(server_address):
        return True


# All the buttons for app
class ButtonsApp:
    
    path_file_to_read = ''
    udp_socket = None
    send_data_thread = None

    def __init__(self):
        super().__init__()

    # Function select file choose by user
    def confirmDataLocal(self):

        if createWindow.adding_list_local.curselection():                       # If a file is selected in app
            index = createWindow.adding_list_local.curselection()[0]            # Index of the name file, in list
            self.path_file_to_read = './datas/' + readFile.local_list[index]    # Setup path of selected file
            createWindow.CreateWindowSendData(self.path_file_to_read)           # Open second window with data of this file
        else:
            messagebox.showerror("Error", "You have to choose a file")          
    # Delete file in local folder
    def deleteDataLocal(self):
        if createWindow.adding_list_local.curselection():                       # If a file is selected in app 
            index = createWindow.adding_list_local.curselection()[0]            # Index of the name file, in list
            self.path_file_to_read = './datas/' + readFile.local_list[index]    # Path of file to delete
            if os.path.exists(self.path_file_to_read):                          # If path is available, delete the file
                os.remove(self.path_file_to_read)
                messagebox.showinfo("Infos", "Local file deleted !")
            else:
                messagebox.showerror("Error", "File not found")                 # Else : file not found
        else:
            messagebox.showerror("Error", "You have to choose a file")

    # Download online file
    def confirmDataOnline(self):
        if createWindow.adding_list_online.curselection():                          # If a file is selected
            index = createWindow.adding_list_online.curselection()[0]               # Get file name

            try:
                self.path_file_to_read = "./datas/"+readFile.online_list[index]     # Path to save the file                         
                uri = url+":3005/file/"+readFile.online_list[index]                 # Server url to dowload file, and port (3005)
                response = requests.get(uri)                                        # Request

                if response.status_code == 200:                                     # If request good
                    with open(self.path_file_to_read, "wb") as file:                # Download file in 'datas' folder
                        file.write(response.content)                        

                    createWindow.CreateWindowSendData(self.path_file_to_read)       # Open new window, with datas of downloaded file
                else:
                    print("Error :", response.status_code)
                    messagebox.showerror("Error", "File not available on the server")  

            except Exception as e:
                messagebox.showerror("Error", "Error due to server connection")
           
        else:
            messagebox.showerror("Error", "You have to choose a file")
    # Delete a file on server
    def deleteDataOnline(self):
        if createWindow.adding_list_online.curselection():                          # If a file is selected
            index = createWindow.adding_list_online.curselection()[0]               # Get file name
            try:
                self.path_file_to_read = "./datas/"+readFile.online_list[index]     # Path save file                         
                uri = url+":3005/delete/"+readFile.online_list[index]               # Server url to dowload file, and port (3005)
                response = requests.post(uri)                                       # Request to delete file

                if response.status_code == 200:                                     # If request good, response file deleted
                    messagebox.showinfo("Infos", "File deleted !")
                
                else:
                    print("Error :", response.status_code)
                    messagebox.showerror("Error", "File not deleted on the server")

            except Exception as e:
                messagebox.showerror("Error", "Error due to server connection")
           
        else:
            messagebox.showerror("Error", "You have to choose a file")
    
    def directLink(self):                               # Start direct vizualisation
        if start_receiving():                           # If server OK
            global direct_link  
            direct_link = True
            createWindow.createWindowDirectLink()       # Open direct link window
        else:
            messagebox.showerror("Error", "Direct link not available")

    
    # Back to first window
    def backToFirstWindow(self):
        # If direct link in progress, stop socket
        global direct_link
        if direct_link:
            try: 
                remove_from_relay()
                direct_link = False
            except Exception :
                print("No socket to stop")
            
            
        # Refresh files lists
        readFile.getLocalFiles()
        readFile.getOnlineFiles()
        # Open first window        
        createWindow.CreateWindowChooseFile()

    # Fake data for init
    def fakeData(self):
        # Update btns
        createWindow.fake_btn.config(state=tk.DISABLED)
        createWindow.start_btn.config(state=tk.DISABLED)
        createWindow.stop_btn.config(state=tk.NORMAL)
        # Create a separate thread for data sending
        fake_data_thread = threading.Thread(target=self.fakeDataThread)
        fake_data_thread.start()

    def fakeDataThread(self):
        # Fake data to send (with actif sensors)
        fake_data_list = readFile.fake_data_list
        data_list = []
        
        for fake_data in fake_data_list: 
            # Set fake data into list, without last element (timestamp)
            fake_data = fake_data.split(',')
            integer_list = [int(string) for string in fake_data] # List str to list int
            integer_list.pop()
            # Encode format 
            data_format = 'l' * 12
            udp_data = struct.pack(data_format, *integer_list)
            data_list.append(udp_data)

        # Create UDP socket
        # Address to send fake UDP packets (Bewegungsfelder.exe)
        udp_target_address = ("127.0.0.1", 5555)

        # Create an instance of the UDPSenderProtocol for fake data
        udp_protocol = UDPSenderProtocol(udp_target_address)

        # Start the Twisted reactor
        reactor.listenUDP(0, udp_protocol)

        # Index
        i = 0
        global stop
        # While socket is open
        while not stop:
            
            # Index for data to send in list
            if i == len(data_list): 
                i = 0
            # Send to UDP server, with socket
            data = data_list[i]
            udp_protocol.sendData(data)

            # Wait 50ms
            time.sleep(0.06)  
            i += 1
        stop = False

    def startVizualisation(self): 

        # Update btns
        createWindow.fake_btn.config(state=tk.DISABLED)
        createWindow.start_btn.config(state=tk.DISABLED)
        createWindow.stop_btn.config(state=tk.NORMAL)

        # Create a separate thread for data sending
        self.send_data_thread = threading.Thread(target=self.sendDataThread)
        self.send_data_thread.start()

    # In your thread
    def sendDataThread(self):

        # Address to send UDP packets
        udp_target_address = ("127.0.0.1", 5555)

        # Create an instance of the UDPSenderProtocol
        udp_protocol = UDPSenderProtocol(udp_target_address)

        # Start the Twisted reactor
        reactor.listenUDP(0, udp_protocol)

        # Your data sending logic
        data_list = readFile.data_list
        # Initial time with first data timestamp
        first_timestamp = float(data_list[0].split(",")[-1])
        initial_time = first_timestamp / 1000.0  # Millisecondes to secondes

        # Rejouez les données avec des délais basés sur le temps du dernier élément
        for data in data_list:
            
            # Time stamp for same debit as the run
            timestamp_ms = float(data.split(",")[-1])  # Millisecondes time
            time_diff = (timestamp_ms - initial_time) / 1000.0  # Secondes to millisecondes 
            initial_time = timestamp_ms
            time.sleep(time_diff)  # Wait

            # Encoding format in bytes, for send to Bewe.. app
            data = data.split(",")  # Str to list
            integer_list = [int(string) for string in data] # List str to list int
            integer_list.pop() # Remove last element (timestamp for replay)
            data_format = 'l' * 12  
            udp_data = struct.pack(data_format, *integer_list)
            udp_protocol.sendData(udp_data)
            global stop
            if stop:
                stop = False
                break


    # Stop send data for vizu
    def stopVizualisation(self):
        global stop
        stop = True
        # Update btns
        createWindow.fake_btn.config(state=tk.NORMAL)
        createWindow.start_btn.config(state=tk.NORMAL)
        createWindow.stop_btn.config(state=tk.DISABLED)




class UDPSenderProtocol(DatagramProtocol):
    def __init__(self, udp_target_address):
        self.udp_target_address = udp_target_address

    def startProtocol(self):
        # Start sending data when the protocol is ready
        # You can add your data sending logic here
        pass

    def sendData(self, udp_data):
        # Send data to the UDP server
        self.transport.write(udp_data, self.udp_target_address)


class ReadFiles():

    local_list = []
    online_list = []
    data_list = []
    sensor_list = []
    fake_data_list = []

    def __init__(self):
        super().__init__()
        self.getLocalFiles()
        self.getOnlineFiles()

    def getLocalFiles(self):
        # Check nb of data file 
        folder_path = './datas'
        self.local_list = os.listdir(folder_path)


    def getOnlineFiles(self):
        try:
            # Requete to url + uri to get all files name
            uri = url+":3005/files"
            response = requests.get(uri, timeout=4)
            # If good request, set online_list with files name
            if response.status_code == 200:
                self.online_list = response.json()  # JSON to Python structur
            else:
                print("Error :", response.status_code)

        except Exception as e:
            print("Server not available")

        
    # Set data into list
    def analyseFile(self, file_path):
        # Init var
        self.data_list = []
        self.sensor_list = []
        # Read data of file
        with open(file_path, 'r') as fichier:
            data_list = fichier.read()
        # Split data
        data_list = data_list.split("]")
        # Add new data to list
        for element in data_list:
            self.data_list.append(element.replace('[','').replace('"',''))
        self.data_list.pop()
        self.checkSensorsNumber()

    ## 
    def checkSensorsNumber(self):

        for datas in self.data_list:
            # Get id of sensor for this data
            data = datas.split(',')[0]
            # Add to list of sensors if not added
            if data not in self.sensor_list:
                self.sensor_list.append(data)
                self.fake_data_list.append(datas)

class CreateWindows():

    window = None
    adding_list_local = None
    adding_list_online = None
    fake_btn = None
    start_btn = None
    stop_btn = None
    canvas = None
    angle = None
    new_url = None

    def __init__(self):
        super().__init__()

    
    # First window
    def CreateWindowChooseFile(self):
        # Try destroy, if second window is available
        if self.window != None:
            self.window.destroy()

        self.window = tk.Tk()
        self.window.geometry("")
        self.window.title("Select data file")
        # Direct link button
        direct_btn = tk.Button(self.window, text="Direct link", command=buttonsApp.directLink)
        direct_btn.pack()

        # LOCAL PART
        local_cadre = tk.Frame(self.window)
        local_cadre.pack(pady=10)  
        # Title
        text = tk.StringVar()
        text.set("Choose a LOCAL file for vizualisation")
        # Créez un widget Label pour afficher le texte dynamique
        tk.Label(local_cadre, textvariable=text).pack()
        # Add list of data files
        self.adding_list_local = tk.Listbox(local_cadre, selectmode=tk.SINGLE, width=40, height=10)
        for save in readFile.local_list:
            self.adding_list_local.insert(tk.END, save)
        self.adding_list_local.pack(padx=10, pady=10)
        # Button
        local_btn = tk.Button(local_cadre, text="Confirm", command=buttonsApp.confirmDataLocal)
        local_btn.pack(side="left", padx=30)
        local_btn_delete = tk.Button(local_cadre, text="Delete", command=buttonsApp.deleteDataLocal)
        local_btn_delete.pack(side="right", padx=30)

        # ONLINE PART
        online_cadre = tk.Frame(self.window)
        online_cadre.pack(pady=10) 
        # Title 
        text = tk.StringVar()
        text.set("Choose an ONLINE file for vizualisation")
        # Créez un widget Label pour afficher le texte dynamique
        tk.Label(online_cadre, textvariable=text).pack()
        # If data in list, show in app
        if len(readFile.online_list) > 0:
            # Add list of data files
            self.adding_list_online = tk.Listbox(online_cadre, selectmode=tk.SINGLE, width=40, height=10)
            for save in readFile.online_list:
                self.adding_list_online.insert(tk.END, save)
            self.adding_list_online.pack(padx=10, pady=10)
            # Buttons
            online_btn = tk.Button(online_cadre, text="Confirm", command=buttonsApp.confirmDataOnline)
            online_btn.pack(side="left", padx=30)
            online_btn_delete = tk.Button(online_cadre, text="Delete", command=buttonsApp.deleteDataOnline)
            online_btn_delete.pack(side="right", padx=30)
        # Else, show error message
        else:   
            text = tk.StringVar()
            text.set("Error server connection, no file online available.")
            # Créez un widget Label pour afficher le texte dynamique
            tk.Label(self.window, textvariable=text).pack(pady=5)




    def CreateWindowSendData(self, path_file):
        # Destroy first window
        self.window.destroy()
        # Create window
        self.window = tk.Tk()
        self.window.geometry("")
        self.window.title("Manage visualisation for " + path_file)
        # Buttons
        tk.Button(self.window, text="Back to list choice", command=buttonsApp.backToFirstWindow).grid(row=0, column=0, padx=5, pady=10)
        self.fake_btn = tk.Button(self.window, text="Init with fake data", command=buttonsApp.fakeData)
        self.fake_btn.grid(row=3, column=0, padx=5, pady=10)
        self.start_btn = tk.Button(self.window, text="Start", command=buttonsApp.startVizualisation)
        self.start_btn.grid(row=3, column=1, padx=5, pady=10)
        self.stop_btn = tk.Button(self.window, text="Stop", command=buttonsApp.stopVizualisation, state=tk.DISABLED)
        self.stop_btn.grid(row=3, column=2, padx=5, pady=10)
        # Start analyse of file
        readFile.analyseFile(path_file)
        # Add list id of sensors
        sensor_list = tk.StringVar()
        sensor_list.set("Id of sensors in data file : " + ", ".join(readFile.sensor_list))
        # Créez un widget Label pour afficher le texte dynamique
        tk.Label(self.window, textvariable=sensor_list).grid(row=2, column=0, padx=5, pady=10)


    def animate_loading_circle(self):
        try:
            self.canvas.delete("all")
            x, y = 50, 50
            radius = 20
            start_angle = self.angle
            end_angle = self.angle+180
            self.canvas.create_arc(x-radius, y-radius, x+radius, y+radius, start=start_angle, extent=90, outline="blue", width=3)
            self.canvas.create_arc(x-radius, y-radius, x+radius, y+radius, start=end_angle, extent=90, outline="blue", width=3)
            self.angle = (self.angle-5) % 360
            self.window.after(50, self.animate_loading_circle)
        except Exception as e:
            print(e)

    def createWindowDirectLink(self):
        # Destroy first window
        self.window.destroy()
        # Create window
        self.window = tk.Tk()
        self.window.geometry("")
        self.window.title("Link for direct vizualisation")
        # Buttons
        tk.Button(self.window, text="Back to list choice", command=buttonsApp.backToFirstWindow).pack()
        text = tk.StringVar()
        text.set("Direct link for live vizualisation")
        # Add text label
        tk.Label(self.window, textvariable=text).pack()
        # Cercle animation
        self.canvas = tk.Canvas(self.window, width=100, height=100)
        self.canvas.pack(padx=15, pady=2)
        self.angle = 0
        self.animate_loading_circle() 



if __name__ == "__main__":

    # If need to setup new server adress
    print("Actual server is : ", url)
    new_url = input("Press enter to let it, or entrer new adress : ")

    if new_url != '':
        url = new_url

    # Instance class of buttons
    buttonsApp = ButtonsApp()
    # Instance of window creations
    createWindow = CreateWindows()
    
    # Read data files, to get list of files
    readFile = ReadFiles()

    # Create window
    createWindow.CreateWindowChooseFile()
    window = createWindow.window

    # Open Window
    window.mainloop()
    

