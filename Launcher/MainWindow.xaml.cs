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

        public event PropertyChangedEventHandler? PropertyChanged;

        public MainWindow()
        {
            InitializeComponent();
            processes = new ObservableCollection<ProcessInfo>();
            ProcessListView.ItemsSource = processes;

            hotkeyManager = new HotkeyManager();
            pipeServer = new PipeServer();
            injector = new Injector();

            Loaded += MainWindow_Loaded;
            Closing += MainWindow_Closing;
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
    }

    public class ProcessInfo
    {
        public int Id { get; set; }
        public string Name { get; set; } = string.Empty;
        public string Architecture { get; set; } = string.Empty;
    }
} 