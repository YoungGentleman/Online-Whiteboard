#pragma once

#include <QtGlobal>

constexpr quint32 kMagic = 0x57424F44;

enum class PacketType : quint8
{
    Snapshot   = 0x01,
    Draw       = 0x02,
    Erase      = 0x03,
    Fill       = 0x04,
    UserJoined = 0x05,
    UserLeft   = 0x06,
    Ping       = 0x07,
    Pong       = 0x08,
};
