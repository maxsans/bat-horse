from twisted.internet import reactor, protocol, endpoints
from twisted.web import server, resource, static, http
from twisted.web.server import Site
from twisted.web.static import File
import struct
import json
from datetime import datetime
import subprocess
import os
import time
import shutil
# import pyudev




record_data = False
saver = None
direct_retransmission = False
started_timestamp = 0
record_file_name = ""
record_file_path = '' 


class SaveData():


    def __init__(self):
        """Create new file to save data, with today date and hour"""
        now = datetime.now()
        file_name = (str(now.year) + "-" + str(now.month if now.month > 9 else '0' + str(now.month)) + "-" + str(now.day if now.day > 9 else '0' + str(now.day)) + "_" + str(now.hour) + "-" + str(now.minute if now.minute > 9 else '0' + str(now.minute)) +"_name")
        global record_file_name
        record_file_name = file_name
        # Path JSON file
        json_file_path = "./datas/" + file_name
        global record_file_path
        record_file_path = json_file_path
        global started_timestamp
        started_timestamp = int(time.time() * 1000)


    def addData(self, data):
        global record_file_path
        with open(record_file_path, "a") as json_file:
            json.dump(json.dumps(data), json_file, indent=0)


def copy_to_usb():
    global record_file_path
    usb_folder_path = "/media/pi/A8BD-A7E5/Mocap/data"

    # record_file_name = os.path.basename(record_file_path)

    # Create a udev context
    # context = pyudev.Context()

    # # Search for USB storage devices
    # for device in context.list_devices(subsystem='block', DEVTYPE='disk'):
    #     if 'ID_BUS' in device.properties and device.properties['ID_BUS'] == 'usb':
    #         # Check if there is a mount point
    #         if 'ID_FS_LABEL' in device.properties:
    #             usb_folder_path = os.path.join('/media/pi/', device.properties['ID_FS_LABEL'], 'Mocap/data')
    #             usb_file_path = os.path.join(usb_folder_path, record_file_name)

    #             # Copy the file from the source to the USB drive
    #             shutil.copy(record_file_path, usb_file_path)

    #             # Display a message to confirm the copy
    #             print(f"File copied to USB drive: {usb_file_path}")
    #             return

    # print("No USB drive detected.") 
     #check if the file exist

    if not os.path.isfile(record_file_path):
        return

    #check if the folder exist
    os.makedirs(usb_folder_path, exist_ok=True)



    global record_file_name
    usb_file_name = record_file_name

    # Chemin complet du fichier sur la clé USB
    usb_file_path = os.path.join(usb_folder_path, usb_file_name)

    # Copiez le fichier depuis la source vers la clé USB
    shutil.copy(record_file_path, usb_file_path)

    # Affichez un message pour confirmer la copie
    print(f"Fichier copié sur la clé USB : {usb_file_path}")     

class UDPServer(protocol.DatagramProtocol):

    diff_timestamp = 0
    last_timestamp = 0
    last_timestamps = {}

    # def __init__(self, factory, saver):
    def __init__(self):
        super().__init__()
        # self.factory = factory
        for id in range(1, 17):
            self.last_timestamps[id] = {
                "diff": 0,
                "last_timestamp": 0,
                "hour_timestamp": 0,
            }

    def datagramReceived(self, data, addr):
        
        assert(type(data) == bytes)
        print (data)
        data_format = 'l' * 12 
        decoded_data = struct.unpack(data_format, data)
        json_data = json.dumps({"sensorID": decoded_data[0], "quat": decoded_data[1:5], "accel": decoded_data[5:8], "gyro": decoded_data[8:11], "timestamp": decoded_data[11]})

        # Tabs timestamp all sensors : id, last_timestamp, diff
        self.last_timestamps[decoded_data[0]]["diff"] = decoded_data[11] - self.last_timestamps[decoded_data[0]]["last_timestamp"]
        self.last_timestamps[decoded_data[0]]["last_timestamp"] = decoded_data[11]
        self.last_timestamps[decoded_data[0]]["hour_timestamp"] = int(datetime.now().timestamp())

        for id, data in self.last_timestamps.items():
            if(int(datetime.now().timestamp()) - data["hour_timestamp"]) > 2:
                data["diff"] = 0


        # # Déplacez le curseur au début de la ligne
        # sys.stdout.write("\r")

        # # Effacez la ligne en cours (peut ne pas fonctionner sur tous les terminaux)
        # sys.stdout.write("\033[K")

        # # Affichez la ligne mise à jour
        # line = " | ".join([f"{id} : {item['diff']}" for id, item in self.last_timestamps.items()])
        # sys.stdout.write(line)
        # sys.stdout.flush()        # Send to front with WebSockets in Bytes

        # self.factory.protocol.sendMessage(self=websocket_instance, payload=byte_data, isBinary=False)

        global record_data, saver, direct_retransmission, started_timestamp
        # Save data
        if record_data:
            current_timestamp = int(time.time() * 1000) - started_timestamp
            list_data = list(decoded_data)  # Convert to a list
            list_data.append(current_timestamp)
            saver.addData(list_data)

        if direct_retransmission:
            udp_data = struct.pack(data_format, *decoded_data)
            self.transport.write(udp_data, ("127.0.0.1", 5555))


