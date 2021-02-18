#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// The Serial port connecting to the ESP
SoftwareSerial ESP_Serial(6,7); //Tx, Rx - Green, Yellow

// DHT Settings
#define DHTPIN     12
#define DHTTYPE    DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);

// Setup the application
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  ESP_Serial.begin(9600);
  bootstrapTemperature();
}


// Loop through all the threads (they are only blocking if they have work to do)
void loop() {
  serialForwardMessagesThread();  
  readTemperatureThread();
}


// This thread will manage the periodic update of the temperature
unsigned long nextTemperatureUpdate = 0;       // Set starting point (immediate)
const int minTempReadDelay = 5000;    // minimum refresh time
int tempReadDelay = 5000;             // sensor refresh time
void readTemperatureThread() {
    if (nextTemperatureUpdate < millis())
    {
      sensors_event_t event;
      dht.temperature().getEvent(&event);
      if (isnan(event.temperature)) {
        Serial.println(F("Error reading temperature!"));
      }
      else {
        serialReportTemperature(event.temperature);
      }
      int readDelay = tempReadDelay > minTempReadDelay ? tempReadDelay : minTempReadDelay;
      nextTemperatureUpdate = millis() + readDelay;
    }
}

// Writes the temperature to the ESP
void serialReportTemperature(float temperature) {
  Serial.print(F("Temperature: ")); Serial.print(temperature); Serial.println(F("°C")); // Write to host debug
  ESP_Serial.print("TEMPINCEL "); ESP_Serial.print(temperature); // Write to ESP
}



// Forwards all data received on the ESP_Serial bus to the Serial bus
void serialForwardMessagesThread() {
  if (ESP_Serial.available() > 0) {
    char c = ESP_Serial.read();
    while (c != -1) {
      Serial.write(c);
      c = ESP_Serial.read();
    }
  }
}


// Starts the Temperature sensor
void bootstrapTemperature(){
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  tempReadDelay = sensor.min_delay / 1000;
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Refresh ms:  ")); Serial.println(tempReadDelay);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
}
