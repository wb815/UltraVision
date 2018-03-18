#include <math.h>
#include <SoftwareSerial.h>
#include <TimerOne.h>
#include <string.h>

//Too many bugs, too many many bugs

int trig1 = 2;
int echo1 = 3;
int trig2 = 4; 
int echo2 = 5; 
int audiopin1 = 9;
int audiopin2 = 10;
float mastervol = 2;
double duration1, duration2, cm1, cm2, x, y, hz;
int side;
int amp1, amp2, t, del;



void setup(){
  pinMode(trig1, OUTPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(echo2, INPUT);
  pinMode(audiopin1, OUTPUT);
  pinMode(audiopin2, OUTPUT);
  Timer1.initialize(10);
  Timer1.pwm(audiopin2, 0);
  Timer1.pwm(audiopin1, 0);
  //pinMode(avoid_data, INPUT);
  Serial.begin(9600);
}

//-------------------//
void ping(double& cm1, double&cm2){
  //fix pulse LOW
  digitalWrite(trig1, LOW);
  digitalWrite(trig2, LOW);
  delayMicroseconds(5);
  //10us HIGH pulse to trigger 8 cycle burst
  digitalWrite(trig1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig1, LOW);
  duration1 = pulseIn(echo1, HIGH);
  delayMicroseconds(5);
  digitalWrite(trig2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig2, LOW);
  duration2 = pulseIn(echo2, HIGH);
  
  cm1 = (duration1/2) / 29.1;
  cm2 = (duration2/2) / 29.1;
  
  Serial.print("\ncm1: ");
  Serial.print(cm1);
  Serial.print("   cm2: ");
  Serial.print(cm2);
}
//------------------//
void coords(double cm1, double cm2, double& x, double& y){
 double w = 12;
 int k;
 x = 0; y = 0;
 //Calculate x distance
 if (cm1 < 30 || cm2 < 30){
   x = 0;
   y = 0;
 }
 else{
 double cosa = (sq(w) + sq(cm2) - sq(cm1)) / (2*w*cm2);
 //Select correct sign for w/2
 if (cm2 > cm1){
   k = 9;
 }
 else {
   k = -3;
 }
 x = k - cm1*cosa;
 Serial.print(x);
 //Calculate y distance
 y = cm2*sin(acos(cosa));
 Serial.print(y);
 }
}
//----------------------//
void speak(double x, double y, double cm1, double cm2, double& hz, int& amp1, int& amp2){
  int fmin = 400;
  int fmax = 1500;
  int range = 400;
  double avg = sqrt(pow(x,2) + pow(y,2));
  double diff = cm1 - cm2;
  if (avg < range){
    hz = log(fmax) - log(fmax/fmin)*avg/range;
    hz = pow(2.718,hz);
    amp1 = 200  + 5*diff/(0.0025*range);
    amp2 = 200 - 5*diff/(0.0025*range);
  }

  else {
    hz = 400;
    amp1 = 0;
    amp2 = 0;
  }
    if (cm1 > range || cm2 > range){
    amp1 = 0; amp2 = 0; hz = 400;
  }
} 
//----------------------//
void delfn(double cm1, double cm2, int& del, int& side ){
    del = abs(1000000*(cm2 - cm1)/34300);
    if (cm2 > cm1) {
      side = -1;
    }
    else if (cm1 > cm2) {
    side = 1;
    }
    else{
      side = 0;
    }
}
//---------------------//

void stereo(double hz, int t, int amp1, int amp2, int del, int side){
  //time in us, except for t, current, start
  
  int hperiod = 500000/hz - 7; 
  int ster1, ster2, vol1, vol2;
  //Set the headphone pin ti be delayed
  if (side == -1){
    ster1 = audiopin1;
    ster2 = audiopin2;
    vol1 = amp1;
    vol2 = amp2;
  }
  else if( side == 0){
    ster1 = audiopin1;
    ster2 = audiopin2;
    vol1 = amp1;
    vol2 = amp2;
    del = 0;
  }
  else if (side == 1){
    ster1 = audiopin2;
    ster2 = audiopin1;
    vol1 = amp2;
    vol2 = amp1;
  }
  int start = millis();
  int current = start;
  Timer1.setPwmDuty(ster1, vol1);
  //for frequencies less than 850Hz, where the maximum delay is less than half a period
  if (del < hperiod){
  while(current - start < t){
      delayMicroseconds(del); //start with the smaller delay
      Timer1.setPwmDuty(ster2, vol2);
      delayMicroseconds(hperiod - del); //then the remainder of the larger half period
      Timer1.setPwmDuty(ster1, 0);
      delayMicroseconds(del); //wait the smaller delay
      Timer1.setPwmDuty(ster2, 0);
      delayMicroseconds(hperiod - del - 1); //then the remainder of the larger half period, with a -1 to account for sub-us processing delay
      Timer1.setPwmDuty(ster1, vol1);
      current = millis();
    }
  delayMicroseconds(del);
  Timer1.setPwmDuty(ster2, vol2);
  delayMicroseconds(hperiod - del);
  Timer1.setPwmDuty(ster1, 0);
  delayMicroseconds(del);
  Timer1.setPwmDuty(ster2, 0);
  }
  //for frequencies greater than 850Hz, where the maximum delay could exceed half a period
  else {
    while(current - start < t){
      delayMicroseconds(hperiod); //start with the smaller half period
      Timer1.setPwmDuty(ster2, vol2);
      delayMicroseconds(del - hperiod); //then the remainder of the larger delay
      Timer1.setPwmDuty(ster1, 0);
      delayMicroseconds(hperiod); //wait the smaller half period
      Timer1.setPwmDuty(ster2, 0);
      delayMicroseconds(del - hperiod - 1); //then the remainder of the larger delay
      Timer1.setPwmDuty(ster1, vol1);
      current = millis();
    }
  delayMicroseconds(hperiod);
  Timer1.setPwmDuty(ster2, vol2);
  delayMicroseconds(del - hperiod);
  Timer1.setPwmDuty(ster1, 0);
  delayMicroseconds(hperiod);
  Timer1.setPwmDuty(ster2, 0);
  }
}


void loop(){
  ping(cm1, cm2);
  speak(x, y, cm1, cm2, hz, amp1, amp2);
  Serial.print("\namp1: ");
  Serial.print(amp1);
  Serial.print("\namp2: ");
  Serial.print(amp2);
  Serial.print("\nhz: ");
  Serial.print(hz);
  delfn(cm1, cm2, del, side);
  Serial.print("\nside: ");
  Serial.print(side);
  Serial.print("\ndel: ");
  Serial.print(del);
  stereo(hz, 500, mastervol*amp1, mastervol*amp2, del, side);
  delay(250);
}
