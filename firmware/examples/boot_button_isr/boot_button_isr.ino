/*
 * Programmer: Matas Noreika
 * date: 2026-07-06
 * Purpose:
 * Program that showcases how to implement Interrupt Service Routines(ISR's) using arduino core libraries for ESP32.
 * This program utilises GPIO0 (boot button) to prevent the need of using any messy breadboard configurations😁.
*/


const uint8_t button = 0;
volatile uint8_t state = 0; // starts in a low state
uint8_t old_state = 0; //start in a low state too

//definintion of button ISR
void ARDUINO_ISR_ATTR buttonISR(){
  state ^= 1; // toggle the state (XOR bitwise)
}

void setup() {
  pinMode(button, INPUT_PULLUP);
  Serial.begin(115200);
  attachInterrupt(button, buttonISR, FALLING);
  delay(1000);
  startSTA();
}

void loop() {
  if(old_state != state){
    old_state = state; // set the old_State to the previous
    Serial.print("State changed: ");
    Serial.println(state);
    
  }
}
