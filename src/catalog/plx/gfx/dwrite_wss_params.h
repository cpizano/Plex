//#~def plx::FontWSSParams
///////////////////////////////////////////////////////////////////////////////
// plx::FontWSSParams : style, weight, stretch for DirectWrite fonts.
//
namespace plx {

struct FontWSSParams {
  DWRITE_FONT_WEIGHT weight;
  DWRITE_FONT_STYLE style;
  DWRITE_FONT_STRETCH stretch;

  static FontWSSParams MakeNormal() {
    FontWSSParams wss = {
      DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL };
    return wss;
  }
};

}
