#pragma once
#include <vector>
#include <cmath>
#include <random>
#include <utility>
#include <stdexcept>
#include <algorithm>

namespace Graphs
{
    using Point2D = std::pair<float, float>;
    using AdjacencyList = std::vector<std::vector<int>>;

    inline std::vector<Point2D> fruchterman_reingold(
        const AdjacencyList &graph,
        int iterations = 50,
        float k = 40.0f,
        float initialTemp = 5.0f,
        float epsilon = 0.01f)
    {
        if (graph.empty())
        {
            throw std::invalid_argument("Graph cannot be empty");
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        const size_t num_nodes = graph.size();

        std::vector<Point2D> positions(num_nodes);
        for (size_t i = 0; i < num_nodes; ++i)
        {
            positions[i] = std::make_pair(dist(gen), dist(gen));
        }

        for (int iter = 0; iter < iterations; ++iter)
        {
            float cooling_factor = initialTemp * std::pow(1.0f - static_cast<float>(iter) / iterations, 2.0f);

            std::vector<Point2D> displacement(num_nodes, std::make_pair(0.0f, 0.0f));

            for (size_t i = 0; i < num_nodes; ++i)
            {
                for (size_t j = 0; j < i; ++j)
                {
                    if (i != j)
                    {
                        float dx = positions[i].first - positions[j].first;
                        float dy = positions[i].second - positions[j].second;
                        float distance = std::sqrt(dx * dx + dy * dy) + epsilon;

                        float repulsive_force = (k * k) / distance;
                        displacement[i].first += (dx / distance) * repulsive_force;
                        displacement[i].second += (dy / distance) * repulsive_force;
                    }
                }
            }

            for (size_t i = 0; i < num_nodes; ++i)
            {
                for (size_t j = 0; j < i; ++j)
                {
                    size_t neighbor = static_cast<size_t>(graph[i][j]);

                    if (i >= neighbor)
                    {
                        continue;
                    }

                    float dx = positions[i].first - positions[neighbor].first;
                    float dy = positions[i].second - positions[neighbor].second;
                    float distance = std::sqrt(dx * dx + dy * dy) + epsilon;

                    float attractive_force = (distance * distance) / k;
                    displacement[i].first -= (dx / distance) * attractive_force;
                    displacement[i].second -= (dy / distance) * attractive_force;
                    displacement[neighbor].first += (dx / distance) * attractive_force;
                    displacement[neighbor].second += (dy / distance) * attractive_force;
                }
            }

            for (size_t i = 0; i < num_nodes; ++i)
            {
                float disp_length = std::sqrt(
                                        displacement[i].first * displacement[i].first +
                                        displacement[i].second * displacement[i].second) +
                                    epsilon;

                float limited_disp = std::min(disp_length, cooling_factor);

                positions[i].first += (displacement[i].first / disp_length) * limited_disp;
                positions[i].second += (displacement[i].second / disp_length) * limited_disp;
            }
        }

        // Find the bounding box
        float min_x = positions[0].first, min_y = positions[0].second;
        float max_x = positions[0].first, max_y = positions[0].second;

        for (const auto &pos : positions)
        {
            min_x = std::min(min_x, pos.first);
            min_y = std::min(min_y, pos.second);
            max_x = std::max(max_x, pos.first);
            max_y = std::max(max_y, pos.second);
        }

        // Calculate center of the bounding box
        float center_x = (min_x + max_x) / 2.0f;
        float center_y = (min_y + max_y) / 2.0f;

        // Calculate scaling factors to fit within [-1, 1]
        float width = max_x - min_x + epsilon;
        float height = max_y - min_y + epsilon;
        float scale = 2.0f / std::max(width, height);

        // Scale from center and ensure values are between -1 and 1
        for (auto &pos : positions)
        {
            pos.first = (pos.first - center_x) * scale;
            pos.second = (pos.second - center_y) * scale;

            // Clamp values to ensure they're within [-1, 1]
            pos.first = std::max(-1.0f, std::min(1.0f, pos.first));
            pos.second = std::max(-1.0f, std::min(1.0f, pos.second));
        }

        return positions;
    }

    inline std::vector<Point2D> radial_layout(const AdjacencyList &graph)
    {
        if (graph.empty())
        {
            throw std::invalid_argument("Graph cannot be empty");
        }

        // find most suitable center node (node with degree > 1 and almost equal distribution of neighbors)
        int center_node = -1;
        size_t min_diff = graph.size();
        for (size_t i = 0; i < graph.size(); ++i)
        {
            int degree = static_cast<int>(graph[i].size());
            if (degree > 1)
            {
                int left_count = 0, right_count = 0;
                for (int neighbor : graph[i])
                {
                    if (neighbor < static_cast<int>(i))
                        left_count++;
                    else if (neighbor > static_cast<int>(i))
                        right_count++;
                }
                int diff = std::abs(left_count - right_count);
                if (diff < min_diff)
                {
                    min_diff = diff;
                    center_node = static_cast<int>(i);
                }
            }
        }

        if (center_node == -1)
        {
            center_node = 0; // fallback to first node
        }

        // now start from center, place neighbors in concentric circles
        std::vector<Point2D> positions(graph.size());
        std::vector<bool> visited(graph.size(), false);
        std::vector<int> level(graph.size(), 0);

        // Place center node at origin
        positions[center_node] = std::make_pair(0.0f, 0.0f);
        visited[center_node] = true;

        // BFS to assign levels and positions
        std::vector<int> queue;
        queue.push_back(center_node);

        while (!queue.empty())
        {
            int current = queue.front();
            queue.erase(queue.begin());

            float current_level_radius = static_cast<float>(level[current]) * 0.2f + 0.1f;

            // Get all unvisited neighbors
            std::vector<int> unvisited_neighbors;
            for (int neighbor : graph[current])
            {
                if (!visited[neighbor])
                {
                    unvisited_neighbors.push_back(neighbor);
                    level[neighbor] = level[current] + 1;
                    visited[neighbor] = true;
                    queue.push_back(neighbor);
                }
            }

            // Place neighbors evenly in a circle around their level
            size_t n_neighbors = unvisited_neighbors.size();
            for (size_t i = 0; i < n_neighbors; ++i)
            {
                float angle = 2.0f * 3.14f * static_cast<float>(i) / n_neighbors;
                float x = current_level_radius * std::cos(angle);
                float y = current_level_radius * std::sin(angle);
                positions[unvisited_neighbors[i]] = std::make_pair(x, y);
            }
        }

        // Scale positions to [-1, 1] range if needed
        if (!positions.empty())
        {
            float max_distance = 0.0f;
            for (const auto &pos : positions)
            {
                max_distance = std::max(max_distance,
                                        std::sqrt(pos.first * pos.first + pos.second * pos.second));
            }

            if (max_distance > 1.0f)
            {
                for (auto &pos : positions)
                {
                    pos.first /= max_distance;
                    pos.second /= max_distance;
                }
            }
        }

        return positions;
    }
}
