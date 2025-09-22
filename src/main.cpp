#include "app.h"
#include "logger.h"

int main()
{
    try
    {
        Logger::getInstance().init(
            static_cast<int>(LogOutput::IMGUI),
            "olsr-sim.log");

        App app;
        app.run();

        Logger::getInstance().shutdown();
        return 0;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR(std::string("Application error: ") + e.what());
        return 1;
    }
}
