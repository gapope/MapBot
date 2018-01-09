//Motor pins
#define rPin1 6
#define rPin2 9
#define lPin1 10
#define lPin2 11

#define trig 3
#define echo 4

double distance;
long duration;

char junk;
String inputString="";

void setup() {
    
   
  pinMode(rPin1, OUTPUT);
  pinMode(rPin2, OUTPUT);
  pinMode(lPin1, OUTPUT);
  pinMode(lPin2, OUTPUT);

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  Serial.begin(9600);
}

void loop() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delay(200);
  digitalWrite(trig, LOW);
  
  duration = pulseIn(echo, HIGH);
  distance = duration * 0.034/2;
  //Serial.println(distance);

  if(distance < 20){   //stopping as approaching wall
    analogWrite(rPin1, 0);
  analogWrite(rPin2, 0);
  analogWrite(lPin1, 0);
  analogWrite(lPin2, 0);
  }
  
  if(Serial.available()){
  while(Serial.available())
    {
      char inChar = (char)Serial.read(); //read the input
      inputString += inChar;        //make a string of the characters coming on serial
    }
    Serial.println(inputString);
    while (Serial.available() > 0)  
    { junk = Serial.read() ; }      // clear the serial buffer
    if(inputString == "F" && distance > 20){         //in case of 'w' move forward
      analogWrite(rPin1, 0);
      analogWrite(rPin2, 150);
      analogWrite(lPin1, 0);
      analogWrite(lPin2, 150);  
    }else if(inputString == "L"){   //incase of 'a' turn left
      analogWrite(lPin1, 255);
      analogWrite(lPin2, 0);
      analogWrite(rPin1, 0);
      analogWrite(rPin2, 200);
    }else if(inputString == "R"){   //incase of 'd' turn right
      analogWrite(lPin1, 0);
      analogWrite(lPin2, 200);
      analogWrite(rPin1, 255);
      analogWrite(rPin2, 0);
    }else if(inputString == "B"){   //incase of 's' move backwards
      analogWrite(rPin1, 150);
      analogWrite(rPin2, 0);
      analogWrite(lPin1, 150);
      analogWrite(lPin2, 0);
    }else if(inputString == "E"){   //incase of 'x' turn the motors off
      analogWrite(rPin1, 0);
      analogWrite(rPin2, 0);
      analogWrite(lPin1, 0);
      analogWrite(lPin2, 0);
    }
    inputString = "";
}
}
