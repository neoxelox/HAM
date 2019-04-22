//  Radiations Sensor by @Neoxelox
//  Rev. 1 | Last modification: 12/10/2017

//  Define Section:

  #define btPin 2   // Bluetooth Pin
  #define efPin A0  // Electromagnetic Field Sensor Pin
  #define mfPin A1  // Magnetic Field Sensor Pin
  #define irPin A2  // Infrared Radation Sensor Pin
  #define dbPin A3  // Decibel Meter Sensor Pin

//------------------\\

//  Variable Section:

//  Bluetooth
  int btTimeout = 3000; // Time that Bluetooth will be off after finishing transmitting data

//  EF Sensor
  #define efSamples 100 // Samples from the sensor (More = More accurate but less SRAM)                                                         
  int efArray[efSamples];                    
  unsigned long efAverage;
  int efValue;

//  MF Sensor
  #define noField 535 // Calibrated data where there is not field
  #define toMilligauss 1.953125

//  IR Sensor
  #define irSamples 100 // Samples from the sensor (More = More accurate but less SRAM)                                                         
  int irArray[irSamples];                    
  unsigned long irAverage;
  int irValue;

//  DB Sensor
  #define dbSampleTime 50
  unsigned long dbSample;
  #define maxDb 1024
  #define minDb 0
  unsigned long dbMinSample;
  unsigned long dbMaxSample;
  unsigned long dbMillis;
  float peakDb;
  #define dbMaxOut 90
  #define dbMinOut 49.5

//  Battery
  long battery;

//  HAM PROTOCOL  
  const byte numChars = 64;
  char receivedChars[numChars];
  char tempChars[numChars]; // Temporary array for use when parsing

// Variables to hold the parsed data
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

//  EF Sensor
  pinMode(efPin, INPUT);  // Prepare pin to receive data
  
//  MF Sensor
  pinMode(mfPin, INPUT);  // Prepare pin to receive data

//  IR Sensor  
  pinMode(irPin, INPUT);  // Prepare pin to receive data

//  DB Sensor  
  pinMode(dbPin, INPUT);  // Prepare pin to receive data
  
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

int readMf()  // Reads the Magnetic Field in MilliGauss
{
  return((analogRead(mfPin) - noField) * toMilligauss);  
}

int readEf()  // Reads the concentration of ElectroMagnetic Fields
{
  for(int i = 0; i < efSamples; i++)
  {              
  efArray[i] = analogRead(efPin);      
  efAverage += efArray[i];  
  }
                
  efValue = efAverage / efSamples;                  
  efValue = constrain(efValue, 0, 100);                                                    
  efAverage = 0;    
  return(efValue);              
}

int readIr()  // Reads the concentration of Infrared Radation Fields
{
  for(int i = 0; i < irSamples; i++)
  {              
  irArray[i] = analogRead(irPin);      
  irAverage += irArray[i];  
  }
                
  irValue = irAverage / irSamples;                  
  irValue = constrain(irValue, 0, 100);                                                    
  irAverage = 0;    
  return(irValue);              
}

float readDb()  // Reads de Decibels
{

  dbMillis = millis();

  while (millis() - dbMillis < dbSampleTime)
  {
    dbSample = analogRead(dbPin);
    
    if (dbSample < maxDb)
    {
      if (dbSample > minDb)
      {
        dbMinSample = dbSample;
      }
      else if(dbSample < maxDb)
      {
        dbMaxSample = dbSample;
      }
    }
  }

  peakDb = dbMinSample - dbMaxSample;

  return(map(peakDb, 20, 900, dbMinOut, dbMaxOut));
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
    
    if (BT_REQUEST == "ready.request") /*-->*/ {Serial.println(buildHAMProtocol("HAM-RDT", 0, 0, "OK-READY"));}
    if (BT_REQUEST == "HAM-RDT.requestAction.OFF") /*-->*/ {Serial.println(buildHAMProtocol("HAM-RDT", 0, 0, "OK-OFF")); delay(250); /*-->*/ btRestart();}
    if (BT_REQUEST == "batteryRDT.request") /*-->*/ {Serial.println(buildHAMProtocol("batteryRDT", readBattery(), 0, "0"));}
    if (BT_REQUEST == "emfield.requestData") /*-->*/ {Serial.println(buildHAMProtocol("emfield", readEf(), 0, "0"));}
    if (BT_REQUEST == "mfield.requestData") /*-->*/ {Serial.println(buildHAMProtocol("mfield", readMf(), 0, "0"));}
    if (BT_REQUEST == "irrad.requestData") /*-->*/ {Serial.println(buildHAMProtocol("irrad", readIr(), 0, "0"));}
    if (BT_REQUEST == "decibel.requestData") /*-->*/ {Serial.println(buildHAMProtocol("decibel", 0, readDb(), "0"));}

    //=======================================================
    
}