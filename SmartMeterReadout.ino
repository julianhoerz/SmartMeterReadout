/*
  Multiple Serial test

  Receives from the main serial port, sends to the others.
  Receives from serial port 1, sends to the main serial (Serial 0).

  This example works only with boards with more than one serial like Arduino Mega, Due, Zero etc.

  The circuit:
  - any serial device attached to Serial port 1
  - Serial Monitor open on Serial port 0

  created 30 Dec 2008
  modified 20 May 2012
  by Tom Igoe & Jed Roach
  modified 27 Nov 2015
  by Arturo Guadalupi

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/MultiSerialMega
*/

#include <SPI.h>
#include <EthernetENC.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 178, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

byte InitArray[] = {0x2F,0x3F,0x21,0x0D,0x0A};
byte OverallReceivedPowerPattern[] = {'1','.','8','.','0','('};
int OverallReceivedPowerPatternLength = 6;
int PatternMatchCounter = 0;
long HighPowerNumber = 100000000;
long OverallReceivedPower = 0;
int debugpower =0 ;
long ReceivedPower = 0;
bool startingNewLine = true;

//byte InitArray[] = {0x2F,0x3F,0x21,0x0D,0x0A};
int InitLength = 5;
byte AcknowledgeArray[] = {0x06,0x30,0x30,0x30,0x0D,0x0A};
int AcknowledgeLength = 6;
byte* sendData = NULL;
int sendLength = 0;

int lastmillis = 0;
int currentmillis = 0;
bool initpassed = false;


void setup() {
  /* Initialize with 300 baud and 7e1 according to https://wiki.volkszaehler.org/hardware/channels/meters/power/edl-ehz/landisgyr_e350 */
  Serial.begin(300, SERIAL_7E1);

    // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    //Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    //Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
}

void loop() {


  EthernetClient client = server.available();
  if (client) {
      if (client.connected() && client.available()) {
        client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.print("Received Power: ");
          client.print(ReceivedPower);
          client.println("<br />");
          client.println("</html>");
          
      }
  }
      delay(1);
    // close the connection:
    client.stop();

  currentmillis = millis();
  if (Serial.available()) 
  {
    int inByte = Serial.read();

    if((startingNewLine == true || PatternMatchCounter > 0) && inByte == OverallReceivedPowerPattern[PatternMatchCounter] && PatternMatchCounter < OverallReceivedPowerPatternLength)
    {
      PatternMatchCounter ++;
    }
    else if(PatternMatchCounter < OverallReceivedPowerPatternLength)
    {
      PatternMatchCounter = 0;
    }
    else if(PatternMatchCounter >= OverallReceivedPowerPatternLength)
    {
      if(inByte != '.' && inByte != '*')
      {
        int recNum = (int) (inByte - 48);
        OverallReceivedPower = OverallReceivedPower + (HighPowerNumber * recNum);
        HighPowerNumber = HighPowerNumber / 10;
      }
      else if(inByte == '*')
      {
        HighPowerNumber = 100000000;
        PatternMatchCounter = 0;  
        ReceivedPower = OverallReceivedPower; 
        OverallReceivedPower = 0;     
      }
    }

    

    if(inByte == '\n')
    {
      startingNewLine = true;
    }
    else
    {
      startingNewLine = false;
    }
    /*Look for messages starting with:
    1.8.0
    1.8.1
    1.8.2
    2.8.0
    31.7
    51.7
    71,7
    32.7
    52.7
    72,7
    16.7*/
    lastmillis = currentmillis;
  }
  else
  {
    if(abs(currentmillis - lastmillis) > 10000)
    {
      if(initpassed == false)
      {
        sendData = InitArray;
        sendLength = InitLength;
        initpassed = true;
      }
      else
      {
        sendData = AcknowledgeArray;
        sendLength = AcknowledgeLength;
        initpassed = false;
      }
      Serial.write(sendData,sendLength);
      lastmillis = currentmillis;
    }
  }
}
