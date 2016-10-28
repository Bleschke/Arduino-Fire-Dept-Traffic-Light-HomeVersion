/*
 New email alarm/notification for google gmail account.
 Receives emails from Codemessaging.net
 
 Created December 8, 2011
 
 Based on the WebClient example by David A. Mellis,
 modified by Tom Igoe, based on work by Adrian McEwen.
 -----------------------------------------------------
 
 Modified by : Brian Leschke 13617
 
 Date: October 27,2016
 Version 6.1
 
 Created to connect to server on Raspberry Pi or EasyPHP.
 This program will mock the traffic light at Co. 13 Fallston
 FVFAC using corresponding colors for "leaving the station"
 actions. Light will be displayed in a project box using LED's.
 
 "Red" means WAIT (light turns on when call is received)
 "Yellow" means GET SET (turns on at 3 minutes) (red turns off)
 "Green" means GO (turns on at 5 minutes) (red/yell off)
 "Green" stays on for 5 minutes

 This is designed to receive multiple fire/ems calls at any given time. (Up to 3 calls only).
 
 
 *** UPDATE HISTORY ***
 
 12/8/2011 - Initial release
 2012-2015 - Unspecified Updates
 3/14/2014 - Device and Call Information will be displayed on a 20x4 character line LCD screen.
 6/19/2016 - "cleaned up", organized, and simplified code.
 6/19/2016 - ADDED UDP SERVER: This unit will also receive UDP packets that are sent by the Firehouse Traffic Light UDP Server.
             This is to make sure that the Traffic Light is working correctly and the network is active. 
 10/27/2016 - UDP response packet is now sent out 4 times.
 

 */
 
// ---------------- LIBRARIES ---------------- 
 
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
//#include <VirtualWire.h>
#include <stdlib.h>
#include <Wire.h>


#include <FastIO.h>
#include <I2CIO.h>
#include <LCD.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SR.h>
#include <LiquidCrystal_SR2W.h>
#include <LiquidCrystal_SR3W.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address


// ---------------- PIN AND TIME CONFIGURATION ---------------- 

int PowerLED      = 2;
int TrafficRED    = 3;
int TrafficYELLOW = 4;
int TrafficGREEN  = 5;
int Buzzer        = 6;

char Str[11];
int prevNum  = 0;
int num      = 0;
long onUntil = 0;
long pollingInterval = 1000;    // in milliseconds
long onTimeGreen     = 299000;  // time, in milliseconds, for Green light to be on after new alert.
long onTimeRed       = 179000;  //  time, in milliseconds, for Red light to be on after new alert.
long onTimeYellow    = 119000;  // time, in milliseconds, for Yellow light to be on after new alert.


// ---------------- NETWORK CONFIGURATION ---------------- 

byte mac[]                    = {  0x90, 0xA2, 0xEA, 0x01, 0x97, 0x10 }; // Arduino MAC Address
IPAddress ip( 192, 168, 1, 20 );     // IP address of this arduino.
const char SERVER_NAME[]      = "WEBSERVER ADDRESS";  // webserver address
unsigned int SERVER_PORT      = WEBSERVER PORT;       // webserver port
unsigned int UDP_PORT         = UDP PORT;             // port to listen for UDP packets
const char SEARCH_LOC[]       = "SEARCH LOCATION";    // User specified location of traffic light. Just a name, serves no other purpose.

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet.
char  ReplyBuffer[] = "UDP ACK MESSAGE";    // UDP Acknowledgement message to send back.


// ---------------- INITIALIZATION ---------------- 

EthernetClient client;
EthernetUDP Udp;

