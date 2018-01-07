#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

int blue = D5; //BLUE
int red = D6; //RED
int green = D7; //GREEN

ESP8266WebServer server(80);

void setup() {

  const char *ssid = "RGB";
  const char *pass = "00000000";
  
  Serial.begin(9600);

  WiFi.softAP(ssid, pass);
  IPAddress myIP = WiFi.softAPIP();
  
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  
  server.on("/led", ledControl);
  server.begin();

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  analogWrite(red, 1024);
  analogWrite(green, 1024);
  analogWrite(blue, 1024);

}

void loop() {

  server.handleClient();

}

void ledControl() {

  String message = server.arg("plain");
  Serial.println(message.length());
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& parsed= jsonBuffer.parseObject(message);

  analogWrite(red, parsed["RGB"][0]);
  analogWrite(green, parsed["RGB"][1]);
  analogWrite(blue, parsed["RGB"][2]);

  server.send(200, "application/json", "{ \"RGB\" : \"done\" }");
  
}

