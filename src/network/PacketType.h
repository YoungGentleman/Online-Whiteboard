#pragma once

#include <QtGlobal>


enum class PacketType : quint8
{
    Snapshot   = 0x01,  
    Draw       = 0x02,  
    Erase      = 0x03,  
    UserJoined = 0x04,  
    UserLeft   = 0x05,  
    Ping       = 0x06,  
    Pong       = 0x07,
};
