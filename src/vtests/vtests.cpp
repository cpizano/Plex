// vtests.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "vtests.h"


int __stdcall wWinMain(HINSTANCE instance, HINSTANCE,
                       wchar_t* cmdline, int cmd_show) {

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
