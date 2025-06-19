using System;
using System.Windows;
using System.Windows.Controls;
using System.Diagnostics;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Launcher
{
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        private ObservableCollection<ProcessInfo> processes;
        private HotkeyManager hotkeyManager;
        private PipeServer pipeServer;
        private bool isPaused;

        public event PropertyChangedEventHandler? PropertyChanged;

        public MainWindow()
        {
            InitializeComponent();
            processes = new ObservableCollection<ProcessInfo>();
            ProcessListView.ItemsSource = processes;

            hotkeyManager = new HotkeyManager();
            pipeServer = new PipeServer();
            isPaused = false;

            Loaded += MainWindow_Loaded;
            Closing += MainWindow_Closing;

            InjectButton.Click += InjectButton_Click;
            PauseButton.Click += PauseButton_Click;
        }

        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            RefreshProcessList();
            hotkeyManager.RegisterHotkeys(this);
        }

        private void MainWindow_Closing(object sender, CancelEventArgs e)
        {
            hotkeyManager.UnregisterHotkeys();
        }

        private void RefreshProcessList()
        {
            processes.Clear();
            foreach (var process in Process.GetProcesses())
            {
                try
                {
                    if (process.MainWindowHandle != IntPtr.Zero)
                    {
                        processes.Add(new ProcessInfo
                        {
                            Id = process.Id,
                            Name = process.ProcessName,
                            Architecture = GetProcessArchitecture(process)
                        });
                    }
                }
                catch { }
            }
        }

        protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        private void InjectButton_Click(object sender, RoutedEventArgs e)
        {
            if (ProcessListView.SelectedItem is not ProcessInfo info)
                return;

            string arch = info.Architecture.ToLower();
            string dllName = arch == "x64" ? "HookDLL_x64.dll" : "HookDLL_x86.dll";
            string dllPath = System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory,
                $"Plugins/HookDLL/{arch}", dllName);

            bool result = Injector.InjectDLL(info.Id, dllPath);

            MessageBox.Show(result ? "DLL injected" : "Injection failed");
        }

        private void PauseButton_Click(object sender, RoutedEventArgs e)
        {
            isPaused = !isPaused;
            if (isPaused)
            {
                PauseButton.Content = "Resume";
                TimeMultiplierSlider.Value = 0.0;
            }
            else
            {
                PauseButton.Content = "Pause";
                TimeMultiplierSlider.Value = 1.0;
            }
        }

        private static string GetProcessArchitecture(Process process)
        {
            const ushort IMAGE_FILE_MACHINE_UNKNOWN = 0;
            const ushort IMAGE_FILE_MACHINE_I386 = 0x014c;
            const ushort IMAGE_FILE_MACHINE_AMD64 = 0x8664;
            const ushort IMAGE_FILE_MACHINE_ARM64 = 0xAA64;

            try
            {
                if (IsWow64Process2(process.Handle, out ushort processMachine, out ushort nativeMachine))
                {
                    ushort machine = processMachine == IMAGE_FILE_MACHINE_UNKNOWN ? nativeMachine : processMachine;
                    return machine switch
                    {
                        IMAGE_FILE_MACHINE_AMD64 => "x64",
                        IMAGE_FILE_MACHINE_ARM64 => "arm64",
                        _ => "x86"
                    };
                }
            }
            catch (EntryPointNotFoundException)
            {
                // Fallback to IsWow64Process on older systems
                if (IsWow64Process(process.Handle, out bool isWow64))
                    return isWow64 ? "x86" : "x64";
            }
            catch
            {
            }

            return Environment.Is64BitOperatingSystem ? "x64" : "x86";
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool IsWow64Process(IntPtr hProcess, out bool Wow64Process);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool IsWow64Process2(IntPtr hProcess, out ushort processMachine, out ushort nativeMachine);
    }

    public class ProcessInfo
    {
        public int Id { get; set; }
        public string Name { get; set; } = string.Empty;
        public string Architecture { get; set; } = string.Empty;
    }
} 