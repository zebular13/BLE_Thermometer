#include "CurieBLE.h"

BLEPeripheral blePeripheral;   // BLE Peripheral Device (the board you're programming)
BLEService TempService("1809"); // Temperature Service - it doesn't matter what you call it as long as you have the correct UUID

// BLE temperature Measurement Characteristic"
BLECharacteristic TempChar("2A1C",  // standard 16-bit characteristic UUID
    BLERead | BLEIndicate, 2);  // remote clients will be able to get notifications if this characteristic changes
                              // the characteristic is 2 bytes long as the first field needs to be "Flags"
                              // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.temperature_measurement.xml
// Define the pin to which the temperature sensor is connected.
const int pinTemp = A0;

// Define the B-value of the thermistor.
// This value is a property of the thermistor used in the Grove - Temperature Sensor,
// and used to convert from the analog value it measures and a temperature value.
const int B = 3975;

long previousMillis = 0;  // last time the temperature was checked, in ms

void setup() {
  Serial.begin(9600);    // initialize serial communication
  pinMode(13, OUTPUT);   // initialize the LED on pin 13 to indicate when a central is connected

  /* Set a local name for the BLE device
     This name will appear in advertising packets
     and can be used by remote devices to identify this BLE device
     The name can be changed but maybe be truncated based on space left in advertisement packet */
  blePeripheral.setLocalName("TempSketch");
  blePeripheral.setAdvertisedServiceUuid(TempService.uuid());  // add the service UUID
  blePeripheral.addAttribute(TempService);   // Add the BLE temperatureservice
  blePeripheral.addAttribute(TempChar); // add the RSC Measurement characteristic

  /* Now activate the BLE device.  It will start continuously transmitting BLE
     advertising packets and will be visible to remote BLE central devices
     until it receives a new connection */
  blePeripheral.begin();
  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {

     // Configure the serial communication line at 9600 baud (bits per second.)
    Serial.begin(9600);
    
  // listen for BLE peripherals to connect:
  BLECentral central = blePeripheral.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());
    // turn on the LED to indicate the connection:
    digitalWrite(13, HIGH);

    

    // check the temperature every 200ms
    // as long as the central is still connected:
    while (central.connected()) {
      long currentMillis = millis();
      // if 200ms have passed, check the temperaturemeasurement:
      if (currentMillis - previousMillis >= 200) {
        previousMillis = currentMillis;
        updateTemperature();
      }
    }
    // when the central disconnects, turn off the LED:
    digitalWrite(13, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

void updateTemperature() {
  /* Read the current voltage level on the A0 analog input pin.
     This is used here to simulate the heart rate's measurement.
  */
  // Get the (raw) value of the temperature sensor.
    int val = analogRead(pinTemp);

    // Determine the current resistance of the thermistor based on the sensor value.
    float resistance = (float)(1023-val)*10000/val;

    // Calculate the temperature based on the resistance value.
    float temperature = 1/(log(resistance/10000)/B+1/298.15)-273.15;
    
 
    Serial.print("Temperature is now: "); // print it
    Serial.println(temperature);
    
    const unsigned char temperatureCharArray[2] = { 63, (char)temperature};
    TempChar.setValue(temperatureCharArray, 2);  // and update the heart rate measurement characteristic
    delay(1000);
}
