#pragma once

#include "../data/PacketType.h"
#include "VectorClock.h"
#include <QByteArray>
#include <QDataStream>

namespace Protocol {

constexpr quint32 kMagic   = 0x57424F44;
constexpr quint8  kVersion = 1;

struct Packet {
    PacketType  type{};
    VectorClock clock;
    QByteArray  payload;
};

inline QByteArray makePacket(PacketType type,
                              const VectorClock &clock,
                              const QByteArray &payload)
{
    QByteArray frame;
    QDataStream out(&frame, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);

    out << kMagic;
    out << kVersion;
    out << quint8(type);

    const auto &cv = clock.data();
    out << quint32(cv.size());
    for (uint32_t v : cv) out << quint32(v);

    out << quint32(payload.size());
    if (!payload.isEmpty())
        out.writeRawData(payload.constData(), payload.size());

    return frame;
}

inline bool tryExtractPacket(QByteArray &buffer, Packet &outPacket)
{
    constexpr int kMinHeader = 4 + 1 + 1 + 4;
    if (buffer.size() < kMinHeader) return false;

    QDataStream in(buffer);
    in.setVersion(QDataStream::Qt_6_0);

    quint32 magic = 0;
    quint8  ver   = 0;
    quint8  type  = 0;
    in >> magic >> ver >> type;

    if (magic != kMagic || ver != kVersion) {
        buffer.clear();
        return false;
    }

    quint32 clockSize = 0;
    in >> clockSize;
    if (clockSize > 256) { buffer.clear(); return false; }

    const int clockBytes  = int(clockSize) * 4;
    const int headerSoFar = kMinHeader + clockBytes;
    if (buffer.size() < headerSoFar + 4) return false;

    VectorClock vc{int(clockSize)};
    for (quint32 i = 0; i < clockSize; ++i) {
        quint32 v; in >> v;
        vc.set(int(i), v);
    }

    quint32 payloadSize = 0;
    in >> payloadSize;

    if (payloadSize > (16u * 1024u * 1024u)) { buffer.clear(); return false; }

    const int totalSize = headerSoFar + 4 + int(payloadSize);
    if (buffer.size() < totalSize) return false;

    outPacket.type    = PacketType(type);
    outPacket.clock   = vc;
    outPacket.payload = buffer.mid(headerSoFar + 4, int(payloadSize));
    buffer.remove(0, totalSize);
    return true;
}

}
