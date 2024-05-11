#include "BlueCarCAN.h"
#if defined(ESP32)
BlueCarCAN can(5, 21);  // Pines para ESP8266
#else
BlueCarCAN can(10, 2);  // Pines para Arduino
#endif

void setup() {
    can.config_node("Radar");
}

void loop() {
    can.json_received = "{Propulsion:255, Direccion:150, Radar:255}"; // Lee la línea completa desde Serial
    can.readJetson();
    can.send2Jetson();
    can.isForMe();
    Serial.println(can.json);
    
}
