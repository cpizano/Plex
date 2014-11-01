///////////////////////////////////////////////////////////////////////////////////////////////////
//  
//  +------------+               +--------------+    
//  | Direct3D   |          QI   |              |  
//  | Device     +---------------> IDXGIDevice3 |  
//  |            |               |              |   
//  +------------+               +---------+----+    
//                                         |    
//  +------------+                         |  
//  | Direct2D   |                         |   
//  | Factory    +-----------------------+ |  
//  |            |                       | | 
//  +------------+                       | | CreateDevice()  
//                                   +---v-v--------+  
//                                   | Direct2D     |  
//                                   |              |  
//                                   +-------+------+ 
//                                           |  DCompositionCreateDevice2() 
//                                           | 
//                                           |                           +---------------+
//  +-------------+                  +-------v------+                  +-+-------------+ | 
//  | Visual      |  CreateVisual()  |  Direct      |  CreateSurface() |  Direct       | | 
//  |             <------------------+  Composition +------------------>  Composition  | | 
//  | (root)      |                  |  Device      |                  |  Surface      + + 
//  +--+------^---+                  |              |                  +--+------+-----+  
//     |      |                      +-------+------+                     |      |  
//     |      |       +--------+             |                            |      |   
//     |      |       | HWND   +-----------+ |                            |      |  
//     |      |       +--------+           | | CreateTargetForHwnd()      |      |  
//     |      |                      +-----v-v------+                     |      | BeginDraw()
//     |      |                      | Direct       |                     |   +--v----------+  
//     |      +----------------------+ Composition  |                     |   |  D2D1       |  
//     |         SetRoot()           | Target       |                     |   |  Device     |  
//     |                             |              |                     |   |  Context    |  
//     |                             +--------------+                     |   +-------------+  
//     |                                                                  |  
//     |                              +------------+                      |  
//     |                            +-+----------+ |                      |  
//     |                            |   Visual   | |                      |  
//     |   AddVisual()              |            <------------------------+  
//     +---------------------------->  (child)   | |     SetContent()   
//                                  |            | +
//                                  +------------+   
//

#include "stdafx.h"
#include "vtests.h"

//$$ fix this in plex itself
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "shcore.lib")

extern "C" IMAGE_DOS_HEADER __ImageBase;

class SampleWindow : private plx::Window <SampleWindow> {
  friend class plx::Window<SampleWindow>;

  bool sizing_loop_;

public:
  SampleWindow() : sizing_loop_(false) {
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
    switch (message) {
      case WM_DESTROY: {
        ::PostQuitMessage(0);
        return 0;
      }
      case WM_SIZE: {
        return SizeHandler(MAKEPOINTS(lparam));
      }
      case WM_PAINT: {
        PaintHandler();
        break;
      }
      case WM_ENTERSIZEMOVE: {
        sizing_loop_ = true;
        break;
      }
      case WM_EXITSIZEMOVE: {
        sizing_loop_ = false;
        break;
      }
    }

    return ::DefWindowProc(window_, message, wparam, lparam);
  }

  void PaintHandler() {
    
  }

  LRESULT hwnd_destroyed() {
    return 0;
  }

  LRESULT SizeHandler(POINTS pts) {
    if (sizing_loop_)
      return 0;

    return 0;
  }
};

