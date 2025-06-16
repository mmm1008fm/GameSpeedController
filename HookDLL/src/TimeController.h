#pragma once

#include <atomic>
#include <string>

// Глобальный множитель времени
extern std::atomic<double> g_timeMultiplier;

// Потокобезопасные функции работы с множителем времени
double GetTimeMultiplier();
void SetTimeMultiplier(double multiplier);

// Разбор команд, приходящих по IPC.
// Поддерживаются команды:
//   SET <value>  - задать новый множитель времени
//   RESET        - сбросить множитель на 1.0
// Возвращает true при успешном выполнении команды.
bool ParseIPCCommand(const std::string& command);
