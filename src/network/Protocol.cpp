#include "Protocol.h"

#include <QDataStream>
#include <QIODevice>
#include <QDebug>

namespace Protocol {

QByteArray makePacket(PacketType type, const QByteArray &payload)
{
    QByteArray frame;
    QDataStream out(&frame, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);

    out << kMagic;
    out << quint8(type);
    out << quint32(payload.size());
    if (!payload.isEmpty())
        out.writeRawData(payload.constData(), payload.size());

    return frame;
}

bool tryExtractPacket(QByteArray &buffer, Packet &outPacket)
{
    constexpr int kHeaderSize = 4 + 1 + 4;
    if (buffer.size() < kHeaderSize)
        return false;

    QDataStream in(buffer);
    in.setVersion(QDataStream::Qt_6_0);

    quint32 magic       = 0;
    quint8  type        = 0;
    quint32 payloadSize = 0;
    in >> magic >> type >> payloadSize;

    if (magic != kMagic) {
        qWarning() << "[Protocol] bad magic, resyncing buffer";
        buffer.clear();
        return false;
    }

    if (payloadSize > (16u * 1024u * 1024u)) {
        qWarning() << "[Protocol] payload too big:" << payloadSize;
        buffer.clear();
        return false;
    }

    const int totalSize = kHeaderSize + int(payloadSize);
    if (buffer.size() < totalSize)
        return false;

    outPacket.type    = PacketType(type);
    outPacket.payload = buffer.mid(kHeaderSize, int(payloadSize));
    buffer.remove(0, totalSize);
    return true;
}

}
