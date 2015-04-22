//------------------------------------------------------------------------------
// libraries
//------------------------------------------------------------------------------
#include <MotorWheel.h>
#include <Omni3WD.h>
#include <Omni4WD.h>
#include <PID_Beta6.h>
#include <PinChangeInt.h>
#include <PinChangeIntConfig.h>
#include <AccelStepper.h>
//------------------------------------------------------------------------------
// RESET FUNCTION
//------------------------------------------------------------------------------
void software_Reset(){
  asm volatile(" jmp 0");
}
//------------------------------------------------------------------------------
// constants
//------------------------------------------------------------------------------

#define BAUD    (9600)
#define MAX_BUF (64)

//------------------------------------------------------------------------------
// agv config
//------------------------------------------------------------------------------

// Motor 1, M1 på arduino
int dir1 = 2;
int pwm1 = 3;

// Motor 2, M2 på arduino
int dir2 = 12;
int pwm2 = 11;

// Motor 3, M1 på IO kortet
int dir3 = 8;
int pwm3 = 9;

// Motor 4, M2 på IO kortet
int dir4 = 7;
int pwm4 = 10;

int pwm[] = {pwm1, pwm2, pwm3, pwm4};
int dir[] = {dir1, dir2, dir3, dir4};

irqISR(irq1,isr1);
MotorWheel wheel1(pwm1, dir1, 4, 5, &irq1);

irqISR(irq2,isr2);
MotorWheel wheel2(pwm2, dir2, 14, 15, &irq2);

irqISR(irq3,isr3);
MotorWheel wheel3(pwm3, dir3, 16, 17, &irq3);

irqISR(irq4,isr4);
MotorWheel wheel4(pwm4, dir4, 18, 19, &irq4);

// wrong definition according to forward, backward, left,right
//Omni4WD Omni(&wheel1, &wheel2, &wheel3, &wheel4); 

Omni4WD Omni(&wheel4, &wheel3, &wheel2, &wheel1);

//------------------------------------------------------------------------------
// serial config
//------------------------------------------------------------------------------
char buffer[MAX_BUF];
int sofar;
boolean stopp = false;

//------------------------------------------------------------------------------
// Stepper config
//------------------------------------------------------------------------------
// ** Possible to change via labview
int liftDist = 30; //mm
// **
int stepPin = 13;
int dirPin = 6;
int rev = 800;

int liftPerRev = 4; //mm
int steps = rev*(liftDist/liftPerRev);
boolean lifted = false;

// Stepper init
AccelStepper stepper(1,stepPin,dirPin); 


//------------------------------------------------------------------------------
// global variables
//------------------------------------------------------------------------------
// ** Possible to change via Labview
unsigned int agvSpeed = 200;
int uptime = 150;
int duration = 100;
// **

// FIX in arduino
unsigned int agvSpeeds = 75;
float agvRadius = sqrt(pow(Omni.getWheelspan()/2,2)*2);
int agvAdjTime = 10;
unsigned int agvAdjSpeed = 75;
int agvAdjUptime = 150;

int agvRotSpeed= 100; // rotationspeed during rotation
int agvRotUptime = 50; // upTime during rotation

//------------------------------------------------------------------------------
//AGV methods
//------------------------------------------------------------------------------
void goForward(unsigned int speedMMPS){ // Car moves advance
  if(Omni.getCarStat()!=Omni4WD::STAT_ADVANCE) 
    Omni.setCarSlow2Stop(uptime);
  Omni.setCarAdvance(0); 
  Omni.setCarSpeedMMPS(speedMMPS, uptime);
}

void goBack(unsigned int speedMMPS){ // Car moves advance
  if(Omni.getCarStat()!=Omni4WD::STAT_BACKOFF) 
    Omni.setCarSlow2Stop(uptime);
  Omni.setCarBackoff(0); 
  Omni.setCarSpeedMMPS(speedMMPS, uptime);
}

void goLeft(unsigned int speedMMPS){
  if(Omni.getCarStat()!=Omni4WD::STAT_LEFT) 
    Omni.setCarSlow2Stop(uptime);
  Omni.setCarLeft(0);
  Omni.setCarSpeedMMPS(speedMMPS, uptime);
}

void goRight(unsigned int speedMMPS){
  if(Omni.getCarStat()!=Omni4WD::STAT_RIGHT) 
    Omni.setCarSlow2Stop(uptime);
  Omni.setCarRight(0);
  Omni.setCarSpeedMMPS(speedMMPS, uptime);
}

void rotateRight(unsigned int speedMMPS){
  if(Omni.getCarStat()!=Omni4WD::STAT_ROTATERIGHT) 
    Omni.setCarSlow2Stop(uptime);
  Omni.setCarRotateRight(0);
  Omni.setCarSpeedMMPS(speedMMPS, agvRotUptime);
}

void rotateLeft(unsigned int speedMMPS){
  if(Omni.getCarStat()!=Omni4WD::STAT_ROTATELEFT) 
    Omni.setCarSlow2Stop(uptime);
  Omni.setCarRotateLeft(0);
  Omni.setCarSpeedMMPS(speedMMPS, agvRotUptime);
}

