//  HAM HUB by @Neoxelox
//  Rev. 1 | Last modification: 12/10/2017

//  Library Section:

  #include <SoftwareSerial.h> // We need another Serial Communication for the Bluetooth
  #include <CayenneMQTTESP8266.h> // Libary for Cayenne via MQTT protocol
  #include <IRremoteESP8266.h>  // IR Library for HAM-BLINDS
  #include <IRsend.h>

//------------------\\

//  Define Section:

  #define btPin 5   // Bluetooth TX Pin
  #define brPin 4   // Bluetooth RX Pin
  #define tpPin A0  // Temperature Sensor Pin
  #define irPin 14  // IR Emitter Pin
  #define moPin 13  // Ir Motion Sensor Pin
  

  // Network INFO:
    char ssid[] = "";
    char wifiPassword[] = "";

  // MQQT INFO:
    char username[] = "";
    char password[] = "";
    char clientID[] = "";

//------------------\\

//  Variable Section:

//  Bluetooth
  SoftwareSerial btSerial (btPin,brPin); //(TX,RX)
  int btCycleIX = 1;
  char cycleHelper;
  String cHelper, channelRequesting;
  boolean goodTogo = false;
  int tryouts;
  #define tryout 15
  #define TimeBetweenCycle 20000
  // For MQs switches
    boolean mq135OFF, mq2OFF, mq5OFF, mq6OFF, mq7OFF;

//  IR Emitter
  IRsend irsend(irPin);
  
//  HAM PROTOCOL  
  const byte numChars = 64;
  char receivedChars[numChars];
  char tempChars[numChars]; // Temporary array for use when parsing

//  Variables to hold the parsed data
  String BT_CHANNEL, messageFromBTplaceHolder, BT_PROTOCOL, stringFromBT;
  int intFromBT;
  float floatFromBT;
  
  boolean newData = false;

//----------------------\\


void setup() 
{
  
//  Bluetooth
  btSerial.begin(38400); // Prepare communication through BT
  Serial.begin(9600);
  irsend.begin(); // Prepare IR Emitter
  pinMode(tpPin, INPUT); // Prepare pin to receive data
  pinMode(moPin, INPUT); // Prepare pin to receive data
  delay(500);
  btSerial.print("AT+INIT\r\n");  // Initialitze BT libraries
  Serial.println("INICIALIZANDO...");
  
  Cayenne.begin(username, password, clientID, ssid, wifiPassword);  // Initialize Cayenne
}

void loop()
{ 
  Cayenne.loop();
  readMotion();
  
  static unsigned long BTTimeToRead = millis();
  if (millis() >= BTTimeToRead + TimeBetweenCycle)
  {
    Cayenne.virtualWrite(28, readTemp());
    Serial.println("ENTRANDO EN CICLO");
    btCycle();
    BTTimeToRead = millis();
  }
  
}

CAYENNE_IN(20)
{
  if (getValue.asInt() == 1) 
  {
    mq135OFF = false;
  }else if (getValue.asInt() == 0)
  {
    mq135OFF = true;
  }
}

CAYENNE_IN(21)
{
  if (getValue.asInt() == 1) 
  {
    mq2OFF = false;
  }else if (getValue.asInt() == 0)
  {
    mq2OFF = true;
  }
}

CAYENNE_IN(22)
{
  if (getValue.asInt() == 1) 
  {
    mq5OFF = false;
  }else if (getValue.asInt() == 0)
  {
    mq5OFF = true;
  }
}

CAYENNE_IN(23)
{
  if (getValue.asInt() == 1) 
  {
    mq6OFF = false;
  }else if (getValue.asInt() == 0)
  {
    mq6OFF = true;
  }
}

CAYENNE_IN(24)
{
  if (getValue.asInt() == 1) 
  {
    mq7OFF = false;
  }else if (getValue.asInt() == 0)
  {
    mq7OFF = true;
  }
}

