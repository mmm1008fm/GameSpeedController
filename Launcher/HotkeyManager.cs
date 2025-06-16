using System;
using System.Windows;
using System.Windows.Interop;
using System.Runtime.InteropServices;

namespace Launcher
{
    public class HotkeyManager
    {
        [DllImport("user32.dll")]
        private static extern bool RegisterHotKey(IntPtr hWnd, int id, uint fsModifiers, uint vk);

        [DllImport("user32.dll")]
        private static extern bool UnregisterHotKey(IntPtr hWnd, int id);

        private const int HOTKEY_ID_INCREASE = 1;
        private const int HOTKEY_ID_DECREASE = 2;
        private const int HOTKEY_ID_TOGGLE_PAUSE = 3;

        private const uint MOD_CONTROL = 0x0002;
        private const uint VK_F9 = 0x78;
        private const uint VK_F10 = 0x79;
        private const uint VK_F11 = 0x7A;

        private Window? window;
        private IntPtr windowHandle;

        public event EventHandler? Increase;
        public event EventHandler? Decrease;
        public event EventHandler? TogglePause;

        public void RegisterHotkeys(Window window)
        {
            this.window = window;
            windowHandle = new WindowInteropHelper(window).Handle;

            RegisterHotKey(windowHandle, HOTKEY_ID_INCREASE, MOD_CONTROL, VK_F9);
            RegisterHotKey(windowHandle, HOTKEY_ID_DECREASE, MOD_CONTROL, VK_F10);
            RegisterHotKey(windowHandle, HOTKEY_ID_TOGGLE_PAUSE, MOD_CONTROL, VK_F11);

            ComponentDispatcher.ThreadPreprocessMessage += ComponentDispatcher_ThreadPreprocessMessage;
        }

        public void UnregisterHotkeys()
        {
            if (windowHandle != IntPtr.Zero)
            {
                UnregisterHotKey(windowHandle, HOTKEY_ID_INCREASE);
                UnregisterHotKey(windowHandle, HOTKEY_ID_DECREASE);
                UnregisterHotKey(windowHandle, HOTKEY_ID_TOGGLE_PAUSE);
            }

            ComponentDispatcher.ThreadPreprocessMessage -= ComponentDispatcher_ThreadPreprocessMessage;
        }

        private void ComponentDispatcher_ThreadPreprocessMessage(ref MSG msg, ref bool handled)
        {
            if (msg.message == 0x0312) // WM_HOTKEY
            {
                switch (msg.wParam.ToInt32())
                {
                    case HOTKEY_ID_INCREASE:
                        Increase?.Invoke(this, EventArgs.Empty);
                        handled = true;
                        break;

                    case HOTKEY_ID_DECREASE:
                        Decrease?.Invoke(this, EventArgs.Empty);
                        handled = true;
                        break;

                    case HOTKEY_ID_TOGGLE_PAUSE:
                        TogglePause?.Invoke(this, EventArgs.Empty);
                        handled = true;
                        break;
                }
            }
        }
    }
}