plx::ComPtr<IDCompositionTarget> CreateWindowTarget(
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

plx::ComPtr<IDCompositionSurface> CreateSurface(
    plx::ComPtr<IDCompositionDesktopDevice> device, unsigned int w, unsigned int h) {
  plx::ComPtr<IDCompositionSurface> surface;
  auto hr = device->CreateSurface(w, h,
                                  DXGI_FORMAT_B8G8R8A8_UNORM,
                                  DXGI_ALPHA_MODE_PREMULTIPLIED,
                                  surface.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return surface;
}

plx::ComPtr<ID2D1DeviceContext> CreateDeviceCtx(
    plx::ComPtr<IDCompositionSurface> surface, const plx::DPI& dpi) {
  plx::ComPtr<ID2D1DeviceContext> dc;
  POINT offset;
  auto hr = surface->BeginDraw(nullptr,
                               __uuidof(dc),
                               reinterpret_cast<void **>(dc.GetAddressOf()),
                               &offset);
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  dc->SetDpi(float(dpi.get_dpi_x()), float(dpi.get_dpi_y()));
  auto matrix = D2D1::Matrix3x2F::Translation(dpi.to_logical_x(offset.x),
                                              dpi.to_logical_y(offset.y));
  dc->SetTransform(matrix);
  return dc;
}

plx::ComPtr<IWICBitmapDecoder> CreateDecoder(
    plx::ComPtr<IWICImagingFactory> factory, const wchar_t* fname) {
  plx::ComPtr<IWICBitmapDecoder> decoder;
  auto hr = factory->CreateDecoderFromFilename(fname, nullptr,
                                               GENERIC_READ,
                                               WICDecodeMetadataCacheOnDemand,
                                               decoder.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return decoder;
}

plx::ComPtr<IWICFormatConverter> Create32BGRABitmap(unsigned int frame_index,
                                                    plx::ComPtr<IWICBitmapDecoder> decoder,
                                                    plx::ComPtr<IWICImagingFactory> factory) {
  plx::ComPtr<IWICFormatConverter> converter;
  auto hr = factory->CreateFormatConverter(converter.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  plx::ComPtr<IWICBitmapFrameDecode> frame;
  hr = decoder->GetFrame(frame_index, frame.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  hr = converter->Initialize(frame.Get(),
                             GUID_WICPixelFormat32bppPBGRA,
                             WICBitmapDitherTypeNone,
                             nullptr,
                             0.0f,
                             WICBitmapPaletteTypeCustom);
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return converter;
}

plx::ComPtr<ID2D1Bitmap> CreateD2D1Bitmap(
    plx::ComPtr<ID2D1DeviceContext> dc, plx::ComPtr<IWICBitmapSource> src) {
  plx::ComPtr<ID2D1Bitmap> bitmap;
  auto hr = dc->CreateBitmapFromWicBitmap(src.Get(), nullptr, bitmap.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return bitmap;
}

struct Visual {
  D2D_RECT_F rect;
  plx::ComPtr<IDCompositionSurface> ics;
  plx::ComPtr<IDCompositionVisual2> icv;

  Visual(plx::ComPtr<IDCompositionVisual2> icv, const D2D_RECT_F& rect) 
    : icv(icv), rect(rect) {}
};

class VisualManager {
  std::vector<Visual> visuals_;
  plx::ComPtr<IDCompositionDesktopDevice> dc_device_;
  plx::ComPtr<IDCompositionTarget> target_;

public:
  VisualManager(HWND window, plx::ComPtr<IDCompositionDesktopDevice> dc_device) {
    visuals_.reserve(10);
    target_ = CreateWindowTarget(dc_device, window);
    auto root_visual = CreateVisual(dc_device);
    auto hr = target_->SetRoot(root_visual.Get());
    if (hr != S_OK)
      throw plx::ComException(__LINE__, hr);
    visuals_.emplace_back(root_visual, D2D1::RectF());
    dc_device_ = dc_device;
  }

  void add_visual(plx::ComPtr<IDCompositionSurface> ics, const D2D_RECT_F& rect) {
    plx::ComPtr<IDCompositionVisual2> icv = CreateVisual(dc_device_);
    icv->SetContent(ics.Get());
    visuals_[0].icv->AddVisual(icv.Get(), FALSE, nullptr);
    icv->SetOffsetX(rect.left);
    icv->SetOffsetY(rect.top);
    Visual visual(icv, rect);
    visual.ics = ics;
    visuals_.push_back(visual);
  }

  void commit() {
    dc_device_->Commit();
  }

};

int __stdcall wWinMain(HINSTANCE instance, HINSTANCE,
                       wchar_t* cmdline, int cmd_show) {

  ::CoInitializeEx(NULL, COINIT_MULTITHREADED);

  try {
    plx::DPI dpi;
    dpi.set_from_screen(100, 100);

    SampleWindow sample_window;

    // Create device independent resources. FactoryD2D1 and Geometries are such.
    auto wic_factory = plx::CreateWICFactory();
#if defined (_DEBUG)
    auto d2d1_factory = plx::CreateD2D1FactoryST(D2D1_DEBUG_LEVEL_INFORMATION);
#else
    auto d2d1_factory = plx::CreateD2D1FactoryST(D2D1_DEBUG_LEVEL_NONE);
#endif
    const auto circle =  D2D1::Ellipse(D2D1::Point2F(50.0f, 50.0f), 49.0f, 49.0f);
    plx::ComPtr<ID2D1EllipseGeometry> circle_geom;
    auto hr = d2d1_factory->CreateEllipseGeometry(circle, circle_geom.GetAddressOf());
    if (hr != S_OK)
      throw plx::ComException(__LINE__, hr);

    // Device dependent resources.
#if defined (_DEBUG)
    auto device3D = plx::CreateDeviceD3D11(D3D11_CREATE_DEVICE_DEBUG);
#else
    auto device3D = plx::CreateDevice3D(0);
#endif
    auto device2D = plx::CreateDeviceD2D1(device3D, d2d1_factory);
    auto dc_device = plx::CreateDCoDevice2(device2D);

    VisualManager viman(sample_window.window(), dc_device);


    // scale-dependent resources.
    auto surface1 = CreateSurface(dc_device,
        static_cast<unsigned int>(dpi.to_physical_x(100)), 
        static_cast<unsigned int>(dpi.to_physical_y(100)));

    {
      auto dc = CreateDeviceCtx(surface1, dpi);

      plx::ComPtr<ID2D1SolidColorBrush> brush;
      dc->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.5f, 1.0f, 0.4f), brush.GetAddressOf());
      dc->Clear(D2D1::ColorF(0.4f, 0.4f, 0.4f, 0.4f));
      dc->FillGeometry(circle_geom.Get(), brush.Get());
      brush->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f));
      dc->DrawGeometry(circle_geom.Get(), brush.Get());
      surface1->EndDraw();
    }

    unsigned int as_width, as_height;
    auto png1 = CreateDecoder(wic_factory, L"c:\\test\\images\\diamonds_k.png");
    auto png1_cv = Create32BGRABitmap(0, png1, wic_factory);
    png1_cv->GetSize(&as_width, &as_height);

    auto sc_width = dpi.to_physical_x(as_width * 0.5f);
    auto sc_height = dpi.to_physical_y(as_height * 0.5f);
    auto surface2 = CreateSurface(dc_device,
                                  static_cast<unsigned int>(sc_width), 
                                  static_cast<unsigned int>(sc_height));
    {
      auto dc = CreateDeviceCtx(surface2, dpi);
      auto bmp1 = CreateD2D1Bitmap(dc, png1_cv);
      dc->Clear(D2D1::ColorF(0.0f, 0.0f, 0.4f, 0.0f));
      auto dr = D2D1::Rect(0.0f, 0.0f, sc_width, sc_height);
      dc->DrawBitmap(bmp1.Get(), &dr, 1.0f);
      surface2->EndDraw();
    }

    // Add some child visuals.
    for (int ix = 0; ix != 3; ++ix) {
      viman.add_visual(
          ix == 0 ? surface2.Get() : surface1.Get(),
          D2D1::RectF(dpi.to_physical_x(125.0f * ix),
                      dpi.to_physical_y(45.0f * ix),
                      100.0,
                      100.0));
    }
    viman.commit();
    
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
