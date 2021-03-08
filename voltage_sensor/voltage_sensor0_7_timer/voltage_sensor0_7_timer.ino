/*
 * 
 * 
 * 
 * https://github.com/sadr0b0t/arduino-timer-api
 * 
 */


#include <math.h>

#include "timer-api.h"

// #define Pi 3.1415926535
#define maxSampNum 200                // Number of samples. By 50 Hz it means 10 full signal periods with 20 samples within a period.
  
int       amplitude = 496;

int       samp[maxSampNum];
int       sampA[maxSampNum];
int       sPeriod[maxSampNum]; 
float     frequency(0); 
float     effValue(0);
long unsigned int  timerFrame, timerSample, period;
uint32_t  t(0);
uint32_t  n;

bool      timerSet = false;
bool      sampling = false;

/*
typedef struct {
  int i;
  float f;
  } myStruct;
  */

void setup() {
  Serial.begin(115200);  

  timer_init_ISR_1KHz(TIMER_DEFAULT);
  
  sin_sim(samp, maxSampNum, amplitude);          // Samples array initialization
  timerFrame = timerSample = micros();    // Initial timers setup
  // Serial.print(F("Size of: ")); Serial.print( sizeof(uint64_t) ); Serial.println(" uS");
  // Serial.print(F("Size of: ")); Serial.print( sizeof(t) ); Serial.println(" uS");
  // t = 4000000000;
  // Serial.print(F("T max: ")); Serial.println(t);
  
}

void loop() {

  timerFrame = micros();
  t = 0;

  
  // critical, time-sensitive code here
  // interrupts();
  for (int i = 0; i < maxSampNum; ) {
       
    if( sampling ) {
      sampA[i] = analogRead(A0);                    // Reading duration - 114 uS
      sPeriod[i] = micros() - timerFrame - t;
      t += sPeriod[i];
      i++;        
    }
    sampling = false;  
  }
  // noInterrupts();
  timerSample = micros(); 

  Serial.println(F("\nSampling done")); 

  for (int i = 0; i < maxSampNum; i++ ) {
    Serial.print(sPeriod[i]); Serial.print("\t");
    if ( i != 0 && !((i+1) % 10) ) Serial.print("\n");
  }
  Serial.print("\n");
  /*
   t = micros(); n = 0;
   for (uint32_t i = 0; i < 4; i++) {
    for (uint32_t j = 0; j < 100; j++) {
      for (uint32_t l = 0; l < 100; l++) { n = pow( l, 2); }
      }
      // n = i;
    }*/
   
  period = timerSample - timerFrame;
  Serial.print(F("For loop duration: ")); Serial.print( period ); Serial.print(" uS\t");   
  Serial.print(F("Sample T: ")); Serial.print( period / maxSampNum ); Serial.println(" uS");
    
     timerFrame = micros(); 
     Serial.print(F("Calcul T: ")); Serial.print( timerFrame ); Serial.print(" uS\t");
     effectiveValue(samp, maxSampNum, &frequency, &effValue);     
     Serial.print(F("Calcul T: ")); Serial.print( micros() ); Serial.print(" uS\t");
     Serial.print(F("Calcul T: ")); Serial.print( micros() - timerFrame ); Serial.print(" uS\t");
     
     Serial.print(F("EffValue: ")); Serial.print( effValue ); Serial.print("\t");     
     Serial.print(F("EffCoeff: ")); Serial.print( sq( amplitude / effValue ) ); Serial.print("\t");
     Serial.print( amplitude / effValue ); Serial.print("\t");
     Serial.print(F("Frequency: ")); Serial.println( frequency, 2 ); 
//      i = 0;
     timerFrame = timerSample = micros();
   //}
} // End loop()

void sin_sim(int samples[], int sampNum, int ampl) {
  long int sampSum(0);
   for (int i = 0; i < sampNum; i++) {
    samples[i] = 512 + ampl * sin(2 * M_PI / 20 * i);   
    sampSum += samples[i];
    // Serial.println(samples[i]); 
   }
  }

void effectiveValue(int samples[], int sampNum, float *frequency, float *effectiveValue) { 
  float sampSum(0), average(0);
  float dev;
  bool halveWave, lastHalve, zeroCrossing;
  float zeroCrossingTime;
  uint8_t cross(0), beat(0);

  if (samples[0] >= 0) {
    halveWave = true;
    lastHalve = true;
    }
  else {
    halveWave = false;
    lastHalve = false;
    }
  
   for (int i = 0; i < sampNum; i++) {   // ADC average value determination
      sampSum += samples[i];
    }
   average = sampSum / sampNum;
   sampSum = 0;
   
   for (int i = 0; i < sampNum; i++) {
      dev = samples[i] - average;
      if ( dev >= 0 ) halveWave = true; 
      else halveWave = false;

      if ( halveWave != lastHalve ) {
        zeroCrossing = true;
        
        if (cross == 0) {
          zeroCrossingTime = dev / (samples[i-1] - samples[i]);         // Zero crossing time TO the [i]th sample
          beat = i;                                                     // Time of the [i]th sample    
          }
        if (cross == 18) {          
          *frequency = 1000.0 * 9 / ( zeroCrossingTime + (i - 1 - beat) + 1 - dev / (samples[i-1] - samples[i]) ); // 1000, 999.7 with correction coeff.
          // 9 full sinis waves make 18 zero crossing events. The period of 9 waves is a sum of 
          // zero crossint time (to the previous [i] sample + number of beats to the current [i-1] sample + time after [i-1] sample to the zero crossing event
          }

        lastHalve = halveWave;
        cross++;
        } // End if( halveWave != lastHalve )
      
      dev *= dev;
      sampSum += dev;
    }
    *effectiveValue = sqrt( sampSum /= sampNum );  
  }

  void timer_handle_interrupts(int timer) {
    static unsigned long prev_time = 0;
    
    unsigned long _time = micros();
    unsigned long _period = _time - prev_time;
    prev_time = _time;

    sampling = true;
    
    //Serial.print("goodbye from timer: ");
    //Serial.println(_period, DEC);

    // blink led
    // мигаем лампочкой
    // digitalWrite(13, !digitalRead(13));
}
