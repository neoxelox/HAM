# HAM
Home Automation and Monitoring System

Source Code for the 1st Award in Espressif / myDevices / Sparkfun contest: 
ESP8266 IoT Contest - Simplify the Connected World
- https://www.hackster.io/contests/ESP8266
- https://www.hackster.io/neoxelox/ham-home-automation-and-monitoring-system-930ad4

## Welcome to the definition of IoT, welcome to HAM. 

## INTRODUCTION 
HAM, Home Automatation and Monitoring system, is the result of the combination of 4 outstanding things: Cayenne, ESP8266, Arduino and Raspberry. 
The HAM network is formed by 5 different nodes and a central hub. However, thanks to the exclusive protocol inside of all the network, the HAM-PROTOCOL, can be expanded with more and more nodes. 
It also features an exclusive APP, using Cayenne API, which brings together sensors data and the video surveillance system.
## THE NODES 
There are 3 nodes, HAM-EXT (exterior), HAM-RDT (radiation) and HAM-GAS (gases), formed by all types of sensors. RDT and GAS, have gas sensors as Air Pollution, CO2, Natural Gas... which can affect heavily the duration of the battery, because of that, they are controlled by a transistor which the user can switch off/on. There is also the HAM-BLINDS node, which is the controller for the blinds. The last node is HAM-PEEPHOLE, which is a video surveillance system with vibration sensor, illuminance sensor and a loud alarm. The hub, HAM-HUB, is formed by a Sparkfun 8266 dev thing, temperature sensor, IR emitter, bluetooth, and in future updates it will feature a display in case internet access is down. The 3 node sensors have a built-in battery with charger, this allows to make them portable, and stick them in any surface. HAM-HUB is meant to be static, HAM-BLINDS is inside of a blind controller and HAM-PEEPHOLE is attached to a peephole, and it is also meant to be static because of the power consumption of the RPI 0 W. 
## CONNECTIVITY 
The connectivity of the HAM network is not only WiFi, actually only HAM-HUB has it, to send data values to cayenne, and also HAM-PEEPHOLE, where a camera surveillance server is hosted. HAM-EXT, HAM-GAS and HAM-RDT are connected to the hub via bluetooth, but how? Well, there are some BT modules that features piconet, which is a multi-slave bluetooth network. However, that things are $$$$, because of that, I decided to make my own false "piconet". Summarized, the proccess is a cycle for each BT module, divided in 3 parts: Connection, Gathering the sensors value and Disconnection. This is possible by entering the AT mode in a HC-05, and controlling the AT commands. This can seem tricky, but it actually made me end up making a hole protocol for all the nodes, which was a great idea. The protocol is named HAM-PROTOCOL, and it allows transfering, integer, float or string, information by the two ways in all my nodes, and it's more, I can make more nodes whenever I want, and putting them to the network in a few minutes. The protocol is explained in the codes of the nodes. However it looks like this:
To request a value or a sensor or something:
<HAM-PROTOCOL,REQUEST>    
Example:  <HAM-PROTOCOL,batteryRDT.request> 
To answer that request or send information:
<HAM-PROTOCOL,CHANNEL,INT_DATA,FLOAT_DATA,STRING_DATA>
Example:  
<HAM-PROTOCOL,batteryRDT,readBattery(),0,"0">
Another example: Request and responses used in the cycle for the false piconet:
Connection:
Request:
<HAM-PROTOCOL,ready.request> 
Response:
<HAM-PROTOCOL,HAM-EXT,0,0,"OK-READY">
Gathering sensors data: Request and Response like in the examples.
Disconnection:
Request:
<HAM-PROTOCOL,HAM-EXT.requestAction.OFF> 
Response:
<HAM-PROTOCOL,HAM-EXT,0,0,"OK-OFF">
Finnaly, because HAM-BLINDS is inside of a little blind controller, it has to be very small, that's why I went for ATTINY85, and for the connectivity I decided IR using the HAM-PROTOCOL. 
## THE LIST OF SENSORS, ACTUATORS AND MORE, THAT EACH NODE HAS AND WHY 
### HAM-HUB
- TMP-36GZ: Temperature sensor. For room temperature.
- HC-SR501: IR Motion sensor. If anyone wants to touch the hub I will know...
- IR Emitter: To control HAM-BLINDS (or anything with IR).
- HC-05: In MASTER mode, to control all the Slaves.
### HAM-PEEPHOLE
- KY-01: Vibration sensor. It detects if anyone is entering or if the lock is being forced by burglars.
- LDR: Peephole darkness sensor. It detects if someone covers the peephole and blocks the video from the camera. As RPI 0 W doesn't have any analog inputs, it has a special circuit which RPI measures the time the capacitor takes to fill.
- Alarm: Loud alarm, borrowed from a old lego block I had, which RPI activates if the camera is blocked for 5 seconds, or if the vibrations are continuos.
- Webcam: Old webcam from a dead laptop. It can be any usb camera as RPI is using Motion script.
### HAM-BLINDS
- IR Receiver: To receive requests from HAM-HUB.
- Transistors: An ATTINY85 controls the motherboard of the blinds controller with transistors.
### HAM-EXT
- MQ 135: Air pollution sensor. God bless global warming!
- DHT11: Humidity/Temperature sensor. For exterior temperature/humidity.
- GY-91: Barometric pressure and altitude sensor, well altitude is not being measured (useless I think).
- KY-045: Rain sensor. It detects if it is raining / raining hard.
- LDR: Solar Illuminance sensor. Just take sun cream if this sensor says to you.
- UVM-30A: Solar UV sensor. High UV causes skin cancer.
- HC-06: Slave bluetooth for communication.
### HAM-RDT
- EMF Sensor: Electromagnetic field sensor. That radiation can cause cancer. This sensor is based on an antenna with a 3.3MOhm resistor.
- KY-05: Magnetic field sensor. Magnetic waves disturb our dreams.
- TSOP 34138: IR radiaton sensor. The atmosphere is pretty hot! Open the windows!
- SPARKFUN Electret MIC: You can already complain to your neighbor.
- HC-06: Slave bluetooth for communication.
### HAM-GAS
- MQ 2: CO2 sensor. CO2 is never good...
- MQ 5: Natural gas. At least my boiler works with it, so its good to detect any gas leak.
- MQ 6: Butane/Propane. Well I don't have a gas cooker, but my neighbors yes.
- MQ 7: CO is colorless, odorless, it doesn't taste, and last but not least it is letal.
- Thermistor: Temperature sensor. For Kitchen temperature (I run out of digital tmp sensors).
- KY-026: Fire sensor. Self-explanatory.
- HC-06: Slave bluetooth for communication.
## THE HAM APP 
It's in beta phase, so there is a bug with ham-gas displaying a "-", and the camera viewer works when it wants, but if I view it with chrome it works, I don't know why...
See https://www.hackster.io/neoxelox/ham-home-automation-and-monitoring-system-930ad4 for detailed images.
## CONCLUSION 
The nodes doesn't have an enclosure, which would be great, sadly I don't have a 3DPrinter, and I cannot use another material like wood, because of its weight. However, I have already experimented with 3D Modeling and I have made a sketch of how enclosures could be.