CAYENNE_IN(25)
{
  if (getValue.asInt() == 1) 
  {
    irsend.sendNEC(0xDC23, 38);
  }
}

CAYENNE_IN(26)
{
  if (getValue.asInt() == 1) 
  {
    irsend.sendNEC(0xDE21, 38);
  }
}

CAYENNE_IN(27)
{
  if (getValue.asInt() == 1) 
  {
    irsend.sendNEC(0xFC03, 38);
  }
}

void btReset()
{
  btSerial.print("AT+RESET\r\n");
  delay(1500);
  btSerial.print("AT+INIT\r\n");
  loop();
}

float readTemp()
{
  return((((analogRead(tpPin)/1024)*5) - 0.5) * 100);
}

void readMotion()
{
  int motion = 0;
  int oldMotion = 0;
  motion = digitalRead(moPin);
  
  if (motion =! oldMotion)
  {
    oldMotion = 1;
    Cayenne.virtualWrite(29, motion);
  }
}

void receiveHAMProtocol() // Receives the message via HAM PROTOCOL
{
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (btSerial.available() > 0 && newData == false) 
    {
        rc = btSerial.read();

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
        yield();
    }
}

String buildHAMProtocol(String channelName, int intData, float floatData, String stringData)  // Constructs the necessary HAM protocol to send
{
  
  return("<HAM-PROTOCOL,"+ channelName + "," + String(intData) + "," + String(floatData) + "," + stringData + ">");
  
}

boolean parseData() // Splits the data into its parts --> Expects <HAM-PROTOCOL,Channel,IntData,FloatData,StringData>
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

    // EXPECTED SECOND PART: THE CHANNEL
    
    strtokIndx = strtok(NULL, ","); // Gets the second part (a String)
    
    messageFromBTplaceHolder += strtokIndx;    
    BT_CHANNEL = messageFromBTplaceHolder;   
    messageFromBTplaceHolder = "";

    //=====================================

    // EXPECTED THIRD PART: THE DATA
    
    strtokIndx = strtok(NULL, ","); // Gets the first data part (an Int)
    
    intFromBT = atoi(strtokIndx);

    strtokIndx = strtok(NULL, ","); // Gets the second data part (a Float)
    
    floatFromBT = atof(strtokIndx);

    strtokIndx = strtok(NULL, ","); // Gets the second data part (a Float)
    
    messageFromBTplaceHolder += strtokIndx;    
    stringFromBT = messageFromBTplaceHolder;   
    messageFromBTplaceHolder = "";

    //=====================================

    return correctProtocol; // Returns true if the protocol used was HAM-PROTOCOL

} 

