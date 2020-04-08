//Thermal Joint Therapy, TCNJ Senior Project 2019-2020
//Author: Blake Capella
//tjt_mcu v1; 3/4/2020
//-----------------------------------------------------

//Pin Layout (these are global declarations)

//LED Pins
const int ledRPin = 0;
const int ledYPin = 1;
const int ledGPin = 2;

//Control Pins to External SOIC: VNH7070
const int heatContPin = 4;
const int coolContPin = 5;

//Pins to TBD62783 Load Switch
const int fan1Pin = 6; //Heat Fan
const int fan2Pin = 7; //Cool Fan
const int gate4Pin = 8;
const int gate3Pin = 9;
const int gate2Pin = 10; //unclear on EAGLE
const int gate1Pin = 11; //unclear on EAGLE
const int pump1Pin = 12; //unclear on EAGLE
const int pump2Pin = 13; //unclear on EAGLE

//Pins in from Thermistor Divider
const int handTherm1Pin = A1;
const int handTherm2Pin = A2;
const int handTherm3Pin = A3;
const int heatThermPin = A4;
const int coolThermPin = A5;

//Feedback Pullup resistor on DPDT Switch, External Interrupt
const int safetyPin = 3;

//Constant for exponential curve in temperature conversion
const double e = 2.71828;

//Temperature Variables
double temperatures[5] = {};  //arrays of sampled temperatures
double hand_temp_watch = 0;   //hand temp safety monitoring
double hand_temp_previous = 0;
int count_outside = 0;
double heat_lvl; //Setpoint for temp in hot resevoir
double cool_lvl; //Setpoint for temp in cool resevoir
volatile bool emergency_thrown = false; //Manipulated in ISR, declares DPDT switch has been thrown
                                        //This is the only time in the program emergy_thrown is reset!
bool hand_temp_unsafe = false;          //Flag for if Hand Temp exceeds bounds
                                        //This is the only time in the program hand_temp_unsafe is reset!
bool S = false; //if unsafe, flag goes high

//Digital Logic Variables
//-Outputs
bool P1 = false;  //pump 1
bool P2 = false;  //pump 2
bool G1 = false;  //gate 1
bool G2 = false;  //gate 2
bool G3 = false;  //gate 3
bool G4 = false;  //gate 4

//-inputs
bool Pd = false;  //Pumpout done
bool B = true;   //Begin as soon as ready, potentially  make button at later date
                 //This is the only time B is set HIGH in the entire program! Therapy will only run once
bool time_h = false; //Heat cycle time elapsed
bool time_c = false; //Cool cycle time elapsed
bool time_d = false; //treatment time elapsed

//-timing for inputs
int treat_count = 0;
unsigned long t_start;
bool t_cycle_set = false;
const long T_HEAT = 1000*360;     //heat cycle = 6 min
const long T_COOL = 1000*240;     //cool cycle = 4 min
const long T_EVAC = 1000*30;      //evac cycle = 30 sec
const long led_g_interval = 1000; //Toggle LED every X ms -> 1sec

//-state check
bool evac_cycle;
bool STBY;

