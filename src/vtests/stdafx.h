// This is the plex precompiled header, not the same as the VC precompiled header.

#pragma once
#define NOMINMAX

#include <SDKDDKVer.h>





#include <objbase.h>
#include <shellscalingapi.h>
#include <d2d1_2.h>
#include <d3d11_2.h>
#include <dcomp.h>
#include <wincodec.h>
#include <wrl.h>
#include <vector>
#include <string>
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
// plx::CreateDCoSurface : Makes a DirectComposition compatible surface.
//

plx::ComPtr<IDCompositionSurface> CreateDCoSurface(
    plx::ComPtr<IDCompositionDesktopDevice> device, unsigned int w, unsigned int h) ;


///////////////////////////////////////////////////////////////////////////////
// plx::CreateDCoVisual : Makes a DirectComposition Visual.
//

plx::ComPtr<IDCompositionVisual2> CreateDCoVisual(plx::ComPtr<IDCompositionDesktopDevice> device) ;


///////////////////////////////////////////////////////////////////////////////
// plx::CreateDCoWindowTarget : Makes a DirectComposition target for a window.
//

plx::ComPtr<IDCompositionTarget> CreateDCoWindowTarget(
    plx::ComPtr<IDCompositionDesktopDevice> device, HWND window) ;



///////////////////////////////////////////////////////////////////////////////
// plx::CreateD2D1FactoryST : Direct2D fatory.
// plx::CreateDeviceD2D1 : Direct2D device.
//

plx::ComPtr<ID2D1Factory2> CreateD2D1FactoryST(D2D1_DEBUG_LEVEL debug_level) ;

plx::ComPtr<ID2D1Device> CreateDeviceD2D1(plx::ComPtr<ID3D11Device> device3D,
                                          plx::ComPtr<ID2D1Factory2> factoryD2D1) ;


///////////////////////////////////////////////////////////////////////////////
// plx::CreateDeviceD3D11 : Direct3D device fatory.
//

plx::ComPtr<ID3D11Device> CreateDeviceD3D11(int extra_flags) ;


///////////////////////////////////////////////////////////////////////////////
// plx::CreateWICBitmapBGRA : Makes a BGRA 32-bit WIC bitmap.
//

plx::ComPtr<IWICFormatConverter> CreateWICBitmapBGRA(unsigned int frame_index,
                                                     WICBitmapDitherType dither,
                                                     plx::ComPtr<IWICBitmapDecoder> decoder,
                                                     plx::ComPtr<IWICImagingFactory> factory) ;


///////////////////////////////////////////////////////////////////////////////
// plx::IOException
// error_code_ : The win32 error code of the last operation.
// name_ : The file or pipe in question.
//
class IOException : public plx::Exception {
  DWORD error_code_;
  const std::wstring name_;

public:
  IOException(int line, const wchar_t* name)
      : Exception(line, "IO problem"),
        error_code_(::GetLastError()),
        name_(name) {
    PostCtor();
  }
  DWORD ErrorCode() const { return error_code_; }
  const wchar_t* Name() const { return name_.c_str(); }
};


///////////////////////////////////////////////////////////////////////////////
// plx::CreateWICFactory : Windows Imaging components factory.
//

plx::ComPtr<IWICImagingFactory> CreateWICFactory() ;


///////////////////////////////////////////////////////////////////////////////
// plx::CreateDCoDeviceCtx : Makes a DirectComposition DC for drawing.
//

plx::ComPtr<ID2D1DeviceContext> CreateDCoDeviceCtx(
    plx::ComPtr<IDCompositionSurface> surface,
    const plx::DPI& dpi, const D2D1_SIZE_F& extra_offset) ;


///////////////////////////////////////////////////////////////////////////////
// plx::CreateDCoDevice2 : DirectComposition Desktop Device.
//

plx::ComPtr<IDCompositionDesktopDevice> CreateDCoDevice2(
    plx::ComPtr<ID2D1Device> device2D) ;


