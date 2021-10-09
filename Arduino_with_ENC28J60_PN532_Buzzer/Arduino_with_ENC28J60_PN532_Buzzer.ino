#include <EtherCard.h>
#include <Wire.h>
#include <SPI.h>
#include <HttpClient.h>
#include <Adafruit_PN532.h>

#define PN532_IRQ   (2)
#define PN532_RESET (3)

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[700];
static uint32_t timer;

void setup () {
  Serial.begin(57600);
  Serial.println("\n[pings]");

  if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);

  #if 1
    if (!ether.dnsLookup(PSTR("www.google.com")))
      Serial.println("DNS failed");
  #else
    ether.parseIp(ether.hisip, "74.125.77.99");
  #endif
    ether.printIp("SRV: ", ether.hisip);
    
  timer = -9999999;

  //-------------------------------------------------------

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A card");

  //-------------------------------------------------------
  
  Serial.println();
}

void loop () {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if (len > 0 && ether.packetLoopIcmpCheckReply(ether.hisip)) {
    Serial.print("  ");
    Serial.print((micros() - timer) * 0.001, 3);
    Serial.println(" ms");
  }

  if (micros() - timer >= 5000000) {
    ether.printIp("Pinging: ", ether.hisip);
    timer = micros();
    ether.clientIcmpRequest(ether.hisip);
  }

  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;

  //------------------------------------------------------- 
  
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  
  if (success) {
    Serial.println("Found a card!");
    Serial.print("UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("UID Value: ");
    
    for (uint8_t i=0; i < uidLength; i++) {
       Serial.print(" 0x");
       Serial.print(uid[i], HEX); 
    }
    
    Serial.println("");
    delay(1000);

    HttpClient client;
    client.get("http://www.arduino.cc/asciilogo.txt");

    while (client.available()) {
        char c = client.read();
        Serial.println(c);
        break;
    }
    
    Serial.flush();
    delay(5000);
  }
  else 
    Serial.println("Timed out waiting for a card");
}
