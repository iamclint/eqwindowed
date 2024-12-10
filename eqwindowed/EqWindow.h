#pragma once
#include <ddraw.h>
#include <thread>
namespace EqWindowed
{
	class EqWindow
	{
	public:
		static constexpr DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

		EqWindow();
		HWND Handle;
		LPDIRECTDRAWSURFACE Surface = nullptr;
		void MessageLoop();
		int X, Y = 0;
		int Width, Height = 0;
		void AdjustClientSize(int clientWidth, int clientHeight);
		void UpdateClientRegionPosition(HWND hwnd);
		void MaintainAspectRatio();
		bool isFocused = true;
		void* eqMainWndProc = nullptr;
	private:
		std::thread message_thd;
	};
	
}