/* 
  PI = 3.14159265359
  2PI = 6.28318530718
  PI/2 = 1.57079632679
  PI/4 = 0.78539816339
*/

// Car rotate given angle, +clockwise -counterclockwise [rad]
void rotateAngle(float angle){
  int dt = 0;
    if(angle<0){
      if(Omni.getCarStat()!=Omni4WD::STAT_ROTATELEFT) 
        Omni.setCarSlow2Stop(uptime);
      Omni.setCarRotateLeft(0);
    }else{
      if(Omni.getCarStat()!=Omni4WD::STAT_ROTATERIGHT) 
        Omni.setCarSlow2Stop(uptime);
      Omni.setCarRotateRight(0);
    }
  dt = 1000*((agvRadius*abs(angle))/(agvRotSpeed));
  Omni.setCarSpeedMMPS(agvRotSpeed, agvRotUptime);
  Omni.delayMS(dt,false);
  rotStop(agvRotSpeed);
}

// Stop rotation, note agvRotUptime
void rotStop(unsigned int speedMMPS){
  if(Omni.getCarStat()!=Omni4WD::STAT_STOP) 
    Omni.setCarSlow2Stop(agvRotUptime);
  Omni.setCarStop();
}

// Car move +forward -backward [mm]
void moveDistanceFB(float dist){
  int dt = 0;
  if(dist<0){
     if(Omni.getCarStat()!=Omni4WD::STAT_BACKOFF) 
       Omni.setCarSlow2Stop(uptime);
     Omni.setCarBackoff(0);
  }else{
    if(Omni.getCarStat()!=Omni4WD::STAT_ADVANCE) 
      Omni.setCarSlow2Stop(uptime);
    Omni.setCarAdvance(0);
  }
  dt = (1000*abs(dist)/agvAdjSpeed) - agvAdjTime;
  Omni.setCarSpeedMMPS(agvAdjSpeed, agvAdjTime);
  Omni.delayMS(dt,false);
  allStop(agvAdjTime,agvAdjTime);
}

// move distance +right -left [mm]
void moveDistanceRL(float dist){
  int dt = 0;
  if(dist<0){
     if(Omni.getCarStat()!=Omni4WD::STAT_LEFT) 
       Omni.setCarSlow2Stop(uptime);
     Omni.setCarLeft(0);
  }else{
    if(Omni.getCarStat()!=Omni4WD::STAT_RIGHT) 
      Omni.setCarSlow2Stop(uptime);
    Omni.setCarRight(0);
  }
  dt = (1000*abs(dist)/agvAdjSpeed) - agvAdjTime;    
  Omni.setCarSpeedMMPS(agvAdjSpeed, agvAdjTime);
  Omni.delayMS(dt,false);
  allStop(agvAdjSpeed,agvAdjTime);
}

// Car ADJUST rotate given angle, +clockwise -counterclockwise [rad]
void adjRotateAngle(float angle){
  int dt = 0;
     if(angle<0){
      if(Omni.getCarStat()!=Omni4WD::STAT_ROTATELEFT) 
        Omni.setCarSlow2Stop(uptime);
      Omni.setCarRotateLeft(0);
    }else{
      if(Omni.getCarStat()!=Omni4WD::STAT_ROTATERIGHT) 
        Omni.setCarSlow2Stop(uptime);
      Omni.setCarRotateRight(0);
    }
  dt = 1000*((agvRadius*abs(angle))/(agvAdjSpeed));
  Omni.setCarSpeedMMPS(agvAdjSpeed, agvAdjTime);
  Omni.delayMS(dt,false);
  rotStop(agvAdjSpeed);
}

//Stop all
void allStop(unsigned int speedMMPS, int upTime){
  if(Omni.getCarStat()!=Omni4WD::STAT_STOP) 
    Omni.setCarSlow2Stop(upTime);
  Omni.setCarStop();
}
//------------------------------------------------------------------------------
// Development methods
//------------------------------------------------------------------------------
void printCom(String str){
  Serial.println(str);
}
void printCom(int i){
  Serial.println(i);
}
void printComf(float i){
  Serial.println(i);
}

