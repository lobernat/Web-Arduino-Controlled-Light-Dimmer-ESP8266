# Web-Arduino-Controlled-Light-Dimmer-ESP8266
Dimmer light controlled by http get request


  http://ESP8266-IP-address/?temps=5&percent=80
  
  Light control using two parameters:
  - temps -> Time in secconds to reach percentage light brigness
  - percent -> Percentage of light
  
  This example the lamp will increase the light up to 80% during 5 seconds ramp
  
  Also there is a phisical switch to manage the lamp manually with multifuncion press:
  - very short press -> increase light by 10% 
  - long press -> power off
  
  There is a autopoweroff function to swith off the lamp 10 minutes after the get request
  (this is because I use this as an alarm clock and I lauch the get request at 6:40h from a cron script on my raspberrypi. I don't have to worry to swith it off at early morning :)  )

Hardware (very cheap at aliexpress):
![Main screeen](https://raw.githubusercontent.com/lobernat/Web-Arduino-Controlled-Light-Dimmer-ESP8266/master/dimmer.jpg)
