# BLEDeviceDetector
This project contains C++ firmware for flashing a Particle Boron development board to scan for BLE devices nearby. The firmware detects a specific Bluetooth device based on signal strength (RSSI) and name, then publishes real-time event data to the Particle Cloud.

🔧 Features:
- ✅ BLE Scanning – Scans for nearby BLE devices, logs details, and filters based on device name & RSSI
- ✅ Particle Cloud Integration – Publishes events when a device is detected
- ✅ Signal Strength Filtering – Only detects devices within a specified RSSI threshold
- ✅ Advertising Data Extraction – Retrieves UUIDs, Major/Minor values, and manufacturer data
- ✅ Automatic Scan Stop – Stops scanning once the target device is found

📂 Technologies Used:
- C++ (Particle Boron firmware development)
- Particle Cloud API (Cloud event publishing)
- BLE (Bluetooth Low Energy) (Device scanning & data extraction)
- Serial Communication (Logging data via Serial Monitor)
