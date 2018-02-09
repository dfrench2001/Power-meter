

#define ITERATIONS 500000L    // number of iterations

int percent = 0;
int progress = 1;
int buttonState = 0;    

void setup(void) {

  Serial.begin(115200);
  Serial.println("Sketch started");
  delay(1000);
  }

void loop() {
    Serial.println("Calc started");
    startCalculation();
  }
  
void startCalculation()
{
 unsigned long start, time;
 unsigned long niter=ITERATIONS;
 int LEDcounter = 0;
 unsigned long i;
 float x = 1.0;
 float pi=1.0;

 Serial.println( "begin the calc");
 start = millis();  
 for ( i = 2; i < niter; i++) {
   x *= -1.0;
   pi += x / (2.0f*(float)i-1.0f); 
 }

 time = millis() - start;
 Serial.print("Start millis = ");Serial.println(time);
 pi = pi * 4.0;

 String piString = String(pi,7);
 String timeString = String(time)+"ms";
  
 Serial.println(piString);

 Serial.println(timeString);
}