class FileListResource(resource.Resource):
    isLeaf = True

    def render_GET(self, request):
        data_dir = os.path.join(os.path.dirname(__file__), "datas")
        file_list = os.listdir(data_dir)
        return json.dumps(file_list).encode("utf-8")


class UpdateStatusDirectRetransmissionResource(resource.Resource):
    isLeaf = True

    def render_POST(self, request):
        try:
            data = json.loads(request.content.read().decode())
            checkbox_status = data.get("status")
            global direct_retransmission
            direct_retransmission = checkbox_status
            request.setResponseCode(http.OK)
            return b"Status updated successfully."
        except Exception as e:
            print("Error updating status:", str(e))
            request.setResponseCode(http.BAD_REQUEST)
            return b"Error updating status."

class StartRecordResource(resource.Resource):
    isLeaf = True

    def render_POST(self, request):

        global record_data
        global saver

        if not record_data:
            saver = SaveData()
            record_data = True
            request.setResponseCode(http.OK)
            return b"Recording started"

        else:
            request.setResponseCode(http.NOT_FOUND)
            return b"Already in recording"


class StopRecordResource(resource.Resource):
    isLeaf = True

    def render_POST(self, request):

        global record_data
        record_data = False
        copy_to_usb()
        request.setResponseCode(http.OK)

        return b"Recording stopped"


class GetStatusRecordResource(resource.Resource):
    isLeaf = True

    def render_GET(self, request):
        global record_data
        response_data = {"record_status": record_data}
        response_json = json.dumps(response_data).encode("utf-8")        
        request.setResponseCode(http.OK)
        request.setHeader("Content-Type", "application/json")
        return response_json


class GetSensorStatusResource(resource.Resource):
    isLeaf = True

    def render_GET(self, request):
        current_timestamp = int(datetime.now().timestamp())
        sensor_status = []

        for sensor_id, timestamp_data in udp_server.last_timestamps.items():
            if (current_timestamp - timestamp_data["hour_timestamp"]) <= 5:
                status = "Connected"
            else:
                status = "Disconnected"
            sensor_status.append({"sensorID": sensor_id, "status": status})

        request.setResponseCode(http.OK)
        return json.dumps(sensor_status).encode("utf-8")
    
class GetStatusDirectRetransmissionResource(resource.Resource):
    isLeaf = True

    def render_GET(self, request):
        global direct_retransmission
        response_data = {"checkbox_status": direct_retransmission}
        response_json = json.dumps(response_data).encode("utf-8")        
        request.setResponseCode(http.OK)
        request.setHeader("Content-Type", "application/json")
        return response_json


 
class OpenApplicationResource(resource.Resource):
    isLeaf = True

    def render_POST(self, request):
        try:
            subprocess.run(["runas", "/user:Administrator", "./bewegungsfelder_v1.1.0/Bewegungsfelder.exe"])
            request.setResponseCode(http.OK)
            return b"Application Open"

        except subprocess.CalledProcessError as e:
            print(f"Erreur : {e}")
            return b"Failied to open the application"




if __name__ == "__main__":



    udp_server = UDPServer()
    reactor.listenUDP(5555, udp_server)

    root = static.File("tmp/.")
    
    root.putChild(b"start_record", StartRecordResource())
    root.putChild(b"stop_record", StopRecordResource())
    root.putChild(b"get_status_record", GetStatusRecordResource()) 

    root.putChild(b"get_sensor_status", GetSensorStatusResource())

    root.putChild(b"update_status_direct_retransmission", UpdateStatusDirectRetransmissionResource()) 
    root.putChild(b"get_status_direct_retransmission", GetStatusDirectRetransmissionResource())


    site = server.Site(root)
    endpoint = endpoints.TCP4ServerEndpoint(reactor, 8080)

    endpoint.listen(site)   

    reactor.run()


    
