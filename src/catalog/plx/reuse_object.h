//#~def plx::ReuseObject
///////////////////////////////////////////////////////////////////////////////
// plx::ReuseObject
//
namespace plx {

template <typename T>
struct ReuseObject {
  __declspec(thread) static T* th_obj;

  void set(T* obj) {
    if (th_obj)
      __debugbreak();
    th_obj = obj;
  }

  void reset() {
    if (th_obj) {
      delete th_obj;
      th_obj = nullptr;
    }
  }

  template <typename... Args>
  T* get(Args&&... args) {
    if (th_obj) {
      T* t = nullptr;
      std::swap(t, th_obj);
      t->~T();
      return new (t) T(std::forward<Args>(args)...);
    }
    return new T(std::forward<Args>(args)...);
  }
};

template<typename T>
__declspec(thread) T* ReuseObject<T>::th_obj = nullptr;

}
