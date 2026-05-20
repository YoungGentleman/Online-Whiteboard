#pragma once

#include "PacketType.h"
#include <QByteArray>

namespace Protocol {

constexpr quint32 kMagic = 0x57424F44;

struct Packet {
    PacketType type{};
    QByteArray payload;
};

QByteArray makePacket(PacketType type, const QByteArray &payload);
bool       tryExtractPacket(QByteArray &buffer, Packet &outPacket);

}
