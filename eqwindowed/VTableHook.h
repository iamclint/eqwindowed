#pragma once
#include <Windows.h>
#include <string>
#include <unordered_map>
#define czVOID(c) (void)c
namespace EqWindowed
{
    extern std::unordered_map<void*, void*> original_map;
	class VTableHook
	{
    public:
        VTableHook() = default;
        VTableHook(void** objVTable, size_t index, LPVOID newFunction, bool debug = false);
        template<typename T>
        T original(T fnType) {
            czVOID(fnType);
            return (T)orig_function;
        }
        LPVOID new_function;
        LPVOID orig_function;
    private:
        LPVOID ReplaceVTableFunction(void** objectVTable, size_t index, LPVOID newFunction);
	};

}