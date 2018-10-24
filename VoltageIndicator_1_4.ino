/*
This regulator is developed with the goal to optimize the working of a heating system pump.
It has three voltage sensors with galvanic deviding trafos which are connected to the servo valves of the heating lines. 
In case there is 220V on the sensor input the /uC recieves an AC signal from 0 to 5 V of the amplitude.
The signal from three sensors are being evaluating and a control command will be issued to the relay in folloving cases:
- at lease one of the sensors has 220V on its input, the relay should be set to the ON-status
- no one sensor has an apporpriate voltage level, the relay should switch off the heating pump.
In course of the switching on or off procedure relay should change its status after a sertain delay, 
wich in turn can be possible changed in framework of the setup sequence. Usually this delay amount to 0..5 minutes.
The whole regulater should be controlled via one button and can be put in one of the three possible states: ON, OFF and Automatic.
This button reacts for the single pressing less then 1 sec. and switches betwenn the mentioned states.
If the button is pressed for 5 sec. the setup routine is entered. WIthin this routine the delay time can be set for 
the switch-on and -off procedure separately.
*/

#define sensorPin_1 1
#define sensorPin_2 2
#define sensorPin_3 3
#define relayPin 12
#define led1 7
#define led2 8
#define led3 9
#define buttonPin 14
#define controlLED 16
#define setUpLED 18


boolean switching(false);  // Any switching process is going on
boolean relayOn(false);    // Status of the relay

unsigned long blinkTimer(0);
unsigned long onOffTimer(0);

int switchingOnDelay(5000), switchingOffDelay(15000);
int blinkPeriod(1000);

int sensorSignal[3][10]{};        // analog signal from 3 voltage sensors x 10 samples
int voltage[3]{};                 // average voltage signal from 3 sensors
int average = 513;                // threshold on/off
int voltageSum(0), vMax(10), vMin(10000);


// *****------ S-E-T-U-P---------****************************************
void setup() {
  Serial.begin(9600);           // Serial link to PC created
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);    // Relay switch Off
  switching = false;
  relayOn = false;
  blinkPeriod = 1500;              // Blink frequency while relay is switched off
}

// ---------- L-O-O-P-----************************************************
void loop() {
  
  voltageSum = 0;
  
  Serial.print("Sw1: "); Serial.print(switching); Serial.print("  ");
   
  if ( !switching ) {
    // ---------- Sensors signal evaluation-----------------
    for(byte i = 0; i < 3; i++) voltage[i] = 0;
    
    for(byte i = 0; i < 10; i++) {            // average over 10 measurements
      for(byte j = 0; j < 3; j++) {           // 3 voltage sensors, input signal
        sensorSignal[j][i] = analogRead(j+1);
        if ( j == 0 || j == 1) sensorSignal[j][i] = 513; // For test purposes only. Simulation of the switched off state
        //Serial.print(sensorSignal[j][i]); Serial.print("  ");

        if (sensorSignal[j][i] < average) voltage[j] += (average - sensorSignal[j][i]);  // Detection of the sinusoidal signal. Derivation of the average
        else  voltage[j] += (sensorSignal[j][i] - average); 
        // Serial.println(sensorSignal_1[i]);
        // Serial.print(voltage[0]); Serial.print("  "); Serial.print(voltage[1]); Serial.print("  "); Serial.println(voltage[2]);
      }
    }
  }
    Serial.print(voltage[0]); Serial.print("  "); Serial.print(voltage[1]); Serial.print("  "); Serial.print(voltage[2]); Serial.print("  "); Serial.print(millis() - onOffTimer);
    
    // ------------- On/Off switching logic -----------------------
    if ( ( (voltage[0] > 300) || (voltage[1] > 300) || (voltage[2] > 300) ) && !switching && !relayOn ) { // If at least one servo is on
      // Serial.println("ON");    
      blinkPeriod = 150;    
      onOffTimer = millis(); 
      switching = true;
      }
  
    if( ( (voltage[0] < 300) && (voltage[1] < 300) && (voltage[2] < 300) ) && !switching && relayOn ) { // If everythimg is off
      // Serial.println("OFF");
      blinkPeriod = 1500;
      onOffTimer = millis();
      switching = true;
      }
  
   Serial.print("  RelOn: "); Serial.println(relayOn);
   
  if(switching && !relayOn && ((millis() - onOffTimer) >= switchingOnDelay) ) {
      digitalWrite(relayPin, LOW);  // Switch On the relay
      relayOn = true;
      switching = false;
      }

  if(switching && relayOn && ((millis() - onOffTimer) >= switchingOffDelay) ) {
      digitalWrite(relayPin, HIGH);  // Switch Off the relay
      relayOn = false;
      switching = false;
      }
  
  if ((millis() - blinkTimer) >= blinkPeriod) 
    digitalWrite(LED_BUILTIN, HIGH);                // turn the LED on  
  if ((millis() - blinkTimer) >= 2 * blinkPeriod) {
    digitalWrite(LED_BUILTIN, LOW);                 // turn the LED off
    blinkTimer = millis();
  }  
 
  
  /*if (voltage > vMax) vMax = voltage;
  if (voltage < vMin) vMin = voltage;
  Serial.print("Sum:  "); Serial.print(voltage); 
  Serial.print("  Max: "); Serial.print(vMax); Serial.print("  Min: "); Serial.println(vMin);   // debug value
  */

}
