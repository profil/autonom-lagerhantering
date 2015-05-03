// Motor 1, M1 på arduino Test
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

void setup() {

  for(int i = 0; i < 4; i++) {
    pinMode(pwm[i], OUTPUT);
    pinMode(dir[i], OUTPUT);
  }
  TCCR1B = TCCR1B & 0xf8 | 0x01;
  TCCR2B = TCCR2B & 0xf8 | 0x01;
}

void loop() {
  test_motor(dir1, pwm1);
  test_motor(dir2, pwm2);
  test_motor(dir3, pwm3);
  test_motor(dir4, pwm4);
} 
  
  
void test_motor(int dir, int pwm) {
  digitalWrite(dir, LOW);
  delay(500);
  analogWrite(pwm, 50);
  
  delay(1000);
  Serial.println(Omni.getCarSpeedMMPS());
  analogWrite(pwm, 0);
  digitalWrite(dir, HIGH);
  delay(500);
  analogWrite(pwm, 50);
  delay(1000);
  Serial.println(Omni.getCarSpeedMMPS());
  analogWrite(pwm, 0);
}
