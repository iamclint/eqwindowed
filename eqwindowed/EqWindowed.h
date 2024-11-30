#pragma once
#include "EqGame.h"
#include "EqMain.h"
#include "EqGFX.h"
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
}


