//Wireless board communication
//When IOO6 activated, other board turns on LED

const int INPUT_PIN = 5;

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }   // important for C3 USB
  pinMode(INPUT_PIN, INPUT);
}

void loop() {
  Serial.println(digitalRead(INPUT_PIN));
  delay(200);
}
