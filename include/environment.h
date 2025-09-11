#pragma once
#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "messages.h"

class Node;

/**
 * @brief Message structure for inter-node communication
 * @param srcId Source node ID
 * @param dstId Destination node ID (-1 for broadcast)
 * @param type Message type (e.g., "HELLO", "TC")
 * @param payload Message content
 */


/**
 * @brief Environment class simulating a network environment.
 * Manages nodes, their connections, and message passing
 */
class Environment
{
public:
    void registerNode(int nodeId, Node *node);
    void connect(int nodeA, int nodeB);
    void sendMessage(const Message &msg);
    std::vector<Message> receiveMessages(int nodeId);
    bool waitForMessages(int nodeId, std::chrono::milliseconds timeout);
    std::vector<int> getNeighbors(int nodeId);

private:
    std::unordered_map<int, Node *> nodes;
    std::unordered_map<int, std::vector<int>> adjacency;
    std::unordered_map<int, std::queue<Message>> inboxes;
    std::mutex mtx;
    std::condition_variable cv;
};
