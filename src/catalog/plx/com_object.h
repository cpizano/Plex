//#~def plx::ComObject
///////////////////////////////////////////////////////////////////////////////
// plx::ComObject : helps implement COM objects.
//
namespace plx {
template <typename... T> using ComObject =
    Microsoft::WRL::RuntimeClass <
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, T...>;
}
