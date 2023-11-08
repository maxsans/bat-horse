let startRecord = false;

function updateSensorStatusTable() {
  const sensorTableBody = document.querySelector("#sensorTable tbody");

  // Fetch sensor status data from the server
  fetch("/get_sensor_status")
    .then((response) => {
      if (response.ok) {
        return response.json();
      } else {
        throw new Error("Failed to fetch sensor list.");
      }
    })
    .then((sensorStatusList) => {
      sensorTableBody.innerHTML = "";
      sensorStatusList.forEach((sensor) => {
        const row = document.createElement("tr");
        const sensorIdCell = document.createElement("td");
        const statusCell = document.createElement("td");

        sensorIdCell.textContent = sensor.sensorID;
        statusCell.textContent = sensor.status;

        row.appendChild(sensorIdCell);
        row.appendChild(statusCell);
        sensorTableBody.appendChild(row);
      });
    })
    .catch((error) => {
      console.error("Error fetching sensor list:", error);
    });
}

function updateRecordingStatus() {
  fetch("/get_status_record")
    .then((response) => {
      if (response.ok) {
        return response.json();
      } else {
        throw new Error("Failed to fetch sensor list.");
      }
    })
    .then((data) => {
      const recodingElement  = document.getElementById("recordingStatus")
      const recordingStatus = data.record_status
      if (recordingStatus){
        recodingElement.textContent = "Recording in Progress...";
        const startRecodingElement = document.getElementById("startRecord")
        startRecodingElement.disabled  = true;
        const stopRecodingElement = document.getElementById("stopRecord")
        stopRecodingElement.disabled  = false;
      }else{
        recodingElement.textContent = ""  
        const startRecodingElement  = document.getElementById("startRecord")
        startRecodingElement.disabled = false;
        const stopRecodingElement  = document.getElementById("stopRecord")
        stopRecodingElement.disabled = true;
      }
    })
    .catch((error) => {
      console.error("Error fetching sensor list:", error);
    });
}

function updateCheckboxStatus() {

  fetch("/get_status_direct_retransmission")
    .then((response) => {
      if (response.ok) {
        return response.json();
      } else {
        throw new Error("Failed to fetch sensor list.");
      }
    })
    .then((data) => {
      const checkboxElement  = document.getElementById("statusCheckbox")
      const checkboxStatus = data.checkbox_status
      checkboxElement.checked = checkboxStatus;
    })
    .catch((error) => {
      console.error("Error fetching sensor list:", error);
    });
}


// Call the updateSensorStatusTable function on page load
window.addEventListener("load", () => {
  updateSensorStatusTable();
  updateCheckboxStatus();
  updateRecordingStatus();

  setInterval(updateCheckboxStatus, 2000);
  setInterval(updateRecordingStatus, 2000);
  setInterval(updateSensorStatusTable, 2000);
});



document.getElementById("statusCheckbox").addEventListener("click", function () {
  // Get the current status of the checkbox
  const status = statusCheckbox.checked;
  console.log(status);
  // Send the status to the server using fetch
  fetch("/update_status_direct_retransmission", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({ status: status }), // Send the status as JSON
  })
    .then((response) => {
      if (response.ok) {
      } else {
        console.error("Failed to update status.");
      }
    })
    .catch((error) => {
      console.error("Error updating status:", error);
    });
});


// Add event listeners for the buttons
document.getElementById("startRecord").addEventListener("click", () => {
  // Send a command to start recording
  fetch("/start_record", { method: "POST" })
    .then((response) => {
      if (!response.ok) {
        throw new Error("Failed to start recording.");
      }
      startRecord = true;
      const recordingStatusElement = document.getElementById("recordingStatus");
      recordingStatusElement.textContent = "Recording in Progress...";
      const startRecodingElement = document.getElementById("startRecord")
      startRecodingElement.disabled  = true;
      const stopRecodingElement = document.getElementById("stopRecord")
      stopRecodingElement.disabled  = false;
    })
    .catch((error) => {
      console.error("Error starting recording:", error);
    });
});

document.getElementById("stopRecord").addEventListener("click", () => {
  // Send a command to stop recording
  fetch("/stop_record", { method: "POST" })
    .then((response) => {
      if (!response.ok) {
        throw new Error("Failed to stop recording.");
      }
      if (startRecord){
        startRecord = false;
      }
      const recordingStatusElement = document.getElementById("recordingStatus");
      recordingStatusElement.textContent = "";
      const startRecodingElement = document.getElementById("startRecord")
      startRecodingElement.disabled  = false;
      const stopRecodingElement = document.getElementById("stopRecord")
      stopRecodingElement.disabled  = true;
    })
    .catch((error) => {
      console.error("Error stopping recording:", error);
    });
});

