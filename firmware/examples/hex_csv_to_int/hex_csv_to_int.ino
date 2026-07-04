#include <string.h> // strtok()
#include <stdlib.h> // strtol()

//CSV style data to parse
char data[] = "0x93ed, 0x9c2e, 0x93ed, 0x6ac9, 0x49c5, 0x51e5, 0x6267, 0x6267, 0x6ac9, 0x6aa9, 0x5a88, 0x5a68, 0x5207, 0x3124"; 
uint16_t values[14]; // preinitialise an array of long values

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("---Parse Example---");
  uint16_t index = 0;
  char *token = strtok(data, ",");
  while(token != NULL){
    char *ptr_end;
    long value = strtol(token, &ptr_end,0);
    if(ptr_end != token){
      values[index] = (int16_t) value;
      index++;
      Serial.println(value, 16);
    }
    //retrieve next token
    token = strtok(NULL, ",");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
