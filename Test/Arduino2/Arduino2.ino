#include <SoftwareSerial.h>// Import Software Serial library

#define Intergas_TX D6
#define Intergas_RX D7

SoftwareSerial InterGasSerial(Intergas_RX, Intergas_TX); // RX, TX

void setup() {
  // Open serial communications and wait for port to open:
  Serial        .begin(9600);
  InterGasSerial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println(        "I am arduino 2");
}

void loop() { // run over and over
// Sent to port D6 D7
   InterGasSerial.print("Send from Arduino 2 to 1\r\n");
  
// Recieve to port D6 D7  
  while (InterGasSerial.available() > 0) {
    //Serial.println("readed");
    char inByte = InterGasSerial.read();
    Serial.print(inByte);
  }
  delay(1000);
}
