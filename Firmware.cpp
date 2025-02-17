include "Particle.h"
 
// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_THREAD(ENABLED);
 
// BLE scanning variables
BleScanResult results[10];
const size_t SCAN_RESULTS_LIMIT = sizeof(results) / sizeof(results[0]);
const String TARGET_NAME = "BLEDeviceName"; // Name of the target device
const int RSSI_THRESHOLD = -60;     // Minimum signal strength to report
bool scanEventPublished = false;    // Flag to track if "BLE scanning started" event has been published
bool deviceFound = false;           // Flag to track if the target device has been found
 
// Function to retrieve device name from advertising data
String getDeviceName(BleAdvertisingData &advData) {
    uint8_t buffer[31];
    size_t len = advData.get(BleAdvertisingDataType::COMPLETE_LOCAL_NAME, buffer, sizeof(buffer));
    if (len == 0) {
        len = advData.get(BleAdvertisingDataType::SHORT_LOCAL_NAME, buffer, sizeof(buffer));
    }
 
    if (len > 0) {
        return String((char *)buffer, len); // Convert bytes to String
    }
 
    return "Unknown"; // Default if no name is found
}
 
// Function to extract and log additional advertising data
void extractAdvertisingData(BleAdvertisingData &advData) {
    uint8_t buffer[31];
 
    // Log 128-bit UUIDs
    size_t len = advData.get(BleAdvertisingDataType::SERVICE_DATA_128BIT_UUID, buffer, sizeof(buffer));
    if (len > 0) {
        for (size_t i = 0; i < len; i += 16) {
            BleUuid uuid(buffer + i, 16);
            Serial.printf("  UUID: %s\n", uuid.toString().c_str());
        }
    } else {
        Serial.println("  No 128-bit UUIDs found.");
    }
 
    // Log Major and Minor values if available (specific to beacon formats like iBeacon)
    len = advData.get(BleAdvertisingDataType::MANUFACTURER_SPECIFIC_DATA, buffer, sizeof(buffer));
    if (len >= 4) {
        uint16_t major = (buffer[len - 4] << 8) | buffer[len - 3];
        uint16_t minor = (buffer[len - 2] << 8) | buffer[len - 1];
        Serial.printf("  Major: %d, Minor: %d\n", major, minor);
    } else {
        Serial.println("  No Major/Minor values found.");
    }
}
 
void setup() {
    Particle.connect();
    BLE.on();
    Serial.begin(9600);
 
    // Header for Serial Monitor
    Serial.println("\n=========================================");
    Serial.println("   Particle BLE Device Scanner");
    Serial.println("=========================================\n");
    Log.info("Setup complete, BLE initialized...");
}
 
void loop() {
    // Ensure the device is connected to the cloud
    if (!Particle.connected()) {
        Particle.connect();
        delay(5000);
        return;
    }
 
    // Publish scanning start event only once
    if (!scanEventPublished) {
        Particle.publish("ble_scanning", "Started BLE scanning...", PRIVATE);
        scanEventPublished = true; // Set the flag to prevent future publishing
    }
 
    // Stop scanning if device already found
    if (deviceFound) {
        return; // Exit loop and stop further processing
    }
 
    // Start scanning
    Serial.println("------------------------------------------------");
    Serial.println("Scanning for BLE devices...");
    Serial.println("------------------------------------------------");
 
    // Print table header
    Serial.printf("%-20s %-10s %-20s\n", "Address", "RSSI", "Name");
    Serial.printf("%-20s %-10s %-20s\n", "--------------------", "----------", "--------------------");
 
    // Perform BLE scan
    int count = BLE.scan(results, SCAN_RESULTS_LIMIT);
    Serial.printf("\nScan complete. Found %d devices.\n", count);
 
    for (int i = 0; i < count; i++) {
        BleScanResult result = results[i];
        BleAdvertisingData advData = result.advertisingData();
 
        String address = result.address().toString();
        int rssi = result.rssi();
        String deviceName = getDeviceName(advData);
 
        // Display basic device info in a table row
        Serial.printf("%-20s %-10d %-20s\n", address.c_str(), rssi, deviceName.c_str());
 
        // Check name and RSSI
        if (deviceName == TARGET_NAME && rssi > RSSI_THRESHOLD) {
            Serial.println("\n  >>> Match Found! <<<");
            Serial.printf("  Address: %s, RSSI: %d, Name: %s\n", address.c_str(), rssi, deviceName.c_str());
            Particle.publish("ble_device_found", "Name: " + deviceName + ", RSSI: " + String(rssi), PRIVATE);
 
            // Extract additional advertising data
            Serial.println("  Additional Advertising Data:");
            extractAdvertisingData(advData);
 
            Serial.println("\nStopping scan and serial monitoring...");
            BLE.stopScanning();
            Particle.publish("ble_scanning", "Device found, stopping scan", PRIVATE);
 
            deviceFound = true; // Set the flag to prevent further processing
            delay(1000); // Allow events to propagate
            Serial.end(); // Stop Serial Monitoring
            return; // Exit loop
        }
    }
 
    Serial.println("\nNo matching devices found in this scan.\n");
    delay(1000); // Short delay for optimized scan frequency
}
