/*
 Hologram Demo - GSM Network Jumping
 This example shows how to switch between carriers using a Hologram SIM. 
 Circuit:
 * MKR GSM 1400 board
 * Antenna
 * Hologram SIM card
 Created 12 Dec 2018
 by Benstr
*/

// libraries
#include <MKRGSM.h>

// initialize the library instance
GSM gsmAccess;     // include a 'true' parameter to enable debugging
GSMScanner scannerNetworks;
GSMModem modemTest;
GPRS gprs;

// Save data variables
String IMEI = "";

// connection state
bool connected = false;

void setup() {
  // baud rate used for both Serial ports
  unsigned long baud = 115200;
  Serial.begin(baud);
  SerialGSM.begin(baud);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("Hologram GSM Network Jumper");
  Serial.println("setting up connection....");
  
  scannerNetworks.begin();

  // Start GSM shield
  // This will connect to the last connected carrier
  // If the last network was 2G the modem will favor a similar 3G  
  while (!connected) {
    if ((gsmAccess.begin() == GSM_READY) &&
        (gprs.attachGPRS("hologram", " ", "") == GPRS_READY)) {
      connected = true;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  // get modem parameters
  // IMEI, modem unique identifier
  Serial.print("Modem IMEI: ");
  IMEI = modemTest.getIMEI();
  IMEI.replace("\n", "");
  if (IMEI != NULL) {
    Serial.println(IMEI);
  }

  // allowing time for the slower 2G network to be scanned
  delay(3000);

  // scan for existing networks, displays a list of networks
  Serial.println("Scanning available networks. May take a few minutes.");
  Serial.println(scannerNetworks.readNetworks());

  printCarrierInfo();
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readString();
    //Serial.print(command);

    if (command.indexOf("att") >= 0) 
    {
      Serial.println();
      Serial.println("Jumping to AT&T 3G");
      jumpCarrier(String("AT+COPS=1,2,\"310410\",2\r\n"));
    } 
    else if (command.indexOf("tmo3") >= 0)
    {
      Serial.println();
      Serial.println("Jumping to T-Mobile 3G");
      jumpCarrier(String("AT+COPS=1,2,\"310260\",2\r\n"));
    }
    else if (command.indexOf("tmo2") >= 0)
    {
      Serial.println();
      Serial.println("Jumping to T-Mobile 2G");
      jumpCarrier(String("AT+COPS=1,2,\"310260\",0\r\n"));
    }
    else
    {
      SerialGSM.write(command.c_str());
    }
  }

  if (SerialGSM.available()) {
    Serial.write(SerialGSM.read());
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
    }
  } while (response.indexOf("OK") < 0);

  return true;
}
