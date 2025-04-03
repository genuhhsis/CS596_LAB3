/* main.cpp
 *
 * Genesis Anne Villar (RED ID: 824435476)
 * Steven Gervacio (RedID: 825656527)
 * CS 596 IOT - Prof. Donyanavard
 * Due Date: 4/2/2025
 *
 * File Description:
 * System which records data from temperature and humidity sensors and 
 * sends the collected data to a web server and prints them on the console on the cloud side (AWS)
 * 
 * Much of the following code was from given template that we modified!
*/

#include <WiFi.h>
#include <HttpClient.h>
#include <Wire.h>
#include <DHT20.h>
#include <TFT_eSPI.h>
#include <SPI.h>        

TFT_eSPI tft = TFT_eSPI(); // initialize TFT display
DHT20 dht; //DHT20 sensor setup (temp + humidity)
// does not need pins to be initialized since im using default ESP32 pin 21 & 22

// This example downloads the URL "http://arduino.cc/"

//my wifi information.. 
char ssid[] = "SETUP-2F8A";    // your network SSID (name) 
char pass[] = "fifty4884almost"; // your network password (use for WPA, or use as key for WEP)

// Name of the server we want to connect to
const char kHostname[] = "3.138.34.177";

int port = 5000; //need this to include port when connecting

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

// for tft countdown timer
const int readingInterval = 10000; // 10 seconds between readings
unsigned long lastReadingTime = 0;
void setup() 
{
  Serial.begin(9600);
  Serial.println(__FILE__);
  Serial.print("DHT20 LIBRARY VERSION: ");
  Serial.println(DHT20_LIB_VERSION);
  Serial.println();

  // initialize TFT display
  tft.init();
  tft.setRotation(1); // Landscape orientation
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.println("Initializing...");
  delay(1000); // time to see the message

  // We start by connecting to a WiFi network
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  tft.println("Connecting to WiFi");
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
  tft.println("\nWiFi connected!");
  tft.println(WiFi.localIP());
  delay(2000); // time to see connection information

  Wire.begin();
  delay(100);
  
  tft.println("Initializing sensor...");
  if(!dht.begin())
  {
    Serial.println("DHT20 sensor was not found.");
    tft.println("\nDHT20 SENSOR NOT FOUND!");
    tft.println("Check wiring and restart");
    while(1) delay(10);
  }
  
  tft.println("Sensor ready!");
  delay(1000);
  tft.fillScreen(TFT_BLACK);

  dht.begin(); //if successfully starts

  // add headers to the display
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(10, 10);
  tft.println("Temp. & Humidity");

  lastReadingTime = millis(); //set initial reading time
}


void sendDataToCloud(float temperature, float humidity)
{
  int err = 0;
  
  WiFiClient c;
  HttpClient http(c);

  String kpath = String("/") + "?temp=" + String(temperature) + "&humid=" + String(humidity); //request path w/ sensor values
  Serial.print("Request path: ");
  Serial.println(kpath);

  // show sending status on display
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, 180);
  tft.print("Sending data to cloud...");
  
  err = http.get(kHostname, port, kpath.c_str());
  
  if (err == 0)
  {
    Serial.println("startedRequest ok");
    tft.fillRect(10, 180, tft.width(), 10, TFT_BLACK); //display on tft that request was successful
    tft.setCursor(10, 180);
    tft.print("Request sent successfully");

    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Got status code: ");
      Serial.println(err);

      // Usually you'd check that the response code is 200 or a
      // similar "success" code (200-299) before carrying on,
      // but we'll print out whatever response we get

      err = http.skipResponseHeaders();


      if (err >= 0)
      {
        int bodyLen = http.contentLength();
        Serial.print("Content length is: ");
        Serial.println(bodyLen);
        Serial.println();
        Serial.println("Body returned follows:");
      
        // Now we've got to the body, so we can print it out
        unsigned long timeoutStart = millis();
        char c;
        // Whilst we haven't timed out & haven't reached the end of the body
        while ( (http.connected() || http.available()) &&
               ((millis() - timeoutStart) < kNetworkTimeout) )
        {
            if (http.available())
            {
                c = http.read();
                // Print out this character
                Serial.print(c);
               
                bodyLen--;
                // We read something, reset the timeout counter
                timeoutStart = millis();
            }
            else
            {
                // We haven't got any data, so let's pause to allow some to
                // arrive
                delay(kNetworkDelay);
            }
        }
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    }
    else
    {    
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(err);
  }

  http.stop();
}

void updateDisplay(float temperature, float humidity, int secondsToNext) 
{
  tft.fillRect(0, 30, tft.width(), 140, TFT_BLACK); // clear the entire data area to ensure clean updates
  
  // display temperature with label
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 30);
  tft.print("Temp: ");
  
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(85, 30);
  tft.print(temperature, 1);
  tft.println(" C");
  
  // display humidity with label
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 70);
  tft.print("Humid: ");
  
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(85, 70);
  tft.print(humidity, 1);
  tft.println(" %");
  
  // display countdown timer
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 110);
  tft.print("Next in:");
  
  // update the seconds part with better spacing
  tft.setCursor(110, 110); // Increased spacing between "Next in:" and the number
  tft.print(secondsToNext);
  tft.print("s   ");
}

void loop()
{
   // calculate time remaining until next reading
   unsigned long currentTime = millis();
   unsigned long elapsedTime = currentTime - lastReadingTime;
   int secondsToNext = max(0, (int)((readingInterval - elapsedTime) / 1000));
   
   if (elapsedTime >= readingInterval) // if it's time for a new reading
   {
    float temperature = 0.0;
    float humidity = 0.0;
    
    // display reading message
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 180);
    tft.print("Reading sensor...");
       
    int status = dht.readStatus(); //check if sensor is properly calibrate
    if (status & 0x08) // check if bit 3 is set (sensor is calibrated)
    { 
      int result = dht.read(); // read the sensor data
      if (result == 0) //if dht20 is valid
      {
        temperature = dht.getTemperature();
        humidity = dht.getHumidity();
      
        // print temp + humidity values to serial
        Serial.print("Temperature: "); 
        Serial.print(temperature);
        Serial.println(" °C");
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println(" %");
        
        updateDisplay(temperature, humidity, 10); // update display with new values
        sendDataToCloud(temperature, humidity); // send data to cloud
      } 
      else 
      {
        Serial.print("Error reading sensor: ");
        Serial.println(result);
        tft.fillRect(10, 180, tft.width(), 10, TFT_BLACK);
        tft.setCursor(10, 180);
        tft.print("Sensor read error: ");
        tft.print(result);
      }
    } 
    else 
    {
      Serial.println("Sensor not calibrated or not connected properly");
      tft.fillRect(10, 180, tft.width(), 10, TFT_BLACK);
      tft.setCursor(10, 180);
      tft.print("Sensor not calibrated");
    }
    lastReadingTime = currentTime; // update time of last reading
  }
  else 
  {
    static int lastSecond = -1; // update countdown every second
    if (secondsToNext != lastSecond) 
    {
      lastSecond = secondsToNext;
      
      // update just the countdown timer
      tft.fillRect(110, 110, 70, 20, TFT_BLACK); 
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextSize(2);
      tft.setCursor(110, 110); 
      tft.print(secondsToNext);
      tft.print("s   ");
    }
  }
  delay(100);
}