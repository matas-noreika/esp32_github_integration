#include <WiFi.h>

const char *ap_ssid = "ESP32-Config";
const char *ap_pass = "123456789"; 
const char *sta_ssid = "Matas ltu";
const char *sta_pass = "matas-ltu"; 

// set a server with max 1 client connection
WiFiServer server(80, 1);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  Serial.print("Connecting to ");
  Serial.println(sta_ssid);
  WiFi.begin(sta_ssid, sta_pass);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println(".");
  }
  Serial.println("\nSTA Connected!");
  Serial.print("STA IP Address: ");
  Serial.println(WiFi.localIP());
  // Start the Access Point
  boolean ap_success = WiFi.softAP(ap_ssid, ap_pass);
  if(ap_success){
    Serial.println("AP Started!");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("AP Failed to start.");
  }
}

void loop() {
  WiFiClient client = server.accept();

  if(client){
    
  }

}
