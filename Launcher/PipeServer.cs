using System;
using System.IO.Pipes;
using System.Threading.Tasks;
using System.Text;

namespace Launcher
{
    public class PipeServer
    {
        private NamedPipeServerStream? pipeServer;
        private bool isRunning;
        private string? pipeName;

        public event EventHandler<string>? MessageReceived;

        public void Start(string pipeName)
        {
            if (isRunning)
                return;

            this.pipeName = pipeName;
            isRunning = true;
            Task.Run(ListenForConnections);
        }

        public void Stop()
        {
            isRunning = false;
            pipeServer?.Dispose();
            pipeServer = null;
        }

        private async Task ListenForConnections()
        {
            while (isRunning)
            {
                try
                {
                    if (string.IsNullOrEmpty(pipeName))
                        throw new InvalidOperationException("Pipe name not set.");

                    using (pipeServer = new NamedPipeServerStream(pipeName, PipeDirection.InOut, 1, PipeTransmissionMode.Byte, PipeOptions.Asynchronous))
                    {
                        await pipeServer.WaitForConnectionAsync();

                        var buffer = new byte[1024];
                        var bytesRead = await pipeServer.ReadAsync(buffer, 0, buffer.Length);
                        var message = Encoding.UTF8.GetString(buffer, 0, bytesRead);

                        MessageReceived?.Invoke(this, message);
                    }
                }
                catch (Exception)
                {
                    // Handle connection errors
                }
            }
        }

        public async Task SendCommand(string cmd)
        {
            if (pipeServer?.IsConnected != true)
                return;

            var buffer = Encoding.UTF8.GetBytes(cmd);
            await pipeServer.WriteAsync(buffer, 0, buffer.Length);
        }
    }
}
