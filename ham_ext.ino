//  Exterior Sensors by @Neoxelox
//  Rev. 1 | Last modification: 12/10/2017

//  Libraries Section:

//  DHT11 Sensor
  #include <DHT.h>
  #include <DHT_U.h>

//  BMP280
  #include <Wire.h>
  #include "i2c.h"
  #include "i2c_BMP280.h"

//--------------\\  

//  Define Section:

  #define btPin 2   // Bluetooth Pin
  #define dhPin 11   // Humidity/Temperature Sensor Pin
  #define mqPin A0  // Air Pollution Sensor Pin
  #define smPin 13   // MQ135 Switch
  #define poPin A1  // Illuminance Sensor Pin
  #define rnPin A2  // Rain Sensor Pin
  #define uvPin A3  // UV Sensor Pin
  //BMP280 DEFINITIONS ARE DEFAULT 12C PINS A5/A4

//------------------\\

//  Variable Section:

//  Bluetooth
  int btTimeout = 3000; // Time that Bluetooth will be off after finishing transmitting data

//  DHT11 Sensor
  DHT dht11 (dhPin, DHT11); // Define the type of DHT (11)

// BMP280 Sensor
  BMP280 bmp280;  
  float pressure;
  static float altitude;

//  MQ135 Sensor
  #define mqSamples 50 // Samples from the sensor (More accurate)                                                         
  int mqArray[mqSamples];                    
  unsigned long mqAverage;
  #define mqNoPol 500 // Calibrated data where there is not Pollution
  int mqValue;

//  Illuminance Sensor
  #define alfaIllu 0.0048828125 // Alfa calibrated for unit of lux

//  Rain Sensor
  int rainRaw, rainVal;

//  UV Sensor
  int uvVoltage = 0;
  int uvIndex = 0;    

//  Battery
  long battery;  

//  HAM PROTOCOL  
  const byte numChars = 64;
  char receivedChars[numChars];
  char tempChars[numChars]; // Temporary array for use when parsing

//  Variables to hold the parsed data
  String BT_REQUEST, messageFromBTplaceHolder, BT_PROTOCOL;
  boolean newData = false;

//----------------------\\


void setup() 
{

//  Bluetooth
  pinMode(btPin, OUTPUT);
  digitalWrite(btPin, HIGH);  // Power on Bluetooth via transistor
  delay(100);
  Serial.begin(9600); // Prepare communication through BT

//  DHT11 Sensor
  dht11.begin();  // Begin DHT11

//  MQ135 Sensor
  pinMode(mqPin, INPUT);  // Prepare pin to receive data
  pinMode(smPin, OUTPUT);
  digitalWrite(smPin, HIGH); // Power on MQ135 via transistor

//  Illuminance Sensor
  pinMode(poPin, INPUT);  // Prepare pin to receive data

//  Rain Sensor
  pinMode(rnPin, INPUT);  // Prepare pin to receive data  

//  UV Sensor
  pinMode(uvPin, INPUT);  // Prepare pin to receive data

//  BMP280 Sensor
  bmp280.initialize();
  bmp280.setEnabled(0);
  bmp280.triggerMeasurement();   
  
}

void loop()
{

  receiveHAMProtocol();
  if (newData == true) 
  {
    
    strcpy(tempChars, receivedChars); // Temporary copy to protect the original data for later manipulation
    
    if (parseData() == true)
    
    {
      
      manipulateData();
      
    }
   
    newData = false;
   
   }
  
}

void btRestart()  // Restarts the Bluetooth after transmitting data
{
  digitalWrite(btPin, LOW);
  delay(btTimeout);
  digitalWrite(btPin, HIGH);
  delay(100);
}

int readIl()  // Reads the Illuminance
{
  return((2500/(analogRead(poPin)*alfaIllu)-500)/10);  
}

int readMq()  // Reads the concentration of Pollution
{
  for(int i = 0; i < mqSamples; i++)
  {              
  mqArray[i] = analogRead(mqPin);      
  mqAverage += mqArray[i];  
  }
                
  mqValue = mqAverage / mqSamples;                  
  mqValue = map(mqValue, mqNoPol, 1023, 0, 100);                                                    
  mqAverage = 0;    
  return(mqValue);              
}

int readRn()  // Reads the Rain
{
  rainRaw = analogRead(rnPin);
  if(rainRaw <=1023)
  {
    rainVal = 1;
  }else if(rainRaw >= 1024 && rainRaw <= 8000)
  {
    rainVal = 2;
  }else if(rainRaw > 8000)
  {
    rainVal = 3;
  }

  return(rainVal);
}

int readUv()  // Reads the UV Index
{
  uvVoltage = (analogRead(uvPin) * (5.0 / 1023.0)) * 1000;
  if(uvVoltage<227){
    uvIndex = 0;
  }
  if(uvVoltage>=227){
    uvIndex = 1;
  }
  if(uvVoltage>=318){
    uvIndex = 2;
  }
  if(uvVoltage>=408){
    uvIndex = 3;
  }
  if(uvVoltage>=503){
    uvIndex = 4;
  }
  if(uvVoltage>=606){
    uvIndex = 5;
  }
  if(uvVoltage>=669){
    uvIndex = 6;
  }
  if(uvVoltage>=795){
    uvIndex = 7;
  }
  if(uvVoltage>=881){
    uvIndex = 8;
  }
  if(uvVoltage>=976){
    uvIndex = 9;
  }
  if(uvVoltage>=1079){
    uvIndex = 10;
  }
  if(uvVoltage>=1170){
    uvIndex = 11;
  }
  return(uvIndex);
}

