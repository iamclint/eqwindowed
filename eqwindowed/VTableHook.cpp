#include "pch.h"
#include "VTableHook.h"
#include "Console.h"
namespace EqWindowed
{
	std::unordered_map<void*, void*> original_map;
	VTableHook::VTableHook(void** objVTable, size_t index, LPVOID newFunction, bool debug)
	{
		if (objVTable[index] != newFunction)
		{
			if (debug)
			{
				Console::CreateConsole();
				std::cout << "Create vtable hook on: 0x" << std::hex << objVTable << " index: " << index << " -> 0x" << objVTable[index] << " to new function 0x" << newFunction << std::endl;
			}
			original_map[newFunction] = objVTable[index];
			orig_function = ReplaceVTableFunction(objVTable, index, newFunction);
		}
		else
		{
			if (debug)
				std::cout << "Vtable already pointing at your function!" << std::endl;
			Console::CreateConsole();
			if (original_map.count(newFunction))
				orig_function = original_map[newFunction];
		}
	}
	void* VTableHook::ReplaceVTableFunction(void** objectVTable, size_t index, LPVOID newFunction) {
		if (!objectVTable) return nullptr;
		// Store the original function pointer
		void* originalFunction = objectVTable[index];

		// Replace the function pointer in the vtable
		DWORD oldProtect;
		VirtualProtect(&objectVTable[index], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
		objectVTable[index] = newFunction;
		VirtualProtect(&objectVTable[index], sizeof(void*), oldProtect, &oldProtect);
		return originalFunction;
	}
}