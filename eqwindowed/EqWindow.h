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
		int X, Y = 0;
		int Width, Height = 0;
		void AdjustClientSize(int clientWidth, int clientHeight);
		void UpdateClientSize(HWND hwnd);
		void MaintainAspectRatio();
		bool isFocused = false;
		void* eqMainWndProc = nullptr;
	private:
		std::thread message_thd;
	};
	
}
