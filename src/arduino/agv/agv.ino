#include <MotorWheel.h>
#include <Omni3WD.h>
#include <Omni4WD.h>
#include <PID_Beta6.h>
#include <PinChangeInt.h>
#include <PinChangeIntConfig.h> 

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

void setup() {
	// PWM Timer for pin 9 and 10
	TCCR1B = TCCR1B & 0b11111000 | 0x01;

	// PWM Timer for pin 3 and 11
	TCCR2B = TCCR2B & 0b11111000 | 0x01; 

	Omni.PIDEnable(0.31,0.01,0,10); 

	//Omni.demoActions(200, 5000, 500, false);
	Serial.begin(9600);
}

void loop() {
	Omni.setCarLeft(0);
	Omni.setCarSpeedMMPS(400,1000);

	Omni.delayMS(5000);
	Serial.println(Omni.getCarSpeedMMPS());
	Omni.delayMS(1000);
	Omni.setCarSlow2Stop(500); 
}
