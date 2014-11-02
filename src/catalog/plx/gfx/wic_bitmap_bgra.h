//#~def plx::CreateWICBitmapBGRA
///////////////////////////////////////////////////////////////////////////////
// plx::CreateWICBitmapBGRA : Makes a BGRA 32-bit WIC bitmap.
//
namespace plx {

plx::ComPtr<IWICFormatConverter> CreateWICBitmapBGRA(unsigned int frame_index,
                                                     WICBitmapDitherType dither,
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
                             dither,
                             nullptr,
                             0.0f,
                             WICBitmapPaletteTypeCustom);
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return converter;
}

}
