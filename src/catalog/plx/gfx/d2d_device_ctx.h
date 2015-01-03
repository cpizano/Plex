//#~def plx::ScopedD2D1DeviceContext
///////////////////////////////////////////////////////////////////////////////
// plx::ScopedD2D1DeviceContext : Direct2D DC for drawing.
//
namespace plx {
class ScopedD2D1DeviceContext {
  plx::ComPtr<ID2D1DeviceContext> dc_;
  plx::ComPtr<IDCompositionSurface> ics_;

  ScopedD2D1DeviceContext(const ScopedD2D1DeviceContext&) = delete;
  ScopedD2D1DeviceContext operator=(const ScopedD2D1DeviceContext&) = delete;
  void* operator new(size_t) = delete;

public:
  ScopedD2D1DeviceContext(plx::ComPtr<IDCompositionSurface> ics,
                          const D2D1_SIZE_F& offset,
                          const plx::DPI& dpi,
                          const D2D1_COLOR_F* clear_color) {
    dc_ = plx::CreateDCoDeviceCtx(ics, dpi, offset);
    ics_ =ics;
    if (clear_color)
      dc_->Clear(*clear_color);
  }

  ~ScopedD2D1DeviceContext() {
    ics_->EndDraw();
  }

  ID2D1DeviceContext* operator()() { return dc_.Get(); }
};
}
