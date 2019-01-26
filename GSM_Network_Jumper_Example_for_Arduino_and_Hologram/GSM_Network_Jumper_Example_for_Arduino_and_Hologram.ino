/*
 Hologram Demo - GSM Network Jumping
 This example shows how to switch between carriers using a Hologram SIM. 
 Circuit:
 * MKR GSM 1400 board
 * Antenna
 * Hologram SIM card
 Created 12 Dec 2018
 Updated 26 Jan 2019
 by Benstr
*/

// libraries
#include <MKRGSM.h>

// initialize the library instan
GSM gsmAccess;     // include a 'true' parameter to enable debugging
GSMScanner scannerNetworks;
GSMModem modemTest;
GPRS gprs;
GSMClient client;

// Save data variables
String IMEI = "";

// connection state
bool connected = false;

void setup() {
  // Reset the ublox module
  modemTest.begin();
  
  // baud rate used for both Serial ports
  unsigned long baud = 115200;
  Serial.begin(baud);
  SerialGSM.begin(baud);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("Hologram GSM Network Jumper");

  scannerNetworks.begin();

  // get modem parameters
  // IMEI, modem unique identifier
  Serial.print("Modem IMEI: ");
  IMEI = modemTest.getIMEI();
  IMEI.replace("\n", "");
  if (IMEI != NULL) {
    Serial.println(IMEI);
  }

  // allowing time for the slower 2G network to be scanned
  delay(5000);

  Serial.println();
  Serial.println("Ready to Connect:");
  Serial.println("Send 'netz' to see all available networks");
  Serial.println("Send 'att' to connect to AT&T 3G");
  Serial.println("Send 'tmo3' to connect to T-Mobile 3G");
  Serial.println("Send 'tmo2' to connect to T-Mobile 2G (if available)");
}

void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  while (SerialGSM.available()) {
    Serial.print(SerialGSM.read());
  }
  
  if (Serial.available()) {
    String command = Serial.readString();

    if (command.indexOf("att") >= 0) 
    {
      Serial.print(command);
      Serial.println();
      Serial.println("Jumping to AT&T 3G");
      jumpCarrier(String("AT+COPS=1,2,\"310410\",2\r\n"));
    } 
    else if (command.indexOf("tmo3") >= 0)
    {
      Serial.print(command);
      Serial.println();
      Serial.println("Jumping to T-Mobile 3G");
      jumpCarrier(String("AT+COPS=1,2,\"310260\",2\r\n"));
    }
    else if (command.indexOf("tmo2") >= 0)
    {
      Serial.print(command);
      Serial.println();
      Serial.println("Jumping to T-Mobile 2G");
      jumpCarrier(String("AT+COPS=1,2,\"310260\",0\r\n"));
    }
    else if (command.indexOf("netz") >= 0)
    {
      Serial.print(command);
      Serial.println();
      // scan for existing networks, displays a list of networks
      Serial.println("Scanning available networks. This may take a few minutes.");
      //scannerNetworks.begin();
      Serial.println(scannerNetworks.readNetworks());
      Serial.println("Network scan complete. If there are no networks scan again.");
    }
    else if (command.indexOf("at+") >= 0 || command.indexOf("AT+") >= 0)
    {
      SerialGSM.write(command.c_str());
    }
  }
}

bool jumpCarrier(String network)
{
  bool good = true;
  do
  {
    Serial.println("bouncing from previous network...");
    good = sendGSMCommand(String("AT+CGATT=0\r\n"));
    good = sendGSMCommand(String("AT+COPS=2\r\n"));
    Serial.println("success!");
    
    Serial.println("joining new network...");
    good = sendGSMCommand(network);
    good = sendGSMCommand(String("AT+COPS=0\r\n"));
    good = sendGSMCommand(String("AT+CREG?\r\n"));
    good = sendGSMCommand(String("AT+CGATT=1\r\n"));
    good = sendGSMCommand(String("AT+UPSD=0,1,\"hologram\"\r\n"));
    good = sendGSMCommand(String("AT+UPSD=0,7,\"0.0.0.0\"\r\n"));
    good = sendGSMCommand(String("AT+UPSDA=0,3\r\n"));
    good = sendGSMCommand(String("AT+UPSND=0,8\r\n"));
    Serial.println("success!");
    Serial.println();
    
    printCarrierInfo();
    
    return good;
    
  } while(good != false);

  Serial.println("Error when joining");
}

void printCarrierInfo()
{
  // currently connected carrier
  Serial.print("Current carrier: ");
  Serial.println(scannerNetworks.getCurrentCarrier());
  Serial.println();
  // returns strength and ber
  // signal strength in 0-31 scale. 31 means power > 51dBm
  // BER is the Bit Error Rate. 0-7 scale. 99=not detectable
  Serial.print("Signal Strength: ");
  Serial.print(scannerNetworks.getSignalStrength());
  Serial.println(" out of a possible 31");
}

bool sendGSMCommand(String command)
{
  SerialGSM.write(command.c_str());

  String response;
  
  do {
    if(SerialGSM.available())
    {
      response = SerialGSM.readString();
      Serial.println(response);
    }
  } while (response.indexOf("OK") < 0);

  return true;
}