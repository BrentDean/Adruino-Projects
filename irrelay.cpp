#include <Arduino.h>
#include <IRremote.h>

#define RELAY_PIN 23    // Relay connected to GPIO 23
#define IR_RECEIVER_PIN 22 // IR Receiver connected to GPIO 22

IRrecv irrecv(IR_RECEIVER_PIN);
decode_results results;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  irrecv.enableIRIn(); // Start the receiver
  Serial.begin(9600);
}

void loop() {
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    if (results.value == 0x1FE50AF) { // Replace this with your IR remote button's HEX code
      digitalWrite(RELAY_PIN, HIGH); // activate the relay
      delay(5000); // keep the relay activated for 5 seconds
      digitalWrite(RELAY_PIN, LOW); // deactivate the relay
    }
    irrecv.resume(); // Receive the next value
  }
  delay(100);
}