void manipulateData() // Manipulates the data received and parsed
{

    // COMMAND ZONE =========================================    
    
    if (BT_CHANNEL == channelRequesting && stringFromBT == "OK-READY") /*-->*/ {goodTogo = true;}
    if (BT_CHANNEL == channelRequesting && stringFromBT == "OK-OFF") /*-->*/ {goodTogo = true;}

    if (BT_CHANNEL == "batteryRDT") /*-->*/ {Serial.print("BATTERY-RDT:");Serial.println(intFromBT);Cayenne.virtualWrite(0, intFromBT);}
    if (BT_CHANNEL == "emfield") /*-->*/ {Serial.print("EMFIELD:");Serial.println(intFromBT);Cayenne.virtualWrite(1, intFromBT);}
    if (BT_CHANNEL == "mfield") /*-->*/ {Serial.print("MFIELD:");Serial.println(intFromBT);Cayenne.virtualWrite(2, intFromBT);}
    if (BT_CHANNEL == "irrad") /*-->*/ {Serial.print("IRRAD:");Serial.println(intFromBT);Cayenne.virtualWrite(3, intFromBT);}
    if (BT_CHANNEL == "decibel") /*-->*/ {Serial.print("DECIBEL:");Serial.println(floatFromBT);Cayenne.virtualWrite(4, floatFromBT);}

    if (BT_CHANNEL == "batteryEXT") /*-->*/ {Serial.print("BATTERY-EXT:");Serial.println(intFromBT);Cayenne.virtualWrite(5, intFromBT);}
    if (BT_CHANNEL == "airpol") /*-->*/ {Serial.print("AIRPOL:");Serial.println(intFromBT);Cayenne.virtualWrite(6, intFromBT);}
    if (BT_CHANNEL == "airpol" && stringFromBT != "0") /*-->*/ {Serial.print("AIRPOL:");Serial.println(stringFromBT);}
    if (BT_CHANNEL == "exteriortemp") /*-->*/ {Serial.print("EXTERIORTEMP:");Serial.println(floatFromBT);Cayenne.virtualWrite(7, floatFromBT);}
    if (BT_CHANNEL == "humidity") /*-->*/ {Serial.print("HUMIDITY:");Serial.println(floatFromBT);Cayenne.virtualWrite(8, floatFromBT);}
    if (BT_CHANNEL == "illuminance") /*-->*/ {Serial.print("ILLUMINANCE:");Serial.println(intFromBT);Cayenne.virtualWrite(9, intFromBT);}
    if (BT_CHANNEL == "rain") /*-->*/ {Serial.print("RAIN:");Serial.println(intFromBT);Cayenne.virtualWrite(10, intFromBT);}
    if (BT_CHANNEL == "uvrad") /*-->*/ {Serial.print("UVRAD:");Serial.println(intFromBT);Cayenne.virtualWrite(11, intFromBT);}
    if (BT_CHANNEL == "pressure") /*-->*/ {Serial.print("PRESSURE:");Serial.println(floatFromBT);Cayenne.virtualWrite(12, floatFromBT);}

    if (BT_CHANNEL == "batteryGAS") /*-->*/ {Serial.print("BATTERY-GAS:");Serial.println(intFromBT);Cayenne.virtualWrite(13, intFromBT);}
    if (BT_CHANNEL == "CO2") /*-->*/ {Serial.print("CO2:");Serial.println(intFromBT);Cayenne.virtualWrite(14, intFromBT);}
    if (BT_CHANNEL == "CO2" && stringFromBT != "0") /*-->*/ {Serial.print("CO2:");Serial.println(stringFromBT);}
    if (BT_CHANNEL == "natugas") /*-->*/ {Serial.print("NATUGAS:");Serial.println(intFromBT);Cayenne.virtualWrite(15, intFromBT);}
    if (BT_CHANNEL == "natugas" && stringFromBT != "0") /*-->*/ {Serial.print("NATUGAS:");Serial.println(stringFromBT);}
    if (BT_CHANNEL == "butprop") /*-->*/ {Serial.print("BUTPROP:");Serial.println(intFromBT);Cayenne.virtualWrite(16, intFromBT);}
    if (BT_CHANNEL == "butprop" && stringFromBT != "0") /*-->*/ {Serial.print("BUTPROP:");Serial.println(stringFromBT);}
    if (BT_CHANNEL == "CO") /*-->*/ {Serial.print("CO:");Serial.println(intFromBT);Cayenne.virtualWrite(17, intFromBT);}
    if (BT_CHANNEL == "CO" && stringFromBT != "0") /*-->*/ {Serial.print("CO:");Serial.println(stringFromBT);}
    if (BT_CHANNEL == "flame") /*-->*/ {Serial.print("FLAME:");Serial.println(intFromBT);Cayenne.virtualWrite(18, intFromBT);}
    if (BT_CHANNEL == "kitchentemp") /*-->*/ {Serial.print("KITCHENTEMP:");Serial.println(floatFromBT);Cayenne.virtualWrite(19, floatFromBT);}

    //=======================================================
    
}

