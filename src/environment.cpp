#include <algorithm>
#include <sstream>

#include "environment.h"
#include "node.h"
#include "logger.h"

void Environment::registerNode(int nodeId, Node *node)
{
    std::lock_guard<std::mutex> lock(mtx);
    nodes[nodeId] = node;
    inboxes[nodeId] = std::queue<Message>();
}

void Environment::connect(int nodeA, int nodeB)
{
    std::lock_guard<std::mutex> lock(mtx);
    adjacency[nodeA].push_back(nodeB);
    adjacency[nodeB].push_back(nodeA);
}

void Environment::sendMessage(const Message &msg)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (msg.dstId == -1)
        for (int neighbor : adjacency[msg.srcId])
            inboxes[neighbor].push(msg);
    else
        inboxes[msg.dstId].push(msg);
    cv.notify_all();
}

std::vector<Message> Environment::receiveMessages(int nodeId)
{
    std::unique_lock<std::mutex> lock(mtx);
    std::vector<Message> messages;
    auto &q = inboxes[nodeId];
    while (!q.empty())
    {
        messages.push_back(q.front());
        q.pop();
    }
    return messages;
}

std::vector<int> Environment::getNeighbors(int nodeId)
{
    std::lock_guard<std::mutex> lock(mtx);
    return adjacency[nodeId];
}

bool Environment::waitForMessages(int nodeId, std::chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(mtx);
    return cv.wait_for(lock, timeout, [this, nodeId]()
                       { return !inboxes[nodeId].empty(); });
}
