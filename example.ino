#include <ArduinoJson.h>

#ifdef ESP8266 // aka CWx1
#define R 12
#define G 13
#define B 5
#else //Assuming ESP32 aka CWx2
#define R 25
#define G 26
#define B 27
#endif

#define SYNC "$!$" //Part of the provisioning standard
int blinkLED = 0;

void prov_t() {
  //Transmit Provision Data
  Serial.println(); //Part of the provisioning standard
  Serial.print(SYNC); //Part of the provisioning standard
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["WiFi_Network"] = "Your SSID";
  root["WiFi_Password"] = "Your PSK";
  root["UbiDots_Token"] = "Long odd number";
  JsonArray& data = root.createNestedArray("LED_to_blink");
  data.add("Red");
  data.add("Green");
  data.add("Blue");
  root.printTo(Serial); 
  Serial.println(); //Part of the provisioning standard
  
  //Writes this string, with the CRLF and SYNC prefixed and CRLF suffixed
  //$!${"WiFi_Network":"Your SSID","WiFi_Password":"Your PSK","UbiDots_Token":"Long odd number","LED_to_blink":["Red","Green","Blue"]}
  //A nested array appears as a drop down list, the rest as text entry.
}
bool prov_r() {
  //Receive Provision Data
  String s = Serial.readStringUntil('\n');
  //Reads this string, with CRLF suffixed
  //{"WiFi_Network":"Your SSID","WiFi_Password":"Your PSK","UbiDots_Token":"Long odd number","LED_to_blink":"Red"}
  //Obvisouly with other data inserted, notice the dropdown comes back as the selected value.
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(s);
  if (!root.success()) {
    return false;
  }
  String led = root["LED_to_blink"];
  switch (led.charAt(0)) {
    case 'R':
      blinkLED = R; break;
    case 'G':
      blinkLED = G; break;
    case 'B':
      blinkLED = B; break;
  }
  String ssid = root["WiFi_Network"];
  String psk  = root["WiFi_Password"];
  String ubi  = root["UbiDots_Token"];

  return true;
}

void provisioning() {
  digitalWrite(B, HIGH);
  prov_t();
  while (!Serial.available()); //Wait for data
  if (prov_r()) {
    Serial.print(SYNC); //Part of the provisioning standard
    Serial.print(SYNC); //Part of the provisioning standard
    Serial.print("I will now blink #");
    Serial.println(blinkLED);
  } else {
    digitalWrite(R, HIGH);
    Serial.print(SYNC); //Part of the provisioning standard
    Serial.print(SYNC); //Part of the provisioning standard
    Serial.println("Something went wrong!");
  }
  digitalWrite(B, LOW);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) continue;
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);

  provisioning();
}

void loop() {
  if (blinkLED != 0) {
    digitalWrite(blinkLED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                       // wait for a second
    digitalWrite(blinkLED, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);
  }
}
