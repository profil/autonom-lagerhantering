#include <AccelStepper.h>
#include <AccelStepper.h>

// Define a stepper and the pins it will use
AccelStepper stepper(1, 7, 6);

int steps = 800;
int n = -5;
int pos = n*steps;

void setup()
{  
  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(1000);
  Serial.begin(9600);
}
void loop()
{
    stepper.moveTo(pos);
    while(stepper.distanceToGo ()!=0){
      stepper.run();
    }
    Serial.println("LIFT");
    pos=-pos;
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
