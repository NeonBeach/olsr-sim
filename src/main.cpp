#include <vector>
#include <thread>
#include <memory>
#include <iostream>

#include "environment.h"
#include "node.h"
#include "logger.h"

int main()
{
    Logger::getInstance().init(true, "olsr-sim.log");

    Environment env;
    const int numNodes = 6;
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<std::thread> threads;

    for (int i = 0; i < numNodes; ++i)
    {
        nodes.push_back(std::make_unique<Node>(i, &env));
        env.registerNode(i, nodes.back().get());
    }
    LOG_INFO("Nodes registered");

    env.connect(0, 1);
    env.connect(1, 2);
    env.connect(1, 3);
    env.connect(2, 4);
    env.connect(0, 5);
    env.connect(5, 3);
    LOG_INFO("Nodes connected");

    LOG_INFO("Starting simulation");
    for (int i = 0; i < numNodes; ++i)
    {
        threads.emplace_back([&nodes, i]()
                             { nodes[i]->run(); });
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));

    LOG_INFO("Terminating simulation");
    for (auto &node : nodes)
    {
        node->stop();
    }

    for (auto &thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    LOG_INFO("Simulation finished");
    Logger::getInstance().shutdown();
    return 0;
}
