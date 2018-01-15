/*****************************************************************************
 *  Name: Gregory Pope, Serena Harden, Arya Stevinson                        *
 *  Course: ICS4U                                                            *  
 *  Date: January 14, 2018                                                   * 
 *                                                                           * 
 *  Purpose: Program runs on arduino car. Connects to bluetooth to run car   *
 *  manually, or autonomously explore a room.                                *                                                        *  
 *****************************************************************************/
 
//Calculations in the following gyroscope functions:
//getGyroVal(), calibrateGyro(), timePassed(), updateHeadings()
//Are taken from an example gyroscope code uploaded by Jim Bourke at:
//https://forum.arduino.cc/index.php?topic=147351.0

#include <Wire.h>

#define carLength 26

//Registers for sending data to gyroscope
#define  CTRL_REG1  0x20
#define  CTRL_REG3  0x22
#define  CTRL_REG4  0x23

//Motor pins
#define rPin1 10
#define rPin2 11
#define lPin1 6
#define lPin2 9

//Ultrasonic sensor pins
#define trig 5
#define echo 4
#define trig2 8
#define echo2 7

//Suggested numbers for gyro calibration, taken from Jim Bourke as mentioned above
#define  NUM_GYRO_SAMPLES  50
#define  GYRO_SIGMA_MULTIPLE  3

//Ultrasonic sensor variables
double fDis; //For storing distance calculated from front sensor
double sDis; // "                              from side sensor
int spaceL, spaceW;

//Gyro variables
int gyroAddress = 105; //Address for the particular gyro in i2c mode
int gyroRaw;
double gyroDPS; //Degrees per second
float heading = 0.0f;
int gyroZeroRate; //Calibration data, since sensor does not centre at zero
int gyroThreshold; //Change data less than the threshold is discarded to prevent drift
float dpsPerDigit = .00875f; //Number for conversion to degrees per second

//Bluetooth variables
char junk;
String inputString = "";

void setup() {
  //Setting pin modes
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);
  pinMode(rPin1, OUTPUT);
  pinMode(rPin2, OUTPUT);
  pinMode(lPin1, OUTPUT);
  pinMode(lPin2, OUTPUT);
  pinMode(13, OUTPUT);

  Wire.begin();
  setupGyro();

  Serial.begin(9600);
}

void loop() {
  //Loops to recieve bluetooth communication
  if (Serial.available()) {
    //Reads in bluetooth data
    while (Serial.available())
    {
      char inChar = (char)Serial.read();
      inputString += inChar;
    }

    //Clears serial buffer
    while (Serial.available() > 0) {
      junk = Serial.read() ;
    }
    if (inputString == "S") {    //Starting automatic exploration
      outerWalls(); //Traces walls
      innerSpace(); //Travels through inner space
    } else if (inputString == "F") { //in case of 'F' move forward
      forward();
    } else if (inputString == "B") { //incase of 'B' move backwards
      analogWrite(rPin1, 150);
      analogWrite(rPin2, 0);
      analogWrite(lPin1, 150);
      analogWrite(lPin2, 0);
    } else if (inputString == "L") { //incase of 'L' turn left
      analogWrite(lPin1, 255);
      analogWrite(lPin2, 0);
      analogWrite(rPin1, 0);
      analogWrite(rPin2, 200);
    } else if (inputString == "R") { //incase of 'R' turn right
      analogWrite(lPin1, 0);
      analogWrite(lPin2, 200);
      analogWrite(rPin1, 255);
      analogWrite(rPin2, 0);
    } else if (inputString == "E") { //incase of 'E' turn the motors off
      stopCar();
    }
    inputString = "";
  }
}

