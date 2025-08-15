# InterGasGateway
Arduino (esp8266) Hardware/Software to monitor a Intergas boiler over rs232 interface (x5) of the intergas boiler to Domotica Software with the use of MQTT.

# What to expect
This project consists of:
-  Hardware -> Cable diagram of between the IntergasGateway and the boiler
-  Hardware -> IntergasGateway Schemetic based on KiKad
-  Hardware -> IntergasGateway PCB based on KiKad
-  Software -> IntergasGateway sketch to be placed on arduino eps8266

# Hardware
A few requirements, set by myself, according to the hardware:
- Should be able to run paricitair on the boiler x5 PCB connector (without external power)
- High input RX/TX impedance
- RX/TX gavanically separated
- PCB should fitted in housing
- All parts can be ordered at Aliexpress
- PCb Easy to be ordered
- Easy to build

# Software
A few requirements, set by myself, according to the software:
- Code should easy to read and logically setup
- Code should be expandable with ic3 burner
- Good serial debugging possibilities
- Output of retrieved data should be pushed to the serial port
- Output of retrieved data should be pushed to a mqtt listner (domotica software independent)
- Scheduled retrievel of data should be possible
- Reading the serial port should be 

# Used tools
- Visual Studio code + platrformIO
- KiCad
- MQTT Explorer
- C++ decompiler

# General requirements
- Based on opensource
- Robust
- Well documented
- Low cost
- Components easy to find and easy to order
- Reproducable

# Good to know
- Intergas has it's own software (IDS) for monitoring the X5 port of the boiler. You can download the original software at this link (https://www.intergas-verwarming.nl/zakelijk/download-ids-software/). The only thing you have to do is registrer yourself. The software was extremly usefull during the development of the arduino software.


# Help needed
I own a intergas boiler of type ic2 so I was able to develop the arduino parse code for the ic2 telegrams. I want to add and test also the ic3 telegrams therefor I need someone to test for me the arduino code. 
