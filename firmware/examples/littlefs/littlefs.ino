#include <LittleFS.h>

void setup() {
  Serial.begin(115200);
  if(!LittleFS.begin(false)){
    Serial.println("Mount failed!!!");
    return;
  }
  Serial.println("LittleFS mounted successfully");
  // Show total/used space
  Serial.printf("Total: %u bytes, Used: %u bytes\n", LittleFS.totalBytes(), LittleFS.usedBytes());
  Serial.println("Reading /page.html");
  File file = LittleFS.open("/page.html", FILE_READ);
  if(!file){
    Serial.println("Failed to open /page.html");
    return;
  }
  Serial.print("- content: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  Serial.println();
  file.close();
}

void loop() {
  // put your main code here, to run repeatedly:

}
