//------------------------------------------------------------------------------
// libraries
//------------------------------------------------------------------------------
#include <MotorWheel.h>
#include <Omni3WD.h>
#include <Omni4WD.h>
#include <PID_Beta6.h>
#include <PinChangeInt.h>
#include <PinChangeIntConfig.h> 

//------------------------------------------------------------------------------
// constants
//------------------------------------------------------------------------------

#define DELAY   (5)
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
char test_on=0;  // only used in processCommand as an example.

//------------------------------------------------------------------------------
// global variables
//------------------------------------------------------------------------------
unsigned int agvSpeed = 300;
int uptime = 300;
int duration = 300;
float agvRadius = sqrt(pow(Omni.getWheelspan()/2,2)*2);

int agvRotSpeed= 100; // rotationspeed during rotation
int agvRotUptime = 50; // upTime during rotation
float agvAngleTol = 0.000943396; // == 0.054 grader == minsta rotations

//------------------------------------------------------------------------------
//AGV methods
//------------------------------------------------------------------------------
void goAhead(unsigned int speedMMPS){ // Car moves advance
  if(Omni.getCarStat()!=Omni4WD::STAT_ADVANCE) 
    Omni.setCarSlow2Stop(uptime);
  Omni.setCarAdvance(0); // If the car’s state is not advance.stop it
// else moves advance continue
  Omni.setCarSpeedMMPS(speedMMPS, uptime); // Set the car speed at 300  
}

void goBack(unsigned int speedMMPS){ // Car moves advance
  if(Omni.getCarStat()!=Omni4WD::STAT_BACKOFF) 
    Omni.setCarSlow2Stop(uptime);
  Omni.setCarBackoff(0); // If the car’s state is not advance.stop it
// else moves advance continue
  Omni.setCarSpeedMMPS(speedMMPS, uptime); // Set the car speed at 300
}

void turnLeft(unsigned int speedMMPS){
  if(Omni.getCarStat()!=Omni4WD::STAT_LEFT) 
    Omni.setCarSlow2Stop(uptime);
  Omni.setCarLeft(0);
  Omni.setCarSpeedMMPS(speedMMPS, uptime);
}

void turnRight(unsigned int speedMMPS){
  if(Omni.getCarStat()!=Omni4WD::STAT_RIGHT) 
    Omni.setCarSlow2Stop(uptime);
  Omni.setCarRight(0);
  Omni.setCarSpeedMMPS(speedMMPS, uptime);
}
void rotateRight(unsigned int speedMMPS){
  if(Omni.getCarStat()!=Omni4WD::STAT_ROTATERIGHT) 
    Omni.setCarSlow2Stop(uptime);
  Omni.setCarRotateRight(0);
  Omni.setCarSpeedMMPS(speedMMPS, uptime);
}

void rotateLeft(unsigned int speedMMPS){
  if(Omni.getCarStat()!=Omni4WD::STAT_ROTATELEFT) 
    Omni.setCarSlow2Stop(uptime);
  Omni.setCarRotateLeft(0);
  Omni.setCarSpeedMMPS(speedMMPS, uptime);
}

//PI = 3.14159265359
//2PI = 6.28318530718
//PI/2 = 1.57079632679
//PI/4 = 0.78539816339

