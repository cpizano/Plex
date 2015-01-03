//#~def plx::CreateDWriteSystemTextFormat
///////////////////////////////////////////////////////////////////////////////
// plx::CreateDWriteSystemTextFormat :  DirectWrite font object.
//
namespace plx {

plx::ComPtr<IDWriteTextFormat> CreateDWriteSystemTextFormat(
    plx::ComPtr<IDWriteFactory> dw_factory,
    const wchar_t* font_family, float size,
    const plx::FontWSSParams& params) {
  plx::ComPtr<IDWriteTextFormat> format;
  auto hr = dw_factory->CreateTextFormat(font_family, nullptr,
                                         params.weight, params.style, params.stretch,
                                         size, L"",
                                         format.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return format;
}

}
