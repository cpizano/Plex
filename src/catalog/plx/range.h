//#~def plx::Range
///////////////////////////////////////////////////////////////////////////////
// plx::Range  (alias for ItRange<T*>)
//
namespace plx {
template <typename T>
using Range = plx::ItRange<T*>;

}