//Setup Code
void setup() {
  // local variable declarations
  bool rdy = false; //preheat complete flag
  bool hot = false; //preheat complete for hot resevoir
  bool cold = false; // preheat complete for cool resevoir
  int count_cold = 0; // int to record number of successive determinations of SS
  int count_hot = 0;
  heat_lvl = 42; //Setpoint for Preheat in resevoir
  cool_lvl = 16; //Setpoint for Precool in resevoir
  double temperature_history[5] = {}; //store old temperature values in array
  double delta[2] = {};               //store difference in temperature values this sample with the previous sample
  double delta_history[2] = {};
  
  //Pin Configuration
  pinMode(ledRPin,OUTPUT);
  pinMode(ledYPin,OUTPUT);
  pinMode(ledGPin,OUTPUT);
  pinMode(heatContPin,OUTPUT);
  pinMode(coolContPin,OUTPUT);
  pinMode(fan1Pin,OUTPUT);
  pinMode(fan2Pin,OUTPUT);
  pinMode(gate4Pin,OUTPUT);
  pinMode(gate3Pin,OUTPUT);
  pinMode(gate2Pin,OUTPUT);
  pinMode(gate1Pin,OUTPUT);
  pinMode(pump1Pin,OUTPUT);
  pinMode(pump2Pin,OUTPUT);
  
  pinMode(handTherm1Pin,INPUT);
  pinMode(handTherm2Pin,INPUT);
  pinMode(handTherm3Pin,INPUT);
  pinMode(heatThermPin,INPUT);
  pinMode(coolThermPin,INPUT);
 
  //Safety Interrupt
  pinMode(safetyPin,INPUT_PULLUP); //Pullup configuration for safety switch, wired so that when DPDT goes to GND, Output goes LOW
  attachInterrupt(digitalPinToInterrupt(safetyPin), safety_isr, LOW); //Trigger ISR when safety pin goes LOW -> DPDT to GND
  
  //Use Serial communication via USB for debugging; 9600 Baud Rate
  Serial.begin(9600);
  Serial.write("LEDG ",digitalRead(ledGPin));
  Serial.write("LEDY ",digitalRead(ledYPin));
  Serial.write("LEDR ",digitalRead(ledRPin));
  
  //LED Startup Sequence
  digitalWrite(ledGPin,HIGH);
  delay(500);
  digitalWrite(ledYPin,HIGH);
  delay(500);
  digitalWrite(ledRPin,HIGH);
  delay(750);
  digitalWrite(ledGPin,LOW);
  delay(500);
  digitalWrite(ledYPin,LOW);
  delay(500);
  digitalWrite(ledRPin,LOW);
  delay(500);

  Serial.write("LED Display");

  //Close All Valves and Turn off Pumps for Pre-Heat
  //initial conditions for STBY in state diagram
  digitalWrite(gate4Pin,LOW);
  digitalWrite(gate3Pin,LOW);
  digitalWrite(gate2Pin,LOW);
  digitalWrite(gate1Pin,LOW);
  digitalWrite(pump1Pin,LOW);
  digitalWrite(pump2Pin,LOW);

  //Preheat Protocol
  while(!rdy && !emergency_thrown){

    blink_green(true);
    
    heat(heat_lvl); 
    cool(cool_lvl);

    sense_temps(temperatures);

    //Detect Steady State
    for(int i = 3; i < 5; i++){
      delta[i-3] = abs(temperature_history[i] - temperatures[i]); //compute temp difference at resevoirs
    }

    //Store current value of temperatures in history for next cycle's comparison
    for(int j = 0; j < 5; j++){
      temperature_history[j] = temperatures[j];
    }

    //store current values of delta to determine overall trend
    for( int k = 0; k<2; k++){
      delta_history[k] = delta[k];
    }

    //If temperature change is minimal (less than X) declare we have reached SS therefore end preheat/cool
    if ((delta[0] < 0.1)&&(delta_history[0] < 0.1)){ //count only increments with successive changes lower than 0.1
      count_hot = count_hot + 1;
      if (count_hot > 5){
        hot = true;
      } 
    }
    else{
      hot = false;
      count_hot = 0; //if nonconsecutive, reset count
    }

    if((delta[1] < 0.1)&& (delta_history[1] < 0.1)){
      count_cold = count_cold + 1;
      if (count_cold > 5){
        cold = true;
      }
    }
    else{
      cold = false;
      count_cold = 0; // if count nonconsecutive, reset count
    }

    //Determine if at SS for both resevoirs, if so, exit
    rdy = (cold && hot);
  }


   Serial.write("Ready");
   blink_green(false);
   STBY = true;     //one of two spots where STBY can be entered from (this is end of preheat)
  
}

void heat(double heat_lvl){
  int t_off; //determine length of time heat is off
  
  if(temperatures[3] < heat_lvl){
    digitalWrite(heatContPin, HIGH);  //turn on heat via external H bridge IC
    digitalWrite(fan1Pin, HIGH);
    t_off = 0;
  }
  else{
    digitalWrite(heatContPin, LOW); //turn off heat via external H bridge IC
    t_off = t_off+1;
  }
  
  //turn off fan when heater has been off for X cycles
  if(t_off > 100){
    digitalWrite(fan1Pin, LOW);
  }
}

