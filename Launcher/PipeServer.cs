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
        private const string PipeName = "TimeScalerPipe";

        public event EventHandler<string>? MessageReceived;

        public void Start()
        {
            if (isRunning)
                return;

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
                    using (pipeServer = new NamedPipeServerStream(PipeName, PipeDirection.InOut, 1, PipeTransmissionMode.Byte, PipeOptions.Asynchronous))
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

        public async Task SendMessage(string message)
        {
            if (pipeServer?.IsConnected != true)
                return;

            var buffer = Encoding.UTF8.GetBytes(message);
            await pipeServer.WriteAsync(buffer, 0, buffer.Length);
        }
    }
} 