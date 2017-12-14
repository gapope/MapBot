#define R1 5
#define R2 6
#define L1 9
#define L2 10
#define AT 11
#define trig 3
#define echo 4

double distance;
long duration;

char junk;
String inputString="";

void setup() {
    
   
  pinMode(R1, OUTPUT);  
  pinMode(R2, OUTPUT);  
  pinMode(L1, OUTPUT);  
  pinMode(L2, OUTPUT);
  pinMode(AT, OUTPUT);

   digitalWrite(AT, HIGH);

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
    digitalWrite(R1, LOW);
    digitalWrite(R2, LOW);
    digitalWrite(L1, LOW);
    digitalWrite(L2, LOW);
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
    if(inputString == "w" && distance > 20){         //in case of 'w' move forward
      digitalWrite(R1, LOW);
      digitalWrite(R2, HIGH);
      digitalWrite(L1, LOW);
      digitalWrite(L2, HIGH);  
    }else if(inputString == "a"){   //incase of 'a' turn left
      digitalWrite(R1, LOW);
      digitalWrite(R2, HIGH);
      digitalWrite(L1, HIGH);
      digitalWrite(L2, LOW);
    }else if(inputString == "d"){   //incase of 'd' turn right
      digitalWrite(R1, HIGH);
      digitalWrite(R2, LOW);
      digitalWrite(L1, LOW);
      digitalWrite(L2, HIGH);
    }else if(inputString == "s"){   //incase of 's' move backwards
      digitalWrite(R1, HIGH);
      digitalWrite(R2, LOW);
      digitalWrite(L1, HIGH);
      digitalWrite(L2, LOW);
    }else if(inputString == "x"){   //incase of 'x' turn the motors off
      digitalWrite(R1, LOW);
      digitalWrite(R2, LOW);
      digitalWrite(L1, LOW);
      digitalWrite(L2, LOW);
    }
    inputString = "";
}
}
