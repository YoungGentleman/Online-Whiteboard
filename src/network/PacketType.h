#pragma once
 
#include <QtGlobal>
 
enum class PacketType : quint8
{
    Join       = 0x10,  // client -server 
    Snapshot   = 0x01,  // server - client
    Draw       = 0x02,  
    Erase      = 0x03,  
    Clear      = 0x08,   
    UserJoined = 0x04,
    UserLeft   = 0x05,
    Ping       = 0x06,
    Pong       = 0x07,
};