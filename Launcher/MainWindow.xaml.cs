using System;
using System.Windows;
using System.Windows.Controls;
using System.Diagnostics;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace Launcher
{
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        private ObservableCollection<ProcessInfo> processes;
        private HotkeyManager hotkeyManager;
        private PipeServer pipeServer;
        private Injector injector;
        private bool isPaused;
        private bool updatingFromPause;

        public event PropertyChangedEventHandler? PropertyChanged;

        public MainWindow()
        {
            InitializeComponent();
            processes = new ObservableCollection<ProcessInfo>();
            ProcessListView.ItemsSource = processes;

            hotkeyManager = new HotkeyManager();
            pipeServer = new PipeServer();
            injector = new Injector();
            isPaused = false;
            updatingFromPause = false;

            Loaded += MainWindow_Loaded;
            Closing += MainWindow_Closing;

            InjectButton.Click += InjectButton_Click;
            PauseButton.Click += PauseButton_Click;
            TimeMultiplierSlider.ValueChanged += TimeMultiplierSlider_ValueChanged;

            hotkeyManager.Increase += Hotkey_Increase;
            hotkeyManager.Decrease += Hotkey_Decrease;
            hotkeyManager.TogglePause += Hotkey_TogglePause;
        }

        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            RefreshProcessList();
            hotkeyManager.RegisterHotkeys(this);
            pipeServer.Start("TimePipe");
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
                            Architecture = Environment.Is64BitProcess ? "x64" : "x86"
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

            bool manual = ManualMappingCheckBox.IsChecked == true;
            bool result = injector.InjectDLL(info.Id, dllPath, manual);

            MessageBox.Show(result ? "DLL injected" : "Injection failed");
        }

        private async void PauseButton_Click(object sender, RoutedEventArgs e)
        {
            isPaused = !isPaused;
            if (isPaused)
            {
                PauseButton.Content = "Resume";
                updatingFromPause = true;
                TimeMultiplierSlider.Value = 0.0;
                updatingFromPause = false;
            }
            else
            {
                PauseButton.Content = "Pause";
                updatingFromPause = true;
                TimeMultiplierSlider.Value = 1.0;
                updatingFromPause = false;
            }
            await pipeServer.SendCommand("RESET");
        }

        private async void TimeMultiplierSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (updatingFromPause)
                return;

            await pipeServer.SendCommand($"SET {e.NewValue:F1}");
        }

        private void Hotkey_Increase(object? sender, EventArgs e)
        {
            TimeMultiplierSlider.Value = Math.Min(TimeMultiplierSlider.Value + 0.1, TimeMultiplierSlider.Maximum);
        }

        private void Hotkey_Decrease(object? sender, EventArgs e)
        {
            TimeMultiplierSlider.Value = Math.Max(TimeMultiplierSlider.Value - 0.1, TimeMultiplierSlider.Minimum);
        }

        private void Hotkey_TogglePause(object? sender, EventArgs e)
        {
            PauseButton_Click(sender!, new RoutedEventArgs());
        }
    }

    public class ProcessInfo
    {
        public int Id { get; set; }
        public string Name { get; set; } = string.Empty;
        public string Architecture { get; set; } = string.Empty;
    }
} 