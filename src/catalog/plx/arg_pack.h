//#~def plx::ArgInfo
//#~def plx::ArgType
///////////////////////////////////////////////////////////////////////////////
// plx::ArgInfo, ArgType.
//
namespace plx {

enum class ArgType : int {
  boolean,
  signed_int,
  unsigned_int,
  string_zt,
  pointer 
};

struct ArgInfo {
  union {
    // An integer-like value.
    uint64_t integer;
    // A C-style text string.
    const char* str;
    // A pointer to an arbitrary object.
    const void* ptr;
  };

  const unsigned char width;
  const ArgType type;
  ArgInfo(bool v) : integer(v), width(0), type(ArgType::boolean) {}

  ArgInfo(signed char v) : integer(v), width(sizeof(v)), type(ArgType::signed_int) {}
  ArgInfo(signed short v) : integer(v), width(sizeof(v)), type(ArgType::signed_int) {}
  ArgInfo(signed int v) : integer(v), width(sizeof(v)), type(ArgType::signed_int) {}
  ArgInfo(signed long v) : integer(v), width(sizeof(v)), type(ArgType::signed_int) {}
  ArgInfo(signed long long v) : integer(v), width(sizeof(v)), type(ArgType::signed_int) {}
  
  ArgInfo(unsigned char v) : integer(v), width(sizeof(v)), type(ArgType::unsigned_int) {}
  ArgInfo(unsigned short v) : integer(v), width(sizeof(v)), type(ArgType::unsigned_int) {}
  ArgInfo(unsigned int v) : integer(v), width(sizeof(v)), type(ArgType::unsigned_int) {}
  ArgInfo(unsigned long v) : integer(v), width(sizeof(v)), type(ArgType::unsigned_int) {}
  ArgInfo(unsigned long long v) : integer(v), width(sizeof(v)), type(ArgType::unsigned_int) {}

  // A C-style text string.
  ArgInfo(const char* s) : str(s), width(sizeof(*s)), type(ArgType::string_zt) { }
  ArgInfo(char* s)       : str(s), width(sizeof(*s)), type(ArgType::string_zt) { }

  // Any pointer value that can be cast to a "void*".
  template<class T>
  ArgInfo(T* p) : ptr((void*)p), type(ArgType::pointer) { }

};

}
