// vtests.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "vtests.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

class SampleWindow : private plx::Window <SampleWindow> {
  friend class plx::Window<SampleWindow>;

public:
  SampleWindow() {
    create_window(WS_EX_NOREDIRECTIONBITMAP,
                  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                  L"Window Title",
                  nullptr, nullptr,
                  CW_USEDEFAULT, CW_USEDEFAULT,
                  CW_USEDEFAULT, CW_USEDEFAULT,
                  nullptr,
                  nullptr);
  }

  LRESULT message_handler(const UINT message, WPARAM wparam, LPARAM lparam) {
    if (WM_PAINT == message)
      return PaintHandler();

    if (WM_DESTROY == message) {
      ::PostQuitMessage(0);
      return 0;
    }

    return ::DefWindowProc(window_, message, wparam, lparam);
  }

  LRESULT PaintHandler() {
    // Render ...
    return 0;
  }

  LRESULT hwnd_destroyed() {
    return 0;
  }
};


int __stdcall wWinMain(HINSTANCE instance, HINSTANCE,
                       wchar_t* cmdline, int cmd_show) {

  SampleWindow sample_window;
  plx::ComPtr<IUnknown> iuk;

  HACCEL accel_table = ::LoadAccelerators(instance, MAKEINTRESOURCE(IDC_VTESTS));
  MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		if (!::TranslateAccelerator(msg.hwnd, accel_table, &msg)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}