void setup()
{
  Serial.begin(9600);
  Serial.println("Powering up ...");
  // set up the LCD's number of columns and rows: 
  lcd.begin(20, 4);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Powering up . ");
  delay(750);
  lcd.clear();
  lcd.print("Powering up .. ");
  delay(750);
  lcd.clear();
  lcd.print("Powering up ... ");
  delay(750);
  lcd.clear();
  lcd.print("Fire Call Box");
  lcd.setCursor(0,1);
  lcd.print("Version 6.0");
  lcd.setCursor(0,2);
  lcd.print("Brian Leschke");  
  delay(1000);
  
  pinMode(PowerLED,OUTPUT);
  pinMode(TrafficRED,OUTPUT);
  pinMode(TrafficYELLOW,OUTPUT);
  pinMode(TrafficGREEN,OUTPUT);
  pinMode (Buzzer, OUTPUT);

  digitalWrite(PowerLED, HIGH);  
  
// ---------------- All Lights Off ----------------  
  
  digitalWrite(TrafficRED,LOW);     // Red lamp off
  digitalWrite(TrafficYELLOW,LOW);  // Yellow lamp off
  digitalWrite(TrafficGREEN,LOW);   // Green lamp off

// ---------------- Test Lights ----------------

  digitalWrite(TrafficRED,HIGH);    // Red lamp test: ON
  delay(500);
  digitalWrite(TrafficRED,LOW);     // Red lamp test: OFF
  delay(500);  
  digitalWrite(TrafficYELLOW,HIGH); // Yellow lamp test: ON
  delay(500);
  digitalWrite(TrafficYELLOW,LOW);  // Yellow lamp test: OFF
  delay(500);
  digitalWrite(TrafficGREEN,HIGH);  // Green lamp test: ON
  delay(500);
  digitalWrite(TrafficGREEN,LOW);   // Green lamp test :OFF
  delay(500);
  digitalWrite(TrafficRED,HIGH);    // Error lamp combo test: ON
  digitalWrite(TrafficYELLOW,HIGH); // Error lamp combo test: ON
  delay(500);
  digitalWrite(TrafficRED,LOW);     // Error lamp combo test: OFF
  digitalWrite(TrafficYELLOW,LOW);  // Error lamp combo test: OFF
  delay(500);
  digitalWrite(TrafficRED,HIGH);    // Full lamp test: ON
  digitalWrite(TrafficYELLOW,HIGH); // Full lamp test: ON
  digitalWrite(TrafficGREEN,HIGH);  // Full lamp test: ON
  delay(3000);
  digitalWrite(TrafficRED,LOW);     // Full lamp test: OFF
  digitalWrite(TrafficYELLOW,LOW);  // Full lamp test: OFF
  digitalWrite(TrafficGREEN,LOW);   // Full lamp test: OFF

  
// ---------------- Start the Ethernet connection ----------------

  restart:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    lcd.clear();
    lcd.print("Failed to configure Ethernet using DHCP.");
    lcd.setCursor(0,3);    
    lcd.print("Auto Resetting!");
     for(int x = 0; x < 2; x++){
        digitalWrite(TrafficRED,HIGH);    // red lamp on
        digitalWrite(TrafficYELLOW,HIGH); // yellow lamp on
        delay(500);
        digitalWrite(TrafficRED,LOW);    // red lamp off
        digitalWrite(TrafficYELLOW,LOW); // yellow lamp off
        delay(500);
     }
    delay(3000);  //delay before resetting Traffic Light
     goto restart; 
  }
  // give the Ethernet shield time to initialize:
  delay(2000);
}

