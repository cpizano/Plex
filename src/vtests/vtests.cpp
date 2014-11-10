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
#define NANOSVG_IMPLEMENTATION 1

#include "stdafx.h"
#include "vtests.h"
#include "nanosvg.h"

//$$ fix this in plex itself
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "shcore.lib")

extern "C" IMAGE_DOS_HEADER __ImageBase;

plx::ComPtr<ID2D1Bitmap> CreateD2D1Bitmap(
    plx::ComPtr<ID2D1DeviceContext> dc, plx::ComPtr<IWICBitmapSource> src) {
  plx::ComPtr<ID2D1Bitmap> bitmap;
  auto hr = dc->CreateBitmapFromWicBitmap(src.Get(), nullptr, bitmap.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return bitmap;
}

class Surface {
  bool drawing_;
  D2D_POINT_2F size;
  plx::ComPtr<IDCompositionSurface> ics_;
  const plx::DPI& dpi_;

  friend class VisualManager;

public:
  Surface(plx::ComPtr<IDCompositionSurface> ics,
          const plx::DPI& dpi,
          float width, float height) 
    : drawing_(false),
      size(D2D1::Point2F(width, height)),
      ics_(ics),
      dpi_(dpi) {
  }

  plx::ComPtr<ID2D1DeviceContext> begin_draw(const D2D1_COLOR_F& clear_color) {
    auto dc = plx::CreateDCoDeviceCtx(ics_, dpi_);
    dc->Clear(clear_color);
    drawing_ = true;
    return dc;
  }

  void end_draw() {
    if (drawing_)
      ics_->EndDraw();
  }

  ~Surface() {
    end_draw();
  }
};

struct Visual {
  D2D_RECT_F rect;
  plx::ComPtr<IDCompositionVisual2> icv;
  plx::ComPtr<IDCompositionSurface> ics;

  Visual(plx::ComPtr<IDCompositionVisual2> icv, const D2D_RECT_F& rect) 
    : icv(icv), rect(rect) {
  }
};


class VisualManager {
  const plx::DPI& dpi_;
  plx::ComPtr<IDCompositionVisual2> root_;
  plx::ComPtr<IDCompositionVisual2> background_;
  std::vector<Visual> child_visuals_;
  plx::ComPtr<IDCompositionDesktopDevice> dc_device_;
  plx::ComPtr<IDCompositionTarget> target_;

public:
  VisualManager(HWND window, const plx::DPI& dpi,  plx::ComPtr<ID2D1Device> device2D)
      : dpi_(dpi) {
    dc_device_ = plx::CreateDCoDevice2(device2D);
    target_ = plx::CreateDCoWindowTarget(dc_device_, window);
    root_ = plx::CreateDCoVisual(dc_device_);
    auto hr = target_->SetRoot(root_.Get());
    if (hr != S_OK)
      throw plx::ComException(__LINE__, hr);

    child_visuals_.reserve(10);
  }

  plx::ComPtr<IDCompositionDesktopDevice> dc_device() { return dc_device_; }

  void add_visual(Surface& surface, const D2D_POINT_2F& offset) {
    plx::ComPtr<IDCompositionVisual2> icv = plx::CreateDCoVisual(dc_device_);
    icv->SetContent(surface.ics_.Get());

    if (background_)
      background_->AddVisual(icv.Get(), TRUE, nullptr);
    else
      root_->AddVisual(icv.Get(), TRUE, nullptr);

    icv->SetOffsetX(dpi_.to_physical_x(offset.x));
    icv->SetOffsetY(dpi_.to_physical_y(offset.y));
    
    auto rect = D2D1::RectF(offset.x, offset.y,
                            offset.x + surface.size.x, offset.y + surface.size.y);
    Visual visual(icv, rect);
    visual.ics = surface.ics_;
    child_visuals_.push_back(visual);
  }

  Surface make_background_surface(HWND window) {
    if (!background_) {
      background_ = plx::CreateDCoVisual(dc_device_);
      root_->AddVisual(background_.Get(), FALSE, nullptr);
    }

    RECT rect = {};
    ::GetClientRect(window, &rect);
    auto width = rect.right - rect.left;
    auto height = rect.bottom - rect.top;
    auto ics = plx::CreateDCoSurface(dc_device_, width, height);
    background_->SetContent(ics.Get());

    return Surface(ics, dpi_, dpi_.to_logical_x(width), dpi_.to_logical_y(height));
  }

  Surface make_surface(float width, float height) {
    auto ics = plx::CreateDCoSurface(
        dc_device_,
        static_cast<unsigned int>(dpi_.to_physical_x(width)),
        static_cast<unsigned int>(dpi_.to_physical_x(height)));
    return Surface(ics, dpi_, width, height);
  }

  Surface surface_from_visual(Visual& visual) {
    return Surface(visual.ics, dpi_,
                   visual.rect.right - visual.rect.left,
                   visual.rect.bottom - visual.rect.top);
  }

  std::vector<Visual> get_visuals(D2D1_POINT_2F point) {
    std::vector<Visual> hits;
    for (auto& v : child_visuals_) {
      if ((v.rect.left < point.x) && (v.rect.right > point.x) &&
          (v.rect.top < point.y) && (v.rect.bottom > point.y))
        hits.push_back(v);
    }
    return hits;
  }

  void commit() {
    dc_device_->Commit();
  }

};

class DCoWindow : public plx::Window <DCoWindow> {
  bool sizing_loop_;
  VisualManager* viman_;

public:
  DCoWindow() : sizing_loop_(false), viman_(nullptr) {
    create_window(WS_EX_NOREDIRECTIONBITMAP,
                  WS_POPUP | WS_VISIBLE,
                  L"Window Title",
                  nullptr, nullptr,
                  15, 15,
                  1000, 1000,
                  nullptr,
                  nullptr);
  }

  void set_visual_manager(VisualManager* viman) { viman_ = viman; }

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
      case WM_MOUSEMOVE: {
        return MouseMoveHandler(MAKEPOINTS(lparam));
      }
      case WM_ENTERSIZEMOVE: {
        sizing_loop_ = true;
        break;
      }
      case WM_EXITSIZEMOVE: {
        sizing_loop_ = false;
        break;
      }
      case WM_DPICHANGED: {
        //$$ fix the dpi member, possibly move the the base class.
        break;
      }
    }

    return ::DefWindowProc(window_, message, wparam, lparam);
  }

  void PaintHandler() {
    // if (deviceCreated() {
    //   device_d3d->GetDeviceRemovedReason() == S_OK
    //   if not ok then release all device dependent resources and
    //   not validate the window region so we get a second
    //   paint. (hmmm, not sure if this is the best idea)
    // } else {
    //   CreateDeviceResources();
    // }
  }

  LRESULT SizeHandler(POINTS pts) {
    if (sizing_loop_)
      return 0;

    return 0;
  }

  LRESULT MouseMoveHandler(POINTS pts) {
#if 0
    auto visuals = viman_->get_visuals(D2D1::Point2F(pts.x, pts.y));
    if (visuals.empty())
      return 0;

    auto surface = viman_->surface_from_visual(visuals[0]);
    {
      auto dc = surface.begin_draw(D2D1::ColorF(0.0f, 0.4f, 0.4f, 0.4f));
      plx::ComPtr<ID2D1SolidColorBrush> brush;
      dc->CreateSolidColorBrush(D2D1::ColorF(0.8f, 0.4f, 0.4f, 0.4f), brush.GetAddressOf());
      dc->FillRectangle(D2D1::RectF(0.0f, 0.0f, 200.0f, 200.0f), brush.Get());
      surface.end_draw();
    }

    viman_->commit();
#endif
    return 0;
  }

};

