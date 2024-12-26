#pragma once
#include <ddraw.h>
#include <thread>
#include <unordered_map>
namespace EqWindowed
{
	enum WindowStyle
	{
		Windowed,
		//Maximized,
		MaximizedBorderless
	};


	class EqWindow
	{
	public:
		DWORD dwStyle = WindowStyle::Windowed;
		EqWindow();
		HWND Handle;
		LPDIRECTDRAWSURFACE Surface = nullptr;
		void MessageLoop();
		int X, Y = 0;
		int Width, Height = 0;
		void AdjustClientSize(int clientWidth, int clientHeight);
		//void UpdateClientRegionPosition(HWND hwnd);
		void SetClientSize(int clientWidth, int clientHeight);
		void MaintainAspectRatio();
		void UpdateClientRegion();
		bool isFocused = true;
		void* eqMainWndProc = nullptr;
		RECT storedDimensions;
	private:
		std::thread message_thd;
	};
	
}
