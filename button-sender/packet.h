#pragma once
#include <nRF24L01.h> // include the nRF24L01 library
#include <RF24.h>     // include the RF24 library
#include "utils.hpp"

template<typename T>
inline bool sendPacket(RF24& pRadio, T& packet)
{
    pRadio.stopListening();

    if(!pRadio.write(&packet, sizeof(T)))
    {
        utils::Print("Failed to send\n");
        return false;
    }

    delay(50);
    pRadio.startListening();

    return true;
}

template<typename T>
inline bool receivePacket(RF24& pRadio, T& packet)
{
    if(pRadio.available())
    {
        pRadio.read(&packet, sizeof(T));
        return true;
    }

    return false;
}

enum PacketType : char
{
    Ping,
    ButtonPressed,
    Battery,
    Timeout,
};

struct Packet
{
    PacketType type;

    struct Ping
    {
        unsigned long millis;
        unsigned long millisPlusTimeout;
    };

    struct Button
    {
        char value;
        char repeated;
    };

    struct Battery
    {
        unsigned char level;
    };

    union Data
    {
        Ping ping;
        Button button;
        Battery battery;
    };

    Data data;
};
