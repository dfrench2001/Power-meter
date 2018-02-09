//
// Pi_2
//
// Steve Curd
// December 2012
//
// This program approximates pi utilizing the Newton's approximation.  It quickly
// converges on the first 5-6 digits of precision, but converges verrrry slowly
// after that.  For example, it takes over a million iterations to get to 7-8
// significant digits.
//
// For demonstration purposes, drives a JY-LKM1638 display module to show the
// approximated value after each 1,000 iterations, and toggles the pin13 LED for a
// visual "sign of life".
//
// I wrote this to evaluate the performance difference between the 8-bit Arduino Mega,
// and the 32-bit Arduino Due.
//
// Benchmark results for 100,000 iterations (pi accurate to 5 significant digits):
//
// Due: 1785 ms
// Mega: 6249 ms
//
// 1638 display module connections:
// VCC -> 3.3v
// GND -> GND
// DIO -> Pin 8
// CLK -> Pin 9
// STB0 -> Pin 7
//
//
#define ITERATIONS 2000000L    // number of iterations

void setup() {

	 Serial.begin(115200);
}

void loop() {
 
 unsigned long start, time;
 unsigned long niter=ITERATIONS;
 int LEDcounter = 0;
 boolean alternate = false;
 unsigned long i, count=0;        /* # of points in the 1st quadrant of unit circle */
 unsigned long limit=100000;
 double x = 1.0;
 double temp, pi=1.0;

 start = millis();  

 Serial.print("Beginning ");
 Serial.print(niter);
 Serial.println(" iterations...");
 Serial.println();
 
 count=0;
 for ( i = 2; i < niter; i++) {
   x *= -1.0;
   pi += x / (2.0*(double)i-1);
   if (count++ > limit)
   { count = 0;
      Serial.print(i);Serial.print("  ");Serial.print("pi= ");Serial.println(pi);
      delay(10);
   }
 }

 time = millis() - start;
 
 pi = pi * 4.0;

 Serial.print("# of trials = ");
 Serial.println(niter);
 Serial.print("Estimate of pi = ");
 Serial.println(pi, 10);
 
 Serial.print("Time: "); Serial.print(time); Serial.println(" ms");
 
 delay(10000);
}

