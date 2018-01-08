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
#define rPin1 6
#define rPin2 9
#define lPin1 10
#define lPin2 11

//Ultrasonic sensor pins
#define trig 5
#define echo 4
#define trig2 8
#define echo2 7

//Ultrasonic sensor variables
double fDis; //I meant this as front distance, but fuck this is another applicable interpretation
double sDis; 
int spaceL, spaceW;

//Gyro variables
int gyroAddress = 105;
int gyroRaw;                         // Raw sensor data
double gyroDPS;                      // Gyro degrees per second
float heading = 0.0f;              
int gyroZeroRate;                    // Calibration data (sensor does not centre at zero)
int gyroThreshold;                   // Change data less than the threshold is not used
float dpsPerDigit=.00875f;           // Number for conversion to degrees per second

//Suggested numbers for calibration, taken from Jim Bourke as mentioned above
#define  NUM_GYRO_SAMPLES  50          
#define  GYRO_SIGMA_MULTIPLE  3         

//Prototypes (are they necessary in this ide?)
void turnLeft();
void turnRight();
double getDistance(int trigPin, int echoPin);
void outerWalls();
void rectangularObject();
void innerSpace();
int gyroWrite(byte address, byte val);
int gyroRead (byte address);
void setGyroSensitivity500(void);
void getGyroVal();
void calibrateGyro();
void setupGyro();
unsigned long timePassed();
void updateHeadings();

void setup() {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);
  pinMode(rPin1, OUTPUT);
  pinMode(rPin2, OUTPUT);
  pinMode(lPin1, OUTPUT);
  pinMode(lPin2, OUTPUT);

  Wire.begin();
  setupGyro();
  
  outerWalls(); //Traces walls
  innerSpace(); //Travels through inner space
  /* Note: 
   *  The inner space function is where the issues are.
   *  It sometimes doesn't go forward the right amount, as either
   *  the data is stored wrong or the sensor is reading wrong.
   *  
   *  I haven't had the time to uncomment the code I added for 
   *  checking if there was an obstacle in the way other than the wall,
   *  so I'm not sure if it works or not.
   */
}

void loop() { 
}

void outerWalls(){
  //Gets length (takes average of three for accuracy)
  fDis = (getDistance(trig, echo) + getDistance(trig, echo) + getDistance(trig, echo))/3 + carLength;
  spaceL = fDis;
  
  forward();

  //For each wall
  for(int i = 0; i < 4; i++){
    //Continue forward until reaching a wall
    do{
        delay(50);
        fDis = getDistance(trig, echo);
    }while(fDis > 10);
    turnLeft();

    //On first turn, stops to get width data
    if(i == 0){
      stopCar();
      spaceW = (getDistance(trig, echo) + getDistance(trig, echo) + getDistance(trig, echo))/3 + carLength;
      forward();
    }
  }

  stopCar();
}

//Travels through space in zigzag pattern
//(Forward, left to wall, forward, right to wall, etc)
void innerSpace(){
  int rows = (spaceL - carLength) / carLength;
  //Number of rows to follow in zigzag pattern. 
  //Removes car's length from total distance and divides by car's length

  for(int i = 0; i < rows; i++){
    //Gets distance to wall
    stopCar();
    double distance = (getDistance(trig, echo) + getDistance(trig, echo) + getDistance(trig, echo))/3;
    forward();

    //Moves forward 26 cm
    //I've had a lot of sporadic trouble with this
    //Maybe you can figure out why this straightforward thing doesn't work
    do{
        delay(50);
        fDis = getDistance(trig, echo);
     }while(fDis > (distance - carLength));

    //Turns left or right depending on position in loop
    if(i%2 == 0){
      turnLeft();
    }
    else{
      turnRight();
    }

    fDis = getDistance(trig, echo);
    
    //Checks if next object is too close to be the wall 
    //In that case it is an obstacle
    /*if(fDis < spaceW - carLength * 2){
      do{
        delay(50);
        fDis = getDistance(trig, echo);
      }while(fDis > 10);
      rectangularObject();
    }*/

    do{
        delay(50);
        fDis = getDistance(trig, echo);
    }while(fDis > 10);

    if(i%2 == 0){
      turnRight();
    }
    else{
      turnLeft();
    }
  }

  //Continues to wall and stops
  do{
    delay(10);
    fDis = getDistance(trig, echo);
  }while(fDis > 10);
    
  stopCar();
}

void stopCar(){
  analogWrite(rPin1, 0);
  analogWrite(rPin2, 0);
  analogWrite(lPin1, 0);
  analogWrite(lPin2, 0);
}

