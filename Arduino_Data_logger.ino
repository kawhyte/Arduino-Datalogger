/*
  SD card Temp & Humidity datalogger


 It also uses the Adafruit DHT sensor library from
 https://github.com/adafruit/DHT-sensor-library

 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 * DHT11 sensor connected as follows:
 ** Ground connected to 5
 ** Voltage connected to 8
 ** data out connected to 7
 ** 10K resistor connected from 7 to +5V

 edited  10 april 2014

 This example code is in the public domain, subject to the licenses
 of the libraries used.

 */
#include <SPI.h>
#include <Ethernet.h>

#include "plotly_ethernet.h"

#include <SD.h>
#include "DHT.h"

#define DHTPIN 2        // what pin the sensor is connected to
#define DHTTYPE DHT22   // Which type of DHT sensor you're using:


long id = 1;                //Use this to store the id # of our reading.

int soil= 0; // For Soil Moisture sensor

// initialize the sensor:
DHT dht(DHTPIN, DHTTYPE);
//const int interval = 1000; // the interval between reads, in ms
//long lastReadTime = 0;        // the last time you read the sensor, in ms
plotly plotly; // initialize a plotly object, named plotly

//initialize plotly global variables
char layout[]="{}";
char filename[] = "Plant Datalogger System4"; // name of the plot that will be saved in your plotly account -- resaving to the same filename will simply extend the existing traces with new data

// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const int chipSelect = 4;

/// Added data ////
//PhotoResistor Pin
int lightPin = 3; //the analog pin the photoresistor is
                  //connected to
                  //the photoresistor is not calibrated to any units so
                  //this is simply a raw sensor value (relative light)
//LED Pin
int ledPin = 9;   //the pin the LED is connected to
                  //we are controlling brightness so
                  //we use one of the PWM (pulse width
                  // modulation pins)

// Ethernet Setup
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xDD, 0x9A };

byte my_ip[] = { 192,168,0,102 };

void startEthernet(){
    Serial.println("Initializing ethernet");
    if(Ethernet.begin(mac) == 0){
        Serial.println("Failed to configure Ethernet using DHCP");
        // no point in carrying on, so do nothing forevermore:
        // try to congifure using IP address instead of DHCP:
        Ethernet.begin(mac, my_ip);
    }
    Serial.println("Done initializing ethernet");
    delay(1000);
}


void setup() {

  pinMode(ledPin, OUTPUT); //sets the led pin to output///// added
  dht.begin();
  Serial.begin(9600);

  //boolean result = false;
  Serial.print("Initializing Micro SD Card");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(53, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card Failure or not present");
    // don't do anything more:
    return;
  }
  else {

    //Write Header to file
    Serial.println("Card Ready");
    File dataFile = SD.open("datalog2.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.println(", , , ,"); //Just a leading blank line, incase there was previous data
      String header = "Log ID, Humidity, Temp(c), Temp(f), Light, Soil Moisture(%)";
      dataFile.println(header);

      //dataFile.println("rH (%) \t temp. (*C)");
      dataFile.close();
      Serial.println (header);
      //result = true;
    }
  }
startEthernet();    // initialize ethernet

  // Initialize plotly settings
  plotly.VERBOSE = true; // turn to false to suppress printing over serial
  plotly.DRY_RUN = false; // turn to false when you want to connect to plotly's servers
  plotly.username = "k**$^^"; // your plotly username -- sign up at https://plot.ly/ssu or feel free to use this public account. password of the account is "password"
  plotly.api_key = "######"; // "public_arduino"'s api_key -- char api_key[10]
  plotly.timestamp = true; // tell plotly that you're stamping your data with a millisecond counter and that you want plotly to convert it into a date-formatted graph
  plotly.timezone = "US/Eastern"; // full list of timezones is here:

}

void loop()
{

  int sensorValue = analogRead(A2);
  sensorValue = constrain(sensorValue, 485, 1023);
  // print out the value you read:
  //Serial.println(sensorValue);

  //map the value to a percentage
  soil = map(sensorValue, 485, 1023, 100, 0);

  // print out the soil water percentage you calculated:
  Serial.print(soil);
  Serial.println("%");
 // delay(1000);        // delay in between reads for stability

    int humidity = dht.readHumidity();
    int temperature = dht.readTemperature();


 int lightLevel = analogRead(lightPin); //Read the
                                        // lightlevel
 lightLevel = map(lightLevel, 1023, 0, 0, 179);
         //adjust the value 0 to 900 to
         //span 0 to 255

lightLevel = constrain(lightLevel, 0, 255);//make sure the
                                           //value is betwween
                                           //0 and 255
 analogWrite(ledPin, lightLevel);  //write the value for the LED PWM value

 int f = (((temperature*9)/5) +32);// convert temperature to (F)

  //Create Data string for storing to SD card
  //We will use CSV Format
      String dataString = String(id) + ", " + String(humidity) + ", " + String(temperature) + ", " + String(f) + ", " + String(lightLevel)+ ", " + String(soil);

    // open the file:
    File dataFile = SD.open("datalog2.csv", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {

      dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString);
      // Open the Stream
      plotly.open_stream(1, 4, filename, layout); // plotlystream(number_of_points, number_of_traces, filename, layout)

      plotly.post(millis(),humidity); // post temperature to plotly (trace 1)
      delay(150);
      //plotly.post(millis(),temperature); // post humidity to plotly (trace 2)
      //delay(150);//added
      plotly.post(millis(), f); // added
      delay(150);//added
      plotly.post(millis(), lightLevel); // added
      delay(150);
      plotly.post(millis(), soil); // added// post soil mositure

      for(int i=0; i<70; i++){ //number of seconds to wait before posting again (300 = 5 minutes)
      delay(1000);

      }

}
    // if the file isn't open, pop up an error:
    else {
      Serial.println("Error opening Log File");
    }
 //Increment ID number
  id++;
  delay (1000);
  }
//}