void btCycle()  // Cycle for false piconet
{
  
  switch(btCycleIX)
  {
    
    case 1: // Cycle for HAM-RDT
    Serial.println("ENTRADO EN CICLO HAM-RDT"); //DELETE DEBUG
    channelRequesting = "HAM-RDT";
    Serial.println("ENVIANDO AT");  //DELETE DEBUG
    tryouts = 0;
    cHelper = "";
    while (cHelper != "OK\r\n") // Wait until BT says OK
    {
      cHelper = "";
      btSerial.print("AT\r\n");
      
      while (btSerial.available())
      {
        cycleHelper = btSerial.read();
        cHelper += cycleHelper;
        yield();
      }
      
      Serial.print(cHelper);  //DELETE DEBUG
      yield();
            
      if (tryouts == tryout)
      {
        tryouts = 0;
        btReset();
      }else if (tryouts != tryout)
      {
        tryouts++;
      }
      
      delay(100);      
    }
    cHelper = "";
    tryouts = 0;
    
    delay(100);
    
    Serial.println("ENVIANDO AT+LINK"); //DELETE DEBUG 
    btSerial.print("AT+LINK=98D3,31,B30806\r\n"); // Connect with HAM-RDT
   
    delay(5000);
    
    tryouts = 0;
    goodTogo = false;
    while (goodTogo == false) // Wait until ready.request is OK
    {
      btSerial.print("<HAM-PROTOCOL,ready.request>");
      getData();
      
      Serial.println("READY REQUEST ENVIADO");  //DELETE DEBUG
      yield();
      
      if (tryouts == tryout)
      {
        tryouts = 0;
        btReset();
      }else if (tryouts != tryout)
      {
        tryouts++;
      }
      delay(250);
    }
    goodTogo = false;
    tryouts = 0;
    
    // Connected!
    // Sending all the data requests
    for (int i = 1; i <= 6; i++)
    {

      switch(i)
      {
        case 1:
        btSerial.print("<HAM-PROTOCOL,batteryRDT.request>");
        break;
        case 2:
        btSerial.print("<HAM-PROTOCOL,emfield.requestData>");
        break;
        case 3:
        btSerial.print("<HAM-PROTOCOL,mfield.requestData>");
        break;
        case 4:
        btSerial.print("<HAM-PROTOCOL,irrad.requestData>");
        break;
        case 5:
        btSerial.print("<HAM-PROTOCOL,decibel.requestData>");
        break;        
      }
      
      getData();
      yield();
      Cayenne.loop();
      delay(250);
    }

    // Ending the communication
    tryouts = 0;
    goodTogo = false;
    Serial.println("ENTRANDO EN OFF");  //DELETE DEBUG
    while (goodTogo == false) // Wait until HAM-RDT.requestAction.OFF is OK
    {
      btSerial.print("<HAM-PROTOCOL,HAM-RDT.requestAction.OFF>");
      getData();
      
      Serial.println("OFF ENVIADO");  //DELETE DEBUG
      yield();
      
      if (tryouts == tryout)
      {
        tryouts = 0;
        btReset();
      }else if (tryouts != tryout)
      {
        tryouts++;
      }
      delay(250);
    }
    goodTogo = false;
    tryouts = 0;
        
    btCycleIX = 2;
    delay(2000);
    break;
    //---------------------------
    //---------------------------
    case 2: // Cycle for HAM-EXT
    Serial.println("ENTRADO EN CICLO HAM-EXT"); //DELETE DEBUG
    channelRequesting = "HAM-EXT";
    Serial.println("ENVIANDO AT");  //DELETE DEBUG
    tryouts = 0;
    cHelper = "";
    while (cHelper != "OK\r\n") // Wait until BT says OK
    {
      cHelper = "";
      btSerial.print("AT\r\n");
      
      while (btSerial.available())
      {
        cycleHelper = btSerial.read();
        cHelper += cycleHelper;
        yield();
      }
      
      Serial.print(cHelper);  //DELETE DEBUG
      yield();
      
      if (tryouts == tryout)
      {
        tryouts = 0;
        btReset();
      }else if (tryouts != tryout)
      {
        tryouts++;
      }
      
      delay(100);      
    }
    cHelper = "";
    tryouts = 0;
    
    delay(100);
    
    Serial.println("ENVIANDO AT+LINK"); //DELETE DEBUG 
    btSerial.print("AT+LINK=98D3,31,80A173\r\n"); // Connect with HAM-EXT
   
    delay(5000);
    
    tryouts = 0;
    goodTogo = false;
    while (goodTogo == false) // Wait until ready.request is OK
    {
      btSerial.print("<HAM-PROTOCOL,ready.request>");
      getData();
      
      Serial.println("READY REQUEST ENVIADO");  //DELETE DEBUG
      yield();
      
      if (tryouts == tryout)
      {
        tryouts = 0;
        btReset();
      }else if (tryouts != tryout)
      {
        tryouts++;
      }
      delay(250);
    }
    goodTogo = false;
    tryouts = 0;
    
    // Connected!
    // Sending all the data requests
    for (int i = 1; i <= 10; i++)
    {

      switch(i)
      {
        
        case 1:
        btSerial.print("<HAM-PROTOCOL,batteryEXT.request>");
        break;
        case 2:
        btSerial.print("<HAM-PROTOCOL,airpol.requestData>");
        break;
        case 3:
        btSerial.print("<HAM-PROTOCOL,exteriortemp.requestData>");
        break;
        case 4:
        btSerial.print("<HAM-PROTOCOL,humidity.requestData>");
        break;
        case 5:
        btSerial.print("<HAM-PROTOCOL,illuminance.requestData>");
        break;
        case 6:
        btSerial.print("<HAM-PROTOCOL,rain.requestData>");
        break;
        case 7:
        btSerial.print("<HAM-PROTOCOL,uvrad.requestData>");
        break;
        case 8:
        btSerial.print("<HAM-PROTOCOL,pressure.requestData>");
        break;
        case 9:
        if (mq135OFF == false)
        {
          btSerial.print("<HAM-PROTOCOL,airpol.requestAction.ON>"); 
        }else if (mq135OFF == true)
        {
          btSerial.print("<HAM-PROTOCOL,airpol.requestAction.OFF>");  
        }
        break;
                  
      }
      
      getData();
      yield();
      Cayenne.loop();
      delay(250);
    }

    // Ending the communication
    tryouts = 0;
    goodTogo = false;
    Serial.println("ENTRANDO EN OFF");  //DELETE DEBUG
    while (goodTogo == false) // Wait until HAM-EXT.requestAction.OFF is OK
    {
      btSerial.print("<HAM-PROTOCOL,HAM-EXT.requestAction.OFF>");
      getData();
      
      Serial.println("OFF ENVIADO");  //DELETE DEBUG
      yield();
      
      if (tryouts == tryout)
      {
        tryouts = 0;
        btReset();
      }else if (tryouts != tryout)
      {
        tryouts++;
      }
      delay(250);
    }
    goodTogo = false;
    tryouts = 0;
    
    btCycleIX = 3;
    delay(2000);
    break;
    //---------------------------
    //---------------------------
    case 3: // Cycle for HAM-GAS
    Serial.println("ENTRADO EN CICLO HAM-GAS"); //DELETE DEBUG
    channelRequesting = "HAM-GAS";
    Serial.println("ENVIANDO AT");  //DELETE DEBUG
    tryouts = 0;
    cHelper = "";
    while (cHelper != "OK\r\n") // Wait until BT says OK
    {
      cHelper = "";
      btSerial.print("AT\r\n");
      
      while (btSerial.available())
      {
        cycleHelper = btSerial.read();
        cHelper += cycleHelper;
        yield();
      }
      
      Serial.print(cHelper);  //DELETE DEBUG
      yield();

      if (tryouts == tryout)
      {
        tryouts = 0;
        btReset();
      }else if (tryouts != tryout)
      {
        tryouts++;
      }
      
      delay(100);      
    }
    cHelper = "";
    tryouts = 0;
    
    delay(100);
    
    Serial.println("ENVIANDO AT+LINK"); //DELETE DEBUG 
    btSerial.print("AT+LINK=98D3,31,FD5B2E\r\n"); // Connect with HAM-GAS
   
    delay(5000);
    
    tryouts = 0;
    goodTogo = false;
    while (goodTogo == false) // Wait until ready.request is OK
    {
      btSerial.print("<HAM-PROTOCOL,ready.request>");
      getData();
      
      Serial.println("READY REQUEST ENVIADO");  //DELETE DEBUG
      yield();
      
      if (tryouts == tryout)
      {
        tryouts = 0;
        btReset();
      }else if (tryouts != tryout)
      {
        tryouts++;
      }
      delay(250);
    }
    goodTogo = false;
    tryouts = 0;
    
    // Connected!
    // Sending all the data requests
    for (int i = 1; i <= 12; i++)
    {

      switch(i)
      {
        
        case 1:
        btSerial.print("<HAM-PROTOCOL,batteryGAS.request>");
        break;
        case 2:
        btSerial.print("<HAM-PROTOCOL,CO2.requestData>");
        break;
        case 3:
        btSerial.print("<HAM-PROTOCOL,natugas.requestData>");
        break;
        case 4:
        btSerial.print("<HAM-PROTOCOL,butprop.requestData>");
        break;
        case 5:
        btSerial.print("<HAM-PROTOCOL,CO.requestData>");
        break;
        case 6:
        btSerial.print("<HAM-PROTOCOL,flame.requestData>");
        break;
        case 7:
        btSerial.print("<HAM-PROTOCOL,kitchentemp.requestData>");
        break;
        case 8:
        if (mq2OFF == false)
        {
          btSerial.print("<HAM-PROTOCOL,CO2.requestAction.ON>"); 
        }else if (mq2OFF == true)
        {
          btSerial.print("<HAM-PROTOCOL,CO2.requestAction.OFF>");  
        }
        break;
        case 9:
        if (mq5OFF == false)
        {
          btSerial.print("<HAM-PROTOCOL,natugas.requestAction.ON>"); 
        }else if (mq5OFF == true)
        {
          btSerial.print("<HAM-PROTOCOL,natugas.requestAction.OFF>");  
        }
        break;
        case 10:
        if (mq6OFF == false)
        {
          btSerial.print("<HAM-PROTOCOL,butprop.requestAction.ON>"); 
        }else if (mq6OFF == true)
        {
          btSerial.print("<HAM-PROTOCOL,butprop.requestAction.OFF>");  
        }
        break;
        case 11:
        if (mq7OFF == false)
        {
          btSerial.print("<HAM-PROTOCOL,CO.requestAction.ON>"); 
        }else if (mq7OFF == true)
        {
          btSerial.print("<HAM-PROTOCOL,CO.requestAction.OFF>");  
        }
        break;          
      }
      
      getData();
      yield();
      Cayenne.loop();
      delay(250);
    }

    // Ending the communication
    tryouts = 0;
    goodTogo = false;
    Serial.println("ENTRANDO EN OFF");  //DELETE DEBUG
    while (goodTogo == false) // Wait until HAM-GAS.requestAction.OFF is OK
    {
      btSerial.print("<HAM-PROTOCOL,HAM-GAS.requestAction.OFF>");
      getData();
      
      Serial.println("OFF ENVIADO");  //DELETE DEBUG
      yield();

      if (tryouts == tryout)
      {
        tryouts = 0;
        btReset();
      }else if (tryouts != tryout)
      {
        tryouts++;
      }
      delay(250);
    }
    goodTogo = false;
    tryouts = 0;
    
    btCycleIX = 1;
    delay(2000);
    break;
    
  }
  
}

void getData()
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
  yield();
  Cayenne.loop();
  
}
