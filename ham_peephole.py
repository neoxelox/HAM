#HAM-PEEPHOLE BY @Neoxelox
#Last modification 29/9/2017
import cayenne.client
import time
import RPi.GPIO as GPIO

#Sleep to allow wireless to connect before starting MQTT
time.sleep(5)

#Setting the GPIO
GPIO.setmode(GPIO.BOARD)

#Setting GPIO Vars
photoresistor_pin = 7
vibration_pin = 11
alarm_pin = 13
GPIO.setup(vibration_pin, GPIO.IN)
GPIO.setup(alarm_pin, GPIO.OUT)
GPIO.output(alarm_pin, GPIO.LOW)

#Time between packets and alarm
timePhr = 2  #Photoresistor
timeAlarm = 3  #Alarm

#Vars to store data
dataPhr = 0
dataVib = 0
alarm = False

# Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
MQTT_USERNAME  = ""
MQTT_PASSWORD  = ""
MQTT_CLIENT_ID = ""

# The callback for when a message is received from Cayenne.
def on_message(message):
  print("message received: " + str(message))

#Function for Photoresistor
def photoRead (photoresistor_pin):
    prCount = 0
    GPIO.setup(photoresistor_pin, GPIO.OUT)
    GPIO.output(photoresistor_pin, GPIO.LOW)
    time.sleep(0.1)
    GPIO.setup(photoresistor_pin, GPIO.IN)

    while (GPIO.input(photoresistor_pin) == GPIO.LOW):
        prCount += 1
    print("RAW VALUE " + str (prCount)) #COMMENT THIS BEFORE RELEASE
    print("MAP VALUE " + str(pimap(prCount, 0, 3000, 0, 100))) #COMMENT THIS BEFORE RELEASE
    return pimap(prCount, 0, 27000, 0, 100)

#Function for Vibration sensor
def vibRead(vibration_pin):
    return GPIO.input(vibration_pin)

#Function for map
def pimap(x, in_min, in_max, out_min, out_max):
    return int((x-in_min) * (out_max-out_min) / (in_max-in_min) + out_min)

#Setting Cayenne Vars
client = cayenne.client.CayenneMQTTClient()
client.on_message = on_message
client.begin(MQTT_USERNAME, MQTT_PASSWORD, MQTT_CLIENT_ID)

#Setting Time and Index Vars
i=0
timestamp = 0
timestamp2 = 0
oldVib = 0
vibCounts = 0
alarmCounts = 0

while True:
  client.loop()

  #Read Vibration data always
  dataVib = vibRead(vibration_pin)

  #Read Photoresistor data every 2 seconds
  if (time.time() > timestamp + timePhr):
    #Send to V1 Photoresistor data
    dataPhr = photoRead(photoresistor_pin)
    client.virtualWrite(1, dataPhr, "lum", "p")
    timestamp = time.time()
    i += 1

  if (dataVib != oldVib):
    #Get Vibration if its different
    oldVib = dataVib
    if (dataVib == 0):
      client.virtualWrite(2, 0, "null", " ")
    vibCounts += 1

  if (vibCounts == 5):
    #Send vibration data to V2 and start alarm if vib its different for 5 times
    vibCounts = 0
    client.virtualWrite(2, 1, "null", " ")
    alarm = True

  if (dataPhr >= 55):
    alarm = True

  if (alarm == True and time.time() > timestamp2 + timeAlarm):
    #Start the alarm!
    alarm = False
    GPIO.output(alarm_pin, GPIO.HIGH)
    time.sleep(0.1)
    GPIO.output(alarm_pin, GPIO.LOW)
