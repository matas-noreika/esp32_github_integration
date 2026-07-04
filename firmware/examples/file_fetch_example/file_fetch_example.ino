/*
 * Programmer: Matas Noreika
 * Date: 2026-07-04
 * Purpose:
 * simple program that uses the arduino core WiFiClientSecure object to send a simple GET request to a JSON file stored on the github repository for the project.
 * Github repository: https://github.com/matas-noreika/esp32_github_integration
 * File link on repository: 
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const char* ssid     = "Matas ltu";
const char* password = "matas-ltu";

// Example: https://raw.githubusercontent.com/espressif/arduino-esp32/master/README.md
const char* url = "https://raw.githubusercontent.com/matas-noreika/esp32_github_integration/main/data/images.json";

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nConnected, IP: " + WiFi.localIP().toString());

  WiFiClientSecure client;
  client.setInsecure();  // skip certificate validation

  HTTPClient https;
  https.
  Serial.println("Requesting: " + String(url));

  if (https.begin(client, url)) {
    int httpCode = https.GET();

    if (httpCode > 0) {
      Serial.printf("HTTP response code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        String payload = https.getString();
        Serial.println("---- File content ----");
        Serial.println(payload);
        Serial.println("---- End of file ----");
      }
    } else {
      Serial.printf("GET failed: %s\n", https.errorToString(httpCode).c_str());
    }

    https.end();
  } else {
    Serial.println("Unable to connect");
  }
}

void loop() {
  // nothing here
}