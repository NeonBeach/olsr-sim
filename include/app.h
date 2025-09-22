#pragma once
#include <unordered_set>
#include <memory>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <ImGuiFileDialog.h>

#include "logger.h"
#include "graphs.h"
#include "environment.h"
#include "node.h"

class App
{
public:
    App();
    ~App();

    void run();

private:
    void init();
    void setupImGui();
    void setupImGuiStyle();
    void render();
    void handleLogMessage(const std::string &message, LogLevel level);
    void cleanup();

    void renderGetStarted();
    void setupSimulation(std::string path);

    void startSimulation();
    void stopSimulation();

    void renderControlPanel();
    void renderTopology();
    void renderEventLog();

    GLFWwindow *window;
    struct LogEntry
    {
        std::string message;
        LogLevel level;
        std::chrono::system_clock::time_point timestamp;
    };
    std::vector<LogEntry> logEntries;
    struct EventLogParams
    {
        bool AutoScroll = true;
        ImGuiTextFilter Filter;
        char InputBuf[256];
    };
    EventLogParams eventLogParams;

    Environment m_env;
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<std::thread> threads;
    std::vector<std::vector<int>> adjacency;

    struct TopologyDetails
    {
        std::vector<std::vector<int>> adjacencyList;
        std::vector<Graphs::Point2D> normalized_positions;
        float scale = 1.0f;
    };
    TopologyDetails topologyDetails;

    bool networkLoaded = false;
    bool topologyCalculated = false;
    bool simulationRunning = false;
};
