# Arduino-Fire-Dept-Traffic-Light-HomeVersion

Brian Leschke 2016
 
 - Update 12/8/2011 - Initial release
 - Update 2012-2015 - Unspecified Updates
 - Update 3/14/2014 - Device and Call Information will be displayed on a 20x4 character line LCD screen.
 - Update 6/19/2016 - "cleaned up", organized, and simplified code.
 - Update 6/19/2016 - ADDED UDP SERVER: This unit will also receive UDP packets that are sent by the Firehouse Traffic Light UDP Server.
             This is to make sure that the Traffic Light is working correctly and the network is active. 
 - Update 10/27/2016 - UDP response packet is now sent out 4 times.

## **Overview**

This program will light up LED's simulating a traffic light using corresponding colors of RED, YELLOW, and GREEN for "leaving the station" actions as well as receive and send UDP packets from/to the "Arduino-Fire-Dept-Traffic-Light" program.  

Project Breakdown:
* Traffic Light
    * Displays Alert Countdown status
        * RED - "Get Ready": 0-3 minutes (Flashes 3 times, then solid)
        * YELLOW - "Get Set": 3-5 Minutes (Flashes 3 times, then solid)
        * GREEN - "Go": 5-10 Minutes (Flashes 3 times, then solid)

### **Prerequisities**

You will need:

1. Arduino Uno
2. Sainsmart 4 module high voltage relay
3. Arduino programming software
4. Webserver hosting script.

### **Usage**

In order to use successfully implement this project and its code, you will need to install the Arduino Programming Software.
    
## **Uploading**

The code can be uploaded to the ESP8266 by a serial (USB) connection. 


