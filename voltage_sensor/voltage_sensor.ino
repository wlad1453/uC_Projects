#include <math.h>

#define Pi 3.1415926535

const int maxSampNum = 200;
const float amplitude = 496;

int samp[maxSampNum];
int sampA[maxSampNum];
float frequency(0);
long int timer;

void setup() {
  Serial.begin(9600);
  // Samples area initialization
/*
  for (int i = 0; i < maxSampNum; i++) {
    samp[i] = 512 + 496 * sin(2*Pi/20*i);   
    average += samp[i];
    Serial.println(samp[i]); 
    }
    average /= maxSampNum;
     */

    sin_sim(samp, maxSampNum);
}

void loop() {

   Serial.print("Start reading: "); Serial.println(timer = micros()); 
   for (int i = 0; i < maxSampNum; i++) {
    sampA[i] = analogRead(A0);
    //Serial.println(sampA[i]); 
    }
   Serial.print("Stop reading: "); Serial.println( (micros() - timer) / 200 ); 
   Serial.print("Effective value: "); Serial.println( effectiveValue(samp, maxSampNum, &frequency) ); 
   Serial.print("Coeff1: "); Serial.println( effectiveValue(samp, maxSampNum, &frequency) / amplitude ); 
   Serial.print("Coeff2: "); Serial.println( sq( amplitude / effectiveValue(samp, maxSampNum, &frequency) ) ); 
  Serial.print("Frequency: "); Serial.println( frequency ); 
  // put your main code here, to run repeatedly:
}

void sin_sim(int samples[], int sampNum) {
  long int sampSum(0);
   for (int i = 0; i < sampNum; i++) {
    samples[i] = 512 + 496 * sin(2*Pi/20*i);   
    sampSum += samples[i];
    Serial.println(samples[i]); 
   }
  }

float effectiveValue(int samples[], int sampNum, float * frequency) { 
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
  
   for (int i = 0; i < sampNum; i++) {
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
          *frequency = 1000 * 9 / ( zeroCrossingTime + (i - 1 - beat) + 1 - dev / (samples[i-1] - samples[i]) );
          // 9 full sinis waves make 18 zero crossing events. The period of 9 waves is a sum of 
          // zero crossint time (to the previous [i] sample + number of beats to the current [i-1] sample + time after [i-1] sample to the zero crossing event
          }

        lastHalve = halveWave;
        cross++;
        } // End if( halveWave != lastHalve )
      
      dev *= dev;
      sampSum += dev;
    }
    return sqrt( sampSum /= sampNum );  
  }