plx::ComPtr<ID2D1PathGeometry> RealizeSVG(
    const char* file, const plx::DPI& dpi, plx::ComPtr<ID2D1Factory2> dd_factory) {

  auto image = nsvgParseFromFile(file, "px", static_cast<float>(dpi.get_dpi_x()));
  if (!image)
    return nullptr;

  plx::ComPtr<ID2D1PathGeometry> geom;
  auto hr = dd_factory->CreatePathGeometry(geom.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);

  plx::ComPtr<ID2D1GeometrySink> sink;
  hr = geom->Open(sink.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);

  for (auto shape = image->shapes; shape != nullptr; shape = shape->next) {
    for (auto path = shape->paths; path != nullptr; path = path->next) {
      sink->BeginFigure(
          D2D1::Point2F(path->pts[0], path->pts[1]), D2D1_FIGURE_BEGIN_FILLED);
      for (auto i = 0; i < path->npts - 1; i += 3) {
        float* p = &path->pts[i*2];
        sink->AddBezier(
            D2D1::BezierSegment(D2D1::Point2F(p[2], p[3]),
                                D2D1::Point2F(p[4], p[5]),
                                D2D1::Point2F(p[6], p[7])));
      }
      sink->EndFigure(
          (path->closed == 1) ? D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
    }
  }

  sink->Close();
  nsvgDelete(image);
  return geom;
}

int __stdcall wWinMain(HINSTANCE instance, HINSTANCE,
                       wchar_t* cmdline, int cmd_show) {

  ::CoInitializeEx(NULL, COINIT_MULTITHREADED);

  try {
    DCoWindow window;
    
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

    VisualManager viman(window.window(), window.dpi(), device2D);
    window.set_visual_manager(&viman);

    auto background = viman.make_background_surface(window.window());
    {
      auto dc = background.begin_draw(D2D1::ColorF(0.3f, 0.3f, 0.3f, 0.7f));
      background.end_draw();
    }

    // scale-dependent resources.
    auto surface1 = viman.make_surface(100.0f, 100.0f);
    {
      auto dc = surface1.begin_draw(D2D1::ColorF(0.4f, 0.4f, 0.4f, 0.4f));
      plx::ComPtr<ID2D1SolidColorBrush> brush;
      dc->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.5f, 1.0f, 0.4f), brush.GetAddressOf());
      dc->FillGeometry(circle_geom.Get(), brush.Get());
      brush->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f));
      dc->DrawGeometry(circle_geom.Get(), brush.Get());
      surface1.end_draw();
    }

