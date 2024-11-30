#include "pch.h"
#include "VTableHook.h"
#include "Console.h"
namespace EqWindowed
{
	VTableHook::VTableHook(void** objVTable, size_t index, LPVOID newFunction, bool debug)
	{
		if (debug)
		{
			Console::CreateConsole();
			std::cout << "Create vatble hook on: 0x" << std::hex << objVTable << " index: " << index << " -> 0x" << objVTable[index] << " to new function 0x" << newFunction << std::endl;
		}
		orig_function = ReplaceVTableFunction(objVTable, index, newFunction);
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