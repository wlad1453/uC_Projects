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

void setup() {
  Serial.begin(115200);  

  timer_init_ISR_1KHz(TIMER_DEFAULT);             // Sampling period is 1 S / 1000 = 1 mS = 1000 uS

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
    // Serial.print(samples[i]); Serial.print("\t"); 
   }
   // Serial.print(F("Phase: ")); Serial.print( phase * 180 / M_PI ); Serial.print(" degrees\n");
}

void effectiveValue(int samples[], int sampNum, float *frequency, float *effectiveValue) { 
  float sampSum(0), average(0);
  float dev;                                    // Signal deviation from the average. Can be positive or negative
  bool halfWavePositive, lastHalfWavePositive;
  float zeroCrossingTime;
  uint8_t cross(0);     // Number of zero crosses from the begin of the sampling
  uint8_t beat(0);      // sample number (?), step (?)
  
  
  for (int i = 0; i < sampNum; i++) {          // Sum of ADC values
      sampSum += samples[i];
    }
  average = sampSum / sampNum;                 // Average value determination
  sampSum = 0;                                 // It will be used later for the sum of squers

  for (int i = 0; i < sampNum; i++) {
    dev = samples[i] - average;
    
    if ( dev >= 0 ) halfWavePositive = true;    // Determines whether the half wave is positive or negative
    else halfWavePositive = false;

    if( i == 0 ) lastHalfWavePositive = halfWavePositive;     // Initial status. First sample has no previous value

    if ( halfWavePositive != lastHalfWavePositive ) {         // Zero crossing condition

      if (cross == 0) {
        zeroCrossingTime = dev / (samples[i-1] - samples[i]);         // The first zero crossing time point. From [i-1]th TO the [i]th sample. < 1 uS
        beat = i;                                                     // Sampling step of the [i]th sample. In our case - time in mS
                                                                      // For the common case the frequebcy of sampling should be introduced as a factor
      }
      if (cross == 18) {          
        *frequency = 1000.0 * 9 / ( zeroCrossingTime + (i - 1 - beat) + 1 - dev / (samples[i-1] - samples[i]) ); 
        // (i-1)  - time point of the [i-1]th sample, beat - time point of the 'previous' [i]th sample by cross == 0
        // dev/(sam[i-1] - sam[i]) - time TO the [i]th sample!!! -> 1 - dev/(s[i-1] - s[i]) - timi after [i-1] sample
        // 9 full sinis waves make 18 zero crossing events. The period of 9 waves is a sum of 
        // zero crossint time (TO the previous [i] sample + number of beats to the current [i-1] sample + time after [i-1] sample to the zero crossing event
      }

      lastHalfWavePositive = halfWavePositive;
      cross++;
    } // End if( halfWavePositive != lastHalfWavePositive )
      
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
