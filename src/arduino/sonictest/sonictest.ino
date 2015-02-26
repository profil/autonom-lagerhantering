#include <SoftwareSerial.h>
#include <SONAR.h>

//SONAR s11 = SONAR(0x11);

// RX TX
SoftwareSerial myRIO(8, 9);

void setup() {
	/*
	SONAR::init();
	delay(100);
	*/

	myRIO.begin(9600);
	myRIO.println("HEJSAN!");

	Serial.begin(9600);
	Serial.println("Starting");
}

void loop() {
	/*
	s11.trigger();

	delay(SONAR::duration);

	s11.getDist();
	delay(500);
	*/

	if(myRIO.available()) {
		Serial.write(myRIO.read());
	}
	if(Serial.available()) {
		myRIO.write(Serial.read());
	}
} 
