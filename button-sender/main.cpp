#include <Arduino.h>

#include <SPI.h>      // include the SPI library
#include <nRF24L01.h> // include the nRF24L01 library
#include <RF24.h>     // include the RF24 library
#include "utils.hpp"
#include "packet.h"
#define TIMEOUT_TIME 250

// create an RF24 object called "radio" and pass it the pins for the chip enable (CE) and chip select (CSN) connections
RF24 radio(8, 9);
const byte addresses[][6] = {"00002", "00001"};

unsigned long timeoutmillisCount = 0;
unsigned long pingMillisCount = 0;

int buttonOldMaxState;

void setup()
{
    Serial.begin(115200); // start serial communication at 115200 baud
    utils::Print("Serial init ... OK\n");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    if (!radio.begin())
    {
        utils::Print("FAIL\n");
        Serial.println(F("radio hardware not responding!"));

        // hold program in infinite loop to prevent subsequent errors
        while (1) {} 
    }

    utils::Print("addresse: %s\n", addresses[0]);
    utils::Print("Radio init ... ");
    radio.begin();                          // start radio communication
    radio.setAddressWidth(5);               // set the address width to 5
    radio.openWritingPipe(addresses[1]);    // set the address for transmitting
    radio.openReadingPipe(1, addresses[0]); // set the address for receiving
    radio.setPALevel(RF24_PA_MIN);          // set the power level for radio transmission
    radio.startListening();
    utils::Print("OK\n");
    
    utils::Print("Sending Initial message ... ");

    Packet packet;
    packet.type = PacketType::ButtonPressed;

    sendPacket(radio, packet);
    timeoutmillisCount = millis() + TIMEOUT_TIME;
}

void loop()
{
    //// Example for receiving a packet
    // Packet packet;
    // if(receivePacket(radio, &packet))
    // {
    //     switch(packet.type)
    //     {
    //         case PacketType::Ping:
    //             Serial.println("Ping");
    //             break;

    //         case PacketType::ButtonPressed:
    //             Serial.println("Button pressed");
    //             break;

    //         case PacketType::Battery:
    //             Serial.println("Battert");
    //             break;

    //         default:
    //             break;
    //     }
    // }

    // repeat key 
    // utils::Print("A0: %d\n", analogRead(A0));
    /* if (analogRead(A0) > 512)
    {
        timeoutmillisCount = millis() + TIMEOUT_TIME;
    } */

    utils::Print("Millis: %lu\nMillisCount: %lu\n", millis(), timeoutmillisCount);
    if (millis() > timeoutmillisCount)
    { // poweroff arduino by putting pin 12 high and relesing main relay
        powerOff();
    }
    if (millis() > pingMillisCount)
    {
        pingMillisCount = millis() + 100;
        Packet pingPacket;
        pingPacket.type = PacketType::Ping;
        pingPacket.data.ping.millis = millis();
        pingPacket.data.ping.millisPlusTimeout = timeoutmillisCount;

        sendPacket(radio, pingPacket);
    }
}

void powerOff()
{
    Packet packet;
    packet.type = PacketType::Timeout;
    sendPacket(radio, packet);

    pinMode(12, OUTPUT);

    // sendMessage("F");
    while (true)
    {
        digitalWrite(12, HIGH);
    }
}