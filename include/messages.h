#pragma once
#include <string>
#include <vector>
#include <unordered_set>

enum class MessageType
{
    HELLO,
    TC
};

inline const char *MessageTypeToString(MessageType type)
{
    switch (type)
    {
    case MessageType::HELLO:
        return "HELLO";
    case MessageType::TC:
        return "TC";
    default:
        return "UNKNOWN";
    }
}

struct Message
{
    int srcId;
    int dstId;
    int seqNum;
    MessageType type;
    std::unordered_set<int> payload_nbr;
    std::unordered_set<int> payload_mpr;
    std::vector<std::pair<int, int>> payload_tc;
};
