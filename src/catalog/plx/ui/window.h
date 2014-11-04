//#~def plx::Window
///////////////////////////////////////////////////////////////////////////////
// plx::Window
//
namespace plx {

template <typename Derived>
class Window {
protected:
  HWND window_ = nullptr;
  plx::DPI dpi_;

  HWND create_window(DWORD ex_style, DWORD style,
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
      throw plx::User32Exception(__LINE__, plx::User32Exception::wclass);

    return ::CreateWindowExW(ex_style,
                             MAKEINTATOM(atom),
                             window_name,
                             style,
                             x, y, width, height,
                             parent,
                             menu,
                             wcex.hInstance,
                             this);
  }

  static Derived* this_from_window(HWND window) {
    return reinterpret_cast<Derived*>(GetWindowLongPtr(window, GWLP_USERDATA));
  }

  static LRESULT __stdcall WndProc(HWND window,
                                   const UINT message,
                                   WPARAM  wparam, LPARAM  lparam) {
    if (WM_NCCREATE == message) {
      auto cs = reinterpret_cast<CREATESTRUCT*>(lparam);
      auto obj = static_cast<Derived*>(cs->lpCreateParams);
      if (!obj)
        throw plx::User32Exception(__LINE__, plx::User32Exception::window);
      if (obj->window_)
        throw plx::User32Exception(__LINE__, plx::User32Exception::window);
      obj->window_ = window;
      ::SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(obj));
    } else {
      auto obj = this_from_window(window);
      if (obj) {
        if (message == WM_CREATE) {
          auto monitor = ::MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
          obj->dpi_.set_from_monitor(monitor);
        } else if (message == WM_NCDESTROY) {
          ::SetWindowLongPtrW(window, GWLP_USERDATA, 0L);
          obj->window_ = nullptr;
        } else if (message == WM_DPICHANGED) {
          obj->dpi_.set_dpi(LOWORD(wparam), HIWORD(wparam));
        }
        return obj->message_handler(message, wparam, lparam);
      }
    }

    return ::DefWindowProc(window, message, wparam, lparam);
  }

public:
  HWND window() {
    return window_;
  }

  const plx::DPI& dpi() const {
    return dpi_;
  }
};

}