void forward(){
  analogWrite(rPin1, 0);
  analogWrite(rPin2, 150);
  analogWrite(lPin1, 0);
    analogWrite(lPin2, 150);
}

void rectangularObject(){
    //Turns to start circling object
    turnLeft();

    //Repeats for each side of object
    for(int i = 0; i < 5; i++){
      //Continue forward until object is no longer on right side
      do{
        delay(100);
        sDis = getDistance(trig2, echo2);
      }while(sDis < 30);

      //Extra movement for clearance
      delay(200);

      //Turns right and moves forward until object is on right side
      turnRight();
      do{
        delay(100);
        sDis = getDistance(trig2, echo2);
      }while(sDis > 30);
    }
}

void turnLeft(){
  //Stopping to calibrate gyro
  stopCar();
  calibrateGyro();
  delay(50);

  //Inside motors have more power than outside
  analogWrite(lPin1, 255);
  analogWrite(lPin2, 0);
  analogWrite(rPin1, 0);
  analogWrite(rPin2, 150);

  timePassed(); //Resets time for gyroscope
  while(heading > -80){ //90 degree turn, but 85 provides better accuracy
    delay(25);
    getGyroVal();
    updateHeadings();
  }
  Serial.println("Done");
  Serial.println();

  stopCar();
  delay(50);
  forward();
}

void turnRight(){
  stopCar();
  calibrateGyro();
  delay(50);
  
  analogWrite(lPin1, 0);
  analogWrite(lPin2, 150);
  analogWrite(rPin1, 255);
  analogWrite(rPin2, 0);

  timePassed();
  while(heading < 80){ 
    delay(25);
    getGyroVal();
    updateHeadings();
  }
  Serial.println("Done");
  Serial.println();

  stopCar();
  delay(50);
  forward();
}

double getDistance(int trigPin, int echoPin){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delay(20);
  digitalWrite(trigPin, LOW);
  
  double duration = pulseIn(echoPin, HIGH);
  return duration*0.034/2;
}

void setupGyro()
{
  gyroWrite(CTRL_REG1, 0x1F);        // Turn on all axes, disable power down
  gyroWrite(CTRL_REG3, 0x08);        // Enable control ready signal
  setGyroSensitivity500();

  delay(100);
}

void calibrateGyro()
{
  heading = 0;
  
  long int valSum=0;
  long int gyroSigma=0;

  for (int i=0;i<NUM_GYRO_SAMPLES;i++)
  {
    getGyroVal();
    valSum+=gyroRaw;
    gyroSigma+=gyroRaw*gyroRaw;
  }

  gyroZeroRate=valSum/NUM_GYRO_SAMPLES;
    
  // Per STM docs, we create a threshold for each axis based on the standard deviation of the samples times 3.
  gyroThreshold=sqrt((double(gyroSigma) / NUM_GYRO_SAMPLES) - (gyroZeroRate * gyroZeroRate)) * GYRO_SIGMA_MULTIPLE;    
}

void getGyroVal() {

  while (!(gyroRead(0x27) & B00001000)){}      // Without this line you will get bad data occasionally

  byte zMSB = gyroRead(0x2D); 
  byte zLSB = gyroRead(0x2C); 
  gyroRaw = ((zMSB << 8) | zLSB); 

  int deltaGyro;
  deltaGyro=gyroRaw-gyroZeroRate;      // Use the calibration data to modify the sensor value.
  if (abs(deltaGyro) < gyroThreshold)
    deltaGyro=0;
  gyroDPS= dpsPerDigit * deltaGyro;      // Multiply the sensor value by the sensitivity factor to get degrees per second.
}

void setGyroSensitivity500(void)
{
  dpsPerDigit=.0175f;
  gyroWrite(CTRL_REG4, 0x90);        // Set scale (500 deg/sec)
}

int gyroRead (byte address) {
  Wire.beginTransmission(gyroAddress);
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(gyroAddress, 1);
  while(!Wire.available()) {};
  return (Wire.read());
}

int gyroWrite(byte address, byte val){
  Wire.beginTransmission(gyroAddress);
  Wire.write(address);
  Wire.write(val);
  Wire.endTransmission();
}

void updateHeadings()
{
  float timeDif=timePassed();
  heading -= (gyroDPS*timeDif)/1000000.0f;
}

//Returns the elapsed time since the last call in microseconds
unsigned long timePassed()
{
  static unsigned long lastTime=0;
  unsigned long currentTime=micros();
  unsigned long timeDif=currentTime-lastTime;
  if (timeDif < 0.0)
     timeDif=currentTime+(4294967295-lastTime);
   
  lastTime=currentTime;
  return timeDif;
}