#if 0
    unsigned int as_width, as_height;
    auto png1 = plx::CreateWICDecoder(
        wic_factory, plx::FilePath(L"c:\\test\\images\\diamonds_k.png"));
    auto png1_cv = plx::CreateWICBitmapBGRA(0, WICBitmapDitherTypeNone, png1, wic_factory);
    png1_cv->GetSize(&as_width, &as_height);

    auto sc_width = window.dpi().to_physical_x(as_width * 0.5f);
    auto sc_height = window.dpi().to_physical_y(as_height * 0.5f);
    auto surface2 = viman.make_surface(sc_width, sc_height);
    {
      auto dc = surface2.begin_draw(D2D1::ColorF(0.0f, 0.0f, 0.4f, 0.0f));
      auto bmp1 = CreateD2D1Bitmap(dc, png1_cv);
      auto dr = D2D1::Rect(0.0f, 0.0f, sc_width, sc_height);
      dc->DrawBitmap(bmp1.Get(), &dr, 1.0f);
      surface2.end_draw();
    }

    // add the image at the bottom.
    viman.add_visual(surface2, D2D1::Point2F(0.0f, 0.0f));
#endif

    auto svg = RealizeSVG(
        "C:\\Users\\cpu\\Documents\\GitHub\\nanosvg\\example\\nano.svg",
        //"C:\\Test\\svg\\2_elipse_red_black.svg",
        window.dpi(), d2d1_factory);

    auto surface3 = viman.make_surface(600.0f, 600.0f);
    {
      auto dc = surface3.begin_draw(D2D1::ColorF(0.7f, 0.7f, 0.7f, 0.4f));
      plx::ComPtr<ID2D1SolidColorBrush> brush;
      dc->CreateSolidColorBrush(D2D1::ColorF(0.5f, 0.0f, 1.0f, 0.4f), brush.GetAddressOf());
      dc->FillGeometry(svg.Get(), brush.Get());
      brush->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f));
      dc->DrawGeometry(svg.Get(), brush.Get());
      surface3.end_draw();
    }

    viman.add_visual(surface3, D2D1::Point2F(10.0f, 80.0f));

    // Add some elipses on top.
    for (int ix = 0; ix != 5; ++ix) {
      viman.add_visual(surface1, D2D1::Point2F(125.0f * ix, 45.0f * ix));
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
