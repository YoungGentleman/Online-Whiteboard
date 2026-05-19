#pragma once
#include <QVector>
#include <QDataStream>
#include <QtGlobal>


class VectorClock
{
public:
    VectorClock() = default;
    explicit VectorClock(int nodeCount) : m_clock(nodeCount, 0u) {}

    void    increment(int nodeId)   { if (nodeId < m_clock.size()) ++m_clock[nodeId]; }
    quint32 at(int nodeId)    const { return (nodeId < m_clock.size()) ? m_clock[nodeId] : 0u; }
    int     size()            const { return m_clock.size(); }

    void merge(const VectorClock &other) {
        int n = qMax(m_clock.size(), other.m_clock.size());
        m_clock.resize(n, 0u);
        for (int i = 0; i < other.m_clock.size(); ++i)
            m_clock[i] = qMax(m_clock[i], other.m_clock[i]);
    }

    friend QDataStream &operator<<(QDataStream &out, const VectorClock &vc) {
        out << quint32(vc.m_clock.size());
        for (quint32 v : vc.m_clock) out << v;
        return out;
    }
    friend QDataStream &operator>>(QDataStream &in, VectorClock &vc) {
        quint32 n; in >> n;
        vc.m_clock.resize(int(n));
        for (quint32 &v : vc.m_clock) in >> v;
        return in;
    }

private:
    QVector<quint32> m_clock;
};
