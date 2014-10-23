// This is the plex precompiled header, not the same as the VC precompiled header.

#pragma once
#define NOMINMAX

#include <SDKDDKVer.h>





#include <shellscalingapi.h>
#include <d2d1_2.h>
#include <d3d11_2.h>
#include <dcomp.h>
#include <wrl.h>
#include <windows.h>








///////////////////////////////////////////////////////////////////////////////
// plx::Exception
// line_ : The line of code, usually __LINE__.
// message_ : Whatever useful text.
//
namespace plx {
class Exception {
  int line_;
  const char* message_;

protected:
  void PostCtor() {
    if (::IsDebuggerPresent()) {
      //__debugbreak();
    }
  }

public:
  Exception(int line, const char* message) : line_(line), message_(message) {}
  virtual ~Exception() {}
  const char* Message() const { return message_; }
  int Line() const { return line_; }
};


///////////////////////////////////////////////////////////////////////////////
// plx::ComPtr : smart COM pointer.
//
template <typename T> using ComPtr = Microsoft::WRL::ComPtr <T>;


///////////////////////////////////////////////////////////////////////////////
// plx::ComException (thrown when COM fails)
//
class ComException : public plx::Exception {
  HRESULT hr_;

public:
  ComException(int line, HRESULT hr)
      : Exception(line, "COM failure"), hr_(hr) {
    PostCtor();
  }

  HRESULT hresult() const {
    return hr_;
  }

};


///////////////////////////////////////////////////////////////////////////////
// plx::DPI
//
//  96 DPI = 1.00 scaling
// 120 DPI = 1.25 scaling
// 144 DPI = 1.50 scaling
// 192 DPI = 2.00 scaling
class DPI {
  float scale_x_;
  float scale_y_;
  unsigned int dpi_x_;
  unsigned int dpi_y_;

public:
  DPI() : scale_x_(1.0f), scale_y_(1.0f), dpi_x_(96), dpi_y_(96) {
  }

  void set_from_monitor(HMONITOR monitor) {
    unsigned int dpi_x, dpi_y;
    auto hr = ::GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y);
    if (hr != S_OK)
      throw plx::ComException(__LINE__, hr);
    set_dpi(dpi_x, dpi_y);
  }

  void set_from_screen(int x, int y) {
    POINT point = {x, y};
    auto monitor = ::MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
    set_from_monitor(monitor);
  }

  void set_dpi(unsigned int dpi_x, unsigned int dpi_y) {
    dpi_x_ = dpi_x;
    dpi_y_ = dpi_y;
    scale_x_ = dpi_x_ / 96.0f;
    scale_y_ = dpi_y_ / 96.0f;
  }

  bool isomorphic_scale() const { return (scale_x_ == scale_y_); }

  float get_scale_x() const { return scale_x_; }

  float get_scale_y() const { return scale_y_; }

  unsigned int get_dpi_x() const { return dpi_x_; }

  unsigned int get_dpi_y() const { return dpi_y_; }

  template <typename T>
  float to_logical_x(const T physical_pix) const {
    return physical_pix / scale_x_;
  }

  template <typename T>
  float to_logical_y(const T physical_pix) const {
    return physical_pix / scale_y_;
  }

  template <typename T>
  float to_physical_x(const T logical_pix) const {
    return logical_pix * scale_x_;
  }

  template <typename T>
  float to_physical_y(const T logical_pix) const {
    return logical_pix * scale_y_;
  }

};


///////////////////////////////////////////////////////////////////////////////
// plx::User32Exception (thrown by user32 functions)
//
class User32Exception : public plx::Exception {
public:
  enum Kind {
    wclass,
    window,
    menu,
    device,
    cursor,
    icon,
    accelerator
  };

  User32Exception(int line, Kind type)
      : Exception(line, "user 32"), type_(type) {
    PostCtor();
  }

  Kind type() const {
    return type_;
  }

private:
  Kind type_;
};


///////////////////////////////////////////////////////////////////////////////
// plx::Window
//

template <typename Derived>
class Window {
protected:
  HWND window_ = nullptr;

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
        if (WM_NCDESTROY == message) {
          ::SetWindowLongPtrW(window, GWLP_USERDATA, 0L);
          obj->window_ = nullptr;
          return obj->hwnd_destroyed();
        }
        return obj->message_handler(message, wparam, lparam);
      }
    }

    return ::DefWindowProc(window, message, wparam, lparam);
  }

};

}
