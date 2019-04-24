//Included Library
#include <SoftwareSerial.h>

//Defining of initial variables for the motors
int leftMotor[] = {6, 5};
int rightMotor[] = {10, 11};
int motorEnable[] = {3, 9};
int motorPower[] = {212.5, 255};
//Defines the software serial which is used so we can monitor what messages are being sent
SoftwareSerial mySerial(13, 12); // RX, TX

//Triggered once at start
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  //Tell the pins what they are used for 
  pinMode(leftMotor[0], OUTPUT);
  pinMode(leftMotor[1], OUTPUT);
  pinMode(rightMotor[0], OUTPUT);
  pinMode(rightMotor[1], OUTPUT);
  // set the data rate for the SoftwareSerial port
  mySerial.begin(4800);
  mySerial.println("");
}
//Drives forward or stops based on what is read from the software serial, also send message to Serial so we can see whether the if statement is triggered
void loop() { // run over and over
  if(char(mySerial.read()) == 'Z'){
    drive();
    Serial.println("Going Forward");
  } else if (char(mySerial.read()) == 'X') {
    halt();
    Serial.println("Stopping");
  } 
}

//Drive forward and Stop Methods
void drive()
{
  analogWrite(motorEnable[0], 200);
  digitalWrite(leftMotor[0], HIGH);
  digitalWrite(leftMotor[1], LOW);

  analogWrite(motorEnable[1], 200);
  digitalWrite(rightMotor[0], HIGH);
  digitalWrite(rightMotor[1], LOW);  
}
void halt()
{
  digitalWrite(leftMotor[0], LOW);
  digitalWrite(leftMotor[1], LOW);
  digitalWrite(rightMotor[0], LOW);
  digitalWrite(rightMotor[1], LOW);  

}
