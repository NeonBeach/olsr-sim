#include <thread>
#include <chrono>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "node.h"
#include "environment.h"
#include "logger.h"
#include "messages.h"

Node::Node(int id, Environment *env) : id(id), environment(env) {}
Node::~Node() {}

void Node::run()
{
    auto lastHelloTime = std::chrono::steady_clock::now();
    auto lastTCUpdate = std::chrono::steady_clock::now();
    constexpr auto HELLO_INTERVAL = std::chrono::seconds(1);
    constexpr auto TC_INTERVAL = std::chrono::seconds(2);

    while (running)
    {
        bool hasMessages = environment->waitForMessages(id, std::chrono::milliseconds(500));

        if (hasMessages)
        {
            std::vector<Message> messages = environment->receiveMessages(id);
            for (const auto &msg : messages)
            {
                if (msg.type == MessageType::HELLO && HELLO_seq_nums[msg.srcId] < msg.seqNum)
                {
                    HELLO_seq_nums[msg.srcId] = msg.seqNum;
                    if (msg.seqNum == 1)
                    {
                        neighbors.insert(msg.srcId);
                    }
                    for (int two_hop_neighbor : msg.payload_nbr)
                    {
                        if (two_hop_neighbor != id && neighbors.find(two_hop_neighbor) == neighbors.end())
                        {
                            two_hop_neighbors[msg.srcId].insert(two_hop_neighbor);
                        }
                    }
                    for (int mpr : msg.payload_mpr)
                    {
                        if (mpr == id)
                        {
                            ms.insert(msg.srcId);
                        }
                    }
                }
                else if (msg.type == MessageType::TC)
                {
                    // ignore for now
                }

                std::stringstream ss;
                ss << "Node " << id << " received " << MessageTypeToString(msg.type) << " [" << msg.seqNum << "] from Node " << msg.srcId;
                LOG_DEBUG(ss.str());
            }
        }

        std::unordered_map<int, int> two_hops_covered;
        multipoint_relays.clear();
        std::vector<std::pair<int, int>> sorted_by_degree;
        for (const auto &[neighbor, two_hop_set] : two_hop_neighbors)
        {
            sorted_by_degree.push_back({neighbor, static_cast<int>(two_hop_set.size())});
            for (int two_hop : two_hop_set)
                two_hops_covered[two_hop] = 0;
        }
        std::sort(sorted_by_degree.begin(), sorted_by_degree.end(), [](const auto &a, const auto &b)
                  { return a.second > b.second; });

        for (const auto &[neighbor, degree] : sorted_by_degree)
        {
            const auto &two_hop_set = two_hop_neighbors[neighbor];
            bool all_covered = true;
            for (int two_hop : two_hop_set)
                if (two_hops_covered[two_hop] == 0)
                {
                    all_covered = false;
                    break;
                }
            if (all_covered)
                continue;
            multipoint_relays.insert(neighbor);
            for (int two_hop : two_hop_set)
                two_hops_covered[two_hop] = 1;
        }

        if (!ms.empty())
            is_relay = true;
        else
            is_relay = false;

        auto currentTime = std::chrono::steady_clock::now();
        if (currentTime - lastHelloTime >= HELLO_INTERVAL)
        {
            HELLO_seq_num++;
            Message helloMsg{id, -1, HELLO_seq_num, MessageType::HELLO, std::unordered_set<int>(neighbors.begin(), neighbors.end()), std::unordered_set<int>(multipoint_relays.begin(), multipoint_relays.end()), {}};
            environment->sendMessage(helloMsg);

            std::stringstream ss;
            ss << "Node " << id << " sent HELLO [" << helloMsg.seqNum << "]";
            LOG_DEBUG(ss.str());

            lastHelloTime = currentTime;
        }
    }
}

void Node::stop()
{
    std::stringstream ss;
    ss << "\nNode " << id << " (final state)\n";
    ss << "nbr: ";
    for (const auto &nbr : neighbors)
        ss << nbr << " ";
    ss << "\nmpr: ";
    for (const auto &mpr : multipoint_relays)
        ss << mpr << " ";
    ss << "\nms: ";
    for (const auto &m : ms)
        ss << m << " ";
    ss << "\n";
    LOG_DEBUG(ss.str());
    running = false;
}

// doesnt reset for new simulation (todo)
void Node::reset()
{
    std::lock_guard<std::mutex> lock(inboxMutex);
    while (!inbox.empty())
        inbox.pop();
    neighbors.clear();
    multipoint_relays.clear();
    ms.clear();
    two_hop_neighbors.clear();
    HELLO_seq_nums.clear();
    TC_seq_nums.clear();
    HELLO_seq_num = 0;
    TC_seq_num = 0;
    is_relay = false;
}
