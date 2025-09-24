#include <iostream>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "app.h"

App::App() : window(nullptr)
{
    init();
}

App::~App()
{
    cleanup();
}

void App::setupImGuiStyle()
{
    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.2f, 0.5f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.2f, 0.2f, 0.5f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1f, 0.1f, 0.3f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.5f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.7f, 0.3f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1f, 0.4f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.7f, 0.7f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.2f, 0.5f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.3f, 0.6f, 0.9f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.4f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.5f, 0.7f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.4f, 0.6f, 0.8f, 1.0f);
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.WindowRounding = 7.0f;
}

void App::init()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(1280, 720, "OLSR Simulation", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    setupImGui();
    setupImGuiStyle();

    Logger::getInstance().setCallback([this](const std::string &msg, LogLevel level)
                                      { handleLogMessage(msg, level); });
}

void App::setupImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void App::run()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        render();
    }
}

void App::render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::Begin("DockSpace", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);

    ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f));

    // renderNodeDetails();
    if (!networkLoaded)
    {
        renderGetStarted();
    }
    else
    {
        renderControlPanel();
        renderTopology();
    }
    renderEventLog();

    ImGui::End();

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

void App::renderGetStarted()
{
    ImGui::Begin("Get Started");

    ImGui::Spacing();
    ImGui::Text("Choose a network file to load the simulation.");
    ImGui::Spacing();

    if (ImGui::Button("Choose Network File"))
    {
        IGFD::FileDialogConfig config;
        config.path = "../data";
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose Network File", ".txt", config);
    }
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            setupSimulation(filePathName);
        }
        ImGuiFileDialog::Instance()->Close();
    }
    ImGui::End();
}

void App::setupSimulation(std::string path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        LOG_ERROR("Failed to open network file: " + path);
        return;
    }

    int numNodes;
    file >> numNodes;
    m_env.setNumNodes(numNodes);
    nodes.clear();
    threads.clear();
    adjacency.clear();
    adjacency.resize(numNodes, std::vector<int>(numNodes, 0));
    for (int i = 0; i < numNodes; ++i)
    {
        nodes.push_back(std::make_unique<Node>(i, &m_env));
        m_env.registerNode(i, nodes.back().get());
    }
    for (int i = 0; i < numNodes; ++i)
    {
        for (int j = 0; j < numNodes; ++j)
        {
            int connected;
            file >> connected;
            if (connected && i < j)
            {
                m_env.connect(i, j);
            }
            adjacency[i][j] = connected;
        }
    }
    LOG_INFO("Network loaded from file: " + path);
    networkLoaded = true;
}

void App::startSimulation()
{
    LOG_INFO("Starting simulation");
    for (auto &node : nodes)
    {
        threads.emplace_back(&Node::run, node.get());
    }
}

void App::stopSimulation()
{
    LOG_INFO("Stopping simulation");
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
    threads.clear();
    // for (auto &node : nodes)
    // {
    //     node->reset();
    // }
}

void App::renderControlPanel()
{
    ImGui::Begin("Control Panel");

    if (!simulationRunning)
    {
        if (ImGui::Button("Start Simulation"))
        {
            startSimulation();
            simulationRunning = true;
        }
    }
    else
    {
        if (ImGui::Button("Stop Simulation"))
        {
            stopSimulation();
            simulationRunning = false;
        }
    }
    ImGui::Separator();
    ImGui::TextDisabled("To restart the simulation, restart the application.");
    ImGui::End();
}

