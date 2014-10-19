//#~def plx::ComPtr
///////////////////////////////////////////////////////////////////////////////
// plx::ComPtr : smart COM pointer.
//
namespace plx {
template <typename T> using ComPtr = Microsoft::WRL::ComPtr <T>;
}
