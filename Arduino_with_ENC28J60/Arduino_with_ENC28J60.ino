#include <EtherCard.h>

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[700];
static uint32_t timer;

static void gotPinged (byte* ptr) {
  ether.printIp(">>> ping from: ", ptr);
}

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
  ether.registerPingCallback(gotPinged);
  timer = -9999999;
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
}
