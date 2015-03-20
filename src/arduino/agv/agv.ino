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

Omni4WD Omni(&wheel1, &wheel2, &wheel3, &wheel4); 

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
int speedMMPS = 100;


//------------------------------------------------------------------------------
// methods
//------------------------------------------------------------------------------
void printCom(String str){
  Serial.println(str);
}
void printCom(int i){
  Serial.println(i);
}

void ok(){
  Serial.println("OK");
}

void done(){
  Serial.println("DONE");
}

//------------------------------------------------------------------------------
void processCommand() {
  if(!strncmp(buffer,"STOP",5)){
    Omni.setCarStop(10);
    printCom("STOP");
    
  }else if(!strncmp(buffer,"PAUSE",5)){
     printCom("PAUSE");
    
  }else if(!strncmp(buffer,"FORWARD",5)){
    Omni.setCarMove(speedMMPS,0,0);
    printCom("FORWARD");
     
  }else if(!strncmp(buffer,"BACKWARD",5)){
     printCom("BACKWARD");
     
  }else if(!strncmp(buffer,"LEFT",5)){
    Omni.setCarLeft(speedMMPS);
     printCom("LEFT");
     
  }else if(!strncmp(buffer,"RIGHT",5)){
    Omni.setCarRight(speedMMPS);
     printCom("RIGHT");
     
  }else if(!strncmp(buffer,"ROTATE",6)){
     char *state = strchr(buffer,' ')+1;
     float angle = atof(state);
     printCom("ROTATE");
     printCom(angle);
     Omni.setCarRotate(angle);
     if(angle<0){
       Omni.setCarRotateLeft(speedMMPS);
     }else{
       Omni.setCarRotateRight(speedMMPS);
     }
     
  }else if(!strncmp(buffer,"LIFT",5)){
     printCom("LIFT");
     
  }else if(!strncmp(buffer,"LOWER",5)){
     printCom("LOWER");
     
  }

  /*else if(!strncmp(buffer,"test",4)) {
    // one whole number parameter
    char *state=strchr(buffer,' ')+1;
    Serial.println(state);
    if(state[0]=='0') {
      Serial.println("End test");
      test_on=0;
    } else {
      Serial.println("Start test");
      test_on=1;
    }
  } */
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
  Serial.println("Init...");
  Serial.println("Stretching...");
  sofar=0;
  Serial.println("** AWAKE **");
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
    // what if message fails/garbled?
    
    // echo confirmation
    buffer[sofar]=0;
    Serial.println(buffer);

    // do something with the command
    processCommand();
    
  
    // reset the buffer
    sofar=0;
    
    // echo completion
    Serial.println("Done.");
  }
  
  /*
	Omni.setCarLeft(0);
	Omni.setCarSpeedMMPS(400,1000);

	Omni.delayMS(5000);
	Serial.println(Omni.getCarSpeedMMPS());
	Omni.delayMS(1000);
	Omni.setCarSlow2Stop(500); 
*/
}
