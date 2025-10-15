void setup() {
  pinMode(2, OUTPUT); // Built-in LED (GPIO2 or D4)
}

void loop() {
  digitalWrite(2, LOW);  // LED ON
  delay(1000);
  digitalWrite(2, HIGH); // LED OFF
  delay(1000);
}