//Traces walls of a rectangular room
void outerWalls() {
  //Gets length (takes average of three for accuracy)
  fDis = (getDistance(trig, echo) + getDistance(trig, echo) + getDistance(trig, echo)) / 3 + carLength;
  spaceL = fDis;
  forward();

  //For each wall
  for (int i = 0; i < 4; i++) {
    double startingSDis = (getDistance(trig2, echo2) + getDistance(trig2, echo2)) / 2;

    //Continue forward until reaching a wall or reaching an object
    do {
      delay(50);
      sDis = getDistance(trig2, echo2);

      //Checks if there is no longer a wall on the right
      if (sDis > startingSDis + 20) {
        delay(100);
        sDis = getDistance(trig2, echo2);

        //Checks a second time in case of error
        if (sDis > startingSDis + 20) {
          i--;

          //Delay gives room to turn
          delay(300);
          turnRight();

          //Continues forward until reaching a wall on its right side
          do {
            delay(50);
            sDis = getDistance(trig2, echo2);
          } while (sDis > 20);

          //Continues forward until wall ends again
          do {
            delay(50);
            sDis = getDistance(trig2, echo2);
          } while (sDis < 20);

          //Delay gives room to turn
          delay(300);
          turnRight();

          //Continues forward until reaching wall in front of it
          do {
            delay(50);
            fDis = getDistance(trig, echo);
          } while (fDis > 10);
          turnLeft();
        }
      }

      fDis = getDistance(trig, echo);
    } while (fDis > 10);
    turnLeft();

    //On first turn, stops to get width data
    if (i == 0) {
      stopCar();
      spaceW = (getDistance(trig, echo) + getDistance(trig, echo) + getDistance(trig, echo)) / 3 + carLength;
      forward();
    }
  }

  stopCar();
}

//Travels through space in zigzag pattern
//(Forward, left to wall, forward, right to wall, etc)
void innerSpace() {
  //Number of rows to follow in zigzag pattern
  //Removes car's length from total distance and divides by car's length
  int rows = (spaceL - carLength) / carLength - 1;

  for (int i = 0; i < rows; i++) {
    //Gets distance to wall
    stopCar();
    double distance = (getDistance(trig, echo) + getDistance(trig, echo) + getDistance(trig, echo)) / 3;
    forward();

    //Moves forward 26 cm
    //Checks if distance - carLength is less than zero, in which case an object is in the way
    do {
      delay(50);
      fDis = getDistance(trig, echo);
    } while (fDis > (distance - carLength) && (distance - carLength) > 0);

    //Turns left or right depending on position in loop
    if (i % 2 == 0) {
      turnLeft();
    }
    else {
      turnRight();
    }

    fDis = getDistance(trig, echo);

    //Checks if next object is too close to be the wall
    //In that case it is an obstacle
    if (fDis < (spaceW - carLength * 2 - 10)) {

      //Delays and checks again to prevent errors
      delay(20);
      fDis = getDistance(trig, echo);

      if (fDis < (spaceW - carLength * 2 - 10)) {
        //Travels forward to object
        do {
          delay(50);
          fDis = getDistance(trig, echo);
        } while (fDis > 10);

        rectangularObject();
      }
    }

    //Travels to wall
    do {
      delay(50);
      fDis = getDistance(trig, echo);
    } while (fDis > 10);

    if (i % 2 == 0) {
      turnRight();
    }
    else {
      turnLeft();
    }
  }

  //Continues to wall and stops
  do {
    delay(10);
    fDis = getDistance(trig, echo);
  } while (fDis > 10);

  stopCar();
}

//Function to skirt around an object
void rectangularObject() {
  double dis1, dis2;

  //Turns so that side sensor faces object
  turnLeft();

  dis1 = getDistance(trig, echo);

  //Continue forward until object is no longer on right side
  do {
    delay(50);
    sDis = getDistance(trig2, echo2);
  } while (sDis < 30);

  double disDif = dis1 - getDistance(trig, echo);

  //Extra movement for clearance
  delay(300);

  //Turns right and moves forward until object is on right side
  turnRight();
  do {
    delay(50);
    sDis = getDistance(trig2, echo2);
  } while (sDis > 30);

  //Continue forward until object is no longer on right side
  do {
    delay(50);
    sDis = getDistance(trig2, echo2);
  } while (sDis < 30);

  //Extra movement for clearance
  delay(300);
  turnRight();

  //Drives forward same distance as it drove forward on other side of object
  fDis = getDistance(trig, echo);

  do {
    delay(50);
  } while (getDistance(trig, echo) > fDis - disDif);

  turnLeft();
}

//Shuts off motors
void stopCar() {
  analogWrite(rPin1, 0);
  analogWrite(rPin2, 0);
  analogWrite(lPin1, 0);
  analogWrite(lPin2, 0);
}

//Begins forward motion
void forward() {
  //Starts with higher voltage, since motors can have trouble starting
  //when battery is low
  analogWrite(rPin1, 0);
  analogWrite(rPin2, 255);
  analogWrite(lPin1, 0);
  analogWrite(lPin2, 255);
  delay(100);

  analogWrite(rPin1, 0);
  analogWrite(rPin2, 150);
  analogWrite(lPin1, 0);
  analogWrite(lPin2, 150);
}

