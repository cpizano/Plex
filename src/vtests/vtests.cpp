// vtests.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "vtests.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

template <typename Derived>
class Window {
protected:

  HWND window_ = nullptr;

  HWND Create_Window(DWORD ex_style, DWORD style,
                     LPCWSTR window_name,
                     HICON small_icon, HICON icon,
                     int x, int y, int width, int height,
                     HWND parent,
                     HMENU menu) {

    WNDCLASSEX wcex = { sizeof(wcex) };
    wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hInstance = reinterpret_cast<HINSTANCE>(&__ImageBase);
    wcex.lpszClassName = L"PlexWindowClass";
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hIcon = icon;
    wcex.hIconSm = small_icon;

    auto atom = ::RegisterClassEx(&wcex);
    if (!atom)
      throw 68;

    return ::CreateWindowExW(ex_style,
                             MAKEINTATOM(atom),
                             window_name,
                             style,
                             x, y, width, height,
                             parent,
                             menu,
                             wc.hInstance,
                             this);
  }

  static Derived * this_from_window(HWND window) {
    return reinterpret_cast<Derived*>(GetWindowLongPtr(window, GWLP_USERDATA));
  }

  static LRESULT __stdcall WndProc(HWND window,
                                   const UINT message,
                                   WPARAM  wparam,
                                   LPARAM  lparam) {

    if (WM_NCCREATE == message) {
      CREATESTRUCT * cs = reinterpret_cast<CREATESTRUCT*>(lparam);
      auto obj = static_cast<Derived*>(cs->lpCreateParams);
      if (!obj)
        throw 66;
      if (obj->window_)
        throw 67;

      obj->window_ = window;
      ::SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(obj));
    } else {
      auto obj = this_from_window(window);
      if (obj)
        return obj->message_handler(message, wparam, lparam);
    }

    return ::DefWindowProc(window, message, wparam, lparam);
  }

};

class SampleWindow : private Window<SampleWindow> {
  friend class Window<SampleWindow>;

public:
  SampleWindow() {
    WNDCLASS wc = {};
    wc.hCursor       = ::LoadCursor(nullptr, IDC_ARROW);
    wc.hInstance     = reinterpret_cast<HINSTANCE>(&__ImageBase);
    wc.lpszClassName = L"SampleWindow";
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    ::RegisterClass(&wc);
    ::CreateWindowEx(WS_EX_NOREDIRECTIONBITMAP,
                     wc.lpszClassName,
                     L"Window Title",
                     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                     CW_USEDEFAULT, CW_USEDEFAULT,
                     CW_USEDEFAULT, CW_USEDEFAULT,
                     nullptr,
                     nullptr,
                     wc.hInstance,
                     this);
    //ASSERT(m_window);
  }

  LRESULT message_handler(const UINT message, WPARAM  wparam, LPARAM  lparam) {
    if (WM_PAINT == message)
      return PaintHandler();
    
    return ::DefWindowProc(window_, message, wparam, lparam);
  }

  LRESULT PaintHandler() {
    // Render ...
    return 0;
  }
};


int __stdcall wWinMain(HINSTANCE instance, HINSTANCE,
                       wchar_t* cmdline, int cmd_show) {

  SampleWindow sample_window;

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