//------------------------------------------------------------------------------
// Method to compare serial commands
void processCommand(){
  // Stop agv
  if(!strncmp(buffer,"STOP",4)){
    allStop(agvSpeed, uptime);
    printCom("Stop");
    
  // Go forward
  }else if(!strncmp(buffer,"FORWARD",7)){
    goForward(agvSpeed);
    printCom("Forward");
    
  // Go backward
  }else if(!strncmp(buffer,"BACKWARD",8)){
    goBack(agvSpeed);
    printCom("Backward");
    
  // Go left
  }else if(!strncmp(buffer,"LEFT",4)){
    goLeft(agvSpeed);
    printCom("Left");
    
  // Go right
  }else if(!strncmp(buffer,"RIGHT",5)){
    goRight(agvSpeed);
    printCom("Right");
    
  // Search forward
  }else if(!strncmp(buffer,"SFORWARD",8)){
    goForward(agvSpeeds);
    printCom("sForward");
    
  // Search backward
  }else if(!strncmp(buffer,"SBACKWARD",9)){
    goBack(agvSpeeds);
    printCom("sBackward");
    
  // Search left
  }else if(!strncmp(buffer,"SLEFT",5)){
    goLeft(agvSpeeds);
    printCom("sLeft");
    
  // Search right
  }else if(!strncmp(buffer,"SRIGHT",6)){
    goRight(agvSpeeds);
    printCom("sRight");
        
  // Rotate given angle, +clockwise -counterclockwise [rad]
  }else if(!strncmp(buffer,"ROTATE",6)){
     char *state = strchr(buffer,' ')+1;
     float angle = atof(state);
     rotateAngle(angle);
     printCom("Rotate");
     
  // Adjust given distance +forward -backward [mm]
  }else if(!strncmp(buffer,"ADJFB",5)){
     char *state = strchr(buffer,' ')+1;
     float dist = atof(state);
     moveDistanceFB(dist);
     printCom("moveDistanceFB");
     
  // Adjust given distance +right -left [mm]
  }else if(!strncmp(buffer,"ADJRL",5)){
     char *state = strchr(buffer,' ')+1;
     float dist = atof(state);
     moveDistanceRL(dist);
     printCom("moveDistanceRL");
     
  // Adjust given angle rotate +clockwise -counterclockwise [rad]
  }else if(!strncmp(buffer,"ADJROT",6)){
     char *state = strchr(buffer,' ')+1;
     float angle = atof(state);
     adjRotateAngle(angle);
     printCom("adjRotate");
     
  // Lift
  }else if(!strncmp(buffer,"LIFT",4)){
    steps = rev*(liftDist/liftPerRev);
    if(!lifted){
      stepper.moveTo(-steps);
      while(stepper.distanceToGo()!=0){
        stepper.run();
      }
    lifted = true;
    printCom("LIFT");
  }
  
  // Lower
  }else if(!strncmp(buffer,"LOWER",5)){
    steps = rev*(liftDist/liftPerRev);
    if(lifted){
      stepper.moveTo(steps);
      while(stepper.distanceToGo()!=0){
        stepper.run();
      }
      lifted = false;
      printCom("LOWER");
    }
  // Set agv Speed
  }else if(!strncmp(buffer,"SETSPEED",8)){
     char *state = strchr(buffer,' ')+1;
     agvSpeed = atoi(state);
     printCom("SETSPEED");
     
  // Set agv uptime
  }else if(!strncmp(buffer,"SETUPTIME",9)){
     char *state = strchr(buffer,' ')+1;
     uptime = atoi(state);
     printCom("SETUPTIME");
     
  // Set agv Regulate
  }else if(!strncmp(buffer,"SETREGULATE",11)){
     char *state = strchr(buffer,' ')+1;
     duration = atoi(state);
     printCom("SETREGULATE");
     
  // Set liftdist
  }else if(!strncmp(buffer,"HIGHT",5)){
     char *state = strchr(buffer,' ')+1;
     liftDist = atoi(state);
     printCom("HIGHT");
     
  // Get agv parameters
  }else if(!strncmp(buffer,"GETPARA",7)){
    printCom(liftDist);
    printCom(duration);
    printCom(uptime);
    printCom(agvSpeed);
    
  // Get agv Speed
  }else if(!strncmp(buffer,"GETSPEED",8)){
    printCom(Omni.getCarSpeedMMPS());
    
  // Software RESET function
  }else if(!strncmp(buffer,"RESET",5)){
    printCom("ARDUINO RESET");
    software_Reset();
    
  // Invalid command
  }else{
    printCom("ERROR, INVALID COMMAND");
  }
}
    //Omni.demoActions(200,5000,500,false);

//------------------------------------------------------------------------------
// void setup
//------------------------------------------------------------------------------

void setup() {
  // PWM Timer for pin 9 and 10
  TCCR1B = TCCR1B & 0b11111000 | 0x01;
  // PWM Timer for pin 3 and 11
  TCCR2B = TCCR2B & 0b11111000 | 0x01; 
  // Enable PID regulator (KP,KI,KD) or (TauP,TauI,TauD) ?
  Omni.PIDEnable(0.31,0.01,0,10); 
  // Start serial port
  Serial.begin(BAUD);
  sofar=0;
  // Set stepper parameters
  stepper.setMaxSpeed(8000);
  stepper.setAcceleration(1000);
}

//------------------------------------------------------------------------------
// void loop
//------------------------------------------------------------------------------

void loop() {
  // listen for serial commands
  while(Serial.available() > 0) {
    buffer[sofar++]=Serial.read();
    if(buffer[sofar-1]==';') 
      break;  // in case there are multiple instructions
  }
  // if we hit a semi-colon, assume end of instruction.
  if(sofar>0 && buffer[sofar-1]==';') {
   
    // echo confirmation
    buffer[sofar]=0;

    // do something with the command
    processCommand();
  
    // reset the buffer
    sofar=0;
  }
  // Regulate agv speed
  Omni.delayMS(duration,false);
}
