#pragma once

#include <QtGlobal>

enum class PacketType : quint8
{
    Join       = 0x10,
    Snapshot   = 0x01,
    Draw       = 0x02,
    Erase      = 0x03,
    Clear      = 0x08,
    Fill       = 0x09,
    Move       = 0x0A,
    UserJoined = 0x04,
    UserLeft   = 0x05,
    Ping       = 0x06,
    Pong       = 0x07,
};
