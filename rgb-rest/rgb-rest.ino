#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

int blue = D5; //BLUE
int red = D6; //RED
int green = D7; //GREEN
int providedRed, providedGreen, providedBlue;
int actualBlue, actualRed, actualGreen;
int deltaBlue, deltaRed, deltaGreen;
int brightness;
int newRed, newGreen, newBlue;
int pwmInterval;
int pwmSteps;

unsigned long previousMillis;
unsigned long currentMillis;
unsigned long interval;
unsigned long counter; 
unsigned long executed; 
unsigned long pwmExecuted; 

ESP8266WebServer server(80);

void setup() {

  const char *ssid = "RGB";
  const char *pass = "00000000";
  
  Serial.begin(9600);

  WiFi.softAP(ssid, pass);
  IPAddress myIP = WiFi.softAPIP();
  
  Serial.print("AP IP address: ");
  Serial.println(myIP);
 
  server.on("/led", HTTP_POST, []() {
    led();
  });

  server.on("/led", HTTP_GET, []() {
    server.send(400, "text/plain", "Wrong request. Send GET request to / for getting the list of available request types.");
  });
  
  server.on("/status", HTTP_GET,[]() {
    printStatus();
  });

  server.on("/flash", HTTP_POST,[]() {
    flash();
  });

  server.on("/pwm", HTTP_POST,[]() {
    pwm();
  });

  server.on("/led", HTTP_GET, []() {
    wrongRequest();
  });

  server.on("/flash", HTTP_GET, []() {
    wrongRequest();
  });

  server.on("/pwm", HTTP_GET, []() {
    wrongRequest();
  });

  server.on("/", help);

  server.begin();

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  
  setLow();
  brightness = 4;
  counter = 0;
  executed = 0;
  pwmExecuted = 0;
  pwmInterval = 0;
  pwmSteps = 0;
  interval = 0;  
  newRed = 0;
  newGreen = 0;
  newBlue = 0;
}

void wrongRequest() {
  server.send(400, "text/plain", "Wrong request. Send GET request to / for getting the list of available request types.");
}

void setLow() {
  analogWrite(red, 1024);
  analogWrite(green, 1024);
  analogWrite(blue, 1024);
}

void setActual(int providedRed, int providedGreen, int providedBlue) {
  analogWrite(red, 1024 - providedRed * brightness);
  analogWrite(green, 1024 - providedGreen * brightness);
  analogWrite(blue, 1024 - providedBlue * brightness);
}

void flash() {
  String message = server.arg("plain");
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& parsed= jsonBuffer.parseObject(message);

  counter  = parsed["FLASH"][0];
  counter *= 2;
  interval = parsed["FLASH"][1];
  executed = 0;
  previousMillis = 0;
  printStatus();
}

void help() {
    String result = "Available endpoints: \n\n/led [POST] { \"LED\" : [$brightness, $R, $G, $B]} - set brightness [1; 4] and values to each color [0; 255] in decimal format. You MUST specify the correct Content-Type and Content-Length headers.";
    result+="\n\tExample: {\"RGB\" : [4, 255, 165, 0]} - sets the orange color with maximum brightness.\n";
    result += "\n/flash [POST] { \"FLASH\" : [$counter, $interval]} - flash the CURRENT color $counter times with the specified duration (milliseconds).";
    result+="\n\tExample: {\"FLASH\" : [10, 100]} - flashes the current color 10 times with 100ms delay.\n";
    result += "\n/pwm [POST] { \"PWM\" : [$new_R, $new_G, $new_B, $interval, $steps]} - change the CURRENT color smoothly to the new values. Brightness will not be changed. Interval should be provided with milliseconds.";
    result+="\n\tExample: {\"PWM\" : [128, 128, 0, 50, 50]} - changes the current color to the provided values with 50ms delay between previous and new color and for the 10 steps for all colors simultaneously.\n";
    result+="\n\tPlease note: $interval is lower - color changed smoothly and quickly. Recommended value - 60ms.\n";
    result+="\n\tPlease note: $steps is greater - color may be changes with jump at the end. Recommended value - 51 (255 / 51 = 5.0, so the integer part is 5 and 5 * 51 = 255).\n";
    result+= "\n/status [GET] - returns current values for each color in JSON format.";
    server.send(200, "text/plain", result);
}

void pwm() {
  String message = server.arg("plain");
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& parsed= jsonBuffer.parseObject(message);

  newRed = parsed["PWM"][0];
  newGreen = parsed["PWM"][1];
  newBlue = parsed["PWM"][2];
  interval = parsed["PWM"][3];
  pwmSteps = parsed["PWM"][4];
  pwmExecuted = 0;

  previousMillis = 0;
  if ((newRed > 255) || (newGreen > 255) || (newBlue > 255)) {
    server.send(400, "text/plain", "Color value should be between 0 and 255");
    return;
  }

  deltaRed = (providedRed - newRed) / pwmSteps;
  deltaGreen = (providedGreen - newGreen) / pwmSteps; 
  deltaBlue = (providedBlue - newBlue) / pwmSteps; 
  
  String result = "{ \"PWM\" : [" + String(newRed) + ", " + String(newGreen) + ", " + String(newBlue) + ", " + String(interval) + ", " + String(pwmSteps) + " ] }";
  char res[50];
  result.toCharArray(res, 50);
  server.send(200, "application/json", res);

}

void loop() {
  currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (executed < counter) {
      if (executed % 2) {
        setActual(providedRed, providedGreen, providedBlue);
      } else {
        setLow();
      }
      executed+=1;
    } else if ((executed == counter) && (executed != 0)) {
      counter = 0;
      interval = 0;
      executed = 0;
      setActual(providedRed, providedGreen, providedBlue);
    }

    if (pwmExecuted < pwmSteps) {

      if (deltaRed > 0) {
        providedRed -= deltaRed;
      } else if (deltaRed < 0) {
        providedRed += abs(deltaRed);
      }

      if (deltaGreen > 0) {
        providedGreen -= deltaGreen;
      } else if (deltaGreen < 0) {
        providedGreen += abs(deltaGreen);
      }

      if (deltaBlue > 0) {
        providedBlue -= deltaBlue;
      } else if (deltaBlue < 0) {
        providedBlue += abs(deltaBlue);
      }

      pwmExecuted+=1;
      setActual(providedRed, providedGreen, providedBlue);
      
    } else if ((pwmExecuted == pwmSteps) && (pwmExecuted != 0)) {
      pwmSteps = 0;
      interval = 0;
      pwmExecuted = 0;
      deltaRed = 0;
      deltaGreen = 0;
      deltaBlue = 0;
      setActual(newRed, newGreen, newBlue);
    }
   }
   
  server.handleClient();
}

void printStatus() {
  String result = "{ \"LED\" : [" + String(providedRed) + ", " + String(providedGreen) + ", " + String(providedBlue) + " ] }";
  char res[30];
  result.toCharArray(res, 30);
  server.send(200, "application/json", res);
}

void led() {
  String message = server.arg("plain");
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& parsed= jsonBuffer.parseObject(message);

  brightness = parsed["LED"][0];
  providedRed  = parsed["LED"][1];
  providedGreen = parsed["LED"][2];
  providedBlue = parsed["LED"][3];

  if ((brightness <= 0) || (brightness > 4)) {
    server.send(400, "text/plain", "Brightness value should be between 1 and 4");
    return;
  }

  if ((providedRed > 255) || (providedGreen > 255) || (providedBlue > 255)) {
    server.send(400, "text/plain", "Color value should be between 0 and 255");
    return;
  }

  setActual(providedRed, providedGreen, providedBlue);

  printStatus();
}

