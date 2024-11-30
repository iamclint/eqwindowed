#pragma once
#include <ddraw.h>
#include <thread>
namespace EqWindowed
{
	class EqWindow
	{
	public:
		EqWindow();
		HWND Handle;
		LPDIRECTDRAWSURFACE Surface = nullptr;
		void MessageLoop();
	private:
		std::thread message_thd;
	};
	
}
