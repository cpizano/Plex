//#~def plx::JsonException
///////////////////////////////////////////////////////////////////////////////
// plx::JsonException 
// 
namespace plx {
class JsonException : public plx::Exception {

public:
  JsonException(int line)
      : Exception(line, "Json exception") {
    PostCtor();
  }
};
}
