#pragma once

#include "../data/PacketType.h"
#include <QByteArray>

namespace Protocol {

struct Packet {
    PacketType type{};
    QByteArray payload;
};

QByteArray makePacket(PacketType type, const QByteArray &payload);

bool tryExtractPacket(QByteArray &buffer, Packet &outPacket);

} // namespace Protocol
