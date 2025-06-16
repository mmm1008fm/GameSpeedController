#include "TimeController.h"

double g_timeMultiplier = 1.0;

double GetTimeMultiplier()
{
    return g_timeMultiplier;
}

void SetTimeMultiplier(double multiplier)
{
    g_timeMultiplier = multiplier;
} 