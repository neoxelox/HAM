//  Gases Sensor by @Neoxelox
//  Rev. 1 | Last modification: 12/10/2017

//  Define Section:

  #define btPin 2   // Bluetooth Pin
  #define m2Pin A0  // CO2 Gas Sensor Pin
  #define s2Pin 11   // MQ2 Switch
  #define m5Pin A1  // Natural Gas Sensor Pin
  #define s5Pin 9   // MQ5 Switch
  #define m6Pin A2  // Butane/Propane Gas Sensor Pin
  #define s6Pin 10   // MQ6 Switch
  #define m7Pin A3  // CO Gas Sensor Pin
  #define s7Pin 12   // MQ7 Switch
  #define flPin 13   // Flame Sensor Pin
  #define tpPin A4  // Temperature Sensor Pin

//------------------\\

//  Variable Section:

//  Bluetooth
  int btTimeout = 3000; // Time that Bluetooth will be off after finishing transmitting data

//  MQ2 Sensor
  #define m2Samples 50 // Samples from the sensor (More = More accurate but less SRAM)                                                         
  int m2Array[m2Samples];                    
  unsigned long m2Average;
  #define m2NoGas 500 // Calibrated data where there is not Gas
  int m2Value;

//  MQ5 Sensor
  #define m5Samples 50 // Samples from the sensor (More = More accurate but less SRAM)                                                         
  int m5Array[m5Samples];                    
  unsigned long m5Average;
  #define m5NoGas 500 // Calibrated data where there is not Gas
  int m5Value;

//  MQ6 Sensor
  #define m6Samples 50 // Samples from the sensor (More = More accurate but less SRAM)                                                         
  int m6Array[m6Samples];                    
  unsigned long m6Average;
  #define m6NoGas 500 // Calibrated data where there is not Gas
  int m6Value;

//  MQ7 Sensor
  #define m7Samples 50 // Samples from the sensor (More = More accurate but less SRAM)                                                         
  int m7Array[m7Samples];                    
  unsigned long m7Average;
  #define m7NoGas 500 // Calibrated data where there is not Gas
  int m7Value;    
  
//  Temperature Sensor
  #define tpMin 0 // Calibrated data where there is the minimun temperature
  #define tpMax 95.46 // Calibrated data where there is the maximum temperature

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

//  MQ2 Sensor
  pinMode(m2Pin, INPUT);  // Prepare pin to receive data
  pinMode(s2Pin, OUTPUT);
  digitalWrite(s2Pin, HIGH); // Power on MQ2 via transistor

//  MQ5 Sensor
  pinMode(m5Pin, INPUT);  // Prepare pin to receive data
  pinMode(s5Pin, OUTPUT);
  digitalWrite(s5Pin, HIGH); // Power on MQ5 via transistor

//  MQ6 Sensor
  pinMode(m6Pin, INPUT);  // Prepare pin to receive data
  pinMode(s6Pin, OUTPUT);
  digitalWrite(s6Pin, HIGH); // Power on MQ6 via transistor

//  MQ7 Sensor
  pinMode(m7Pin, INPUT);  // Prepare pin to receive data
  pinMode(s7Pin, OUTPUT);
  digitalWrite(s7Pin, HIGH); // Power on MQ7 via transistor    
  
//  Flame Sensor
  pinMode(flPin, INPUT);  // Prepare pin to receive data

//  Temperature Sensor  
  pinMode(tpPin, INPUT);  // Prepare pin to receive data
  
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

float readTp()  // Reads the Temperature
{
  return(map(analogRead(tpPin), 0, 1023, tpMin, tpMax));  
}

int readM2()  // Reads the concentration of CO2 Gas
{
  for(int i = 0; i < m2Samples; i++)
  {              
  m2Array[i] = analogRead(m2Pin);      
  m2Average += m2Array[i];  
  }
                
  m2Value = m2Average / m2Samples;                  
  m2Value = map(m2Value, m2NoGas, 1023, 0, 100);                                                    
  m2Average = 0;    
  return(m2Value);              
}

int readM5()  // Reads the concentration of Natural Gas
{
  for(int i = 0; i < m5Samples; i++)
  {              
  m5Array[i] = analogRead(m5Pin);      
  m5Average += m5Array[i];  
  }
                
  m5Value = m5Average / m5Samples;                  
  m5Value = map(m5Value, m5NoGas, 1023, 0, 100);                                                    
  m5Average = 0;    
  return(m5Value);              
}

