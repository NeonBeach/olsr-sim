#pragma once
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <memory>
#include <ctime>
#include <functional>

enum class LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

enum class LogOutput
{
    CONSOLE = 1,
    FILE = 2,
    IMGUI = 4
};

using LogCallback = std::function<void(const std::string &, LogLevel)>;

class Logger
{
public:
    static Logger &getInstance()
    {
        static Logger instance;
        return instance;
    }

    void init(int outputFlags = static_cast<int>(LogOutput::CONSOLE),
              const std::string &logFile = "")
    {
        std::lock_guard<std::mutex> lock(logMutex);
        this->outputFlags = outputFlags;

        if (!logFile.empty() && (outputFlags & static_cast<int>(LogOutput::FILE)))
        {
            logFilePath = logFile;
            logFileStream.open(logFile, std::ios::out | std::ios::app);
        }

        if (!workerThread.joinable())
        {
            done = false;
            workerThread = std::thread(&Logger::loggerWorker, this);
        }
    }

    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(logMutex);
            done = true;
        }
        cv.notify_one();

        if (workerThread.joinable())
            workerThread.join();

        if (logFileStream.is_open())
            logFileStream.close();
    }

    void log(LogLevel level, const std::string &message)
    {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::system_clock::to_time_t(now);

        std::tm tm_buf;
        localtime_s(&tm_buf, &timestamp);

        std::stringstream ss;
        ss << "[" << std::put_time(&tm_buf, "%H:%M:%S") << "] ";

        if (outputFlags & static_cast<int>(LogOutput::CONSOLE))
        {
            switch (level)
            {
            case LogLevel::DEBUG:
                ss << "\033[36m[DEBUG]\033[0m ";
                break;
            case LogLevel::INFO:
                ss << "\033[32m[INFO]\033[0m ";
                break;
            case LogLevel::WARNING:
                ss << "\033[33m[WARNING]\033[0m ";
                break;
            case LogLevel::ERROR:
                ss << "\033[31m[ERROR]\033[0m ";
                break;
            }
        }
        else
        {
            switch (level)
            {
            case LogLevel::DEBUG:
                ss << "[DEBUG] ";
                break;
            case LogLevel::INFO:
                ss << "[INFO] ";
                break;
            case LogLevel::WARNING:
                ss << "[WARNING] ";
                break;
            case LogLevel::ERROR:
                ss << "[ERROR] ";
                break;
            }
        }

        ss << message;

        {
            std::lock_guard<std::mutex> lock(logMutex);
            logQueue.push({ss.str(), level});
        }
        cv.notify_one();
    }

    void debug(const std::string &message)
    {
        log(LogLevel::DEBUG, message);
    }

    void info(const std::string &message)
    {
        log(LogLevel::INFO, message);
    }

    void warning(const std::string &message)
    {
        log(LogLevel::WARNING, message);
    }

    void error(const std::string &message)
    {
        log(LogLevel::ERROR, message);
    }

public:
    void setCallback(LogCallback callback)
    {
        std::lock_guard<std::mutex> lock(logMutex);
        imguiCallback = callback;
    }

private:
    Logger() = default;
    ~Logger()
    {
        shutdown();
    }

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;

    struct LogMessage
    {
        std::string text;
        LogLevel level;
    };

    void loggerWorker()
    {
        std::unique_lock<std::mutex> lock(logMutex);
        while (!done || !logQueue.empty())
        {
            cv.wait(lock, [this]
                    { return done || !logQueue.empty(); });

            while (!logQueue.empty())
            {
                LogMessage message = logQueue.front();
                logQueue.pop();

                lock.unlock();

                if (outputFlags & static_cast<int>(LogOutput::CONSOLE))
                {
                    std::cout << message.text << std::endl;
                }

                if ((outputFlags & static_cast<int>(LogOutput::FILE)) && logFileStream.is_open())
                {
                    logFileStream << message.text << std::endl;
                    logFileStream.flush();
                }

                if ((outputFlags & static_cast<int>(LogOutput::IMGUI)) && imguiCallback)
                {
                    imguiCallback(message.text, message.level);
                }

                lock.lock();
            }
        }
    }

    std::mutex logMutex;
    std::condition_variable cv;
    std::queue<LogMessage> logQueue;
    std::atomic<bool> done{false};
    std::thread workerThread;
    LogCallback imguiCallback;

    int outputFlags{static_cast<int>(LogOutput::CONSOLE)};
    std::string logFilePath;
    std::ofstream logFileStream;
};

#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
