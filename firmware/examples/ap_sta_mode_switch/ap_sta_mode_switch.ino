/*
 * Programmer: Matas Noreika
 * date: 2026-07-06
 * Purpose:
 * This program uses the boot pin to toggle between access point and station mode on the esp32.
*/

#include <WiFi.h>

const char *ap_ssid = "ESP32-Config";
const char *ap_pass = "123456789"; 
const char *sta_ssid = "Matas ltu";
const char *sta_pass = "matas-ltu"; 

//definintion of button ISR
void ARDUINO_ISR_ATTR buttonISR(){
  state ^= 1; // toggle the state (XOR bitwise)
}

void startSTA(){
  Serial.println("\nSwitching to Station (STA) Mode...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(sta_ssid, sta_pass);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void startAP(){
  Serial.println("Starting up AP mode....");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);
  Serial.print("Soft AP IP: ");
  Serial.println(WiFi.softAPIP());
}



void setup() {
  pinMode(button, INPUT_PULLUP);
  Serial.begin(115200);
  attachInterrupt(button, buttonISR, FALLING);
}

void loop() {
  if(old_state != state){
    old_state = state; // set the old_State to the previous
    Serial.print("State changed: ");
    Serial.println(state);
    switch(state){
      case 0:
        startSTA();
        break;
      case 1:
        startAP();
        break;
    }
  }
}
