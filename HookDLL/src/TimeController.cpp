#include "TimeController.h"
#include <sstream>
#include <algorithm>

std::atomic<double> g_timeMultiplier{1.0};

double GetTimeMultiplier()
{
    return g_timeMultiplier.load();
}

void SetTimeMultiplier(double multiplier)
{
    g_timeMultiplier.store(multiplier);
}

bool ParseIPCCommand(const std::string& command)
{
    std::istringstream iss(command);
    std::string action;
    if (!(iss >> action))
        return false;

    std::transform(action.begin(), action.end(), action.begin(), ::toupper);

    if (action == "SET")
    {
        double value;
        if (iss >> value)
        {
            SetTimeMultiplier(value);
            return true;
        }
    }
    else if (action == "RESET")
    {
        SetTimeMultiplier(1.0);
        return true;
    }

    return false;
}