///////////////////////////////////////////////////////////////////////////////
// plx::FilePath
// path_ : The actual path (ucs16 probably).
//
class FilePath {
private:
  std::wstring path_;
  friend class File;

public:
  explicit FilePath(const wchar_t* path)
    : path_(path) {
  }

  explicit FilePath(const std::wstring& path)
    : path_(path) {
  }

  FilePath parent() const {
    auto pos = path_.find_last_of(L'\\');
    if (pos == std::string::npos)
      return FilePath();
    return FilePath(path_.substr(0, pos));
  }

  std::wstring leaf() const {
    auto pos = path_.find_last_of(L'\\');
    if (pos == std::string::npos) {
      return is_drive() ? std::wstring() : path_;
    }
    return path_.substr(pos + 1);
  }

  FilePath append(const std::wstring& name) const {
    if (name.empty())
      throw plx::IOException(__LINE__, path_.c_str());

    std::wstring full(path_);
    if (!path_.empty())
      full.append(1, L'\\');
    full.append(name);
    return FilePath(full);
  }

  bool is_drive() const {
    return (path_.size() != 2) ? false : drive_impl();
  }

  bool has_drive() const {
    return (path_.size() < 2) ? false : drive_impl();
  }

  const wchar_t* raw() const {
    return path_.c_str();
  }

private:
  FilePath() {}

  bool drive_impl() const {
    if (path_[1] != L':')
      return false;
    auto dl = path_[0];
    if ((dl >= L'A') && (dl <= L'Z'))
      return true;
    if ((dl >= L'a') && (dl <= L'z'))
      return true;
    return false;
  }
};


///////////////////////////////////////////////////////////////////////////////
// plx::CreateWICDecoder : Makes a Windows Imaging Components decoder.
//

plx::ComPtr<IWICBitmapDecoder> CreateWICDecoder(
    plx::ComPtr<IWICImagingFactory> factory, const plx::FilePath& fname) ;


///////////////////////////////////////////////////////////////////////////////
// plx::SizeL : windows compatible SIZE wrapper.
//

struct SizeL : public ::SIZE {
  SizeL() : SIZE() {}
  SizeL(long width, long height) : SIZE({width, height}) {}
  bool empty() const { return (cx == 0L) && (cy == 0L); }
};


///////////////////////////////////////////////////////////////////////////////
// plx::RectL : windows compatible RECT wrapper.
//

struct RectL : public ::RECT {
  RectL() : RECT() {}
  RectL(long x0, long y0, long x1, long y1) : RECT({x0, y0, x1, y1}) {}
  RectL(const plx::SizeL& size): RECT({ 0L, 0L, size.cx, size.cy }) {}

  plx::SizeL size() { return SizeL(right - left, bottom - top); }
  long width() const { return right - left; }
  long height() const { return bottom - top; }
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
//
//

float WidthRectF(const D2D_RECT_F& r) ;

float HeightRectF(const D2D_RECT_F& r) ;


///////////////////////////////////////////////////////////////////////////////
// plx::Window
//

template <typename Derived>
class Window {
  HWND window_;
  plx::DPI dpi_;

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

public:
  HWND window() { return window_; }
  const plx::DPI& dpi() const { return dpi_; }

protected:
  Window() : window_(nullptr) {}

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
          auto cs = reinterpret_cast<CREATESTRUCT*>(lparam);
          if ((cs->cx != CW_USEDEFAULT) && (cs->cy != CW_USEDEFAULT)) {
            RECT r = {
              0, 0,
              static_cast<long>(obj->dpi_.to_physical_x(cs->cx)),
              static_cast<long>(obj->dpi_.to_physical_x(cs->cy))
            };
            ::AdjustWindowRectEx(&r, cs->style, (cs->hMenu != NULL), cs->dwExStyle);
            ::SetWindowPos(window, nullptr, 0, 0,
                           r.right - r.left, r.bottom - r.top,
                           SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
          }

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
};

}

extern "C" IMAGE_DOS_HEADER __ImageBase;
