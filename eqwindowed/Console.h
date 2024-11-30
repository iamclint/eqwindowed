#pragma once
#include <iostream>
#include <string>
#include <Windows.h>
namespace Console
{
    static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
    {
        int* monitorIndex = reinterpret_cast<int*>(dwData);

        // Let's print the monitor's coordinates (optional)
        std::cout << "Monitor " << *monitorIndex << ": ";
        std::cout << "Left: " << lprcMonitor->left << ", Top: " << lprcMonitor->top << ", ";
        std::cout << "Right: " << lprcMonitor->right << ", Bottom: " << lprcMonitor->bottom << std::endl;

        if (*monitorIndex == 1) {  // Check if we are at the second monitor (index 1)
            // Get the current console window handle
            HWND hwnd = GetConsoleWindow();

            // Move the console window to the second monitor (adjusting based on the monitor's position)
            MoveWindow(hwnd, lprcMonitor->left, lprcMonitor->top, 800, 600, TRUE);  // Example size 800x600
        }

        (*monitorIndex)++;  // Increment the monitor index
        return TRUE;  // Continue enumerating monitors
    }

    static void MoveConsoleToSecondMonitor()
    {
        int monitorIndex = 0;  // Start with the first monitor
        EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitorIndex);
    }

    static inline void CreateConsole(const std::string& title="") {
        // Allocate a new console for the calling process
        if (!AttachConsole(ATTACH_PARENT_PROCESS))
            if (!GetConsoleWindow())
            {
                AllocConsole();
                FILE* file;
                freopen_s(&file, "CONOUT$", "w", stdout);

                // Redirect standard input (stdin)
                freopen_s(&file, "CONIN$", "r", stdin);

                // Redirect standard error (stderr)
                freopen_s(&file, "CONOUT$", "w", stderr);
                MoveConsoleToSecondMonitor();
            }


        if (title.length())
            SetConsoleTitleA(title.c_str());
    }
}