float readPressure() // Reads the pressure
{
  bmp280.awaitMeasurement();
  bmp280.getPressure(pressure);
  bmp280.triggerMeasurement();

  return(pressure);
}

float readAltitude() // Reads the altitude
{
  bmp280.awaitMeasurement();
  bmp280.getAltitude(altitude);
  bmp280.triggerMeasurement();

  return(altitude);
}

long readBattery()  // Reads the battery percent
{
ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); 
delay(2); // Wait for Vref to settle
ADCSRA |= _BV(ADSC);  // Convert
while (bit_is_set(ADCSRA,ADSC)); 
battery = ADCL;
battery |= ADCH<<8; 
battery = 1126400L / battery; // Back-calculate AVcc in mV
return (map(battery, 4500, 5000, 0, 100)); 
}

void receiveHAMProtocol() // Receives the message via HAM PROTOCOL
{
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial.available() > 0 && newData == false) 
    {
        rc = Serial.read();

        if (recvInProgress == true) 
        {
            if (rc != endMarker) // Searchs for the endMarker
            {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) 
                {
                    ndx = numChars - 1;
                }
            }
            else 
            {
                receivedChars[ndx] = '\0'; // Terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) // Searchs for the startMarker
        {
            recvInProgress = true;
        }
    }
}

String buildHAMProtocol(String channelName, int intData, float floatData, String stringData)  // Constructs the necessary HAM protocol to send
{
  
  return("<HAM-PROTOCOL,"+ channelName + "," + String(intData) + "," + String(floatData) + "," + stringData + ">");
  
}

boolean parseData() // Splits the data into its parts --> Expects <HAM-PROTOCOL, request>
{      

    boolean correctProtocol;  // Boolean var to store if the readed protocol is the correct one
    char * strtokIndx;  // This is used by strtok() as an index

    // EXPECTED FIRST PART: THE HAM PROTOCOL

    strtokIndx = strtok(tempChars,","); // Gets the first part (a String)
     
    messageFromBTplaceHolder += strtokIndx;    
    BT_PROTOCOL = messageFromBTplaceHolder;   
    messageFromBTplaceHolder = "";

    if (BT_PROTOCOL == "HAM-PROTOCOL"){correctProtocol = true;} else {correctProtocol = false;}

    //======================================

    // EXPECTED SECOND PART: THE REQUEST
    
    strtokIndx = strtok(NULL, ","); // Gets the second part (a String)
    
    messageFromBTplaceHolder += strtokIndx;    
    BT_REQUEST = messageFromBTplaceHolder;   
    messageFromBTplaceHolder = "";

    //=====================================

    return correctProtocol; // Returns true if the protocol used was HAM-PROTOCOL

} 

void manipulateData() // Manipulates the data received and parsed
{

    // COMMAND ZONE =========================================    
    
    if (BT_REQUEST == "ready.request") /*-->*/ {Serial.println(buildHAMProtocol("HAM-EXT", 0, 0, "OK-READY"));}
    if (BT_REQUEST == "HAM-EXT.requestAction.OFF") /*-->*/ {Serial.println(buildHAMProtocol("HAM-EXT", 0, 0, "OK-OFF")); delay(250); /*-->*/ btRestart();}
    if (BT_REQUEST == "batteryEXT.request") /*-->*/ {Serial.println(buildHAMProtocol("batteryEXT", readBattery(), 0, "0"));}
    if (BT_REQUEST == "airpol.requestData") /*-->*/ {Serial.println(buildHAMProtocol("airpol", readMq(), 0, "0"));}
    if (BT_REQUEST == "exteriortemp.requestData") /*-->*/ {Serial.println(buildHAMProtocol("exteriortemp", 0, dht11.readTemperature(), "0"));}
    if (BT_REQUEST == "humidity.requestData") /*-->*/ {Serial.println(buildHAMProtocol("humidity", 0, dht11.readHumidity(), "0"));}
    if (BT_REQUEST == "illuminance.requestData") /*-->*/ {Serial.println(buildHAMProtocol("illuminance", readIl(), 0, "0"));}
    if (BT_REQUEST == "rain.requestData") /*-->*/ {Serial.println(buildHAMProtocol("rain", readRn(), 0, "0"));}
    if (BT_REQUEST == "uvrad.requestData") /*-->*/ {Serial.println(buildHAMProtocol("uvrad", readUv(), 0, "0"));}
    if (BT_REQUEST == "pressure.requestData") /*-->*/ {Serial.println(buildHAMProtocol("pressure", 0, readPressure(), "0"));}
    if (BT_REQUEST == "altitude.requestData") /*-->*/ {Serial.println(buildHAMProtocol("altitude", 0, readAltitude(), "0"));}
   
    if (BT_REQUEST == "airpol.requestAction.OFF") /*-->*/ {Serial.println(buildHAMProtocol("airpol", 0, 0, "OK-OFF")); /*-->*/ digitalWrite(smPin, LOW);}
    if (BT_REQUEST == "airpol.requestAction.ON") /*-->*/ {Serial.println(buildHAMProtocol("airpol", 0, 0, "OK-ON")); /*-->*/ digitalWrite(smPin, HIGH);}
    
    //=======================================================
    
}