
#define OUT_PIN 4   //HIGH, if wind speed is in the desired range
#define ALARM_PIN 5 //HIGH, if wind speed is above a "critical value"(speedThresh)
#define INTERRUPT 2 //If this is HIGH, process() will be executed
unsigned long lastInterrupt = micros();
byte outp = LOW;
unsigned int interval = 5000; //measurement interval in milliseconds
unsigned long startTime = 0; //Helps count the time
unsigned int speedInMPS = 0; //the speed in m/s
unsigned short speedThresh = 25; //speed in m/s, change this to a value you like
unsigned long resetAfterMicros = 1000000; //wait this amount of microseconds for a new pulse to arrive, otherwise reset outp to LOW
unsigned long intervalThresh = 1000000/( speedThresh *4); //minimum falling edge interval in microseconds, don't change this
byte alarm_internal = LOW;
byte alarm = LOW;
byte updateTime = HIGH;

void setup() {
  pinMode(OUT_PIN, OUTPUT);
  pinMode(INTERRUPT, INPUT);
  pinMode(ALARM_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT), process, FALLING);
  Serial.begin(9600);
}

void loop() {
  //detachInterrupt(digitalPinToInterrupt(INTERRUPT));
  
  switch(alarm_internal){
    case LOW:
      startTime = millis();
      break;
    default:
      break;
  }
  //We have to multiply the millis()-startTime with 2 to get the real time value
  //(probably because we are running on an 8MHz clock and not a 16MHz one, which makes
  //the time measured about half as big)
  switch((millis()-startTime)*2 >= interval){
    case true:
      //this gets executed if the wind speed is greater than the "critical value" defined in speedThresh
      alarm = HIGH;
      break;
    default:
      alarm = LOW;
      break;
  }
  digitalWrite(OUT_PIN,outp);
  digitalWrite(ALARM_PIN, alarm);

  //If process doesn't get executed for resetAfterMicros microseconds,
  //it gets executed again just to update the state of outp.
  unsigned long timeNow = (micros()-lastInterrupt)*2;
  switch( ( timeNow >=resetAfterMicros) && ( timeNow < (resetAfterMicros+(resetAfterMicros/2))) ){
    case true:
      updateTime = LOW; //This and the following lines are, so that process() doesn't update the time when the
                        //oscillating source has been disconnected or just doesn't react within a given time.
                        //This way we can scan for a range of time after the last interrupt and avoid wasting power
                        //on checking anything beyond that. 
                        //The only reason why this switch is necessary at all is, because it falsely keeps the
                        //outp tied high when you disconnect the external oscillator. 
                        //And no, you can't just pass updateTime as an argument to process(), because process(),
                        //as it is the ISR, must not take any arguments or return any values. So, this is pretty much
                        //just a roundabout way to pass an argument.
      process();
      updateTime = HIGH;
      Serial.println("a");
      break;
    default:
      break;
  }
  //attachInterrupt(digitalPinToInterrupt(INTERRUPT), process, FALLING);
}

//variables [count] and [sum] are for troubleshooting purposes
void process() {
  detachInterrupt(digitalPinToInterrupt(INTERRUPT));
  unsigned long TI = micros();
  unsigned long TIint = (TI-lastInterrupt)*2;
  //compare the current time to the time of the last interrupt 
  //and when that is greater than 5ms (200 ticks/s or 50 m/s) 
  //or smaller than 250ms (4 ticks/s or 1 m/s)
  //then the code in the statement gets executed
  //right now it is triggering in a range from 1 to 50 m/s
  switch( ( TIint >= 5000) && ( TIint <= 250000) ){
    case true:
      // Code, that gets executed if the frequency is in the specified range
      outp = HIGH;
      switch(TIint < intervalThresh){
        case true:
          alarm_internal = HIGH;
          break;
        default:
          alarm_internal = LOW;
          break;
      }
      break;
    default:
      outp = LOW;
      alarm_internal = LOW;
      break;
  }
  
  lastInterrupt = updateTime == HIGH ? TI : lastInterrupt;
  attachInterrupt(digitalPinToInterrupt(INTERRUPT), process, FALLING);
}