// Car rotate given angle
void rotateAngle(float angle){
  int dt = 0;
  if(abs(angle)<agvAngleTol){
    dt = 0;
  }else{
    if(angle<0){
      if(Omni.getCarStat()!=Omni4WD::STAT_ROTATELEFT) 
        Omni.setCarSlow2Stop(uptime);
      Omni.setCarRotateLeft(0);
      dt = -1000*((agvRadius*angle)/(agvRotSpeed));
    }else{
      if(Omni.getCarStat()!=Omni4WD::STAT_ROTATELEFT) 
        Omni.setCarSlow2Stop(uptime);
      Omni.setCarRotateRight(0);
      dt = 1000*((agvRadius*angle)/(agvRotSpeed));
    }
  }
  //printCom(dt);
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

//Stop all
void allStop(unsigned int speedMMPS){
  if(Omni.getCarStat()!=Omni4WD::STAT_STOP) 
    Omni.setCarSlow2Stop(uptime);
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
void processCommand() {
  // Stop agv
  if(!strncmp(buffer,"STOP",4)){
    allStop(agvSpeed);
    printCom("Stop");
    
  // Go forward
  }else if(!strncmp(buffer,"FORWARD",7)){
    goAhead(agvSpeed);
    printCom("Forward");
  // Go backward
  }else if(!strncmp(buffer,"BACKWARD",8)){
    goBack(agvSpeed);
    printCom("Backward");
    
  //Turn left
  }else if(!strncmp(buffer,"LEFT",4)){
    turnLeft(agvSpeed);
    printCom("Left");
    
  // Turn right
  }else if(!strncmp(buffer,"RIGHT",5)){
    turnRight(agvSpeed);
    printCom("Right");
    
  // Rotate given angle
  }else if(!strncmp(buffer,"ROTATE",6)){
     char *state = strchr(buffer,' ')+1;
     float angle = atof(state);
     rotateAngle(angle);
     //printComf(angle);
     printCom("Rotate");
     
  // Lift
  }else if(!strncmp(buffer,"LIFT",4)){
    Omni.switchMotors();
    printCom("LIFT");
  // Lower
  }else if(!strncmp(buffer,"LOWER",5)){
    Omni.switchMotorsReset();
     printCom("LOWER");
     
  // Set agv Speed
  }else if(!strncmp(buffer,"SETSPEED",8)){
     char *state = strchr(buffer,' ')+1;
     agvSpeed = atoi(state);
     printCom("SETSPEED");
     //printCom(agvSpeed);
     
  // Get agv State
  }else if(!strncmp(buffer,"GETSTATE",8)){
    printCom(Omni.getCarStat());
    
  // Get agv Speed
  }else if(!strncmp(buffer,"GETSPEED",8)){
    printCom(agvSpeed);
    printCom(Omni.getCarSpeedMMPS());
    
  // Demo function
  }else if(!strncmp(buffer,"TEST",4)){
    Omni.demoActions(200,5000,500,false);
    printCom("Test");
    
  // Invalid command
  }else{
    printCom("ERROR, INVALID COMMAND");
  }
}

//------------------------------------------------------------------------------
// void setup
//------------------------------------------------------------------------------

void setup() {
  // PWM Timer for pin 9 and 10
  TCCR1B = TCCR1B & 0b11111000 | 0x01;
  // PWM Timer for pin 3 and 11
  TCCR2B = TCCR2B & 0b11111000 | 0x01; 
  Omni.PIDEnable(0.31,0.01,0,10); 

  //Omni.demoActions(200, 5000, 500, false);
  Serial.begin(BAUD);
  sofar=0;
}

//------------------------------------------------------------------------------
// void loop
//------------------------------------------------------------------------------

void loop() {
  // listen for serial commands
  while(Serial.available() > 0) {
    buffer[sofar++]=Serial.read();
    if(buffer[sofar-1]==';') break;  // in case there are multiple instructions
  }
  
  // if we hit a semi-colon, assume end of instruction.
  if(sofar>0 && buffer[sofar-1]==';') {
   
    // echo confirmation
    buffer[sofar]=0;

    // do something with the command
    processCommand();
  
    // reset the buffer
    sofar=0;
    
    //Serial.print("OK");
  }
  
  // Regulate agv speed
  Omni.delayMS(duration,false);

  /*
	Omni.setCarLeft(0);
	Omni.setCarSpeedMMPS(400,1000);

	Omni.delayMS(5000);
	Serial.println(Omni.getCarSpeedMMPS());
	Omni.delayMS(1000);
	Omni.setCarSlow2Stop(500); 
*/
}
