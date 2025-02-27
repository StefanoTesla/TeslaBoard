#ifndef ALPACA_DISCOVERY
#define ALPACA_DISCOVERY

void alpacaDiscovery(AsyncUDP &udp) {  // Passa l'oggetto `udp` come riferimento
    if (udp.listen(32227)) {
        Serial.println("Listening for discovery requests...");
        udp.onPacket([](AsyncUDPPacket packet) {
            if (packet.length() < 16) {
                return;
            }
            // Confronta il pacchetto con la stringa "alpacadiscovery1"
            if (strncmp("alpacadiscovery1", (char *)packet.data(), 16) != 0) {
                return;
            }
            packet.printf("{\"alpacaport\": %d}", ALPACA_PORT);
        });
    } else {
        Serial.println("Failed to start UDP listening.");
    }
}

#endif