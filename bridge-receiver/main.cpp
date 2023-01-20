#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <httplib.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <thread>

#include <RF24/RF24.h>
#include "utils.hpp"
#include "packet.h"

#define PACKET_SIZE 32
#define ERROR_LED 13

// create an RF24 object called "radio" and pass it the pins for the chip enable (CE) and chip select (CSN) connections
RF24 radio(22, 0);
const unsigned char addresses[][6] = {"00001", "00002"};

bool LED_state;
bool New_Pings;
const int indicatorLedPin = 13;

unsigned long timeoutTime;

void webhookPostMessage()
{
	utils::Print("Posting Message ... ");
	// protocol and domain
	httplib::Client cli("https://company.webhook.office.com");

	cli.Post(
			// Restst of the path
			"/webhookb2/.....",
			"{\"text\": \"Knock Knock, somebody is waiting at the door. 🫑\"}",
			"application/json");
	utils::Print("OK\n");
}

 // Main
void setup()
{
    utils::Print("Logging ... OK\n");

    utils::Print("addresse: %s\n", addresses[0]);
    utils::Print("Radio init ... ");
    if (!radio.begin())
    {
        utils::Print("FAIL\n");
    	utils::Print("radio hardware not responding!\n");
        while (1) {} // hold program in infinite loop to prevent subsequent errors
    }

    radio.begin();                          // start radio communication
    radio.setAddressWidth(5);               // set the address width to 5
    radio.openWritingPipe(addresses[1]);    // set the address for transmitting
    radio.openReadingPipe(1, addresses[0]); // set the address for receiving
    radio.setPALevel(RF24_PA_MIN);          // set the power level for radio transmission

    utils::Print("OK\n");
    radio.startListening();
}

void loop()
{
    Packet packet;

    if (receivePacket(radio, packet))
    {
        switch(packet.type)
        {
            case PacketType::ButtonPressed:
                LED_state = true;
                utils::Print("packet: LED_state: true\n");
				webhookPostMessage();
				utils::Print("Ping Packet:");
                break;

            case PacketType::Timeout:
                LED_state = false;
                utils::Print("packet: LED_state: false\n");
                break;

            case PacketType::Battery:
                utils::Print("Battery level: %d\n", packet.data.battery.level);
                break;

            case PacketType::Ping:
                timeoutTime = millis() + 1000 + packet.data.ping.millisPlusTimeout - packet.data.ping.millis;
				utils::Print(".");
                New_Pings = true;
                break;

            default:
                break;
        }
    } else
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

int main()
{
	setup();
	while (true)
	{
		loop();
	}
	return 0;
}
