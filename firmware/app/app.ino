/*
 * Programmer: Matas Noreika
 * Date: 2026-07-04
 * Purpose:
 * 
 * Github repository: https://github.com/matas-noreika/esp32_github_integration
 * File link on repository: https://raw.githubusercontent.com/matas-noreika/esp32_github_integration/refs/heads/image-data/data/current.csv
*/

#include <WiFi.h> // Main WiFi library that enables all wifi functionality
#include <WiFiClientSecure.h> // enables HTTPS pipe communication
#include <HTTPClient.h> // wrapper to simplify WiFiClientSecure pipe
#include <WebServer.h> // wrapper for WiFiServer to simplify development
#include <Adafruit_ST7789.h> // TFT screen library (will possible be replaced with e_tft.h)
#include <SPI.h> // SPI communication
#include <esp_timer.h> // Internal ESP32 timer (better than millis())
#include <LittleFS.h> // File System
#include <Preferences.h> // nvs memeory partition
#include "tft.h" // TFT screen configuration

#define CARRY_BUFFSIZE 10 // max size of partial token needed to be carried (6 (max token size) + 4 (overhead))
#define READ_BUFFSIZE 1024 // max size of chunk to read from HTTP stream
#define TOKEN_SIZE 7 // size of hex code expected 0x0000\0 <- format

const char* root_url = "https://raw.githubusercontent.com/matas-noreika/esp32_github_integration/refs/heads/image-data/data/";

//create image data map
uint16_t image[240*135*2] = {0};
uint8_t carryBuf[CARRY_BUFFSIZE];
size_t carryLen;
size_t imagebuf_index = 0;

int64_t updateTime = 0;

//create handler objects
SPIClass *tftSPI = new SPIClass(HSPI);
Adafruit_ST7789 tft = Adafruit_ST7789(tftSPI, TFT_CS, TFT_DC, TFT_RST);

WebServer server(80);
Preferences prefs;

