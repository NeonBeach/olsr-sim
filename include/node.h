#pragma once
#include "environment.h"
#include <queue>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

class Environment;

/**
 * @brief Node class representing a network node.
 */
class Node
{
public:
    int id;
    Node(int id, Environment *env);
    ~Node();

    void run();
    void stop();
    void reset();

private:
    Environment *environment;
    std::queue<Message> inbox;
    std::mutex inboxMutex;
    bool running = true;
    bool is_relay = false;

    int HELLO_seq_num = 0;
    int TC_seq_num = 0;

    /**
     * @brief Sequence numbers for HELLO/TC messages for each neighbor
     */
    std::unordered_map<int, int> HELLO_seq_nums, TC_seq_nums;

    /**
     * @brief List of one-hop neighbors and their adjacency count
     */
    std::unordered_set<int> neighbors;

    /**
     * @brief List of multipoint relays
     */
    std::unordered_set<int> multipoint_relays;

    /**
     * @brief Multipoint Relay Selector Set (MS)
     */
    std::unordered_set<int> ms;

    /**
     * @brief Topology table representing the network topology
     */
    std::unordered_map<int, std::unordered_set<int>> two_hop_neighbors;

    // /**
    //  * @brief Routing table mapping destination node IDs
    //  * to hop counts and next hop node IDs
    //  */
    // std::unordered_map<int, std::pair<int, int>> routingTable;
};
