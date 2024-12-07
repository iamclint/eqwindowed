#pragma once
#include "EqGame.h"
#include "EqMain.h"
#include "EqGFX.h"
#include "EqDInput.h"
#include "EqWindow.h"
#include <fstream>
#include <iostream>
extern "C" __declspec(dllexport) void __stdcall InitFromEqClient();
namespace EqWindowed
{
    extern EqGame* EqGameHooks;
    extern EqMain* EqMainHooks;
    extern EqGFX* EqGFXHooks;
    extern EqWindow* Wnd;
    extern EqDInput* DInput;
    HWND WINAPI hCreateWindowEx(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
    HRESULT WINAPI hSetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
    HWND WINAPI hSetCapture(HWND hWnd);
    LONG WINAPI hSetWindowLongA(HWND wnd, int index, long dwNewLong);
    BOOL WINAPI hDestroyWindow(HWND wnd);
}