void turnLeft() {
  //Stopping to calibrate gyro
  stopCar();
  calibrateGyro();
  delay(50);

  //Inside motors have more power than outside, to prevent them dragging
  analogWrite(lPin1, 215);
  analogWrite(lPin2, 0);
  analogWrite(rPin1, 0);
  analogWrite(rPin2, 160);

  timePassed(); //Resets time for gyroscope
  while (heading > -75) { //90 degree turn, but 75 provides better accuracy due to momentum of car
    delay(25);
    getGyroVal();
    updateHeadings();
  }

  forward();
}

void turnRight() {
  stopCar();
  calibrateGyro();
  delay(50);

  analogWrite(lPin1, 0);
  analogWrite(lPin2, 160);
  analogWrite(rPin1, 215);
  analogWrite(rPin2, 0);

  timePassed();
  while (heading < 75) {
    delay(25);
    getGyroVal();
    updateHeadings();
  }

  forward();
}

//Retrieves distance from ultrasonic sensor
double getDistance(int trigPin, int echoPin) {
  double duration, distance;

  //Trigger for ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delay(20);
  digitalWrite(trigPin, LOW);

  //Duration is pulsed in in milliseconds, converted to cm distance
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  //Sometimes when distance is very small, sensor gives error data under 0 or over 3000
  //In this case, returning a small number is more accurate.
  if (distance < 0 || distance > 3000) {
    return 1;
  }
  else {
    return distance;
  }
}

//Sends setup info to gyro device
void setupGyro()
{
  gyroWrite(CTRL_REG1, 0x1F); //Turn on all axes, disable power down
  gyroWrite(CTRL_REG3, 0x08); //Enable control ready signal
  setGyroSensitivity500();

  delay(100);
}

//Calibrates gyro by finding the error data collected when device is still
//**Calculations taken from Jim Bourke, as above
void calibrateGyro()
{
  heading = 0;

  long int valSum = 0;
  long int gyroSigma = 0;

  for (int i = 0; i < NUM_GYRO_SAMPLES; i++)
  {
    getGyroVal();
    valSum += gyroRaw;
    gyroSigma += gyroRaw * gyroRaw;
  }

  gyroZeroRate = valSum / NUM_GYRO_SAMPLES;

  //A threshold based on the standard deviation of the samples times 3
  gyroThreshold = sqrt((double(gyroSigma) / NUM_GYRO_SAMPLES) - (gyroZeroRate * gyroZeroRate)) * GYRO_SIGMA_MULTIPLE;
}

//Sets DPS at the moment of function call
void getGyroVal() {

  while (!(gyroRead(0x27) & B00001000)) {}

  byte zMSB = gyroRead(0x2D);
  byte zLSB = gyroRead(0x2C);
  gyroRaw = ((zMSB << 8) | zLSB);

  int deltaGyro;
  deltaGyro = gyroRaw - gyroZeroRate; //Uses the calibration data to modify the sensor data
  if (abs(deltaGyro) < gyroThreshold)
    deltaGyro = 0;
  gyroDPS = dpsPerDigit * deltaGyro; //Multiply the sensor value by the sensitivity factor to get degrees per second
}

//Sets scale to 500 degress per second
void setGyroSensitivity500(void)
{
  dpsPerDigit = .0175f;
  gyroWrite(CTRL_REG4, 0x90);
}

int gyroRead (byte address) {
  Wire.beginTransmission(gyroAddress);
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(gyroAddress, 1);
  while (!Wire.available()) {};
  return (Wire.read());
}

int gyroWrite(byte address, byte val) {
  Wire.beginTransmission(gyroAddress);
  Wire.write(address);
  Wire.write(val);
  Wire.endTransmission();
}

//Converts DPS to heading, based on time passed since last update
void updateHeadings()
{
  float timeDif = timePassed();
  heading -= (gyroDPS * timeDif) / 1000000.0f;
}

//Returns the time in microseconds since the function's last call
unsigned long timePassed()
{
  static unsigned long lastTime = 0;
  unsigned long currentTime = micros();
  unsigned long timeDif = currentTime - lastTime;
  if (timeDif < 0.0)
    timeDif = currentTime + (4294967295 - lastTime);

  lastTime = currentTime;
  return timeDif;
}


