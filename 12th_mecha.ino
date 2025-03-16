
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); 

Servo myservo;

#define enA 10   //H_bridge Pins   LEFT MOTOR
#define in1 9   //H_bridge Pins    FORWARD LEFT
#define in2 8  //H_bridge Pins    BACKWARD LEFT
#define in3 6  //H_bridge Pins    FORWARD RIGHT
#define in4 7   //H_bridge Pins    BACKWARD RIGHT
#define enB 5   //H_bridge Pins    RIGHT MOTOR


#define R_S A2  //IR Sensor pins
#define M_S A1  //IR Sensor pins
#define L_S A0  //IR Sensor pins


#define trig 2  //ultrasonic pins
#define echo 3  //ultrasonic pins

float duration, distance;

int flag = 0;
char x = "";

#define maxspeed 65 //100

int error = 0, P = 0, I = 0, D = 0, lastError = 0;

int maxspeed_left = 65;   //100
int maxspeed_right = 65;  //100

float KP = 90;
float KI = 0;
float KD = 30;


/////////  LCD pattern /////////////

byte a[8] = { B00001, B00011, B00111, B01110, B11110, B11111, B11111, B11111 };

byte b[8] = { B11111, B11111, B11111, B01110, B01110, B11111, B11111, B11111 };

byte c[8] = { B10000, B11000, B11100, B01110, B01111, B11111, B11111, B11111 };

byte d[8] = { B11011, B11000, B11100, B11110, B01111, B00111, B00011, B00001 };

byte e[8] = { B11111, B00000, B00000, B00000, B00000, B11111, B11111, B11111 };

byte f[8] = { B11011, B00011, B00111, B01111, B11110, B11100, B11000, B10000 };




void setup() {
 
  myservo.attach(12);

  Serial.begin(9600);

  lcd.init();  //initialize the lcd
  lcd.backlight();

  smile();  ///// Lcd

  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  pinMode(L_S, INPUT);
  pinMode(R_S, INPUT);
  pinMode(M_S, INPUT);


  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

}


void loop() {

  x = Serial.read();
  if (x == 'V') {
    if (flag == 0)
      flag = 1;
  }
  if (x == 'W') {
    if (flag == 1)
      flag = 0;
  }


  if (flag == 1)
    blue_tooth_mode();
  if (flag == 0) {

   ultrasonic();
    if (distance > 25)
      pid_control();
    else {
      Stop();
      moving_servo();
    }
  }
}

/********Functions********/

void pid_control()
{
    read_sensor_values();
	
    P = error;
    I += error;
    D = error - lastError;
    lastError = error;
    
    float correction =( P * KP + I * KI + D * KD);

    if (correction < -maxspeed) 
    {
      correction = -maxspeed; //when error -ve correction is -ve
    }
     else if (correction > maxspeed)
    {
      correction = maxspeed;  //when error +ve correction is +ve
    }
       

      // Serial.println(correction); 
    if (error >= 0 && error <= 2) //forward+RIGHTturns

    { digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);

      analogWrite(enA ,  maxspeed_left  + correction);
      analogWrite(enB ,  maxspeed_right - correction);
    }

     if (error < 0 && error >= -2) //LEFT turns

    {
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);

      analogWrite(enA ,  maxspeed_left  + correction);
      analogWrite(enB ,  maxspeed_right - correction);
    }
    
}


void read_sensor_values() {


  int left = digitalRead(L_S);
  int center = digitalRead(M_S);
  int right = digitalRead(R_S);

//when sensor on black gives 0 & ground 1

  if (left == 0 && center == 0 && right == 1)
    error = 2;

  if (left == 0 && center == 1 && right == 1)
    error = 1;

  // +ve errors car moved left

  if (left == 0 && center == 1 && right == 0)
    error = 0;

  // -ve errors car moved right

  if (left == 1 && center == 1 && right == 0)
    error = -1;

  if (left == 1 && center == 0 && right == 0)
    error = -2;

}


void blue_tooth_mode() {

  analogWrite(enA , 150);
  analogWrite(enB , 150);

  x = Serial.read();

  switch (x) {
    case 'F':
      forward();
      break;

    case 'B':
      Backward();
      break;

    case 'R':
      turnRight();
      break;

    case 'L':
      turnLeft();
      break;

    case 'S':
      Stop();
      break;
  }
}



void ultrasonic() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  duration = pulseIn(echo, HIGH);
  distance = duration * 0.0342 / 2;  //distance cm
  delay(10);
}


void moving_servo() {
  myservo.write(0);
  delay(600);
  myservo.write(90);
  delay(1000);
  myservo.write(0);
  delay(1000);
}

void smile() {
  lcd.createChar(0, a);

  lcd.createChar(1, b);

  lcd.createChar(2, c);

  lcd.createChar(3, d);

  lcd.createChar(4, e);

  lcd.createChar(5, f);

  lcd.setCursor(2, 0);
  lcd.write(byte(0));

  lcd.setCursor(3, 0);
  lcd.write(byte(1));

  lcd.setCursor(4, 0);
  lcd.write(byte(2));

  lcd.setCursor(2, 1);
  lcd.write(byte(3));

  lcd.setCursor(3, 1);
  lcd.write(byte(4));

  lcd.setCursor(4, 1);
  lcd.write(byte(5));

  lcd.setCursor(7, 0);
  lcd.print("Bonjour");
}

void forward() {
  digitalWrite(in1, HIGH);  //Right Motor forward Pin
  digitalWrite(in2, LOW);   //Right Motor backward Pin
  digitalWrite(in3, HIGH);  //Left Motor forward Pin
  digitalWrite(in4, LOW);   //Left Motor backward Pin
}

void Backward() {
  digitalWrite(in1, LOW);   //Right Motor forward Pin
  digitalWrite(in2, HIGH);  //Right Motor backward Pin
  digitalWrite(in3, LOW);   //Left Motor forward Pin
  digitalWrite(in4, HIGH);  //Left Motor backward Pin
}

void turnRight() {
  digitalWrite(in1, LOW);   //Right Motor forward Pin
  digitalWrite(in2, LOW);   //Right Motor backward Pin
  digitalWrite(in3, HIGH);  //Left Motor forward Pin
  digitalWrite(in4, LOW);   //Left Motor backward Pin
}

void turnLeft() {
  digitalWrite(in1, HIGH);  //Right Motor forward Pin
  digitalWrite(in2, LOW);   //Right Motor backward Pin
  digitalWrite(in3, LOW);   //Left Motor forward Pin
  digitalWrite(in4, LOW);   //Left Motor backward Pin
}

void Stop() {
  digitalWrite(in1, LOW);  //Right Motor forward Pin
  digitalWrite(in2, LOW);  //Right Motor backward Pin
  digitalWrite(in3, LOW);  //Left Motor forward Pin
  digitalWrite(in4, LOW);  //Left Motor backward Pin
}
