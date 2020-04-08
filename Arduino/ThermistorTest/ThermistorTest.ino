double temperatures[5] = {};
const int handTherm1Pin = A1;
const int handTherm2Pin = A2;
const int handTherm3Pin = A3;
const int heatThermPin = A4;
const int coolThermPin = A5;

void setup() {
  pinMode(handTherm1Pin,INPUT);
  pinMode(handTherm2Pin,INPUT);
  pinMode(handTherm3Pin,INPUT);
  pinMode(heatThermPin,INPUT);
  pinMode(coolThermPin,INPUT);
  Serial.begin(9600);
}

void v_to_c(double temperatures[]){
  //Call by Reference of temperatures[]
  //If Rref = exactly 30k, therm characteristics follow from voltage divider and datasheet:
  double temp;

  for (byte i = 0; i < 4; i++)
  {
    temperatures[i] = 92.002*pow(M_E,-0.447*temperatures[i]);
  }
}

void sense_temps(double temperatures[]){
  //program size/compute speed concerns? change to int or fixed pt
  //in C, modifies arrays directly, changes are stored
  //10-bit ADC -> 5V/1024 Units -> 4.9mV per Unit
  //Max read rate is 10Khz
  
  //for loop? A1-A5
  temperatures[0] = analogRead(handTherm1Pin)*.0049;
  temperatures[1] = analogRead(handTherm2Pin)*.0049;
  temperatures[2] = analogRead(handTherm3Pin)*.0049;
  temperatures[3] = analogRead(heatThermPin)*.0049;
  temperatures[4] = analogRead(coolThermPin)*.0049;

  v_to_c(temperatures);
  //TODO Verify function modifies global var temperatures
}

void loop() {
sense_temps(temperatures);
for (int i = 0; i < 5; i++)
{
  Serial.print("Thermistor ");
  Serial.print(i+1);
  Serial.print(": ");
  Serial.println(temperatures[i]);
}
Serial.println();
delay(5000);
}