void cool(double cool_lvl){
   int t_off; //determine length of time heat is off
  
  if(temperatures[4] < cool_lvl){
    digitalWrite(coolContPin, HIGH);  //turn on heat via external H bridge IC
    digitalWrite(fan2Pin, HIGH);
    t_off = 0;
  }
  else{
    digitalWrite(coolContPin, LOW); //turn off heat via external H bridge IC
    t_off = t_off+1;
  }
  
  //turn off fan when heater has been off for X cycles
  //TODO: experimentally determine X from sampling freq, potentially use timer
  if(t_off > 100){
    digitalWrite(fan2Pin, LOW);
  } 
}

void blink_green(bool command) {
  //blinking green LED output without Delay
  unsigned long currentMillis = millis();
  unsigned long previousMillis;
   
  if(command){
    if(currentMillis - previousMillis >= led_g_interval){ //if time elapsed > interval
      previousMillis = currentMillis;               //save as the last time you toggled the LED
      if(digitalRead(ledGPin))
      {
        digitalWrite(ledGPin,LOW);
      }
      else {
        digitalWrite(ledGPin,HIGH);
      }
    }
    else{
      digitalWrite(ledGPin, LOW);
    }
  }
}

void v_to_c(double temperatures[]){
  //Call by Reference of temperatures[]
  //If Rref = exactly 30k, therm characteristics follow from voltage divider and datasheet:
  double temp;

  for (byte i = 0; i < 4; i++)
  {
    temperatures[i] = 92.002*pow(M_E,-0.447*temperatures[i]); //Use Arduino constant for value of "e"
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
  //TODO Verify function modifies global var temperatures (Jared)
}

void controller(double *ptr_heat_lvl, double *ptr_cool_lvl) {
  //Call by Reference heat_lvl, cool_lvl
  //TODO implement controller
}

void safety_isr(){
  //to make sure values are passed from ISR to main program, declare variables as volatile
  
  //Red LED indicator
  digitalWrite(ledRPin, HIGH);
  
  //Power to Heat/Cool already manually interrupted by switch, as good practice, set MCU output to 0
  digitalWrite(heatContPin, LOW);
  digitalWrite(coolContPin, LOW);

  emergency_thrown = true;
  
  Serial.write("Stop Treatment Button Pressed");
}

void loop() {
  // put your main code here, to run repeatedly:
  // TODO: test loop time therefore MCU sampling freq

  //Sample Temperature Values
  sense_temps(temperatures);
  hand_temp_watch = max(max(temperatures[1],temperatures[2]),temperatures[3]);

  //Automatic Safety Shutoff Detection
  if(hand_temp_watch >= 45 && hand_temp_previous >= 45){ //if hand temp is over 45 and was over 45 last sample
    count_outside = count_outside + 1;
    if (count_outside >= 5){  // if max temp on the hand is above 45 for more than 5 samples in a row, throw a flag
      hand_temp_unsafe = true;  //Variable never reset in program!
    }
  }
  else if(hand_temp_watch <= 15 && hand_temp_previous <= 15){ //if hand temp is under 15 and was under 15 last sample
    count_outside = count_outside + 1;
    if (count_outside >= 5){  // if max temp on the hand is above 45 for more than 5 samples in a row, throw a flag
     hand_temp_unsafe = true; //Variable never reset in program!
    }
  }
  
  if(!hand_temp_unsafe){
    //Use Controller to determine set-point for on/off control
    controller(&heat_lvl,&cool_lvl);
  
    //Pass setpoint to on/off controllers
    heat(heat_lvl);
    cool(cool_lvl);
  }
  else {  // if hand_temp_unsafe, set all outputs to zero
    digitalWrite(heatContPin, LOW);
    digitalWrite(coolContPin, LOW);
  }
  
  //Digital Logic Control for Pumps/Gates for STBY, Heat Mode, Cool Mode, and Bladder Evac
  evac_cycle = !P1 && P2 && !G1 && !G2; //State 01 00XX -> evacuating bladder
  S = hand_temp_unsafe || emergency_thrown;
  
  //Normal Treatment Cycle Timing
  if (treat_count < 6 && !STBY){             // 6 cycles total, 3 hot, 3 cold, total time = 30 min
    digitalWrite(ledYPin, HIGH);     // Yellow LED to indicate treatment
    if(treat_count%2 == 1 &&!t_cycle_set){   // if odd count (1,3,5) -> entering cooling stage, start count for cool cycle
      t_start = millis();
      time_h = false;                // turn off flag for heat timer elapse
      t_cycle_set = true;            // declare timer has begun counting
    }
    else if(treat_count%2 == 1 && t_cycle_set){     //otherwise if in cool cycle and timer already started, check if time elapses against specified T_COOL = 4 min
      if((t_start - millis()) >= T_COOL){
        time_c = true;              //flag transition by declaring cool time elapsed
        t_cycle_set = false;        //essentially reset cycle timer
        treat_count = treat_count + 1;  //increment treatment cycle count
      }
    }
  
    if(treat_count%2 == 0 && !t_cycle_set){  // if even count (0,2,4) -> entering heating stage, start count for heat cycle
      t_start = millis();
      time_c = false;                 // turn off flag for cool timer elapse
      t_cycle_set = true;             // declare timer has begun counting
    }
    else if(treat_count%2 == 0 && t_cycle_set){            //otherwise if in cool cycle and timer already started, check if time elapses against specified T_HEAT = 6 min
      if((t_start - millis()) >= T_HEAT){
        time_h = true;                //flag transition by declaring heat time elapsed
        t_cycle_set = false;          //essentially reset cycle timer
        treat_count = treat_count + 1;   //increment treatment cycle count
        B = false;                    //Reset B to 0 after the first heat cycle (originally set high in varaible defintion)
      }
    }
  }
  else {
    time_d = true;  //if treat count >= 5, then cycle is complete. Flag high transition to evacuate & end cycle
  }

  //Evacuation Cycle Timing
  if(evac_cycle && !t_cycle_set){ // if evacuating, begin timer
    t_start = millis();
    time_d = false;  //reset time elapse signal (reset S?)
    t_cycle_set = true;
  }
  else if (evac_cycle && t_cycle_set){  //if evacuating and timer already begun, check time against T_EVAC to determine if pumpout complete
    if ((t_start - millis()) >= T_EVAC){
      Pd = true;      //signal pumpout elapsed with high flag
      t_cycle_set  = false;
      STBY = true;    //One of two spots STBY can be entered from (end of evac)
    }
  }

  //Standby actions
  if(STBY){
    //Assuming proper DLC function, All states will be set to 0
    digitalWrite(ledYPin, LOW);     //turn off treatment LED
    digitalWrite(ledGPin, HIGH);    //solid green LED to indicate STBY
    //To enable multiple cycles, use B switch to reset treat_count and toggle B High
    if(B){
      STBY = false;              //exit STBY into therapy
      digitalWrite(ledGPin, LOW); //turn off LED
    }
  }

  //STATES
  //-Output State Transition Equations (VERIFIED)
  P1 = (!S)&&(!time_d)&&(P1 || B);
  P2 = (!Pd)&&(P2 || B);
  G1 = (!time_h)&&(!S)&&(G1 || B || (time_c && !S));
  G2 = (!time_c)&&(!S)&&(!time_d)&&(G2 || (time_h && !S));
  G3 = (!(time_h && !S))&&(!Pd)&&(G3 || B || (time_c && !S));
  G4 = (!(time_c && !S))&&(!Pd)&&(G4 || (time_h && !S));

  //Assign output states
  digitalWrite(gate4Pin,G4);
  digitalWrite(gate3Pin,G3);
  digitalWrite(gate2Pin,G2);
  digitalWrite(gate1Pin,G1);
  digitalWrite(pump1Pin,P1);
}
