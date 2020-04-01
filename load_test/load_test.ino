void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(13, LOW);
  Serial.write("Low");
  delay(2500);
  digitalWrite(13, HIGH);
  Serial.write("High");
  delay(2500);
}
