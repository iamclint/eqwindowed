#pragma once
#include <Windows.h>
#include <string>
#define czVOID(c) (void)c
namespace EqWindowed
{
    class IATHook
    {
    public:
        IATHook() = default;
        IATHook(HMODULE hModule, const std::string& dllName, const std::string& functionName, LPVOID newFunction, bool debug = false);
        template<typename T>
        T original(T fnType) {
            czVOID(fnType);
            return (T)orig_function;
        }
        LPVOID new_function;
        LPVOID orig_function;
    private:
        LPVOID ReplaceIATFunction(HMODULE hModule, const std::string& dllName, const std::string& functionName, LPVOID newFunction, bool debug);
    };
}