/*
 * 
 * 
 * 
 * https://github.com/sadr0b0t/arduino-timer-api
 * 
 */


#include <math.h>
#include "timer-api.h"

#define maxSampNum 200                // Number of samples. By 50 Hz it means 10 full signal periods with 20 samples within a period.
  
int                 amplitude = 496;

int                 samp[maxSampNum];
int                 sampA[maxSampNum];
unsigned int        sPeriod[maxSampNum]; 
float               frequency(0); 
float               effValue(0);
unsigned long       frameTimer, sampleTimer, period;
int                 k(0);

bool                timerSet = false;
bool                sampling = false;

/*
typedef struct {
  int i;
  float f;
  } myStruct;
  */

void setup() {
  Serial.begin(115200);  

  timer_init_ISR_1KHz(TIMER_DEFAULT);

  randomSeed(analogRead(1));
  
  sin_sim(samp, maxSampNum, amplitude);           // Samples array initialization
  frameTimer = micros();                          // Initial timers setup
  sampleTimer = frameTimer;                       // Initial timers setup
}

void loop() {

  // frameTimer = micros();
  

  
  // critical, time-sensitive code here
  // interrupts();
  /* for (int i = 0; i < maxSampNum; ) {
    
    if( sampling ) {
      sampA[i] = analogRead(A0);                    // Reading duration - 114 uS
      sPeriod[i] = micros() - frameTimer - t;
      t += sPeriod[i];
      i++;        
    }
    sampling = false;     
  }*/

  if ( k == maxSampNum ) {
    noInterrupts();
    sampleTimer = micros(); 
    period = sampleTimer - frameTimer;
    
    Serial.println(F("\nSampling done")); 
  
    for (int i = 0; i < maxSampNum; i++ ) {
      Serial.print(sPeriod[i]); Serial.print("\t");
      if ( i != 0 && !((i+1) % 10) ) Serial.print("\n");
    }
    Serial.print("\n");    
   
    Serial.print(F("Sampling duration: ")); Serial.print( period ); Serial.print(" uS\t");   
    Serial.print(F("One sample: ")); Serial.print( period / ( maxSampNum - 1 ) ); Serial.print(" uS\t");
      
     frameTimer = micros(); 
     
     effectiveValue(samp, maxSampNum, &frequency, &effValue);     
     
     Serial.print(F("Calculation: ")); Serial.print( micros() - frameTimer ); Serial.print(" uS\n");
     
     Serial.print(F("EffValue: ")); Serial.print( effValue ); Serial.print("\t");     
     Serial.print(F("EffCoeff: ")); Serial.print( sq( amplitude / effValue ) ); Serial.print("\t");
     Serial.print( amplitude / effValue ); Serial.print("\t");
     Serial.print(F("Frequency: ")); Serial.println( frequency, 2 ); 

     sin_sim(samp, maxSampNum, amplitude);        // Signal simulation
     
     k = 0;                                       // Resetting sample counter
     frameTimer = micros();                       // Resetting frame timer
     interrupts();
  } // if ( k == maxSampNum )
} // End loop()

void sin_sim(int samples[], int sampNum, int ampl) {
  long int sampSum(0);
  float phase = random( 360 ) * M_PI / 180;
   for (int i = 0; i < sampNum; i++) {
    samples[i] = 512 + ampl * sin(phase + 2 * M_PI / 20 * i) + random( 10 );   
    sampSum += samples[i];
    // Serial.println(samples[i]); 
   }
   Serial.print(F("Phase: ")); Serial.print( phase * 180 / M_PI ); Serial.print(" degrees\n");
  }

void effectiveValue(int samples[], int sampNum, float *frequency, float *effectiveValue) { 
  float sampSum(0), average(0);
  float dev;                                    // Signal deviation from the average. Can be positive and negative
  bool positiveHalfWave, lastHalfWave, zeroCrossing;
  float zeroCrossingTime;
  uint8_t cross(0), beat(0);
  
  
  for (int i = 0; i < sampNum; i++) {          // Sum of ADC values
      sampSum += samples[i];
    }
   average = sampSum / sampNum;                 // Average value determination
   sampSum = 0;

  for (int i = 0; i < sampNum; i++) {
    dev = samples[i] - average;
    if ( dev >= 0 ) positiveHalfWave = true; 
    else positiveHalfWave = false;

    if( i == 0 ) lastHalfWave = positiveHalfWave;

    if ( positiveHalfWave != lastHalfWave ) {         // Zero crossing detection
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

      lastHalfWave = positiveHalfWave;
      cross++;
    } // End if( positiveHalfWave != lastHalfWave )
      
    dev *= dev;                           // squer value of 'dev'
    sampSum += dev;                       // sum of squers
  }
  *effectiveValue = sqrt( sampSum /= sampNum );  
}

void timer_handle_interrupts(int timer) {
  static unsigned long prev_time = 0;
  // static unsigned long prev_msm = 0;
    
  unsigned long _time = micros();
  unsigned long _period = _time - prev_time;
  prev_time = _time;

  if ( k == 0 ) sPeriod[k] = 0;                 // The first measurement has no previous event
  else sPeriod[k] = _period;                    // The period between two sampling events

  sampA[k] = analogRead(A0);                    // Reading duration - 114 uS     
  k++;                                          // Global sample counter
}