void App::renderTopology()
{
    ImGui::Begin("Network Topology");

    if (networkLoaded)
    {
        if (ImGui::Button("Recalculate Layout"))
        {
            topologyCalculated = false;
        }

        static int iterations = 100;
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        if (ImGui::InputInt("Iterations", &iterations, 5, 10))
        {
            if (iterations < 1)
                iterations = 1;
            if (iterations > 200)
                iterations = 200;
            topologyCalculated = false;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(10, 0));
        ImGui::SameLine();
        static float k = 8.0f;
        ImGui::PushItemWidth(100);
        if (ImGui::InputFloat("k", &k, 1.0f, 5.0f))
        {
            if (k < 0.0f)
                k = 0.0f;
            if (k > 100.0f)
                k = 100.0f;
            topologyCalculated = false;
        }
        ImGui::PopItemWidth();

        ImVec2 window_size = ImGui::GetContentRegionAvail();
        float display_w = window_size.x - 80;
        float display_h = window_size.y - 80;

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 cursor = ImGui::GetCursorScreenPos();

        if (!topologyCalculated)
        {
            topologyCalculated = true;
            topologyDetails.adjacencyList = adjacency;

            // Get normalized positions from Fruchterman-Reingold algorithm
            topologyDetails.normalized_positions = Graphs::fruchterman_reingold(topologyDetails.adjacencyList, iterations, k);
        }

        const ImVec2 canvas_pos = ImVec2(cursor.x + 40, cursor.y + 40);

        std::vector<ImVec2> node_positions;
        node_positions.reserve(topologyDetails.adjacencyList.size());

        for (int i = 0; i < topologyDetails.adjacencyList.size(); ++i)
        {
            float x = canvas_pos.x + (topologyDetails.normalized_positions[i].first + 1.0f) * 0.5f * display_w;
            float y = canvas_pos.y + (topologyDetails.normalized_positions[i].second + 1.0f) * 0.5f * display_h;
            node_positions.push_back(ImVec2(x, y));
        }

        const size_t node_count = topologyDetails.adjacencyList.size();
        // Add a border around the drawing area
        ImVec2 canvas_size = ImVec2(display_w + 80, display_h + 80);
        draw_list->AddRect(cursor, ImVec2(cursor.x + canvas_size.x, cursor.y + canvas_size.y),
                           IM_COL32(150, 150, 150, 100), 0.0f, ImDrawFlags_None, 1.0f);

        // Draw connections first (so they appear behind nodes)
        for (int i = 0; i < topologyDetails.adjacencyList.size(); ++i)
            for (int j = 0; j < i; j++)
            {
                if (topologyDetails.adjacencyList[i][j] == 0)
                    continue;
                int a = j, b = i;
                if (a < node_positions.size() && b < node_positions.size())
                {
                    draw_list->AddLine(node_positions[a], node_positions[b], IM_COL32(100, 200, 255, 255), 3.0f);
                }
            }

        // Draw nodes on top of connections
        for (size_t i = 0; i < node_count && i < node_positions.size(); ++i)
        {
            const float node_radius = 22.0f;

            auto color = IM_COL32(80, 220, 120, 255);
            if (nodes[i]->isRelay())
            {
                color = IM_COL32(220, 120, 80, 255);
            }

            // Draw node background
            draw_list->AddCircleFilled(node_positions[i], node_radius, color, 0);

            // Draw node outline
            draw_list->AddCircle(node_positions[i], node_radius, IM_COL32(0, 0, 0, 255), 0, 2.0f);

            // Draw node label
            char label[16];
            snprintf(label, sizeof(label), "N%zu", i);
            ImVec2 text_size = ImGui::CalcTextSize(label);
            draw_list->AddText(
                ImVec2(node_positions[i].x - text_size.x * 0.5f, node_positions[i].y - text_size.y * 0.5f),
                IM_COL32(0, 0, 0, 255),
                label);
        }

        // Add interaction capabilities - dragging nodes
        ImGui::InvisibleButton("canvas", canvas_size);

        // Handle mouse interactions
        ImVec2 mouse_pos = ImGui::GetMousePos();
        bool mouse_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        bool mouse_down = ImGui::IsMouseDown(ImGuiMouseButton_Left);
        bool mouse_released = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

        // Start dragging
        if (mouse_clicked && ImGui::IsItemHovered())
        {
            for (int i = 0; i < node_count && i < node_positions.size(); ++i)
            {
                float dist_x = mouse_pos.x - node_positions[i].x;
                float dist_y = mouse_pos.y - node_positions[i].y;
                if (dist_x * dist_x + dist_y * dist_y < 22.0f * 22.0f)
                {
                    draggedNodeId = i;
                    isDragging = true;
                    dragOffset = ImVec2(dist_x, dist_y);
                    break;
                }
            }
        }

        // Continue dragging
        if (isDragging && mouse_down && draggedNodeId >= 0)
        {
            // Update the normalized position based on mouse position
            float new_x = (mouse_pos.x - dragOffset.x - canvas_pos.x) / display_w;
            float new_y = (mouse_pos.y - dragOffset.y - canvas_pos.y) / display_h;

            // Convert back from [0,1] to [-1,1] range (undo the +1)*0.5 transformation)
            new_x = new_x * 2.0f - 1.0f;
            new_y = new_y * 2.0f - 1.0f;

            // Clamp to reasonable bounds
            new_x = std::max(-1.0f, std::min(1.0f, new_x));
            new_y = std::max(-1.0f, std::min(1.0f, new_y));

            // Update the stored position
            topologyDetails.normalized_positions[draggedNodeId].first = new_x;
            topologyDetails.normalized_positions[draggedNodeId].second = new_y;
        }

        // End dragging
        if (mouse_released)
        {
            isDragging = false;
            draggedNodeId = -1;
        }

        // Show tooltip when hovering
        if (ImGui::IsItemHovered() && !isDragging)
        {
            // Check if hovering over a specific node
            int hoveredNode = -1;
            for (int i = 0; i < node_count && i < node_positions.size(); ++i)
            {
                float dist_x = mouse_pos.x - node_positions[i].x;
                float dist_y = mouse_pos.y - node_positions[i].y;
                if (dist_x * dist_x + dist_y * dist_y < 22.0f * 22.0f)
                {
                    hoveredNode = i;
                    break;
                }
            }

            if (hoveredNode >= 0)
            {
                std::string tooltip = "Node " + std::to_string(hoveredNode);
                if (nodes[hoveredNode]->isRelay())
                {
                    tooltip += " (Relay)";
                }
                tooltip += "\nClick and drag to move";
                ImGui::SetTooltip("%s", tooltip.c_str());
            }
            else
            {
                ImGui::SetTooltip("Network topology visualization\nDrag nodes to rearrange");
            }
        }
    }
    else
    {
        ImGui::Text("No network data available.");
    }

    ImGui::End();
}

