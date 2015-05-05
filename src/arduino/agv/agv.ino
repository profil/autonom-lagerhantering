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
Omni4WD Omni(&wheel1, &wheel2, &wheel3, &wheel4); 

//Omni4WD Omni(&wheel4, &wheel3, &wheel2, &wheel1);

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
int liftDist = 40; //mm
// **
int stepPin = 13; // röd
int dirPin = 6;   // gul. gul gnd, svart 0 RX, vit 1 TX
int rev = 800;
boolean initLift = true;
int liftPerRev = 4; //mm
int steps = rev*(liftDist/liftPerRev);
boolean lifted = false;

// Stepper init
AccelStepper stepper(1,stepPin,dirPin); 


//------------------------------------------------------------------------------
// global variables
//------------------------------------------------------------------------------
// ** Possible to change via Labview
int duration = 100;

// Parameters when going F,B,R,L
unsigned int agvSpeed = 200;
int uptime = 150;
// **

// FIX in arduino
float agvRadius = sqrt(pow(Omni.getWheelspan()/2,2)*2);

// Parameters when searching for QR
unsigned int agvSpeeds = 75;
int agvUptimes = 150;

// Parameters when adjusting accuracy F,B,R,L
unsigned int agvAdjSpeed = 75;
int agvAdjTime = 10;

// Parameters when adjusting accuracy Rotation
int agvRotSpeed= 100;
int agvRotUptime = 25;

//------------------------------------------------------------------------------
//AGV methods
//------------------------------------------------------------------------------
void goForward(unsigned int speedMMPS, int upTime){ // Car moves advance
  if(Omni.getCarStat()!=Omni4WD::STAT_ADVANCE) 
    Omni.setCarSlow2Stop(upTime);
  Omni.setCarAdvance(0); 
  Omni.setCarSpeedMMPS(speedMMPS, upTime);
}

void goBack(unsigned int speedMMPS, int upTime){ // Car moves advance
  if(Omni.getCarStat()!=Omni4WD::STAT_BACKOFF) 
    Omni.setCarSlow2Stop(upTime);
  Omni.setCarBackoff(0); 
  Omni.setCarSpeedMMPS(speedMMPS, upTime);
}

void goLeft(unsigned int speedMMPS, int upTime){
  if(Omni.getCarStat()!=Omni4WD::STAT_LEFT) 
    Omni.setCarSlow2Stop(upTime);
  Omni.setCarLeft(0);
  Omni.setCarSpeedMMPS(speedMMPS, upTime);
}

void goRight(unsigned int speedMMPS, int upTime){
  if(Omni.getCarStat()!=Omni4WD::STAT_RIGHT) 
    Omni.setCarSlow2Stop(upTime);
  Omni.setCarRight(0);
  Omni.setCarSpeedMMPS(speedMMPS, upTime);
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
        Omni.setCarSlow2Stop(agvRotUptime);
      Omni.setCarRotateLeft(0);
    }else{
      if(Omni.getCarStat()!=Omni4WD::STAT_ROTATERIGHT) 
        Omni.setCarSlow2Stop(agvRotUptime);
      Omni.setCarRotateRight(0);
    }
  dt = 1000*(((agvRadius*abs(angle)))/(agvRotSpeed));//-agvRotUptime;
  Omni.setCarSpeedMMPS(agvRotSpeed, agvRotUptime);
  Omni.delayMS(dt,false);
  rotStop(agvRotSpeed, agvRotUptime);
  //printCom(dt);
  //printCom(agvRadius);
}