//web server root handler
void handleRoot(){
  File file = LittleFS.open("/index.html", "r");
  if(!file){
    server.send(404, "text/plain", "index.html not found!");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

//web server ap handler
void handleAP(){
  if(!server.hasArg("ap_ssid") || !server.hasArg("ap_pass")){
    server.send(400, "text/plain", "Missing ssid or password!");
    return;
  }
  prefs.begin("APConfig", false);
  if(0 >= prefs.putString("ap_ssid", server.arg("ap_ssid")) || 0 >= prefs.putString("ap_pass", server.arg("ap_pass"))){
    server.send(500, "text/plain", "Failed to store credentials!");
  }else{
    server.send(200, "text/plain", "Credentials saved!\nRedirecting...");
    server.sendHeader("Location", "/");
    server.send(303);
    handleRoot();
  }
  prefs.end();
  //reconfigures ap with new credentials
  WiFi.softAP(server.arg("ap_ssid"), server.arg("ap_pass"));
}

//web server ap handler 
void handleSTA(){
  if(!server.hasArg("sta_ssid") || !server.hasArg("sta_pass")){
    server.send(400, "text/plain", "Missing ssid or password!");
    return;
  }
  prefs.begin("STAConfig", false);
  if(0 >= prefs.putString("sta_ssid", server.arg("sta_ssid")) || 0 >= prefs.putString("sta_pass", server.arg("sta_pass"))){
    server.send(500, "text/plain", "Failed to store credentials!");
  }else{
    server.send(200, "text/plain", "Credentials saved!\nRedirecting...");
    server.sendHeader("Location", "/");
    server.send(303);
    handleRoot();
  }
  prefs.end();
  //reconfigures ap with new credentials
  WiFi.disconnect();
  WiFi.begin(server.arg("sta_ssid"), server.arg("sta_pass"));
}

void handleStatus(){
  server.send(200, "text/plain", "Server connected: "
    + String(WiFi.status() == WL_CONNECTED) 
    + "\n Next image update: " 
    + String((esp_timer_get_time() - updateTime - 600000000)/60000000) 
    + " mins");
}

void handleRefresh(){
  if(WiFi.status() == WL_CONNECTED){
    server.send(200, "text/plain", "Refreshing page image");
    updateTime = esp_timer_get_time();
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(0,0);
    tft.println("Updating...");
    fetchImageData();
    tft.drawRGBBitmap(0,0,image,SCREEN_WIDTH,SCREEN_HEIGHT);
    handleRoot();
  }else{
    server.send(500, "text/plain", "server is not connected to wifi");
  }
}

void handleImage(){
  if(!server.hasArg("target")){
    server.send(400, "text/plain", "please provide a image name");
    return;
  }
  prefs.begin("IMGConfig", false);
  if(0 >= prefs.putString("target", server.arg("target"))){
    server.send(500, "text/plain", "failed to set target");
  }else{
    handleRefresh();
  }
  prefs.end();
}

//method to configure AP mode portion of operation
bool startAP(){
  Serial.println("Starting AP...");
  prefs.begin("APConfig", true); // open to read in READONLY
  String ap_ssid = prefs.getString("ap_ssid", "ESP32-CONFIG");
  String ap_pass = prefs.getString("ap_pass", "123456789");
  prefs.end(); // close
  // Force a specific IP for the AP interface
  IPAddress apIP(10, 0, 0, 1);
  IPAddress apGateway(10, 0, 0, 1);   // typically same as apIP for softAP
  IPAddress apSubnet(255, 255, 255, 0);
  WiFi.softAPConfig(apIP, apGateway, apSubnet);
  WiFi.softAP(ap_ssid, ap_pass);
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/refresh", handleRefresh);
  server.on("/ap", HTTP_POST, handleAP);
  server.on("/sta", HTTP_POST, handleSTA);
  server.on("/image", HTTP_POST ,handleImage);
  server.begin(); // starts web server
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0,0);
  tft.println("Server AP IP: " + WiFi.softAPIP().toString());
  Serial.println("AP started!");
  return true;
}

//method to configure the station mode portion of operation (connection to router)
bool startSTA(){
  Serial.println("Starting STA...");
  prefs.begin("STAConfig", true);
  String sta_ssid = prefs.getString("sta_ssid", "");
  String sta_pass = prefs.getString("sta_pass", "");
  prefs.end();
  WiFi.begin(sta_ssid,sta_pass);
  tft.println("Connecting to WiFi");
  delay(1500); // wait 1.5 to give time to connect
  switch (WiFi.status()){
    case WL_CONNECT_FAILED: tft.println("Failed to connect!"); return false; 
    case WL_CONNECTED: tft.println("Connected, IP: " + WiFi.localIP().toString()); return true;
    default: return false;
  }
}

// Called once per complete token (already null-terminated)
void handleToken(const char* token) {
  if (strlen(token) == 0) return;  // skip empty tokens (e.g. from "\n" right after ",")

  char* end; // pointer used for error handling
  long value = strtol(token, &end, 16);
  
  if (end == token || *end != '\0') { //if non of the characters were evaluated or only partially
    //so error handling
  } else {
    image[imagebuf_index] = value;
    imagebuf_index++;
  }
}

// Processes one chunk of raw bytes, handling tokens that span chunk boundaries
void processChunk(const uint8_t* data, size_t len) {
  size_t tokenStart = 0;
  carryLen = 0;  // consumed; will be repopulated below if this chunk also ends mid-token

  char tokenBuf[TOKEN_SIZE];
  // data pre-processing (take all valid tokens in chunk)
  for(size_t i = 0; i < len; i++){
    // current character in stream
    char c = data[i];

    //check if the delimiter characters are hit
    if(c == ',' || c == '\n' || c == '\r'){
      size_t tokenLen = i - tokenStart;
      
      //check if the token is of valid length
      if(tokenLen < TOKEN_SIZE){
        //copy token to token buffer
        memcpy(tokenBuf, (uint8_t *)data+tokenStart, tokenLen);
        tokenBuf[tokenLen] = '\0'; // null terminate to form string
        handleToken(tokenBuf); // process the token
      }else{
        // Serial.println("Token too large discarding");
      }
      //reference the new token start
      tokenStart = i + 1;
    }
  }

  //check for remaining characters (carry over material)
  size_t remaining = len - tokenStart;
  if(remaining > 0){
    if(remaining < TOKEN_SIZE){
      memcpy(carryBuf, data+tokenStart, remaining);
      carryLen = remaining;
    }else {
      Serial.println("Carry over too large, discarding");
      carryLen = 0;
    }
  }else {
    carryLen = 0;
  }
  // char *token = strtok(data, ",");

  // while(token != NULL){
  //   handleToken(token);
  //   token = strtok(NULL, ",");
  // }

}

void fetchImageData(){
  imagebuf_index = 0;
  WiFiClientSecure client; // create socket handle
  client.setInsecure(); // disable certificate validation
  HTTPClient https; // create a http handle
  prefs.begin("IMGConfig", true);
  String target = prefs.getString("target", "current.csv");
  prefs.end();
  if (https.begin(client, root_url + target)) { // http pipe established
    int httpCode = https.GET();
    if (httpCode == HTTP_CODE_OK) {
      int contentLength = https.getSize();
      WiFiClient* stream = https.getStreamPtr();
      // Combined buffer: carry-over space + fresh read space
      uint8_t combinedBuf[CARRY_BUFFSIZE + READ_BUFFSIZE];
      while (https.connected() && (contentLength > 0 || contentLength == -1)) {
        size_t availableBytes = stream->available();
        if (availableBytes) {
          memset(combinedBuf, 0, sizeof(combinedBuf));
          if (carryLen > CARRY_BUFFSIZE) {
            Serial.printf("CORRUPTION: carryLen=%u exceeds CARRY_BUFFSIZE=%u\n", carryLen, CARRY_BUFFSIZE);
            // handle/reset rather than proceeding
          }
          // 1. Copy any carried-over partial token to the front of the buffer
          memcpy(combinedBuf, carryBuf, carryLen);
          // 2. Read new bytes directly after the carry-over
          size_t toRead = min((size_t)READ_BUFFSIZE, availableBytes);
          int readBytes = stream->readBytes((uint8_t*)(combinedBuf + carryLen), toRead);
          size_t totalLen = carryLen + readBytes;
          carryLen = 0;  // consumed; processBuffer() will repopulate if needed
          // 3. Process the combined buffer (carry + new data) as one unit
          processChunk(combinedBuf, totalLen);
          if (contentLength > 0) contentLength -= readBytes;
        }
        delay(1); // used to prevent watchdog timer exhaustion
      }
    } else {
      tft.printf("GET failed, HTTP code: %d\n", httpCode);
    }
    https.end();
  } else {
    Serial.println("Unable to connect");
  }
}

void initTFT(){
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  tftSPI->begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
  // Initialize the screen hardware
  tft.init(SCREEN_WIDTH, SCREEN_HEIGHT);
  // Boost the hardware SPI clock to 40MHz for faster image transfers
  tft.setSPISpeed(40000000); 
  // Set rotation (Try 1 or 3 for landscape, 0 or 2 for portrait)
  tft.setRotation(2);
  // Clear the screen with a solid background
  tft.fillScreen(ST77XX_BLACK);
  //text config
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setTextWrap(true);
  tft.setCursor(0,0);
  tft.println("TFT initialised");
}

void setup() {
  Serial.begin(115200);
  initTFT(); // calls all sets to configure TFT
  delay(1000); // give some time for device to initialise
  WiFi.mode(WIFI_AP_STA);
  if(!LittleFS.begin(true)){
    tft.setCursor(0,0);
    tft.println("File system mount failed!");
    return;
  }
  startAP();
  startSTA();
  if(WiFi.status() == WL_CONNECTED){
    updateTime = esp_timer_get_time();
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(0,0);
    tft.println("Updating...");
    fetchImageData();
    tft.drawRGBBitmap(0,0,image,SCREEN_WIDTH,SCREEN_HEIGHT);
  }else{
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(0,0);  
    tft.println("Not Connected to WiFi!");
  }
}

void loop() {
  server.handleClient(); // blocking handler that will manage web server clients
  int64_t currentTime = esp_timer_get_time();
  //600,000,000ms in 10mins
  if(currentTime - updateTime >= 600000000){
    if(WiFi.status() == WL_CONNECTED){
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(0,0);
      tft.println("Updating...");
      updateTime = esp_timer_get_time();
      fetchImageData();
      tft.drawRGBBitmap(0,0,image,SCREEN_WIDTH,SCREEN_HEIGHT);
    }else{
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(0,0);
      tft.println("Not Connected to WiFi!");
    }
  }
}