#pragma once

#include <atomic>
#include <string>

// Глобальный множитель времени
extern std::atomic<double> g_timeMultiplier;

// Максимальное допустимое значение множителя времени. Значение
// 0 отключает таймеры. Слишком большие значения могут привести
// к нестабильной работе приложения, поэтому вводится ограничение.
constexpr double MAX_TIME_MULTIPLIER = 10.0;

// Потокобезопасные функции работы с множителем времени
double GetTimeMultiplier();
void SetTimeMultiplier(double multiplier);

// Разбор команд, приходящих по IPC.
// Поддерживаются команды:
//   SET <value>  - задать новый множитель времени
//   RESET        - сбросить множитель на 1.0
// Возвращает true при успешном выполнении команды.
bool ParseIPCCommand(const std::string& command);
