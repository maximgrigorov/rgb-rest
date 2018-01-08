# rgb-rest
This project describes RGB lamp, controlled by the ESP8266 module via REST API and WiFi connection.

Provides the following REST API:

###### /led [POST]
```javascript
{  
   "LED":[  
      $brightness,
      $R,
      $G,
      $B
   ]
}
```
set brightness [1; 4] and values to each color [0; 255] in decimal format. You MUST specify the correct Content-Type and Content-Length headers.

_Example: {"RGB" : [4, 255, 165, 0]} - sets the orange color with maximum brightness._

###### /flash [POST]
```javascript
{  
   "FLASH":[  
      $counter,
      $interval
   ]
}
```
flash the CURRENT color $counter times with the specified duration (milliseconds).
_Example: {"FLASH" : [10, 100]} - flashes the current color 10 times with 100ms delay._

###### /pwm [POST]
```javascript
{  
   "PWM":[  
      $new_R,
      $new_G,
      $new_B,
      $interval,
      $steps
   ]
}
```
change the CURRENT color smoothly to the new values. Brightness will not be changed. Interval should be provided with milliseconds.

_Example: {"PWM" : [128, 128, 0, 50, 50]} - changes the current color to the provided values with 50ms delay between previous and new color and for the 10 steps for all colors simultaneously._

	Please note: $interval is lower - color changed smoothly and quickly. Recommended value - 60ms.

	Please note: $steps is greater - color may be changes with jump at the end. Recommended value - 51 (255 / 51 = 5.0, so the integer part is 5 and 5 * 51 = 255).

###### /status [GET]
returns current values for each color in JSON format.
