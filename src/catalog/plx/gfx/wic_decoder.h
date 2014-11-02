//#~def plx::CreateWICDecoder
///////////////////////////////////////////////////////////////////////////////
// plx::CreateWICDecoder : Makes a Windows Imaging Components decoder.
//
namespace plx {

plx::ComPtr<IWICBitmapDecoder> CreateWICDecoder(
    plx::ComPtr<IWICImagingFactory> factory, const plx::FilePath& fname) {
  plx::ComPtr<IWICBitmapDecoder> decoder;
  auto hr = factory->CreateDecoderFromFilename(fname.raw(), nullptr,
                                               GENERIC_READ,
                                               WICDecodeMetadataCacheOnDemand,
                                               decoder.GetAddressOf());
  if (hr != S_OK)
    throw plx::ComException(__LINE__, hr);
  return decoder;
}

}
