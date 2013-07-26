/*
 arduinoFridge

 Copyright (C) 2013 - Markus Kohlhase <mail@markus-kohlhase.de>

 A simple control for your refigerator.

 Circuit:

 * Ethernet shield attached to pins 10, 11, 12, 13
 * Temp sensor input attached to pin A1
 * Door sensor input attached to pin A0

 v0 = 634
 v25 = 341

*/

#include <SPI.h>
#include <Ethernet.h>

const int relaisPin1 = 9;
const int relaisPin2 = 8;
const int doorPin    = A0;
const int sensorPin  = A1;

int doorOpen    = false;
int cooling     = false;
int doorValue   = 0;
int sensorValue = 0;
int threshold   = 655;
int hysteresis  = 15;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,100);

EthernetServer server(80);

void setup() {
  pinMode(relaisPin1, OUTPUT);
  pinMode(relaisPin2, OUTPUT);
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Web Server is at ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  doorValue   = analogRead(doorPin);
  sensorValue = analogRead(sensorPin);

  if (doorValue > 1000){
    doorOpen = false;
  }
  else{
    doorOpen = true;
  }
  if (sensorValue < threshold-hysteresis){
    cooling = true;
  }
  if (sensorValue > threshold+hysteresis) {
    cooling = false;
  }

  Serial.println(sensorValue);
  Serial.println(cooling);
  digitalWrite(relaisPin1, cooling);
  digitalWrite(relaisPin2, doorOpen);
  delay(500);

  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE html>");
          client.println("<html>");
          // add a meta refresh tag, so the browser
          // pulls again every 20 seconds
          client.println("<meta http-equiv=\"refresh\" content=\"20\">");

          client.println("<h1>Arduino Fridge</h1>");
          client.println("<h2>Current Status</h2>");
          client.println("<label>Sensor value:</label>");
          client.println("<input value=/>");
          client.println("<br />");
          client.println("<label>Door is:</label>");
          if(doorOpen){
            client.println("<input value=\"open\" />");
          }else{
            client.println("<input value=\"closed\" />");
          }
          client.println("<input value=/>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }
}
