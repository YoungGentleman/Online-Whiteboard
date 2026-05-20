#pragma once

#include <vector>
#include <cstdint>

class VectorClock
{
public:
    VectorClock() = default;
    explicit VectorClock(int nodeCount) : m_clock(nodeCount, 0u) {}

    void     increment(int nodeId) { if (nodeId >= 0 && nodeId < (int)m_clock.size()) ++m_clock[nodeId]; }
    uint32_t at(int nodeId)  const { if (nodeId >= 0 && nodeId < (int)m_clock.size()) return m_clock[nodeId]; return 0u; }
    void     set(int nodeId, uint32_t value) { if (nodeId >= 0 && nodeId < (int)m_clock.size()) m_clock[nodeId] = value; }
    int      size()          const { return (int)m_clock.size(); }
    void     resize(int n)         { m_clock.resize(n, 0u); }

    void merge(const VectorClock &other) {
        int n = std::max((int)m_clock.size(), other.size());
        m_clock.resize(n, 0u);
        for (int i = 0; i < other.size(); ++i)
            m_clock[i] = std::max(m_clock[i], other.m_clock[i]);
    }

    const std::vector<uint32_t>& data() const { return m_clock; }

private:
    std::vector<uint32_t> m_clock;
};