// void App::renderNodeDetails()
// {
//     ImGui::Begin("Node Details");

//     if (const auto &nodes = simulation->getNodes(); !nodes.empty())
//     {
//         for (const auto &node : nodes)
//         {
//             if (ImGui::TreeNode(("Node " + std::to_string(node->id)).c_str()))
//             {
//                 auto node_details = cum_node_details[node->id];
//                 ImGui::Text("Neighbours:");
//                 for (auto &nbr : node_details.nbrs)
//                 {
//                     ImGui::BulletText("Node %d", nbr);
//                 }

//                 ImGui::Text("Multipoint Relays:");
//                 for (auto &mpr : node_details.mprs)
//                 {
//                     ImGui::BulletText("Node %d", mpr);
//                 }

//                 ImGui::Text("Multipoint Relay Selector Set:");
//                 for (auto &mprs : node_details.ms)
//                 {
//                     ImGui::BulletText("Node %d", mprs);
//                 }
//                 ImGui::TreePop();
//             }
//         }
//     }

//     ImGui::End();
// }

void App::renderEventLog()
{
    ImGui::Begin("Event Log");

    while (logEntries.size() > 1000)
        logEntries.erase(logEntries.begin());

    if (ImGui::Button("Clear"))
        logEntries.clear();
    ImGui::SameLine();

    if (ImGui::BeginPopup("Options"))
    {
        ImGui::Checkbox("Auto-scroll", &eventLogParams.AutoScroll);
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("Options"))
        ImGui::OpenPopup("Options");

    ImGui::SameLine();
    eventLogParams.Filter.Draw("Filter e.g. (\"Node 1\"), (\"HELLO\")", 180);
    ImGui::Separator();

    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y;

    auto formatTime = [](const auto &time_point)
    {
        auto time = std::chrono::system_clock::to_time_t(time_point);
        std::tm tm;
        localtime_s(&tm, &time);
        char timeStr[32];
        std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &tm);
        return std::string(timeStr);
    };

    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
    {
        for (const auto &entry : logEntries)
        {
            if (!eventLogParams.Filter.PassFilter(entry.message.c_str()))
                continue;

            ImVec4 color;
            switch (entry.level)
            {
            case LogLevel::DEBUG:
                color = ImVec4(0.5f, 0.5f, 1.0f, 1.0f); // Light blue
                break;
            case LogLevel::INFO:
                color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
                break;
            case LogLevel::WARNING:
                color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
                break;
            case LogLevel::ERROR:
                color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
                break;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertFloat4ToU32(color));
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));        // Transparent selection
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.3f)); // Light selection on hover
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 0.4f));  // Light selection when active

            if (ImGui::Selectable(entry.message.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    ImGui::SetClipboardText(entry.message.c_str());
                }
            }

            ImGui::PopStyleColor(4);
        }

        if (eventLogParams.AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    ImGui::End();
}

void App::handleLogMessage(const std::string &message, LogLevel level)
{
    LogEntry entry{
        message,
        level,
        std::chrono::system_clock::now()};
    logEntries.push_back(entry);
}

void App::cleanup()
{
    stopSimulation();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window)
    {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}
