#include <Stepper.h>

#include <AccelStepper.h>

// Define a stepper and the pins it will use
// (MODE,stepPin,dirPin)
AccelStepper stepper(1, 7, 6);
int sleep = 5;


int steps = 200;
int n = 5;
int fullStepp = 1;  // MS1 == 0 && MS2 == 0
int halfStepp = 2;  // MS1 == 1 && MS2 == 0
int fourStepp = 4;  // MS1 == 0 && MS2 == 1
int eightStepp = 8;  // MS1 == 1 && MS2 == 1
int pos = n*steps*eightStepp;
boolean slept = false;

char buffer[64];
int sofar;

void setup()
{  
  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(1000);
  Serial.begin(9600);
  pinMode(sleep,OUTPUT);
  sofar = 0;
  digitalWrite(sleep,HIGH);
  stepper.moveTo(-pos);
  while(stepper.distanceToGo ()!=0){
    stepper.run();
  }
}
void loop()
{
  while(Serial.available() > 0) {
    buffer[sofar++]=Serial.read();
    if(buffer[sofar-1]==';') break;  // in case there are multiple instructions
  }
  
  // if we hit a semi-colon, assume end of instruction.
  if(sofar>0 && buffer[sofar-1]==';') {
   
    // echo confirmation
    buffer[sofar]=0;

    // do something with the command
    if(!strncmp(buffer,"LIFT",4)){
      digitalWrite(sleep,HIGH);
      stepper.moveTo(-pos);
      while(stepper.distanceToGo ()!=0){
        stepper.run();
      }
      Serial.println("LIFTED");
      digitalWrite(sleep,LOW);
    
    }else if(!strncmp(buffer,"LOWER",4)){
      digitalWrite(sleep,HIGH);
      stepper.moveTo(pos);
      while(stepper.distanceToGo ()!=0){
        stepper.run();
      }
      Serial.println("LOWERED");
      digitalWrite(sleep,LOW);
    
    }else{ 
    }
    // reset the buffer
    sofar=0;
    
    //Serial.print("OK");
    
  }
}
/*AccelStepper stepper(1, 7, 6); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
int laps = 10;
void setup()
{  
  stepper.setMaxSpeed(8000);
  
}
void loop()
{
  // Read new position
  
  int analog_in = 1600;
  stepper.moveTo(analog_in*laps);
  stepper.setSpeed(8000);
  stepper.runSpeedToPosition();
  if(stepper.distanceToGo() == 0){ 
    laps = -laps;
      delay(10);
}
}
*//*void setup()
{  
  stepper.setMaxSpeed(1500);
  stepper.setAcceleration(1000);
}
void loop()
{    
  stepper.moveTo(500);
  while (stepper.currentPosition() != 300) // Full speed up to 300
    stepper.run();
  stepper.stop(); // Stop as fast as possible: sets new target
  stepper.runToPosition(); 
  // Now stopped after quickstop
  // Now go backwards
  stepper.moveTo(-500);
  while (stepper.currentPosition() != 0) // Full speed basck to 0
    stepper.run();
  stepper.stop(); // Stop as fast as possible: sets new target
  stepper.runToPosition(); 
  // Now stopped after quickstop
}
*/
// Define a stepper and the pins it will use
/*AccelStepper stepper(1, 7, 6);
int steps = 800;
int n = 20;
int pos = n*steps;

void setup()
{  
  stepper.setMaxSpeed(8000);
  stepper.setAcceleration(1000);
}

void loop()
{
      stepper.moveTo(pos);

  if (stepper.distanceToGo() == 0)
  {
    delay(500);
    pos = -pos;
  }
  stepper.runSpeed();
}*/
