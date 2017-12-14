#define R1 5
#define R2 6
#define L1 9
#define L2 10
#define trig 2
#define echo 4

double distance;
long duration;

void setup() {
  pinMode(R1, OUTPUT);  
  pinMode(R2, OUTPUT);  
  pinMode(L1, OUTPUT);  
  pinMode(L2, OUTPUT);  

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

  if (distance > 20 && distance < 500) {
    digitalWrite(R1, LOW);
    digitalWrite(R2, HIGH);
    digitalWrite(L1, LOW);
    digitalWrite(L2, HIGH);
  } else {
    digitalWrite(R1, LOW);
    digitalWrite(R2, LOW);
    digitalWrite(L1, LOW);
    digitalWrite(L2, LOW);
  }


}
