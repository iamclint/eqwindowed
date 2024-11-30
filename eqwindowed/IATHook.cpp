#include "pch.h"
#include "IATHook.h"
#include <iostream>
#include "Console.h"
namespace EqWindowed
{

    IATHook::IATHook(HMODULE hModule, const std::string& dllName, const std::string& functionName, LPVOID newFunction, bool debug)
    {
        new_function = newFunction;
        orig_function = ReplaceIATFunction(hModule, dllName, functionName, newFunction, debug);
    }
    LPVOID IATHook::ReplaceIATFunction(HMODULE hModule, const std::string& dllName, const std::string& functionName, LPVOID newFunction, bool debug) {
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)hModule + dosHeader->e_lfanew);
        PIMAGE_IMPORT_DESCRIPTOR importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD_PTR)hModule + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

        if (debug)
        {
            Console::CreateConsole();
            char moduleName[MAX_PATH];
            DWORD result = GetModuleFileNameA(hModule, moduleName, MAX_PATH);
            std::cout << "Attempting to hook: " << functionName << " in " << dllName << " by replacing the pointer in the import address table for module: " << moduleName << std::endl;
        }
        while (importDescriptor->Name != 0) {
            char* moduleName = (char*)((DWORD_PTR)hModule + importDescriptor->Name);
            if (debug)
                std::cout << "Current dll: " << moduleName << std::endl;
            if (_stricmp(moduleName, dllName.c_str()) == 0) {
                if (debug)
                    std::cout << "DLL found: " << moduleName << std::endl;
                PIMAGE_THUNK_DATA thunkIAT = (PIMAGE_THUNK_DATA)((DWORD_PTR)hModule + importDescriptor->FirstThunk);
                PIMAGE_THUNK_DATA thunkINT = (PIMAGE_THUNK_DATA)((DWORD_PTR)hModule + importDescriptor->OriginalFirstThunk);
                while (thunkINT->u1.AddressOfData != 0) {
                    if (debug)
                        std::cout << "Current function: " << moduleName << std::endl;
                    if (!(thunkINT->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
                        PIMAGE_IMPORT_BY_NAME importByName = (PIMAGE_IMPORT_BY_NAME)((DWORD_PTR)hModule + thunkINT->u1.AddressOfData);
                        const char* funcName = (const char*)importByName->Name;
                        if (_stricmp(funcName, functionName.c_str()) == 0) {
                            DWORD oldProtect;
                            VirtualProtect(&thunkIAT->u1.Function, sizeof(LPVOID), PAGE_READWRITE, &oldProtect);
                            LPVOID originalFunction = (LPVOID)thunkIAT->u1.Function;
                            thunkIAT->u1.Function = (ULONGLONG)newFunction;
                            VirtualProtect(&thunkIAT->u1.Function, sizeof(LPVOID), oldProtect, &oldProtect);
                            return originalFunction; // Return the original function pointer
                        }
                    }
                    ++thunkIAT;
                    ++thunkINT;
                }
            }
            ++importDescriptor;
        }
        if (debug)
            std::cout << "DLL/Function not found";
        return nullptr; // Function or module not found
    }
}