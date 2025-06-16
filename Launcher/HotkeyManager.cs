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

        private const int HOTKEY_ID_TOGGLE = 1;
        private const int HOTKEY_ID_RESET = 2;

        private const uint MOD_CONTROL = 0x0002;
        private const uint VK_F1 = 0x70;
        private const uint VK_F2 = 0x71;

        private Window? window;
        private IntPtr windowHandle;

        public void RegisterHotkeys(Window window)
        {
            this.window = window;
            windowHandle = new WindowInteropHelper(window).Handle;

            RegisterHotKey(windowHandle, HOTKEY_ID_TOGGLE, MOD_CONTROL, VK_F1);
            RegisterHotKey(windowHandle, HOTKEY_ID_RESET, MOD_CONTROL, VK_F2);

            ComponentDispatcher.ThreadPreprocessMessage += ComponentDispatcher_ThreadPreprocessMessage;
        }

        public void UnregisterHotkeys()
        {
            if (windowHandle != IntPtr.Zero)
            {
                UnregisterHotKey(windowHandle, HOTKEY_ID_TOGGLE);
                UnregisterHotKey(windowHandle, HOTKEY_ID_RESET);
            }

            ComponentDispatcher.ThreadPreprocessMessage -= ComponentDispatcher_ThreadPreprocessMessage;
        }

        private void ComponentDispatcher_ThreadPreprocessMessage(ref MSG msg, ref bool handled)
        {
            if (msg.message == 0x0312) // WM_HOTKEY
            {
                switch (msg.wParam.ToInt32())
                {
                    case HOTKEY_ID_TOGGLE:
                        // TODO: Implement toggle time scaling
                        handled = true;
                        break;

                    case HOTKEY_ID_RESET:
                        // TODO: Implement reset time multiplier
                        handled = true;
                        break;
                }
            }
        }
    }
} 