#include <Wire.h>
#include <TimerOne.h>

// ---- GLOBALS ----
// General globals
const int bitPin = 13;
const int ctrlPin = 12;
int sample = 0;
int lastSample = 0;
bool handleSample;

// Midfilter globals
const int N = 20;
int count = 0;
int highSum = 0;
int lowSum = 0;

// Recursive filter globals
int lastHigh = 0;
int lastLow = 0;
const float aHigh = -0.5;
const float aLow = 0.5;

// Verification signal globals (limits of the middled high pass output)
const int highFreqLowLim = 144;
const int highFreqHighLim = 158;
const int lowFreqLowLim = 31;
const int lowFreqHighLim = 33;

// ---- FUNCTIONALITY ----
// Setup
void setup() {
  Timer1.initialize(500);
  Timer1.attachInterrupt(takeSample); 
  pinMode(bitPin, OUTPUT);
  pinMode(ctrlPin, OUTPUT);
  Serial.begin(9600);
}

// Function returning absolute value of num
int absolute(int num){
  return (num < 0) ? -num:num;
}

// Looped code
void loop() {
  if(handleSample){
    int currSample = sample; // to be certain that we're handling the same sample throughout the code block, as an interrupt may occur during the runtime of it, changing the value of sample
    
    // High pass recursive filter
    int highPassed = (currSample - lastSample + aHigh*lastHigh)*(1+aHigh);
    lastHigh = highPassed;
    highSum += absolute(highPassed);
    
    // Low pass recursive filter
    int lowPassed = (currSample - lastSample + aLow*lastLow)*(1-aLow);
    lastLow = lowPassed;
    lowSum += absolute(lowPassed);

    // Midfilter for high passed and low passed values respectively
    if(++count == N){
      int highAvg = 1.0/N*highSum;
      int lowAvg = 1.0/N*lowSum;
      int output = highAvg - lowAvg;
      int ctrl = ((highAvg < highFreqLowLim || highAvg > highFreqHighLim) && (highAvg < lowFreqLowLim || highAvg > lowFreqHighLim)) ? LOW:HIGH; 
      output = (output < 0) ? LOW:HIGH;
      digitalWrite(bitPin, output);
      digitalWrite(ctrlPin, ctrl);
      count = 0;
      highSum = 0;
      lowSum = 0;
    }
    lastSample = currSample;
    handleSample = false;
  }    
}

// Interrupt-handler
void takeSample(void){
  sample = analogRead(0); // Sampler på A0
  handleSample = true;
}
