#pragma once
#include <Windows.h>
#include <string>

class IPCClient
{
public:
    IPCClient();
    ~IPCClient();

    bool Connect(const std::wstring& pipeName);
    void Disconnect();
    bool SendMessage(const std::string& message);
    bool ReadMessage(std::string& message);
    bool IsConnected() const;

private:
    HANDLE m_pipe;
    bool m_connected;
}; 