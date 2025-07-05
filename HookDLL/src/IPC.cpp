#include "IPC.h"

IPCClient::IPCClient() : m_pipe(INVALID_HANDLE_VALUE), m_connected(false)
{
}

IPCClient::~IPCClient()
{
    Disconnect();
}

bool IPCClient::Connect(const std::wstring& pipeName)
{
    m_pipe = CreateFileW(
        pipeName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);

    m_connected = (m_pipe != INVALID_HANDLE_VALUE);
    return m_connected;
}

void IPCClient::Disconnect()
{
    if (m_connected)
    {
        CloseHandle(m_pipe);
        m_pipe = INVALID_HANDLE_VALUE;
        m_connected = false;
    }
}

bool IPCClient::SendMessage(const std::string& message)
{
    if (!m_connected)
        return false;

    DWORD bytesWritten;
    return WriteFile(
        m_pipe,
        message.c_str(),
        static_cast<DWORD>(message.length()),
        &bytesWritten,
        nullptr) != 0;
}

bool IPCClient::ReadMessage(std::string& message)
{
    if (!m_connected)
        return false;

    char buffer[1024];
    DWORD bytesRead = 0;
    if (!ReadFile(m_pipe, buffer, sizeof(buffer), &bytesRead, nullptr) || bytesRead == 0)
        return false;

    message.assign(buffer, bytesRead);
    return true;
}

bool IPCClient::IsConnected() const
{
    return m_connected;
} 