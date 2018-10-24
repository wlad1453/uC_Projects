/*
  
*/

#define sensorPin_1 1
#define sensorPin_2 2
#define sensorPin_3 3
#define relayPin 12


boolean switching(false);  // Any switching process is going on
boolean relayOn(false);    // Status of the relay

unsigned long blinkTimer(0);
unsigned long onOffTimer(0);

int switchingOnDelay(5  000), switchingOffDelay(15000);
int blinkPeriod(1000);

int sensorSignal[3][10]{};        // analog signal from 3 voltage sensors X 10 samples
int voltage[3]{};                 // average voltage signal from 3 sensors
int average = 513;                // threshold on/off
int voltageSum(0), vMax(10), vMin(10000);


// *****------ S-E-T-U-P---------****************************************
void setup() {
  Serial.begin(9600);           // Used to type in characters and for setUp
  
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
