//#~def plx::JsonValue
///////////////////////////////////////////////////////////////////////////////
// plx::JsonValue
//
namespace plx {
template <typename T> using AligedStore =
    std::aligned_storage<sizeof(T), __alignof(T)>;


enum class JsonType {
  NULLT,
  BOOL,
  INT64,
  DOUBLE,
  ARRAY,
  OBJECT,
  STRING,
};

struct JsonObject {
  static JsonObject Make() { return JsonObject(); }
};

class JsonValue {
  typedef std::unordered_map<std::string, JsonValue> ObjectImpl;
  typedef std::vector<JsonValue> ArrayImpl;
  typedef std::string StringImpl;

  JsonType type_;
  union Data {
    explicit Data() {}
    ~Data() {}

    void* nulv;
    bool bolv;
    double dblv;
    int64_t intv;
    AligedStore<StringImpl>::type str;
    AligedStore<ArrayImpl>::type arr;
    //AligedStore<ObjectImpl>::type obj;
    std::aligned_storage<sizeof(ObjectImpl), __alignof(ObjectImpl)>::type obj;
  } u_;

 public:

  JsonValue() : type_(JsonType::NULLT) {
  }

  ~JsonValue() {
    Destroy();
  }

  JsonValue(bool b) : type_(JsonType::BOOL) {
    u_.bolv = b;
  }

  JsonValue(int v) : type_(JsonType::INT64) {
    u_.intv = v;
  }

  JsonValue(int64_t v) : type_(JsonType::INT64) {
    u_.intv = v;
  }

  JsonValue(double v) : type_(JsonType::DOUBLE) {
    u_.dblv = v;
  }

  JsonValue(const std::string& s) : type_(JsonType::STRING) {
    new (&u_.str) std::string(s);
  }

  JsonValue(const char* s) : type_(JsonType::STRING) {
    new (&u_.str) StringImpl(s);
  }

  JsonValue(std::initializer_list<JsonValue> il) : type_(JsonType::ARRAY) {
    new (&u_.arr) ArrayImpl(il.begin(), il.end());
  }

  template<class It>
  JsonValue(It first, It last) : type_(JsonType::ARRAY) {
    new (&u_.arr) ArrayImpl(first, last);
  }

  JsonValue(const JsonObject&) : type_(JsonType::OBJECT) {
    new (&u_.obj) ObjectImpl();
  }

  JsonValue& operator=(const JsonValue& other) {
    if (this != &other) {
      Destroy();

      if (other.type_ == JsonType::BOOL)
        u_.bolv = other.u_.bolv;
      else if (other.type_ == JsonType::INT64)
        u_.intv = other.u_.intv;
      else if (other.type_ == JsonType::DOUBLE)
        u_.dblv = other.u_.dblv;
      else if (other.type_ == JsonType::ARRAY)
        new (&u_.arr) ArrayImpl(*other.GetArray());
      else if (other.type_ == JsonType::OBJECT)
        new (&u_.obj) ObjectImpl(*other.GetObject());
      else if (other.type_ == JsonType::STRING)
        new (&u_.str) StringImpl(*other.GetString());

      type_ = other.type_;
    }
    return *this;
  }

  JsonValue& operator=(JsonValue&& other) {
    if (this != &other) {
      Destroy();

      if (other.type_ == JsonType::BOOL)
        u_.bolv = other.u_.bolv;
      else if (other.type_ == JsonType::INT64)
        u_.intv = other.u_.intv;
      else if (other.type_ == JsonType::DOUBLE)
        u_.dblv = other.u_.dblv;
      else if (other.type_ == JsonType::ARRAY)
        new (&u_.arr) ArrayImpl(std::move(*other.GetArray()));
      else if (other.type_ == JsonType::OBJECT)
        new (&u_.obj) ObjectImpl(std::move(*other.GetObject()));
      else if (other.type_ == JsonType::STRING)
        new (&u_.str) StringImpl(std::move(*other.GetString()));

      type_ = other.type_;
    }
    return *this;
  }

  JsonValue& operator[](const std::string& s) {
    ObjectImpl& obj = *GetObject();
    return obj[s];
  }

 private:

  ObjectImpl* GetObject() {
    if (type_ != JsonType::OBJECT)
      throw 1;
    void* addr = &u_.obj;
    return reinterpret_cast<ObjectImpl*>(addr);
  }

  const ObjectImpl* GetObject() const {
    if (type_ != JsonType::OBJECT)
      throw 1;
    const void* addr = &u_.obj;
    return reinterpret_cast<const ObjectImpl*>(addr);
  }

  ArrayImpl* GetArray() {
    if (type_ != JsonType::ARRAY)
      throw 1;
    void* addr = &u_.arr;
    return reinterpret_cast<ArrayImpl*>(addr);
  }

  const ArrayImpl* GetArray() const {
    if (type_ != JsonType::ARRAY)
      throw 1;
    const void* addr = &u_.arr;
    return reinterpret_cast<const ArrayImpl*>(addr);
  }

  std::string* GetString() {
    if (type_ != JsonType::STRING)
      throw 1;
    void* addr = &u_.str;
    return reinterpret_cast<StringImpl*>(addr);
  }

  const std::string* GetString() const {
    if (type_ != JsonType::STRING)
      throw 1;
    const void* addr = &u_.str;
    return reinterpret_cast<const StringImpl*>(addr);
  }

  void Destroy() {
    if (type_ == JsonType::ARRAY)
      GetArray()->~ArrayImpl();
    else if (type_ == JsonType::OBJECT)
      GetObject()->~ObjectImpl();
    else if (type_ == JsonType::STRING)
      GetString()->~StringImpl();
  }

};
}