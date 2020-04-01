//LED Pins
const int ledRPin = 0;
const int ledYPin = 1;
const int ledGPin = 2;
char data[100] = {};

void setup() {
  // put your setup code here, to run once:
  pinMode(ledRPin,OUTPUT);
  pinMode(ledYPin,OUTPUT);
  pinMode(ledGPin,OUTPUT);

  Serial.begin(9600);
  while (!Serial) {
    delay(10); 
   }
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(ledGPin,HIGH);
  sprintf(data, "LEDG: %d", bitRead(PORTD,ledGPin)); 
  Serial.println(data);

  delay(500);
  
  digitalWrite(ledYPin,HIGH);
  sprintf(data, "LEDY: %d", bitRead(PORTD,ledYPin)); 
  Serial.println(data);

  delay(500);
  
  digitalWrite(ledRPin,HIGH);
  sprintf(data, "LEDR: %d", bitRead(PORTD,ledRPin)); 
  Serial.println(data);
  
  delay(750);
  
  digitalWrite(ledGPin,LOW);
  sprintf(data, "LEDG: %d", bitRead(PORTD,ledGPin)); 
  Serial.println(data);

  delay(500);
  
  digitalWrite(ledYPin,LOW);
  sprintf(data, "LEDY: %d", bitRead(PORTD,ledYPin)); 
  Serial.println(data);

  delay(500);
  
  digitalWrite(ledRPin,LOW);
  sprintf(data, "LEDR: %d", bitRead(PORTD,ledRPin)); 
  Serial.println(data);

  delay(500);
  
}