int readM6()  // Reads the concentration of Butane/Propane Gas
{
  for(int i = 0; i < m6Samples; i++)
  {              
  m6Array[i] = analogRead(m6Pin);      
  m6Average += m6Array[i];  
  }
                
  m6Value = m6Average / m6Samples;                  
  m6Value = map(m6Value, m6NoGas, 1023, 0, 100);                                                    
  m6Average = 0;    
  return(m6Value);              
}

int readM7()  // Reads the concentration of CO Gas
{
  for(int i = 0; i < m7Samples; i++)
  {              
  m7Array[i] = analogRead(m7Pin);      
  m7Average += m7Array[i];  
  }
                
  m7Value = m7Average / m7Samples;                  
  m7Value = map(m7Value, m7NoGas, 1023, 0, 100);                                                    
  m7Average = 0;    
  return(m7Value);              
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

    if (BT_REQUEST == "ready.request") /*-->*/ {Serial.println(buildHAMProtocol("HAM-GAS", 0, 0, "OK-READY"));}
    if (BT_REQUEST == "HAM-GAS.requestAction.OFF") /*-->*/ {Serial.println(buildHAMProtocol("HAM-GAS", 0, 0, "OK-OFF")); /*-->*/ delay(250); /*-->*/ btRestart();}
    if (BT_REQUEST == "batteryGAS.request") /*-->*/ {Serial.println(buildHAMProtocol("batteryGAS", readBattery(), 0, "0"));}
    if (BT_REQUEST == "CO2.requestData") /*-->*/ {Serial.println(buildHAMProtocol("CO2", readM2(), 0, "0"));}
    if (BT_REQUEST == "natugas.requestData") /*-->*/ {Serial.println(buildHAMProtocol("natugas", readM5(), 0, "0"));}
    if (BT_REQUEST == "butprop.requestData") /*-->*/ {Serial.println(buildHAMProtocol("butprop", readM6(), 0, "0"));}
    if (BT_REQUEST == "CO.requestData") /*-->*/ {Serial.println(buildHAMProtocol("CO", readM7(), 0, "0"));}
    if (BT_REQUEST == "flame.requestData") /*-->*/ {Serial.println(buildHAMProtocol("flame", digitalRead(flPin), 0, "0"));}
    if (BT_REQUEST == "kitchentemp.requestData") /*-->*/ {Serial.println(buildHAMProtocol("kitchentemp", 0, readTp(), "0"));}

    // Gas sensors switches:

    if (BT_REQUEST == "CO2.requestAction.OFF") /*-->*/ {Serial.println(buildHAMProtocol("CO2", 0, 0, "OK-OFF")); /*-->*/ digitalWrite(s2Pin, LOW);}
    if (BT_REQUEST == "natugas.requestAction.OFF") /*-->*/ {Serial.println(buildHAMProtocol("natugas", 0, 0, "OK-OFF")); /*-->*/ digitalWrite(s5Pin, LOW);}
    if (BT_REQUEST == "butprop.requestAction.OFF") /*-->*/ {Serial.println(buildHAMProtocol("butprop", 0, 0, "OK-OFF")); /*-->*/ digitalWrite(s6Pin, LOW);}
    if (BT_REQUEST == "CO.requestAction.OFF") /*-->*/ {Serial.println(buildHAMProtocol("CO", 0, 0, "OK-OFF")); /*-->*/ digitalWrite(s7Pin, LOW);}

    if (BT_REQUEST == "CO2.requestAction.ON") /*-->*/ {Serial.println(buildHAMProtocol("CO2", 0, 0, "OK-ON")); /*-->*/ digitalWrite(s2Pin, HIGH);}
    if (BT_REQUEST == "natugas.requestAction.ON") /*-->*/ {Serial.println(buildHAMProtocol("natugas", 0, 0, "OK-ON")); /*-->*/ digitalWrite(s5Pin, HIGH);}
    if (BT_REQUEST == "butprop.requestAction.ON") /*-->*/ {Serial.println(buildHAMProtocol("butprop", 0, 0, "OK-ON")); /*-->*/ digitalWrite(s6Pin, HIGH);}
    if (BT_REQUEST == "CO.requestAction.ON") /*-->*/ {Serial.println(buildHAMProtocol("CO", 0, 0, "OK-ON")); /*-->*/ digitalWrite(s7Pin, HIGH);}
    
    //=======================================================
    
}