#include <Arduino.h>
#include <BLEDevice.h>
<<<<<<< HEAD
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <stdlib.h> 

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 1000;
int iLoop=0;
const int ledPin =  D10;     

const int trigPin = 3;  // GPIO pin for Trig
const int echoPin = 4;  // GPIO pin for Echo
 
// TODO: add new global variables for your sensor readings and processed data

// TODO: Change the UUID to your own (any specific one works, but make sure they're different from others'). You can generate one here: https://www.uuidgenerator.net/

#define SERVICE_UUID        "2c4133b0-1d80-4a87-966b-e71e4cc3a577"
#define CHARACTERISTIC_UUID "b1c1ee71-2ecd-4a6c-9159-59058addefcc"


float getDistance(int n,int issendRaw) {
  float distance=0;
  float d[6];
  for(int i=0;i<6 && i<n;i++){
	  // Send a 10µs pulse to the Trig pin  
	  digitalWrite(trigPin, LOW);
	  delayMicroseconds(2);
	  digitalWrite(trigPin, HIGH);
	  delayMicroseconds(10);
	  digitalWrite(trigPin, LOW);
	  
	  // Measure the duration of the pulse on the Echo pin
	  long duration = pulseIn(echoPin, HIGH);

	  // Calculate distance in centimeters
	  float d = duration * 0.034 / 2;
	  if(issendRaw){
		  Serial.print("raw Distance: ");
		  Serial.print(d);
		  Serial.println(" cm");
	  }
	  distance+=d;
	 
	  // Wait before the next measurement
	  delay(100);
  }
   // Print the distance to the Serial Monitor
  Serial.print("avg Distance: ");
  distance=distance/n;
  Serial.print(distance);
  Serial.println(" cm");
  return distance;
}


class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
		Serial.println("Connected");
        deviceConnected = true;        
    };

    void onDisconnect(BLEServer* pServer) {
		Serial.println("DisConnected");
        deviceConnected = false;
    }
};

// TODO: add DSP algorithm functions here

void setup() {
    Serial.begin(115200);
	pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    Serial.println("Starting BLE work!");

    // TODO: add codes for handling your sensor setup (pinMode, etc.)

    // TODO: name your device to avoid conflictions
    BLEDevice::init("XIAO_ESP32C3_YUJUNXIAN");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );  
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setValue("Hello World");
    pService->start();
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
    // TODO: add codes for handling your sensor readings (analogRead, etc.)

    // TODO: use your defined DSP algorithm to process the readings
	float dis=getDistance(5,1);
    iLoop++;
    int buttonState = iLoop%20;
    if (buttonState == HIGH) {
		  char buf[10];
		  memset(buf,0,10);
		  itoa(iLoop,buf,10);
		  String s(buf);
		  Serial.print("Server high lit:");
		  Serial.println(s);
    }
    if (deviceConnected) {
        // Send new readings to database
        // TODO: change the following code to send your own readings and processed data
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
		char buf[10];
		memset(buf,0,10);
		if(dis<30){
			String s2("Get distance:");
			{
			int d=(int)(dis);
			itoa(d,buf,10);
			String s(buf);			
			s2=s2+s;
			}
			{
			int d=(int)(100*dis);
			d=d%100;
			itoa(d,buf,10);
			String s(buf);
			s2=s2+".";
			s2=s2+s;
			}			
			pCharacteristic->setValue(s2);
			pCharacteristic->notify();
			Serial.print("Notify value:");
			Serial.println(s2);
		}
        }
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising();  // advertise again
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    delay(1000);
}
=======
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
// Client Code
#include "BLEDevice.h"
//#include "BLEScan.h"

// TODO: change the service UUID to the one you are using on the server side.
// The remote service we wish to connect to.
static BLEUUID serviceUUID("2c4133b0-1d80-4a87-966b-e71e4cc3a577");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("b1c1ee71-2ecd-4a6c-9159-59058addefcc");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

// TODO: define new global variables for data collection
int iFoundNumber=0;
int iLoop=0;
int DODEBUG=1;
const int ledPin =  D10;  
float maxdis=-1;
float mindis=-1;
float curdis;
// TODO: define a new function for data aggregation
float getDistance(uint8_t* buf,int length);

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    // TODO: add codes to handle the data received from the server, and call the data aggregation function to process the data
	char buf[128];
    // TODO: change the following code to customize your own data format for printing
	if(DODEBUG){
		Serial.print("Notify callback for characteristic From");
		Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
		Serial.print(" of data length ");
		Serial.println(length);
		Serial.print("data: ");
		Serial.write(pData, length);
		Serial.println();
	}
	curdis=getDistance(pData, length);
	if(curdis>0){
		if(curdis<mindis || mindis<0){
			mindis=curdis;
		}
		if(curdis>maxdis || maxdis<0){
			maxdis=curdis;
		}
	}
	
	{
	std::string sdata = std::to_string(curdis);
	Serial.print(" current distance:");
	Serial.print(sdata.c_str());
	}
	{
	std::string sdata = std::to_string(maxdis);
	Serial.print(" maximum distance:");
	Serial.print(sdata.c_str());
	}
	{
	std::string sdata = std::to_string(mindis);
	Serial.print(" minimum distance:");
	Serial.print(sdata.c_str());
	}
}
float getDistance(uint8_t* buf,int length){
	int i=0;
	int j=0;
	char sbuf[128];
	float num_float=-1;
	memset(sbuf,0,128);
	while(buf[i]!=':') i++;
	i++;
	while(i<length){
		sbuf[j]=buf[i];
		i++;
		j++;
	}	
	if(j>0){
		std::string s(sbuf);	
		num_float = std::stof(s);		
	}
	return num_float;
	
}
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
//    Serial.println(myDevice->getAddress().toString().c_str());
    Serial.print(myDevice->getAddress().toString().c_str());
    Serial.print(" ");
    Serial.println(myDevice->getName());

    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      //std::string 
      String value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());
    iFoundNumber++;
    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks

void setup() {

  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("XIAO_ESP32C3_YUJUNXIAN");
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.

// This is the Arduino main loop function.
void loop() {
  iLoop++;
  int buttonState = iLoop%10;

  if (buttonState == HIGH) {
      char buf[100];
      memset(buf,0,100);
      itoa(iFoundNumber,buf,10);
      String s(buf);
      Serial.print("Number of Bluetooth Devices Detected:");
      Serial.println(s);
  }

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server: XIAO_ESP32C3_YUJUNXIAN.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
 
  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    String newValue = "Time since boot: " + String(millis()/1000);
    Serial.println("Setting new characteristic value to \"" + newValue  + "\"");

    // Set the characteristic's value to be the array of bytes that is actually a string.
    pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  }

  delay(1000); // Delay a second between loops.
} // End of loop
>>>>>>> 4db5e48 (Add TECHIN514_Lab4_Client)
