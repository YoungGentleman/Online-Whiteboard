#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <QDataStream>

class VectorClock
{
public:
    VectorClock() = default;
    explicit VectorClock(int nodeCount) : m_clock(nodeCount, 0u) {}

    void     increment(int nodeId)
    {
        if (nodeId >= 0 && nodeId < (int)m_clock.size())
            ++m_clock[nodeId];
    }

    uint32_t at(int nodeId) const
    {
        if (nodeId >= 0 && nodeId < (int)m_clock.size())
            return m_clock[nodeId];
        return 0u;
    }

    void set(int nodeId, uint32_t value)
    {
        if (nodeId >= 0 && nodeId < (int)m_clock.size())
            m_clock[nodeId] = value;
    }

    int  size()       const { return (int)m_clock.size(); }
    void resize(int n)      { m_clock.resize(n, 0u); }

    void merge(const VectorClock &other)
    {
        int n = std::max((int)m_clock.size(), other.size());
        m_clock.resize(n, 0u);
        for (int i = 0; i < other.size(); ++i)
            m_clock[i] = std::max(m_clock[i], other.m_clock[i]);
    }

    // Правило Лэмпорта при получении: merge + increment собственного компонента
    void receive(const VectorClock &incoming, int myNodeId)
    {
        merge(incoming);
        increment(myNodeId);
    }

    // Правило Лэмпорта при отправке: increment собственного компонента
    void send(int myNodeId)
    {
        increment(myNodeId);
    }

    // Сравнение: true если this случился строго до other
    bool happensBefore(const VectorClock &other) const
    {
        if (m_clock.size() != other.m_clock.size()) return false;
        bool strictlyLess = false;
        for (int i = 0; i < (int)m_clock.size(); ++i) {
            if (m_clock[i] > other.m_clock[i]) return false;
            if (m_clock[i] < other.m_clock[i]) strictlyLess = true;
        }
        return strictlyLess;
    }

    const std::vector<uint32_t>& data() const { return m_clock; }

    friend QDataStream& operator<<(QDataStream &out, const VectorClock &vc)
    {
        out << quint32(vc.m_clock.size());
        for (uint32_t v : vc.m_clock) out << quint32(v);
        return out;
    }

    friend QDataStream& operator>>(QDataStream &in, VectorClock &vc)
    {
        quint32 n; in >> n;
        vc.m_clock.resize(int(n));
        for (uint32_t &v : vc.m_clock) {
            quint32 tmp; in >> tmp;
            v = tmp;
        }
        return in;
    }

private:
    std::vector<uint32_t> m_clock;
};
