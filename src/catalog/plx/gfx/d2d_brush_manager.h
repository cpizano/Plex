//#~def plx::D2D1BrushManager
///////////////////////////////////////////////////////////////////////////////
// plx::D2D1BrushManager : Direct2D brush manager
//
namespace plx {

class D2D1BrushManager {
  std::vector<plx::ComPtr<ID2D1SolidColorBrush>> sb_;

public:
  D2D1BrushManager(size_t size) : sb_(size) {
  }

  void set_solid(ID2D1RenderTarget* rt, size_t index, uint32_t color, float alpha) {
     auto hr = rt->CreateSolidColorBrush(D2D1::ColorF(color, alpha),
                                         sb_[index].ReleaseAndGetAddressOf());
     if (hr != S_OK)
       throw plx::ComException(__LINE__, hr);
  }

  ID2D1SolidColorBrush* solid(size_t index) {
    return sb_[index].Get();
  }

  void release_all() {
    sb_.clear();
  }
};

}