void loop()
{
  
  // ---------------- NETWORK/SYSTEM STABILITY: FIXES SYSTEM FREEZING ----------------
  
  lcd.clear();
  Serial.println("Resetting Ethernet");
  lcd.print("Resetting Ethernet");
  client.stop();
  Udp.stop();
  delay(1000);
  Ethernet.begin(mac, ip);
  delay(2000);
  Udp.begin(UDP_PORT);
  delay(2000); 
  
  // ---------------- LIFE STATUS: Receive life status from Traffic Light UDP Server ----------------

  lcd.clear();
  lcd.print("Searching for Life");
  lcd.setCursor(0,2);
  lcd.print("Search Loc: ");
  lcd.print(SEARCH_LOC);
  lcd.setCursor(0,3);
  lcd.print("Search Port: ");
  lcd.print(UDP_PORT);
  delay(3000);
  
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i = 0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents:");
    Serial.println(packetBuffer);
    
    // Display packet information and contents that were received
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Packet Size: ");
    lcd.print(packetSize);
    lcd.setCursor(0,1);
    lcd.print("IP: ");
    lcd.print(Udp.remoteIP());
    lcd.setCursor(0,2);
    lcd.print("Port: ");
    lcd.print(Udp.remotePort());
    lcd.setCursor(0,3);
    lcd.print(packetBuffer); 
    
    // ---------------- LIFE STATUS: ALIVE. HEARTBEAT SOUND OVER PIEZO ----------------

    for(int x = 0; x < 2; x++){
       tone(6,646,100);
       delay(1000);
       tone(6,646,100);
       delay(1000);
      }

    // ---------------- LIFE STATUS: IF ALIVE, SEND REPLY TO IP AND PORT THAT SENT US PACKET ----------------
    // ---------------- LIFE STATUS: IF DEAD, FLATLINE SOUND OVER PIEZO ----------------

    lcd.clear();
    for(int x = 0; x < 4; x++){  // Send the response packet 4 times if traffic light is alive.
     Serial.println("Sending UDP Packet");
     Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
     Udp.write(ReplyBuffer);
     Udp.endPacket(); 
     delay(1000);
    }
    
    // Display packet information and contents that were sent
    lcd.setCursor(0,0);
    lcd.print("Acknowledgement Sent");
    lcd.setCursor(0,1);
    lcd.print("IP: ");
    lcd.print(Udp.remoteIP());
    lcd.setCursor(0,2);
    lcd.print("Port: ");
    lcd.print(Udp.remotePort());
    lcd.setCursor(0,3);
    lcd.print("Msg: ");
    lcd.print(ReplyBuffer);
  }
  else {
    lcd.clear();
    lcd.print("No Life Found"); // Flatline. Received no life status from UDP server.
    tone(6,646,1000);
  }
      
  delay(2000);   
   
  // ---------------- FIRE/EMS: CONNECT TO WEBSERVER ----------------
  
  Serial.println("connecting...");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connecting . ");
  delay(750);
  lcd.clear();
  lcd.print("Connecting .. ");
  delay(750);
  lcd.clear();
  lcd.print("Connecting ... ");
  delay(750);
  
  if (client.connect(SERVER_NAME, SERVER_PORT)) {
    Serial.println("connected");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Connected");
    //digitalWrite(ledConnect,HIGH);
    // Make a HTTP request:
    //client.println("GET /my%20portable%20files/GetGmail.php");   //EasyPHP pc server pathway
    client.println("GET /GetGmail.php");  // Apache server pathway.
    client.println();
    int timer = millis();
    delay(2000);
  } 
  else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");  //cannot connect to server
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Connection Failed");
    }

  // if there's data ready to be read:
  if (client.available()) {  
     int i = 0;    
     //put the data in the array:
     do {
       Str[i] = client.read();
       i++;
       delay(1);
     } while (client.available());
     
     // Pop on the null terminator:
     Str[i] = '\0';
     //convert server's repsonse to a int so we can evaluate it
     num = atoi(Str); 
     
     Serial.print("Server's response: ");
     Serial.println(num);
     Serial.print("Previous response: ");
     Serial.println(prevNum);
     
     if (prevNum < 0)
     { //the first time around, set the previous count to the current count
      prevNum = num; 
      Serial.println("First email count stored.");
     }
     if (prevNum > num)
     { // handle if count goes down for some reason
      prevNum = num; 
     }
  }
  else
    {
     Serial.println("No response from server."); //cannot connect to server.
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("No Response From Server");
    }
    
    Serial.println("Disconnecting."); //disconnecting from server to reconnect
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Disconnecting");
    client.stop();
    
    // ---------------- FIRE\EMS: ALERT FOR FIRE\EMS CALL ----------------
    
    if(num > prevNum) {
      Serial.println("FIRE ALERT!");  //alert for new email
      for(int x = 0; x < 3; x++)  // Red light blinks 2 times then stays solid.
      {
       tone(6,4000,200);
       delay(250);
       tone(6,4000,200);
       delay(250);
      }
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("FIRE CALL ALERT!");
      lcd.setCursor(0,1);
      lcd.print("RED LIGHT");
      lcd.setCursor(0,2);
      lcd.print("GET READY");
      Serial.println("RED LIGHT ON");
      for(int x = 0; x < 2; x++)  // Red light blinks 2 times then stays solid.
      {
     digitalWrite(TrafficRED,HIGH);
     delay(500);
     digitalWrite(TrafficRED,LOW);
     delay(500);
     digitalWrite(TrafficRED,HIGH);
      }
      delay(onTimeRed);  // 3 minutes delay
      digitalWrite(TrafficRED,LOW);
      Serial.println("RED LIGHT OFF");
      Serial.println("YELLOW LIGHT ON"); 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("FIRE CALL ALERT!");
      lcd.setCursor(0,1);
      lcd.print("YELLOW LIGHT");
      lcd.setCursor(0,2);
      lcd.print("GET SET");    
      for(int x = 0; x < 2; x++)  // Yellow light blinks 2 times then stays solid.
      {
     digitalWrite(TrafficYELLOW,HIGH);
     delay(500);
     digitalWrite(TrafficYELLOW,LOW);
     delay(500);
     digitalWrite(TrafficYELLOW,HIGH);
      }
      delay(onTimeYellow);  // 2 minutes delay
      digitalWrite(TrafficYELLOW,LOW);
      Serial.println("YELLOW LIGHT OFF");
      Serial.println("GREEN LIGHT ON");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("FIRE CALL ALERT!");
      lcd.setCursor(0,1);
      lcd.print("GREEN LIGHT");
      lcd.setCursor(0,2);
      lcd.print("GO");
      for(int x = 0; x < 2; x++)   // Green light blinks 2 times then stays solid.
      {
     digitalWrite(TrafficGREEN,HIGH);
     delay(500);
     digitalWrite(TrafficGREEN,LOW);
     delay(500);
     digitalWrite(TrafficGREEN,HIGH);
      }

      
      prevNum = num;
      onUntil = millis() + onTimeGreen;
    }
    else if(millis() > onUntil)  //if email value is lower/equal to previous, no alert.
    {
     Serial.println("GREEN LIGHT OFF");
     digitalWrite(TrafficGREEN,LOW);
    }
  delay(pollingInterval);  //wait time until restart.
}

