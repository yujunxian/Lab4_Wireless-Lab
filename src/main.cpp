#include <Arduino.h>
#include <BLEDevice.h>
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
