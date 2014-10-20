// vtests.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "vtests.h"

//$$ fix this in plex itself
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dcomp.lib")

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

  HWND window() {
    return window_;
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

plx::ComPtr<ID2D1Factory2> CreateD2D1Factory2() {
  D2D1_FACTORY_OPTIONS options = {};
  #ifdef _DEBUG
  options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
  #endif

  plx::ComPtr<ID2D1Factory2> factory;
  auto hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                              options,
                              factory.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return factory;
}


plx::ComPtr<ID3D11Device> CreateDevice3D() {
  auto flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT |
               D3D11_CREATE_DEVICE_SINGLETHREADED;

  #ifdef _DEBUG
  flags |= D3D11_CREATE_DEVICE_DEBUG;
  #endif

  plx::ComPtr<ID3D11Device> device;
  auto hr = D3D11CreateDevice(nullptr,
                              D3D_DRIVER_TYPE_HARDWARE,
                              nullptr,
                              flags,
                              nullptr, 0,
                              D3D11_SDK_VERSION,
                              device.GetAddressOf(),
                              nullptr,
                              nullptr);
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return device;
}

plx::ComPtr<ID2D1Device> CreateDevice2D(plx::ComPtr<ID3D11Device> device3D,
                                        plx::ComPtr<ID2D1Factory2> factoryD2D1) {
  plx::ComPtr<IDXGIDevice3> deviceX;
  device3D.As(&deviceX);
  plx::ComPtr<ID2D1Device> device2D;

  auto hr = factoryD2D1->CreateDevice(deviceX.Get(),
                                      device2D.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return device2D;
}

plx::ComPtr<IDCompositionDesktopDevice> CreateDeskCompDevice2(
    plx::ComPtr<ID2D1Device> device2D) {
  plx::ComPtr<IDCompositionDesktopDevice> device;
  auto hr = DCompositionCreateDevice2(device2D.Get(),
                                      __uuidof(device),
                                      reinterpret_cast<void **>(device2D.GetAddressOf()));
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return device;
}

plx::ComPtr<IDCompositionTarget> CreateHWDTarget(
    plx::ComPtr<IDCompositionDesktopDevice> device, HWND window) {
  plx::ComPtr<IDCompositionTarget> target;
  auto hr = device->CreateTargetForHwnd(window, TRUE, target.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return target;
}

plx::ComPtr<IDCompositionVisual2> CreateVisual(plx::ComPtr<IDCompositionDesktopDevice> device) {
  plx::ComPtr<IDCompositionVisual2> visual;
  auto hr = device->CreateVisual(visual.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return visual;
}

int __stdcall wWinMain(HINSTANCE instance, HINSTANCE,
                       wchar_t* cmdline, int cmd_show) {
  try {

    SampleWindow sample_window;
    // Create device independent resources. FactoryD2D1 and Geometries are such.
    auto d2d1_factory = CreateD2D1Factory2();
    const auto circle =  D2D1::Ellipse(D2D1::Point2F(50.0f, 50.0f), 49.0f, 49.0f);
    plx::ComPtr<ID2D1EllipseGeometry> circle_geom;
    auto hr = d2d1_factory->CreateEllipseGeometry(circle, circle_geom.GetAddressOf());
    if (hr != S_OK)
      throw plx::ComException(__LINE__, hr);

    // Device dependent resources.
    auto device3D = CreateDevice3D();
    auto device2D = CreateDevice2D(device3D, d2d1_factory);
    auto dc_device = CreateDeskCompDevice2(device2D);
    auto target = CreateHWDTarget(dc_device, sample_window.window());
    auto root_visual = CreateVisual(dc_device);
    hr = target->SetRoot(root_visual.Get());

    //plx::ComPtr<IDCompositionSurface> surface;

    HACCEL accel_table = ::LoadAccelerators(instance, MAKEINTRESOURCE(IDC_VTESTS));
    MSG msg;
	  while (::GetMessage(&msg, NULL, 0, 0)) {
		  if (!::TranslateAccelerator(msg.hwnd, accel_table, &msg)) {
			  ::TranslateMessage(&msg);
			  ::DispatchMessage(&msg);
		  }
	  }

	  return (int) msg.wParam;

  } catch (plx::Exception& ex) {
    ex;
    __debugbreak();
    return -1;
  }
}
