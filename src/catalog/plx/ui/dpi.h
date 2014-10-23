//#~def plx::DPI
///////////////////////////////////////////////////////////////////////////////
// plx::DPI
//
namespace plx {
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

}
