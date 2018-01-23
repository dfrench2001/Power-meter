


int led = 14;

void setup() {
  // put your setup code here, to run once:
  pinMode(led, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
// Make the LED blink
  digitalWrite(led, HIGH);
  delay(2000);
  digitalWrite(led,LOW);
  delay(2000);
}