// Stop rotation, note agvRotUptime
void rotStop(unsigned int speedMMPS, int upTime){
  if(Omni.getCarStat()!=Omni4WD::STAT_STOP) 
    Omni.setCarSlow2Stop(upTime);
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
  
  dt = ((1000*abs(dist))/agvAdjSpeed) - agvAdjTime;
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
  dt = 1000*((agvRadius*abs(angle))/(agvAdjSpeed))-agvAdjTime;
  Omni.setCarSpeedMMPS(agvAdjSpeed, agvAdjTime);
  Omni.delayMS(dt,false);
  rotStop(agvAdjSpeed,agvAdjTime);
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
void printDONE(){
  Serial.print("DONE");
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
    printDONE();
    //printCom("Stop");
    
  // Go forward
  }else if(!strncmp(buffer,"FORWARD",7)){
    goForward(agvSpeed,uptime);
    printDONE();
//    printCom("Forward");
    
  // Go backward
  }else if(!strncmp(buffer,"BACKWARD",8)){
    goBack(agvSpeed,uptime);
    printDONE();

    //printCom("Backward");
    
  // Go left
  }else if(!strncmp(buffer,"LEFT",4)){
    goLeft(agvSpeed,uptime);
    printDONE();
//    printCom("Left");
    
  // Go right
  }else if(!strncmp(buffer,"RIGHT",5)){
    goRight(agvSpeed,uptime);
    printDONE();
    //printCom("Right");
    
  // Search forward
  }else if(!strncmp(buffer,"SFORWARD",8)){
    goForward(agvSpeeds,agvUptimes);
    printDONE();
    //printCom("sForward");
    
  // Search backward
  }else if(!strncmp(buffer,"SBACKWARD",9)){
    goBack(agvSpeeds,agvUptimes);
    printDONE();
    //printCom("sBackward");
    
  // Search left
  }else if(!strncmp(buffer,"SLEFT",5)){
    goLeft(agvSpeeds,agvUptimes);
    printDONE();
    //printCom("sLeft");
    
  // Search right
  }else if(!strncmp(buffer,"SRIGHT",6)){
    goRight(agvSpeeds,agvUptimes);
    printDONE();
    //printCom("sRight");
        
  // Rotate given angle, +clockwise -counterclockwise [rad]
  }else if(!strncmp(buffer,"ROTATE",6)){
     char *state = strchr(buffer,' ')+1;
     float angle = atof(state);
     rotateAngle(angle);
     printDONE();
     //printCom("Rotate");
     
  // Adjust given distance +forward -backward [mm]
  }else if(!strncmp(buffer,"ADJFB",5)){
     char *state = strchr(buffer,' ')+1;
     float dist = atof(state);
     moveDistanceFB(dist);
     printDONE();
     //printCom("moveDistanceFB");
     
  // Adjust given distance +right -left [mm]
  }else if(!strncmp(buffer,"ADJRL",5)){
     char *state = strchr(buffer,' ')+1;
     float dist = atof(state);
     moveDistanceRL(dist);
     printDONE();//printCom("moveDistanceRL");
     
  // Adjust given angle rotate +clockwise -counterclockwise [rad]
  }else if(!strncmp(buffer,"ADJROT",6)){
     char *state = strchr(buffer,' ')+1;
     float angle = atof(state);
     adjRotateAngle(angle);
     printDONE();//printCom("adjRotate");
     
  // Lift
  }else if(!strncmp(buffer,"LIFT",4)){
    steps = rev*(liftDist/liftPerRev);
    if(!lifted){
      stepper.move(-steps);
      int i = 0;
      while(stepper.distanceToGo()!=0){
        stepper.run();
        if(i==500){
          Omni.delayMS(2,false);
          i=0;
        }
        i++;
      }
      lifted = true;
    }
  printDONE();//printCom("LIFT");
  // Lower
  }else if(!strncmp(buffer,"LOWER",5)){
    steps = rev*(liftDist/liftPerRev);
    if(lifted){
      stepper.move(steps);
      int i = 0;
      while(stepper.distanceToGo()!=0){
        stepper.run();
        if(i==500){
          Omni.delayMS(2,false);
          i=0;
        }
        i++;
      }
      lifted = false;
    }
    printDONE();//printCom("LOWER");

  // Set agv Speed
  }else if(!strncmp(buffer,"SETSPEED",8)){
     char *state = strchr(buffer,' ')+1;
     agvSpeed = atoi(state);
     printDONE();//printCom("SETSPEED");
     
  // Set agv uptime
  }else if(!strncmp(buffer,"SETUPTIME",9)){
     char *state = strchr(buffer,' ')+1;
     uptime = atoi(state);
     printDONE();//printCom("SETUPTIME");
     
  // Set agv Regulate
  }else if(!strncmp(buffer,"SETREGULATE",11)){
     char *state = strchr(buffer,' ')+1;
     duration = atoi(state);
     printDONE();//printCom("SETREGULATE");
     
  // Set liftdist
  }else if(!strncmp(buffer,"HIGHT",5)){
     char *state = strchr(buffer,' ')+1;
     liftDist = atoi(state);
     printDONE();//printCom("HIGHT");
     
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
    Serial.print("ERROR");//printCom("ERROR, INVALID COMMAND");
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
