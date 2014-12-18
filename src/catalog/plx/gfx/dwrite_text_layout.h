//#~def plx::CreateDWTextLayout
///////////////////////////////////////////////////////////////////////////////
// plx::CreateDWTextLayout : DirectWrite text layout object.
//
namespace plx {

plx::ComPtr<IDWriteTextLayout> CreateDWTextLayout(
  plx::ComPtr<IDWriteFactory> dw_factory, plx::ComPtr<IDWriteTextFormat> format,
  const plx::Range<const wchar_t>& text, const D2D1_SIZE_F& size) {
  plx::ComPtr<IDWriteTextLayout> layout;
  auto hr = dw_factory->CreateTextLayout(
      text.start(), plx::To<UINT32>(text.size()),
      format.Get(),
      size.width, size.height,
      layout.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return layout;
}

}
