// plex.cpp : This is the plex.exe tool. It implements a funky c++ parser designed
// to be halfway between a c preprocessor and a real c++ lexer. Its core is a
// array of tokens of the form {type, line, column, range} where range refers
// to the start and end of the token in the source file.
//
// Unlike a c preprocessor, nor macro or includes are processed, and tokens are
// tailored for source code transformation rather than code generation.
//
// The purpose behind it is to relieve the average programer from tedious tasks
// like #includes and to allow expressing in code that today requires bespoke tools
// and auxiliary files in json or xml.
//
// Phases of operation
// 1. Tokenize and lex (lexical lift) the input .cc file
// 2. Load the index.plex file
// 3. Find the input dependencies can be resolved by the items in the index.plex
// 4. For each dependency do:
//    4.1 Tokenize and lex the file that contains it.
//    4.2 Find the dependencies of it that can be resolved by the index.plex
//    4.3 Add the dependencies to the dependency list.
// 5. At this point there is a dependency graph, rooted at the input file then:
//    5.1 Confirm that there are no circular dependencies.
//    5.2 Arrange the #include dependencies in alphabetical order. 
//    5.3 Arrange the other depencies in a reasonable order.
// 6. Inject the dependencies into the input token stream.
// 7. Stream to a file the modified input stream.
//
// Terminology:
// Tokenization: basically generate a vector of CppTokens that represent string
// runs, numbers, keywords, parens and such.
//
// Lexical lift: a second pass on the vector of step 1 converts the tokens in
// place into c++ elements, like pragmas, defines, comments, c++ literals, etc.
//
// For example of phase 2:
// a c++ comment '// blah blah' is represented as a single token and so are the
// following:   1) L"nikita is alive" 2) .999e+3 3) #if defined FOO
//
// BUGS to be fixed short term
// ---------------------------
// 001 coalese strings "aaa""bb""cc" even in different lines.
// 007 handle comments at the end of preprocessor lines.
// 016 have a test with printfs.
// 019 coalease templated types in the name like Moo<int> moo;
// 020 handle namespace alias 'namespace foo = bar::doo'.
// 021 handle enum <type> class.
// 023 add NOMINMAX macro when required.
// 024 plx::DecodeString missing features
//
// Medium term
// ---------------------------
// 102 truly manage insert dependencies.
// 103 32-bit compilation tests.
// 104 more useful catalog entities.
// 105 control x64 vs ia32 constructs.
//
// Longer term niceties
// ---------------------------
// 203 git or github integration.
// 204 catalog index (index.plex) automated generation.
//
// features to be done
//----------------------------
// 401 automate enum to string code, see XternDef::Type.
// 402 writes all includes for std:: well known entities.
// 403 writes common includes for <windows> types.
// 404 isolate const strings into bundles. for example
//     foo("error: no space") -> foo(error_no_space);
// 405 partial catalog classes, for example not all methods
//     of plx::CpuId need to be copied.
// 406 annotation to add __LINE__ to user code. See #~ln()
// 407 check_return for example range.advance(x) + range.front().

#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>

#include <stdlib.h>
#include <algorithm>
#include <deque>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <type_traits>
#include <utility>

#pragma region constants
const char anonymous_namespace_mk[] = "<[anonymous]>";
const char scope_namespace_mk[] = "<[scope]>";
char first_include_key[] = "!~";
char last_include_key[] = "$~";
#pragma endregion

#pragma region exceptions

class PlexException {
  int line_;
  const char* message_;

protected:
  void PostCtor() {
    if (::IsDebuggerPresent()) {
      __debugbreak();
    }
  }

public:
  PlexException(int line, const char* message) : line_(line), message_(message) {}
  virtual ~PlexException() {}
  const char* Message() const { return message_; }
  int Line() const { return line_; }
};

class IOException : public PlexException {
  DWORD error_code_;
  const std::wstring name_;

public:
  IOException(int line, const wchar_t* name)
      : PlexException(line, "IO problem"),
        error_code_(::GetLastError()),
        name_(name) {
    PostCtor();
  }
  DWORD ErrorCode() const { return error_code_; }
  const wchar_t* Name() const { return name_.c_str(); }
};

class TokenizerException : public PlexException {
  int source_line_;
  std::wstring file_;

public:
  TokenizerException(const std::wstring& file, int line, int source_line)
      : PlexException(line, "Tokenizer problem"), source_line_(source_line), file_(file)  {
    PostCtor();
  }
  int SourceLine() const { return source_line_; }
  const wchar_t* Path() const { return file_.c_str(); }
};

class CatalogException : public PlexException {
  int source_line_;

public:
  CatalogException(int line, int source_line)
      : PlexException(line, "Catalog problem"), source_line_(source_line)  {
    PostCtor();
  }
  int SourceLine() const { return source_line_; }
};

#pragma endregion

std::string UTF16ToAscii(const std::wstring& utf16) {
  std::string result;
  result.reserve(utf16.size());
  for (auto it = utf16.begin(); it != utf16.end(); ++it) {
    if ((*it) <= 0xFF) {
      result.append(1, static_cast<char>(*it));
    }
  }
  return result;
}

std::wstring AsciiToUTF16(const std::string& ascii) {
  return std::wstring(ascii.begin(), ascii.end());
}

#pragma region memory

template <typename T>
class Range {
private:
  T* start_;
  T* end_;

  bool plex_check_init() {
    return (start_ <= end_);
  }

  typedef typename std::remove_const<T>::type NCT;
  
  friend class LineIterator;

public:
  Range(T* start, T* end) : start_(start), end_(end) {
  }

  Range() : start_(nullptr), end_(nullptr) {
  }

  Range(const Range<NCT>& other)
    : start_(other.Start()), end_(other.End()) {
  }

  // Watch out, it does not de-alloc on destruction.
  explicit Range(size_t alloc_count) 
    : start_(new T[alloc_count]), end_(start_ + sizeof(T)*alloc_count) {
  }

  template <size_t count>
  Range(T (&str)[count])
     : start_(str), end_(&str[count - 1]) {
  }

  T* Start() const {
    return start_;
  }

  T* End() const {
    return end_;
  }

  size_t Size() const { 
    return end_ - start_;
  }

  void Reset() {
    start_ = nullptr;
    end_ = nullptr;
  }

  const T& operator[](size_t ix) const {
    return start_[ix];
  }

  T Next(T*& curr) const {
    if (curr + 1 >= end_) {
      curr = nullptr;
      return T(0);
    }
    return *(++curr);
  }

  Range<T>& operator=(const Range& rhs) {
    start_ = rhs.start_;
    end_ = rhs.end_;
    return *this;
  }

  bool Equal(const Range<const T>& rhs) const {
    if (Size() != rhs.Size())
      return false;
    if (!Size())
      return true;
    return (memcmp(start_, rhs.Start(), Size()) == 0);
  }

  struct SliceToEnd {};
  struct SliceFromStart{};

  Range<T> Find(T what, const SliceToEnd&) const {
    for (auto it = start_; it != end_; ++it) {
      if (what == *it) return Range<T>(it, end_);
    }
    return Range<T>();
  }

  Range<T> Find(T what, const SliceFromStart&) const {
    for (auto it = start_; it != end_; ++it) {
      if (what == *it) return Range<T>(start_, it + 1);
    }
    return Range<T>();
  }

  Range<T> Find(T first, T last) const {
    auto r = Find(first, SliceToEnd());
    if (!r.Size())
      return r;
    return r.Find(last, SliceFromStart());
  }
};

Range<const char> FromString(const std::string& txt) {
  return Range<const char>(&txt[0], &txt[txt.size()]);
}

Range<char> FromString(std::string& txt) {
  return Range<char>(&txt[0], &txt[txt.size()]);
}

// FNV-1a Hash.
// Test "foobar" --> 0x85944171f73967e8ULL.
size_t HashFNV1a(const Range<char>& r) {
  auto bp = reinterpret_cast<const unsigned char*>(r.Start());
  auto be = reinterpret_cast<const unsigned char*>(r.End());

  uint64_t hval = 0xcbf29ce484222325ULL;
  while (bp < be) {
    // xor the bottom with the current octet.
    hval ^= (uint64_t)*bp++;
    // multiply by the 64 bit FNV magic prime mod 2^64. In other words
    // hval *= FNV_64_PRIME; which is 0x100000001b3ULL;
    hval += (hval << 1) + (hval << 4) + (hval << 5) +
            (hval << 7) + (hval << 8) + (hval << 40);
  }
  // Microsoft's std::string hash has: hval ^= hval >> 32;
  return hval;
}

std::string ToString(const Range<char>& r) {
  return std::string(r.Start(), r.Size());
}

std::wstring AsciiToUTF16(const Range<char>& str) {
  auto r = str.Start();
  std::wstring rv;
  rv.reserve(str.Size());
  while(r) {
    rv.append(1, *r);
    str.Next(r);
  }
  return rv;
}

int DecodeUTF8Point(char*& start, const Range<char>& range) {
  return -1;
}

#pragma endregion

#pragma region cmdline

class CmdLine {
  std::unordered_map<std::string, std::string> opts_;
  std::vector<std::wstring> extra_;
  std::string program_;

public:
  CmdLine(int argc, wchar_t* argv[]) {
    if (!argc)
      return;
    program_ = UTF16ToAscii(argv[0]);
    if (!IsProgram(program_))
      throw PlexException(__LINE__, "missing program in argv[0]");

    for (int ix = 1; ix != argc; ++ix) {
      if (IsOption(argv[ix]))
        opts_.insert(NameValue(UTF16ToAscii(&(argv[ix][2]))));
      else
        extra_.push_back(argv[ix]);
    }
  }

  const bool HasSwitch(const std::string& name) const {
    auto pos = opts_.find(name);
    return (pos != end(opts_));
  }

  std::string Value(const std::string& name) const {
    auto pos = opts_.find(name);
    if (pos == end(opts_))
      return std::string();
    return pos->second;
  }

  std::wstring Extra(size_t index) const {
    if (index >= extra_.size())
      return std::wstring();
    return extra_[index];
  }

private:
  bool IsOption(const std::wstring& s) {
    if (s.size() < 3)
      return false;
    return ((s[0] == '-') && (s[1] == '-') && (s[2] != '-') && (s[2] != '='));
  }

  std::pair<std::string, std::string> NameValue(const std::string& s) {
    auto pos = s.find_first_of('=');
    if (pos == std::string::npos)
      return std::make_pair(s, std::string());
    return std::make_pair(s.substr(0, pos), s.substr(pos + 1, std::string::npos));
  }

  bool IsProgram(const std::string& s) const {
    auto pos = s.find(".exe");
    if (pos == std::string::npos)
      return false;
    return (s.size() > 4);
  }

};


#pragma endregion

#pragma region file_io

class FilePath {
private:
  std::wstring path_;
  friend class File;

public:
  explicit FilePath(const wchar_t* path)
    : path_(path) {
  }

  explicit FilePath(const std::wstring& path)
    : path_(path) {
  }

  FilePath Parent() const {
    auto pos = path_.find_last_of(L'\\');
    if (pos == std::string::npos)
      return FilePath();
    return FilePath(path_.substr(0, pos));
  }

  std::wstring Leaf() const {
    auto pos = path_.find_last_of(L'\\');
    if (pos == std::string::npos)
      return path_;
    return path_.substr(pos + 1);
  }

  FilePath Append(const std::wstring& name) const {
    std::wstring full(path_);
    if (!path_.empty())
      full.append(1, L'\\');
    full.append(name);
    return FilePath(full);
  }

  std::string ToAscii() const {
    return UTF16ToAscii(path_);
  }

  bool Exists() const {
    WIN32_FIND_DATAW ffd;
    HANDLE fh = ::FindFirstFileW(path_.c_str(), &ffd);
    if (fh == INVALID_HANDLE_VALUE)
      return false;
    ::FindClose(fh);
    return true;
  }

  const wchar_t* Raw() const { return path_.c_str(); }

private:
  FilePath() {}
};


class FileParams {
private:
  DWORD access_;
  DWORD sharing_;
  DWORD disposition_;
  DWORD attributes_;  // For existing files these are generally ignored.
  DWORD flags_;       // For existing files these are generally combined.
  DWORD sqos_;
  friend class File;

public:
  static const DWORD kShareNone = 0;
  static const DWORD kShareAll = FILE_SHARE_DELETE |
                                 FILE_SHARE_READ |
                                 FILE_SHARE_WRITE;

  FileParams()
    : access_(0),
      sharing_(kShareAll),
      disposition_(OPEN_EXISTING),
      attributes_(FILE_ATTRIBUTE_NORMAL),  
      flags_(0),  
      sqos_(0) {
  }

  FileParams(DWORD access,
             DWORD sharing,
             DWORD disposition,
             DWORD attributes,
             DWORD flags,
             DWORD sqos)
    : access_(access),
      sharing_(sharing),
      disposition_(disposition),
      attributes_(attributes),
      flags_(flags),
      sqos_(sqos) {
  }

  static FileParams AppendSharedRead() {
    return FileParams(FILE_APPEND_DATA, FILE_SHARE_READ,
                      OPEN_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL, 0, 0);
  }

  static FileParams ReadSharedRead() {
    return FileParams(GENERIC_READ, FILE_SHARE_READ,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL, 0, 0);
  }

  static FileParams ReadWriteSharedRead(DWORD disposition) {
    return FileParams(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                      disposition,
                      FILE_ATTRIBUTE_NORMAL, 0, 0);
  }

  bool plex_check_init() const {
    // $$$ some combination of access and dispositions are invalid
    // and possible some attributes + some flags. see msdn.
  }

};

class FileSecurity {
private:
  SECURITY_ATTRIBUTES* sattr_;
  friend class File;
public:
  FileSecurity()
    : sattr_(nullptr) {
  }
};

class File {
  HANDLE handle_;
  unsigned int  status_;
  friend class FileView;

private:
  File(HANDLE handle,
       unsigned int status)
    : handle_(handle),
      status_(status) {
  }

  File();
  File(const File&);
  File& operator=(const File&);

public:
  enum Status {
    none              = 0,
    brandnew          = 1,   // 
    existing          = 2,
    delete_on_close   = 4,
    directory         = 8,   // $$$ just use backup semantics?
    exclusive         = 16,
    readonly          = 32,
    information       = 64,
  };

  File(File&& file) 
    : handle_(INVALID_HANDLE_VALUE),
      status_(none) {
    std::swap(file.handle_, handle_);
    std::swap(file.status_, status_);
  }

  static File Create(const FilePath& path,
                     const FileParams& params,
                     const FileSecurity& security) {
    HANDLE file = ::CreateFileW(path.path_.c_str(),
                                params.access_,
                                params.sharing_,
                                security.sattr_,
                                params.disposition_,
                                params.attributes_ | params.flags_ | params.sqos_,
                                0);
    DWORD gle = ::GetLastError();
    unsigned int status = none;

    if (file != INVALID_HANDLE_VALUE) {

      switch (params.disposition_) {
        case OPEN_EXISTING:
        case TRUNCATE_EXISTING:
          status |= existing; break;
        case CREATE_NEW:
          status |= (gle == ERROR_FILE_EXISTS) ? existing : brandnew; break;
        case CREATE_ALWAYS:
        case OPEN_ALWAYS:
          status |= (gle == ERROR_ALREADY_EXISTS) ? existing : brandnew; break;
        default:
          __debugbreak();
      };

      if (params.flags_ == FILE_FLAG_BACKUP_SEMANTICS) status |= directory;
      if (params.flags_ & FILE_FLAG_DELETE_ON_CLOSE) status |= delete_on_close;
      if (params.sharing_ == 0) status |= exclusive;
      if (params.access_ == 0) status |= information;
      if (params.access_ == GENERIC_READ) status |= readonly;
    }

    return File(file, status);
  }

  ~File() {
    if (handle_ != INVALID_HANDLE_VALUE) {
      if (!::CloseHandle(handle_)) {
        throw IOException(__LINE__, nullptr);
      }
    }
  }

  // $$ this ignores the volume id, so technically incorrect.
  long long GetUniqueId() {
    BY_HANDLE_FILE_INFORMATION bhfi;
    if (!::GetFileInformationByHandle(handle_, &bhfi))
      throw IOException(__LINE__, nullptr);
    LARGE_INTEGER li = { bhfi.nFileIndexLow, bhfi.nFileIndexHigh };
    return li.QuadPart;
  }

  unsigned int Status() const {
    return status_;
  }

  bool IsValid() const {
    return (handle_ != INVALID_HANDLE_VALUE);
  }

  size_t SizeInBytes() const {
    LARGE_INTEGER li = {0};
    ::GetFileSizeEx(handle_, &li);
    return li.QuadPart;
  }

  size_t Read(Range<char>& mem, unsigned int start) {
    OVERLAPPED ov = {0};
    ov.Offset = start;
    DWORD read = 0;
    if (!::ReadFile(handle_, mem.Start(), static_cast<DWORD>(mem.Size()),
                    &read, &ov))
      return 0;
    return read;
  }

  size_t Write(const Range<const char>& mem, int start = -1) {
    return Write(mem.Start(), mem.Size(), start);
  }

  size_t Write(const Range<char>& mem, int start = -1) {
    return Write(mem.Start(), mem.Size(), start);
  }

  size_t Write(const char* buf, size_t len, int start) {
    OVERLAPPED ov = {0};
    ov.Offset = start;
    DWORD written = 0;
    if (!::WriteFile(handle_, buf, static_cast<DWORD>(len),
                     &written, (start < 0) ? nullptr : &ov))
      return 0;
    return written;
  }
};

class FileView : public Range<char> {
private:
  HANDLE map_;

  FileView(HANDLE map,
           char* start,
           char* end)
    : Range(start, end),
      map_(map) {
  }

public:
  enum Mode {
    read_only,
    read_write,
  };

  static FileParams GetParams(FileView::Mode mode) {
    return (mode == read_only) ? 
      FileParams::ReadSharedRead() : FileParams::ReadWriteSharedRead(OPEN_ALWAYS);
  }

  static FileView Create(const File& file,
                         size_t offset,
                         size_t map_size,
                         const wchar_t* name) {
    DWORD protection = (file.status_ & File::readonly) ? PAGE_READONLY : PAGE_READWRITE;
    DWORD access = (protection == PAGE_READONLY) ? FILE_MAP_READ : FILE_MAP_WRITE | FILE_MAP_READ;
    HANDLE map = ::CreateFileMappingW(file.handle_, 0, protection, 0, 0, name);
    if (map == 0) {
      throw IOException(__LINE__, nullptr);
    }

    ULARGE_INTEGER uli;
    uli.QuadPart = offset;
    char* start = reinterpret_cast<char*>(::MapViewOfFile(map, access, uli.HighPart, uli.LowPart, map_size));
    if (!start) {
      throw IOException(__LINE__, nullptr);
    }

    return FileView(map, start, start + file.SizeInBytes());
  }

  size_t RegionSize() const {
    MEMORY_BASIC_INFORMATION mbi = {0};
    ::VirtualQuery(Start(), &mbi, sizeof(mbi));
    if (!mbi.RegionSize) {
      throw IOException(__LINE__, nullptr);
    }
    return mbi.RegionSize;
  }

  ~FileView() {
    if (Start()) {
      if (!::UnmapViewOfFile(Start())) {
        throw IOException(__LINE__, nullptr);
      }
    }
    if (map_) {
      if (!::CloseHandle(map_)) {
        throw IOException(__LINE__, nullptr);
      }
    }
  }
};

class Logger {
  File file_;
  static Logger* instance;

public:
  static Logger& Get() {
    return *instance;
  }

  static bool HasLogger() {
    return (instance != nullptr);
  }

  Logger(FilePath& path) : file_(Create(path)) {
    if (!file_.IsValid()) {
      throw IOException(__LINE__, path.Raw());
    }
    file_.Write("@ Plex genlog [0.4] "__DATE__"\n");
    instance = this;
  }

  ~Logger() {
    file_.Write("@ Session end\n\n");
  }

  void AddFileInfoStart(const FilePath& file) {
    auto text = std::string("file [") + file.ToAscii() + "]\n";
    file_.Write(FromString(text));
  }

  void ReportException(PlexException& ex) {
    auto text = std::string("exception type=plex [") + ex.Message() + "]\n";
    file_.Write(FromString(text));
  }

  void AddExternDef(const Range<char>& def, int line_no) {
    auto text = std::string("adding xdef [") + ToString(def) + "] " + LineNumber(line_no);
    text.append(1, '\n');
    file_.Write(FromString(text));
  }

  void ProcessInclude(const Range<char>& include) {
    auto text = std::string("include target [") + ToString(include) + "]\n";
    file_.Write(FromString(text));
  }

  void ProcessCode(const Range<char>& code_ref) {
    auto text = std::string("code target [") + ToString(code_ref) + "]\n";
    file_.Write(FromString(text));
  }

private:
  static File Create(FilePath path) {
    return File::Create(path, FileParams::AppendSharedRead(), FileSecurity());
  }

  const char* LineNumber(int line_no) {
    static char buf[] = "ln 123456789012";
    _itoa_s(line_no, &buf[3], 12, 10);
    return buf;
  }
};

Logger* Logger::instance = nullptr;

// Loads an entire file into memory, keeping only one copy. Memory is kept
// until program ends. Harcoded limit of 256 MB for all files.
Range<char> LoadFileOnce(const FilePath& path) {
  static std::unordered_map<long long, Range<char>> map;
  static size_t total_size = 0;

  File file = File::Create(path, FileParams::ReadSharedRead(), FileSecurity());
  if (!file.IsValid())
    throw IOException(__LINE__, path.Raw());
  long long id = file.GetUniqueId();
  auto it = map.find(id);
  if (it != map.end())
    return it->second;

  size_t size = file.SizeInBytes();
  if (size + total_size > (1024 * 1024 * 256))
    throw IOException(__LINE__, path.Raw());

  Logger::Get().AddFileInfoStart(path);

  Range<char> range(size);
  size = file.Read(range, 0);
  if (size != range.Size())
    throw IOException(__LINE__, path.Raw());

  map[id] = range;
  total_size += size;
  return range;
}

#pragma endregion

#pragma region cpptoken

struct Insert;

struct KeyRangeHash {
  std::size_t operator()(const Range<char>& k) const {
    return HashFNV1a(k);
  }
};

struct KeyRangeEq {
  bool operator()(const Range<char>& k1, const Range<char>& k2) const {
    return k1.Equal(k2);
  }
};

template <typename T>
using RangeHashTable =
    std::unordered_map<Range<char>, T, KeyRangeHash, KeyRangeEq>;

struct ScopeBlock {
  enum Type { 
    none,
    anons_namespace,
    named_namespace,
    block_aggregate,
    block_scope,
    block_enum,
    block_other};

  Type type;
  size_t top;
  size_t start;
  size_t end;
  const Range<char> name;

  ScopeBlock(Type type, const Range<char>&name, size_t start, size_t top)
      : type(type), top(top), start(start), end(0), name(name) {}
};

struct KeyElements {
  FilePath src_path;
  RangeHashTable<size_t> includes;
  std::vector<ScopeBlock> scopes;
  std::unordered_map<std::string, std::vector<std::string>> properties;

  KeyElements(const FilePath& path) : src_path(path) {
    scopes.reserve(20);
  }
};

struct CppToken {
  enum Type {
    none,
    unknown,
    sos,                // start of token stream.
    eos,                // end of token stream.                 
    string,             // unclasiffied bundle of characters.
    predef_macro,       // a standard or VS predifined macro. 
    identifier,         // c++ identifer.
    const_number,       // c++ numeric constant.
    comment,            // a c++ style comment
    const_str,          // a const string.
    const_wstr,         // a const wide string.
    const_char,         // a const char.
    const_wchar,        // a const wide char.
    symbols_begin,
    // one char tokens block 1
    logical_not,        // 0x21 : !
    double_quote,       // 0x22 : "
    hash,               // 0x23 : #
    dollar_sign,        // 0x24 : $
    percent,            // 0x25 : %
    ampersand,          // 0x26 : &
    single_quote,       // 0x27 : '
    open_paren,         // 0x28 : (
    close_paren,        // 0x29 : )
    asterisc,           // 0x2A : *
    plus,               // 0x2B : +
    comma,              // 0x2C : ,
    minus,              // 0x2D : -
    period,             // 0x2E : .
    fwd_slash,          // 0x2F : /
    // one char tokens block 2
    colon,              // 0x3A : :
    semicolon,          // 0x3B : ;
    less_than,          // 0x3C : <
    equal,              // 0x3D : =
    more_than,          // 0x3E : >
    question_mark,      // 0x3F : ?
    at_sign,            // 0x40 : @
    // one char tokens block 3
    open_sqr_bracket,   // 0x5B : [
    backlash,           // 0x5C :
    close_sqr_bracket,  // 0x5D : ]
    hat,                // 0x5E : ^
    // one char tokens block 4
    tilde,              // 0x60 : `
    // one char tokens block 5
    open_cur_bracket,   // 0x7B : {
    bitwise_and,        // 0x7C : |
    close_cur_bracket,  // 0x7D : }
    bitwise_not,        // 0x7E : ~
    symbols_end,
    // two char tokens alphabetically arranged
    not_eq,               // !=
    ass_mod,              // %=
    logical_and,          // &&
    ass_and,              // &=
    ass_multiplication,   // *=
    increment,            // ++
    ass_increment,        // +=
    decrement,            // --
    ass_decrement,        // -=
    deref_ptr,            // ->
    line_comment,         // //
    ass_division,         // /=
    name_scope,           // ::
    left_shift,           // <<
    less_or_eq,           // <=
    eq,                   // ==
    more_or_eq,           // >=
    right_shift,          // >>
    ass_xor,              // ^=
    logical_or,           // ||
    ass_or,               // |=
    // three char tokens
    ass_left_shift,
    ass_right_shift,
    // All c++ keywords must be alphabetically arranged in the kw_start --> kw_end block.
    kw_start,
    kw_alignas,     // new in c++11
    kw_alignof,     // new in c++11
    kw_and,
    kw_and_eq,
    kw_asm,
    kw_auto,        // meaning changed in c++11
    kw_bitand,
    kw_bitor,
    kw_bool,
    kw_break,
    kw_case,
    kw_catch,
    kw_char,
    kw_char16_t,    // new in c++11
    kw_char32_t,    // new in c++11
    kw_class,
    kw_compl,
    kw_const,
    kw_constexpr,   // new inc c++11
    kw_const_cast,
    kw_continue,
    kw_decltype,
    kw_default,     // meaning changed in c++11
    kw_delete,
    kw_do,
    kw_double,
    kw_dynamic_cast,
    kw_else,
    kw_enum,
    kw_explicit,
    kw_export,
    kw_extern,
    kw_false,
    kw_final,     // technically not a keyword
    kw_float,
    kw_for,
    kw_friend,
    kw_goto,
    kw_if,
    kw_inline,
    kw_int,
    kw_long,
    kw_mutable,
    kw_namespace,
    kw_new,
    kw_noexcept,    // new in c++11
    kw_not,
    kw_not_eq,
    kw_nullptr,     // new in c++11
    kw_operator,
    kw_or,
    kw_or_eq,
    kw_override,    // technically not a keyword
    kw_private,
    kw_protected,
    kw_public,
    kw_register,
    kw_reinterpret_cast,
    kw_return,
    kw_short,
    kw_signed,
    kw_sizeof,
    kw_static,
    kw_static_assert,  // new in c++11
    kw_static_cast,
    kw_struct,
    kw_switch,
    kw_template,
    kw_this,
    kw_thread_local,  // new in c++11
    kw_throw,
    kw_true,
    kw_try,
    kw_typedef,
    kw_typeid,
    kw_typename,
    kw_union,
    kw_unsigned,
    kw_using,     // meaning change in c+11
    kw_virtual,
    kw_void,
    kw_volatile,
    kw_wchar_t,
    kw_while,
    kw_xor,
    kw_xor_eq,
    kw_end,
    // Preprocessor keywords alphabetically aligned.
    prep_start,
    prep_define,
    prep_else,
    prep_endif,
    prep_error,
    prep_if,
    prep_ifdef,
    prep_ifndef,
    prep_include,
    prep_line,
    prep_pragma,
    prep_undefine,
    prep_end,
    // Plex specific tokens
    plex_start,
    plex_comment,
    plex_pragma,
    plex_insert,
  };

  Range<char> range;
  Type type;
  int line;
  int col;

  union {
    Insert* insert;
    KeyElements* kelems;  // Only applies for SOS token.
  };

  CppToken(const Range<char>& range, CppToken::Type type, int line, int col)
    : range(range), type(type), line(line), col(col), insert(nullptr) { }
};

struct Insert {
  enum Kind {
    delete_original,
    keep_original,
  };

  Kind kind;
  std::vector<CppToken> tv;

  Insert(Kind kind, const CppToken& tok) : kind(kind) {
    tv.push_back(tok);
  }

  Insert(Kind kind) : kind(kind) {
  }

  // This returns a singleton deleter that can be used
  // in many places without extra cost.
  static Insert* TokenDeleter() {
    static auto insert = new Insert(delete_original);
    return insert;
  }
};

std::string ToString(const CppToken& tok) {
  return ToString(tok.range);
}

bool EqualToStr(const CppToken& tok, const Range<const char>& r) {
  return tok.range.Equal(r);
}

template<typename T, size_t count>
size_t FindToken(T (&kw)[count], const Range<char>& r) {
 auto f = std::lower_bound(&kw[0], &kw[count], r, 
    [] (const char* kw, const Range<char>& v) {
      return strncmp(kw, v.Start(), v.Size()) < 0;
  });
  if (f == &kw[count]) 
    return -1;
  if (::strncmp(*f,  r.Start(), r.Size()) != 0)
    return -1;
  if ((*f)[r.Size()] != 0)
    return -1;
  return (f - kw );
}

CppToken::Type GetCppKeywordType(const Range<char>& r) {
  // The 86 c++11 keywords.
  const char* kw[] = {
    "alignas", "alignof", "and", "and_eq",
    "asm", "auto", "bitand", "bitor",
    "bool", "break", "case", "catch",
    "char", "char16_t", "char32_t", "class",
    "compl", "const", "constexpr", "const_cast",
    "continue", "decltype", "default", "delete",
    "do", "double", "dynamic_cast", "else",
    "enum", "explicit", "export", "extern",
    "false", "final", "float", "for",
    "friend", "goto", "if", "inline",
    "int", "long", "mutable", "namespace",
    "new", "noexcept", "not", "not_eq",
    "nullptr", "operator", "or", "or_eq",
    "override", "private", "protected", "public",
    "register", "reinterpret_cast", "return", "short",
    "signed", "sizeof", "static", "static_assert", 
    "static_cast", "struct", "switch", "template",
    "this", "thread_local", "throw", "true",
    "try", "typedef", "typeid", "typename",
    "union", "unsigned", "using", "virtual",
    "void", "volatile", "wchar_t", "while",
    "xor", "xor_eq",
  };

  size_t off = FindToken(kw, r);
  return (off == -1) ?
      CppToken::unknown :
      static_cast<CppToken::Type>(CppToken::kw_start + off + 1);
}

CppToken::Type GetCppPreprocessorKeyword(const Range<char>& r) {
  const char* kw[] = {
    "define", "else", "endif", "error",
    "if", "ifdef", "ifndef", "include",
    "line", "pragma", "undefine",
  };
  size_t off = FindToken(kw, r);
  return (off == -1) ?
      CppToken::unknown :
      static_cast<CppToken::Type>(CppToken::prep_start + off + 1);
}

bool IsPredefinedMacro(const Range<char>& r) {
  const char* kw[] = {
    "_ATL_VER", "_CPPRTTI", "_CPPUNWIND", "_DEBUG", "_DLL",
    "_MSC_VER", "_MT", "_M_IX86", "_M_X64", "_WIN32", "_WIN64",
    "__COUNTER__", "__DATE__", "__FILE__",
    "__FUNCDNAME__", "__FUNCSIG__", "__FUNCTION__",
    "__LINE__", "__STDC__", "__TIME__", "__TIMESTAMP__",
  };

  return FindToken(kw, r) != -1;
}

CppToken::Type GetTwoTokenType(const CppToken& first, const CppToken& second) {
  const char* kw[] = {
    "!=", "%=", "&&", "&=", 
    "*=", "++", "+=", "--", 
    "-=", "->", "//", "/=", "::", 
    "<<", "<=", "==", ">=",
    ">>", "^=", "||", "|=" 
  };

  static_assert(_countof(kw) == 
      size_t(CppToken::ass_left_shift - CppToken::not_eq), "TwoToken");

  if (first.range.End() != second.range.Start())
    return CppToken::unknown;

  size_t off = FindToken(kw, Range<char>(first.range.Start(), second.range.End()));
  return (off == -1) ?
      CppToken::unknown :
      static_cast<CppToken::Type>(CppToken::not_eq + off);
}

#pragma endregion

typedef std::vector<CppToken> CppTokenVector;

#pragma region tokenizer

CppTokenVector TokenizeCpp(const FilePath& path, Range<char>* e_range = nullptr) {
  Range<char> range = e_range ? *e_range : LoadFileOnce(path);
  CppTokenVector tv;
  tv.reserve(range.Size() / 3);

  char* curr = range.Start();
  char* str = nullptr;
  char c = *curr;

  // The first token is always (s)tart-(o)f-(s)stream.
  tv.push_back(CppToken(Range<char>(curr, curr), CppToken::sos, 0, 0));
  tv.front().kelems = new KeyElements(path);
 
  int line = 1;
  int column = 1;

  do {
    int point = (c < 0x80) ? c :  DecodeUTF8Point(curr, range);

    if (point < 0) {
      throw TokenizerException(path.Raw(), __LINE__, line);
    }

    if (point == 0) {
      if (curr != range.End()) {
        // End of the file, must only be at the end.
        throw TokenizerException(path.Raw(), __LINE__, line);
      }
      if (str) {
        // Incomplete token, unexpected EoF.
        throw TokenizerException(path.Raw(), __LINE__, line);
      }

    } else if ((point == 0x09) || (point == 0x0A) || (point == 0x20)) {
      // tab (0x09) linefeed (0x0A) and space (0x20).
      if (str) {
        auto r = Range<char>(str, curr);
        int scol = column - static_cast<int>(r.Size());
        tv.push_back(CppToken(r, CppToken::string, line, scol));
        str = nullptr;
      }

      if (point == 0x0A) {
        ++line;
        column = 0;
      }

    } else if ((point <  0x20) || (point == 0x7F)) {
      // Nonprintables are unrecoberable erros.
      throw TokenizerException(path.Raw(), __LINE__, line);
    } else {
      int symbol_type;
      switch (point) {
        case 0x21 :    // !
        case 0x22 :    // "
        case 0x23 :    // #
        case 0x24 :    // $
        case 0x25 :    // %
        case 0x26 :    // &
        case 0x27 :    // '
        case 0x28 :    // (
        case 0x29 :    // )
        case 0x2A :    // *
        case 0x2B :    // +
        case 0x2C :    // ,
        case 0x2D :    // -
        case 0x2E :    // .
        case 0x2F :    // /
          symbol_type = CppToken::logical_not + (point - 0x21); break;
        case 0x3A :    // :
        case 0x3B :    // ;
        case 0x3C :    // <
        case 0x3D :    // =
        case 0x3E :    // >
        case 0x3F :    // ?
        case 0x40 :    // @
          symbol_type = CppToken::colon + (point - 0x3A); break;
        case 0x5B :    // [
        case 0x5C :    // backslash
        case 0x5D :    // ]
        case 0x5E :    // ^
          symbol_type = CppToken::open_sqr_bracket + (point - 0x5B); break;
        case 0x60 :    // `
          symbol_type = CppToken::tilde; break;
        case 0x7B :    // {
        case 0x7C :    // |
        case 0x7D :    // }
        case 0x7E :    // ~
          symbol_type = CppToken::open_cur_bracket + (point - 0x7B); break;
        default:
          // A-Z a-z 0-9 and _ (which is 0x5F)
          symbol_type = 0;
      };  // switch

      if (symbol_type == 0) {
         if (!str)
            str = curr;
      } else {
        if (str) {
          auto r = Range<char>(str, curr);
          int scol = column - static_cast<int>(r.Size());
          tv.push_back(CppToken(r, CppToken::string, line, scol));
          str = nullptr;
        }
        tv.push_back(CppToken(Range<char>(curr, curr + 1), 
                              static_cast<CppToken::Type>(symbol_type),
                              line, column));
      }
    }

    c = range.Next(curr);
    ++column;
  } while (curr);

  // Insert a final token to simplify further processing.
  tv.push_back(CppToken(Range<char>(curr, curr), CppToken::eos, line + 1, 0));
  return tv;
}

bool IsCppTokenNextTo(CppTokenVector::iterator it) {
  const CppToken& next = *(it + 1);
  return (it->range.End() == next.range.Start());
}

bool IsCppTokenNextTo(CppTokenVector::iterator it, CppToken::Type type) {
  const CppToken& next = *(it + 1);
  if (next.type != type)
    return false;
  return IsCppTokenNextTo(it);
}

bool IsCppTokenChar(CppTokenVector::iterator it, char c) {
  return ((it->range.Size() == 1) && (*it->range.Start() == c));
}

void CoaleseToken(CppTokenVector::iterator first,
                  CppTokenVector::iterator last,
                  CppToken::Type type) {
  first->type = type;
  first->range = Range<char>(first->range.Start(),
                                last->range.End());
}

#pragma endregion

enum LexMode {
  PlainCPP,
  PlexCPP,
};

bool LexCppTokens(LexMode mode, CppTokenVector& tokens) {
  auto it = tokens.begin();
  if (it->type != CppToken::sos)
    throw PlexException(__LINE__, "No SOS in tokens stream");

  KeyElements& kelem = *(it->kelems);
  std::vector<ScopeBlock> scopes;
  auto path = kelem.src_path.Raw();

  size_t last_include_pos = 0;

  while (it != tokens.end()) {
    if ((it->type > CppToken::symbols_begin ) && (it->type < CppToken::symbols_end)) {
      switch (it->type) {
        case CppToken::double_quote : {
          // Handle coalesing all tokens inside a string.
          auto it2 = it + 1;

          if (IsCppTokenChar(it - 1, 'R') && (it + 1)->type == CppToken::open_paren) {
            // c++11 raw string, basic case only.
            while (it2->type != CppToken::close_paren) {
              if (it2->type == CppToken::eos)
                throw TokenizerException(path, __LINE__, it2->line);
              ++it2;
            }
            if ((++it2)->type != CppToken::double_quote)
              throw TokenizerException(path, __LINE__, it2->line);
          } else {
            // classic c string.
            while (it2->type != CppToken::double_quote) {
              if (it2->type == CppToken::eos)
                throw TokenizerException(path, __LINE__, it2->line);

              if (it2->type == CppToken::backlash) 
                ++it2;
              ++it2; 
            }
          }
          CoaleseToken(it, it2, CppToken::const_str);
          tokens.erase(it + 1, it2 + 1);
          // Handle size qualifier 'L'.
          if (IsCppTokenChar(it - 1, 'L') && IsCppTokenNextTo(it - 1)) {
            it2 = it;
            --it;
            CoaleseToken(it, it2, CppToken::const_wstr);
            tokens.erase(it2);
          }

        }
        break;
        case CppToken::single_quote : {
          // Handle coalesing all tokens inside a string.
          auto it2 = it + 1;
          while (*it2->range.Start() != '\'') {
            if (it2->line != it->line)
              throw TokenizerException(path, __LINE__, it->line);
            if (*it2->range.Start() == '\\')
              ++it2;
            ++it2; 
          }
          CoaleseToken(it, it2, CppToken::const_char);
          tokens.erase(it + 1, it2 + 1);  
          // Handle size qualifier 'L'.
          if (IsCppTokenChar(it - 1, 'L') && IsCppTokenNextTo(it - 1)) {
            it2 = it;
            --it;
            CoaleseToken(it, it2, CppToken::const_wchar);
            tokens.erase(it2);
          }
        }
        break;
        case CppToken::hash : {
          // Handle coalesing all tokens in a preprocessor or pragma line.
          if ((it - 1)->line == it->line) {
            // can't have tokens before a # in the same line.
            throw TokenizerException(path, __LINE__, it->line);
          }
          
          // Next token is the kind.
          auto it2 = it + 1;
          auto pp_type = GetCppPreprocessorKeyword(it2->range);
          if (pp_type == CppToken::unknown) {
            // unrecongized preprocessor directive;
            throw TokenizerException(path, __LINE__, it->line);
          }
          int count = 0;
          while (it2->line == it->line) {
            ++it2; ++count;
          }
          // $$$ need to handle the 'null directive' which is a # in a single line.         
          if (pp_type == CppToken::prep_pragma) {
            auto pit = it + 2;
            if (EqualToStr(*pit, "comment")) {
              do {
                // Try to parse "comment(user, "x.y=z")". This creates a map[y] = z
                // entry in KeyElements for global plex properties (directives).
                if ((++pit)->type != CppToken::open_paren) break;
                if (!EqualToStr(*(++pit), "user")) break;
                if ((++pit)->type != CppToken::comma) break;
                if ((++pit)->type != CppToken::double_quote) break;
                if (!EqualToStr(*(++pit), "plex")) break;
                if ((++pit)->type != CppToken::period) break;
                if ((++pit)->type != CppToken::string) break;
                std::string key(ToString(*pit));
                if ((++pit)->type != CppToken::equal) break;
                if ((++pit)->type != CppToken::string) break;
                kelem.properties[key].push_back(ToString(*pit));
                if ((++pit)->type != CppToken::double_quote)
                  throw TokenizerException(path, __LINE__, it->line);
              } while (false);
            }
          } else if (pp_type == CppToken::prep_include) {
            if (count < 4)
              throw TokenizerException(path, __LINE__, it->line);
            //Includes go into a special map.
            Range<char> irange((it + 2)->range.Start(), (it2 - 1)->range.End());
            if ((irange[0] != '"') && (irange[0] != '<'))
              throw TokenizerException(path, __LINE__, it->line);
            auto item_pos = it - tokens.begin();
            if (kelem.includes.empty())
              kelem.includes[Range<char>(first_include_key)] = item_pos;
            
            last_include_pos = item_pos;
            kelem.includes[irange] = item_pos;
            
          } else if (pp_type == CppToken::prep_if) {
            auto if_it = it + 2;
            if (EqualToStr(*if_it, "0")) {
              // Found an #if 0. Find the matching #endif.
              int if_count = 1;
              ++if_it;
              while ((if_it->type != CppToken::eos) && (if_count > 0)) {
                if (EqualToStr(*if_it, "endif") && (if_it -1)->type == CppToken::hash)
                  --if_count;
                else if (EqualToStr(*if_it, "else") && (if_it -1)->type == CppToken::hash)
                  --if_count;
                else if (EqualToStr(*if_it, "if") && (if_it -1)->type == CppToken::hash)
                  ++if_count;
                else if (EqualToStr(*if_it, "ifdef") && (if_it -1)->type == CppToken::hash)
                  ++if_count;
                else if (EqualToStr(*if_it, "ifndef") && (if_it -1)->type == CppToken::hash)
                  ++if_count;
                ++if_it;
              }
              if (if_count != 0)
                throw TokenizerException(path, __LINE__, it->line);
              // Erase all the tokens by marking them as 'none' type.
              it2 = if_it;
              pp_type = CppToken::none;
            }  // #if 0.
          }

          CoaleseToken(it, it2 - 1, pp_type);
          tokens.erase(it + 1, it2);

        }
        break;
        case CppToken::open_cur_bracket: {
          size_t pos = (it - tokens.begin());
          if (pos < 3)
            continue;
          auto it2 = it - 1;
          auto block_type = ScopeBlock::block_other;
          Range<char> name;

          if (it2->type == CppToken::kw_namespace) {
            block_type = ScopeBlock::anons_namespace;
          } else if (it2->type == CppToken::semicolon) {
            block_type = ScopeBlock::block_scope;
          } else if (it2->type == CppToken::identifier) {
            auto it3 = it2 - 1;
            name = it2->range;
            if (it3->type == CppToken::kw_namespace)
              block_type = ScopeBlock::named_namespace;
            else if ((it3->type == CppToken::kw_class)   ||
                     (it3->type == CppToken::kw_public)  ||
                     (it3->type == CppToken::kw_private) ||
                     (it3->type == CppToken::kw_struct)  ||
                     (it3->type == CppToken::kw_union))
              block_type = ScopeBlock::block_aggregate;
            else if (it3->type == CppToken::kw_enum)
              block_type = ScopeBlock::block_enum; //$$ handle enum class.
            else
              throw TokenizerException(path, __LINE__, it->line);
          }
          auto top = scopes.empty() ? 0 : scopes.back().start;
          scopes.push_back(ScopeBlock(block_type, name, pos, top));
        }
        break;
        case CppToken::close_cur_bracket : {
          if (scopes.empty())
            throw TokenizerException(path, __LINE__, it->line);
          auto cs = scopes.back();
          cs.end = (it - tokens.begin());
          kelem.scopes.push_back(cs);
          scopes.pop_back();
        }
        break;

        default : {
          // Handle the two char token case
          auto tt_type = GetTwoTokenType(*it, *(it + 1));
          if (tt_type != CppToken::unknown) {
            CoaleseToken(it, it + 1, tt_type);
            tokens.erase(it + 1);

            if (tt_type == CppToken::line_comment) {
              auto it2 = it + 1;
              while (it2->line == it->line) { ++it2; }
              CoaleseToken(it, it2 -1, CppToken::comment);
              tokens.erase(it + 1, it2);
              if (mode == LexMode::PlexCPP) {
                if (it->range.Size() > 3) {
                  if ((it->range[2] == '#') && (it->range[3] == '~'))
                    it->type = CppToken::plex_comment;
                }
              }
            } else {
              // Here we handle the two three-char cases: >>= and <<=.
              if (IsCppTokenNextTo(it, CppToken::eq)) {
                if (it->type == CppToken::left_shift) {
                  CoaleseToken(it, it + 1, CppToken::ass_left_shift);
                  tokens.erase(it + 1);
                } else if (it->type == CppToken::right_shift) {
                  CoaleseToken(it, it + 1, CppToken::ass_right_shift);
                  tokens.erase(it + 1);
                }
              }
            }
          }
        }
      }  // switch
    } else if (it->type == CppToken::string) {
      CppToken::Type type = GetCppKeywordType(it->range);
      if (type != CppToken::unknown) {
        it->type = type;
      } else {
        const char c = *it->range.Start();
        if ((c >= '0') && (c <= '9')) {
          // possible numeric constant. Being it relatively rare we can use iostream.
          std::string number(ToString(*it));
          std::istringstream ss(number);
          long long value;
          ss >> value;
          if (!ss)
            throw TokenizerException(path, __LINE__, it->line);
          size_t np = ss.tellg();
          if (np != ~0ULL) {
            // failed to fully consume the number. See if we have a
            // base or float or size specificator.
            switch (number[np]) {
              case 'e' :
              case 'E' : {
                // Mantissa + exponent form. 'e' must be the last char.
                if ((np + 1) != number.size())
                  throw TokenizerException(path, __LINE__, it->line);
              }
              break;
              case 'x' :
              case 'X' : {
                // Hex number. Verify that iostream can consume it fully.
                ss.seekg(0);
                ss >> std::hex >> value;
                if (!ss)
                  throw TokenizerException(path, __LINE__, it->line);
                np = ss.tellg();
                if ((np > 1) && (np != ~0ULL)) {
                  switch (number[np]) {
                    case 'l':
                    case 'u':
                    case 'L':
                    case 'U':
                      break;
                    default:
                      throw TokenizerException(path, __LINE__, it->line);
                  }
                }
              }
              break;
              // $$$ handle better the f, l ul u and ll cases.
              case 'F' :
              case 'f' : break;
              case 'l' :
              case 'L' : break;
              case 'u' :
              case 'U' : break;
              default:
                throw TokenizerException(path, __LINE__, it->line);
            }
          }
          // ok, it seems to be a number. See if we can coalease with + - or .
          if ((it - 1)->type == CppToken::minus ||
              (it - 1)->type == CppToken::plus ||
              (it - 1)->type == CppToken::period) {
            it = it - 1;
            CoaleseToken(it, it + 1, CppToken::const_number);
            tokens.erase(it + 1);
          } else {
            it->type = CppToken::const_number;
          }
          // If the previous token was a number, then we can coalese it as well
          if ((it - 1)->type == CppToken::const_number) {
            it = it - 1;
            CoaleseToken(it, it + 1, CppToken::const_number);
            tokens.erase(it + 1);
          }

        } else if (::isalpha(c) || (c == '_')) {
          if (IsPredefinedMacro(it->range)) {
            it->type = CppToken::predef_macro;

          } else {
            // identifier.
            it->type = CppToken::identifier;

            // handle the case of a qualified name, first the global scope.
            if ((it - 1)->type == CppToken::name_scope) {
              it = it - 1;
              CoaleseToken(it, it + 1, CppToken::identifier);
              tokens.erase(it + 1);
              // and secondly the fully qualified case.
              if ((it - 1)->type == CppToken::identifier) {
                it = it - 1;
                CoaleseToken(it, it + 1, CppToken::identifier);
                tokens.erase(it + 1);
              }
            }
          }

        } else {
          throw TokenizerException(path, __LINE__, it->line);
        }
      }
    } else if (it->type == CppToken::sos) {
      // first token.
    } else if (it->type == CppToken::eos) {
      if (last_include_pos)
        kelem.includes[Range<char>(last_include_key)] = last_include_pos;
      // The scopes vector should be empty or else we have an unbalanced "{".
      if (!scopes.empty())
        throw TokenizerException(path, __LINE__, 0);
      return true;
    } else {
      throw TokenizerException(path, __LINE__, it->line);
    }
    // advance to next token.
    ++it;
  }

  throw TokenizerException(path, __LINE__, 0);
}

#pragma region xdef
struct XEntity;

struct XternDef {
  enum Type {
    none,
    include,
    item,
  };

  Type type;
  Range<char> name;
  Range<char> path;
  XEntity* entity;

  XternDef(Type type, const Range<char>& name, const Range<char>& path)
      : type(type),
        name(name), path(path),
        entity(nullptr) {
  }

  XternDef()
      : type(none), entity(nullptr) {
  }
};

struct XEntity {
  Range<char> name;
  XternDef::Type type;
  CppTokenVector* tv;
  std::vector<XEntity*> deps;

  XEntity(XternDef& def, CppTokenVector* tv)
    : name(def.name), type(def.type), tv(tv) {
  }

  // The processing order is the Type order.
  bool Order(const XEntity& other) const {
    if (type == other.type)
      return ToString(name) < ToString(other.name);
    else 
      return type < other.type;
  }
};

typedef std::unordered_map<std::string, XternDef> XternDefs;

int GetExternalDefinitions(CppTokenVector& tv,
                           XternDefs& xdefs,
                           XEntity* entity = nullptr) {

  auto IsBuiltIn = [](int t) -> bool {
    return (
      t == CppToken::kw_void ||
      t == CppToken::kw_int ||
      t == CppToken::kw_long ||
      t == CppToken::kw_char ||
      t == CppToken::kw_bool ||
      t == CppToken::kw_float ||
      t == CppToken::kw_wchar_t ||
      t == CppToken::kw_signed ||
      t == CppToken::kw_unsigned ||
      t == CppToken::kw_double);
  };

  auto IsAgregateIntroducer = [](int t) -> bool {
    return (
      t == CppToken::kw_class ||
      t == CppToken::kw_struct ||
      t == CppToken::kw_union);
  };

  auto IsModifier = [](int t) -> bool {
    return (
      t == CppToken::asterisc ||
      t == CppToken::ampersand ||
      t == CppToken::kw_const ||
      t == CppToken::kw_signed ||
      t == CppToken::kw_unsigned ||
      t == CppToken::kw_volatile);
  };

  auto IsInVector = [](const CppTokenVector& v,
                       const Range<const char>& r) -> bool {
    for (auto it = begin(v); it != end(v); ++it) {
      if (EqualToStr(*it, r))
        return true;
    }
    return false;
  };

  auto path = tv[0].kelems->src_path;

  // Here we assume that the code file consists of a series
  // of statement that can be definitions or declarations.
  // Inside definitions we can have more definitions or 
  // expression statements.

  // Local definitions.
  CppTokenVector ldefs;
  // Local references, they don't have a visible definition.
  CppTokenVector xrefs;
  // Local variables, they reset at each scope. Note that other
  // things can end up here, for example function definitions.
  std::vector<CppTokenVector> lvars;

  // adding the global scope.
  lvars.push_back(CppTokenVector());

  std::vector<const char*> enclosing_definition;
  bool in_local_definition = false;

  int new_xdefs = 0;

  // Skip over SOS token.
  auto last = ++begin(tv);

  for(auto it = last; it->type != CppToken::eos; ++it) {
    auto prev = *(it - 1);
    auto next = *(it + 1);

    if (it->type == CppToken::comment)
      continue;

    if (it->type == CppToken::open_cur_bracket) {
      lvars.push_back(CppTokenVector());

      if (prev.type != CppToken::kw_namespace) {
        if (in_local_definition)
          in_local_definition = false;
        else 
          enclosing_definition.push_back(scope_namespace_mk);
      }
      continue;

    } else if (it->type == CppToken::close_cur_bracket) {
      lvars.pop_back();

      if (!enclosing_definition.empty()) {
        enclosing_definition.pop_back();
      }
      continue;
      
    } else if (it->type == CppToken::identifier) {
      if (prev.type == CppToken::kw_namespace) {
        if (next.type != CppToken::open_cur_bracket)
          __debugbreak();
        lvars.push_back(CppTokenVector());
        ++it;
        continue;

      } else if (IsAgregateIntroducer(prev.type)) {
        if (next.type == CppToken::semicolon) {
          if (IsInVector(ldefs, it->range))
            __debugbreak();
          // forward declaration.
          ldefs.push_back(*it);
          continue;
        }
        // local definition.
        enclosing_definition.push_back(it->range.Start());
        ldefs.push_back(*it);
        in_local_definition = true;
        continue;
      }

      if (!IsInVector(ldefs, it->range)) {

        if(IsInVector(xrefs, it->range)) {
          continue;
        }

        if (prev.type == CppToken::period)
          continue;

        // could be variable declaration. Scan backwards.
        bool is_var_decl = false;
        auto rit = it;
        while(--rit != begin(tv)) {
          if (IsBuiltIn(rit->type)) {
            is_var_decl = true;
            break;
          }
          if (rit->type == CppToken::identifier) {
            is_var_decl = true;
            break;
          }
          if (!IsModifier(rit->type))
            break;
        }

        if (is_var_decl) {
          lvars.back().push_back(*it);
          continue;
        } else {
          // check in our stack of local vars.
          bool is_var_use = false;
          for (auto ix = begin(lvars); ix != end(lvars); ++ix) {
            for (auto iy = begin(*ix); iy != end(*ix); ++iy) {
              if (it->range.Equal(iy->range)) {
                is_var_use = true;
                break;
              }  
            }
          }
          if (is_var_use)
            continue;
        }

        // possible external reference, need to check in db.
        auto xdit = xdefs.find(ToString(*it));
        if ( xdit!= xdefs.end()) {
          // reference found.
          auto& found_xdef = xdit->second;
          if (!found_xdef.entity) {
            ++new_xdefs;
            found_xdef.entity = new XEntity(found_xdef, nullptr);
            Logger::Get().AddExternDef(found_xdef.name, it->line);
          }
          if (entity) {
            if (found_xdef.type != XternDef::include) {
              if (found_xdef.entity == entity) {
                // self reference, probably full name in the same file. Could ignore
                // it but best to have the user to fix the reference.
                throw TokenizerException(path.Raw(), __LINE__, it->line);
              }
              entity->deps.push_back(found_xdef.entity);
            }
          }
          xrefs.push_back(*it);
          continue;
        }

        // Many things can end up here for example method invocations
        // on the enclosing aggregate.

      }  // not seen before.

    }  // identifier.
      
  }  // for.

#if 0
    if (!enclosing_definition.empty())
      __debugbreak();
    if (lvars.size() != 1)
      __debugbreak();
#endif
  return new_xdefs;
}

#pragma endregion

#pragma region catalog

void ProcessCatalog(CppTokenVector& tv, XternDefs& defs) {
  auto it = begin(tv);
  auto kind = XternDef::none;

  for (;;) {
    // find block.
    for( ; it->type != CppToken::open_cur_bracket; ++it) {
      if (it->type == CppToken::eos) {
        if (defs.empty())
          throw CatalogException(__LINE__, 0);
        else
          return;
      }  
    }
    // find block kind.
    if (EqualToStr(*(it - 2), "include")) {
      kind = XternDef::include;
    } else if (EqualToStr(*(it - 2), "catalog")) {
      kind = XternDef::item;
    } else {
      throw CatalogException(__LINE__, it->line);
    }
    
    ++it;
    // process block.    
    for( ; it->type != CppToken::close_cur_bracket; ++it) {
      if (it->type != CppToken::identifier)
        throw CatalogException(__LINE__, it->line);
      auto key = it;
      ++it;
      auto val = it;
      for (; it->type != CppToken::semicolon; ++it) {
        if (it->type ==  CppToken::eos)
          throw CatalogException(__LINE__, it->line);
      }
      // insert one entry.
      CoaleseToken(val, it - 1, CppToken::const_str);
      it = tv.erase(val + 1, it);
      defs[ToString(*key)] = XternDef(kind, key->range, val->range);
    }
    ++it;
  }

}

#pragma endregion

struct XInclude {
  Range<char> name;
  Range<char> src;
  // |insert| is only here to keep alive the memory backing
  // the MemRange on the final tree.
  std::string insert;

  XInclude(XternDef& def) : name(def.name), src(def.path) {}

  // The processing order is alphabetic.
  bool Order(const XInclude& other) const {
    return ToString(name) < ToString(other.name);
  }
};

struct XEntities {
  std::deque<XInclude> includes;
  std::deque<XEntity*> code;

  void Add_Front(const XEntities& other) {
    includes.insert(begin(includes), begin(other.includes), end(other.includes));
    code.insert(begin(code), begin(other.code), end(other.code));
  }
};

XEntities LoadEntities(XternDefs& xdefs, const FilePath& path) {
  XEntities ents;
  for (auto it = begin(xdefs); it != end(xdefs); ++it) {
    auto& def = it->second;
    if (!def.entity)
      continue;
    if (def.entity->tv)
      continue;
    // Load, tokenize and lex.
    if (it->second.type == XternDef::include) {
      ents.includes.push_back(XInclude(def));
    } else {
      auto tok = new CppTokenVector(TokenizeCpp(path.Append(AsciiToUTF16(def.path))));
      LexCppTokens(LexMode::PlexCPP, *tok);
      def.entity->tv = tok;
      ents.code.push_back(def.entity);
      // Get external definitions and create/insert the new xentity.
      if (GetExternalDefinitions(*tok, xdefs, def.entity)) {
        // Recurse now.
        auto inner = LoadEntities(xdefs, path);
        ents.Add_Front(inner);
      }
    }
  }

  return ents;
}

void InsertAtToken(CppToken& src, Insert::Kind kind, CppTokenVector& tv) {
  if (!src.insert)
    src.insert = new Insert(kind);
  // Minimal insertion has two tokens: SOS + tv[0].
  auto start = tv[0].range.Start();
  CppToken control(Range<char>(start, start), CppToken::plex_insert, 0, 0);
  if (tv.size() == 1) {
    tv.insert(begin(tv), control);
  } else if (tv[0].type == CppToken::sos) {
    // replace SOS for the insert token.
    tv[0] = control;
  } else {
    throw PlexException(__LINE__, "Missing SOS token");
  }

  // Insert most tokens and renumber the lines as we go along.
  auto& exv = src.insert->tv;
  int b_line = exv.empty() ? src.line : exv[exv.size()-1].line;
  for (auto& tok : tv) {
    auto t = tok.type;
    if (t == CppToken::eos || t == CppToken::plex_comment)
      continue;
    tok.line += b_line;
    src.insert->tv.push_back(tok);
  }
}

void ProcessEntities(CppTokenVector& in_src, XEntities& ent) {
  std::sort(begin(ent.includes), end(ent.includes), 
      [] (const XInclude& e1, const XInclude& e2) {
        return e1.Order(e2);
      }
  );

  std::sort(begin(ent.code), end(ent.code), 
      [] (const XEntity* e1, const XEntity* e2) {
        return e1->Order(*e2);
      }
  );

  // Lazy attempt to re-order the code if the dependency order
  // is wrong. Easy to detect but hard to fix completely. We try
  // several times exchanging the order.
  if (ent.code.size() > 1) {
    int tries = 14;
    bool go_again = true;
    while ((--tries != 0) && go_again) {
      go_again = false;
      for (auto ix = begin(ent.code); ix != end(ent.code); ++ix) {
        for (auto& d : (*ix)->deps) {
          auto xd = std::find(ix + 1, end(ent.code), d);
          if (xd != end(ent.code)) {
            // Sort order is wrong, a dependency is below.
            std::swap(*ix, *xd);
            go_again = true;
            goto leave;
          }
        }
      }
      leave: ;
    }  // while.

    if (go_again) {
      // we could not find an arrangement of code inserts that works.
      throw PlexException(__LINE__, "dependency conflict");
    }
  }

  // Insert includes after the first include.
  auto& kel = *in_src[0].kelems;
  auto fik = kel.includes.find(Range<char>(first_include_key));
  const auto pos_include = (fik != end(kel.includes)) ? fik->second : 1;

  for (auto& incl : ent.includes) {
    auto it = kel.includes.find(incl.src);
    if (it != end(kel.includes))
      continue;
    // Include not found in source. We currently insert after the first include.
    Logger::Get().ProcessInclude(incl.src);

    incl.insert = std::string("#include ") + ToString(incl.src);
    CppToken newtoken(FromString(incl.insert), CppToken::prep_include, 1, 1);
    CppTokenVector itv = {newtoken};
    InsertAtToken(in_src[pos_include], Insert::keep_original, itv);
    // insert in kels to avoid a second include of the same.
    kel.includes[incl.src] = pos_include;
  }

  // collect all the plex metadata from #pragma annotations of all dependents.
  for (auto& e : ent.code) {
    auto x = e->tv->at(0).kelems;
    if (!x)
      continue;
    for (auto& p : x->properties) {
      auto& kp = kel.properties[p.first];
      kp.insert(end(kp), begin(p.second), end(p.second));
    }
  }

  // Insert the integer definitions resulting from "plex.define" pragma.
  auto idefs = kel.properties.find("define");
  if (idefs != end(kel.properties)) {
    for (auto& idef : idefs->second) {
      idef = "const int " + idef + " = 1;";
      auto idr = FromString(idef);
      auto idr_tok = TokenizeCpp(FilePath(L"@define@"), &idr);
      InsertAtToken(in_src[pos_include], Insert::keep_original, idr_tok);
    }
  }

  // Insert code after the integer definitions.
  auto lik = kel.includes.find(Range<char>(last_include_key));
  const auto pos_code = (lik != end(kel.includes)) ? lik->second : 1;

  Range<char> curr_namespace;

  for (auto& cod : ent.code) {
    Logger::Get().ProcessCode(cod->name);
    auto& scopes = (*cod->tv)[0].kelems->scopes;

    auto& top_scope = scopes.back();
    if (top_scope.type == ScopeBlock::named_namespace) {
      if ((curr_namespace.Size() == 0) || 
          (!curr_namespace.Equal(top_scope.name))) {
        // Start tracking new top namespace.
        curr_namespace = top_scope.name;
      } else {
        // Insert has the same namespace as the previous one, we
        // need to collapse them, by removing previous insert.
        auto& exv = in_src[pos_code].insert->tv;
        auto revit = rbegin(exv);
        while (revit != rend(exv)) {
          if (revit->type == CppToken::close_cur_bracket) {
            revit->insert = Insert::TokenDeleter();
            break;
          }
        }
        // Then remove the namespace from this insert. Which means
        // the tree tokens "namespace xxx {".
        size_t rp = top_scope.start;
        (*cod->tv)[rp--].insert = Insert::TokenDeleter();
        (*cod->tv)[rp--].insert = Insert::TokenDeleter();
        (*cod->tv)[rp].insert = Insert::TokenDeleter();

      }
    } else {
      // No top level namespace.
      curr_namespace.Reset();
    }

    InsertAtToken(in_src[pos_code], Insert::keep_original, *cod->tv);
  }

}

int CountInnerLFs(Range<char> range) {
  if (!range.Size())
    return 0;

  int count = 0; 
  auto r = range.Start();
  while(r) {
    if (*r == '\n')
      ++count;
    range.Next(r);
  }
  return count;
}

void WriteOutputFile(File& file, CppTokenVector& src, bool top = true) {
  int line = 1;
  size_t column = 1;

  for (auto it = begin(src); it != end(src); ++it) {
    if (!it->col)
      continue;

    if (it->type == CppToken::none) {
      line = it->line;
      continue;
    }

    std::string out;
    int ldiff = it->line - line;

    size_t cdiff =  ldiff ? it->col - 1 : it->col - column;

    if ((ldiff > 2) && (it->line > 1)) {
      int innlf = CountInnerLFs((it-1)->range);
      if (innlf) {
        ldiff = ldiff - innlf;
        cdiff = 0;
      }
    }
    
    out.append(ldiff, '\n');
    out.append(cdiff, ' ');
    out.append(ToString(it->range));

    if (it->insert) {
      if (it->insert->kind == Insert::keep_original) {
        out.append(1, '\n');
        file.Write(FromString(out));
      }
      WriteOutputFile(file, it->insert->tv, false);
    }
    else {
      file.Write(FromString(out));
    }

    line = it->line;
    column = it->col + it->range.Size();
  }
  if (top)
    file.Write("\n");
}

#pragma region testing

void DumpTokens(const CppTokenVector& src, std::ostream& oss);

std::string DumpName(const CppToken& tok) {
  return (tok.range.Size() ? ToString(tok.range) : std::string("<nullstr>"));
}

void DumpToken(const CppToken& tok, std::ostream& oss) {
  oss << std::setw(4) << tok.line << " {" << std::setw(3) << tok.type << "} ";
  if (tok.col) {
    oss << std::string(tok.col - 1, '_') << " " << DumpName(tok);
  }
  else {
    oss << "<ctrl>";
  }
  oss << std::endl;
}

void DumpInsert(const CppToken& tok, std::ostream& oss) {
  Insert& ins = *tok.insert;
  if (ins.kind == Insert::delete_original) {
    oss << "<deleted>" << " {" << std::setw(3) << tok.type << "} ";
    oss << DumpName(tok) << std::endl;
  } else {
    DumpToken(tok, oss);
  }
  if (ins.tv.empty())
    return;
  oss << "<+insert> count: " << ins.tv.size() << std::endl;
  DumpTokens(ins.tv, oss);
  oss << "<~insert>\n";
}

void DumpTokens(const CppTokenVector& src, std::ostream& oss) {
  int item_no = 0;
  for (auto it = begin(src); it != end(src); ++it, ++item_no) {
    oss << std::setw(4) << item_no << ":";
    if (it->type == CppToken::sos)
      oss << "[SOS]\n";
    else if (it->type == CppToken::eos)
      oss << "[EOS]\n";
    else if (it->type == CppToken::none)
      oss << "[NONE] (" << it->range.Size() << " chars elided)\n";
    else {
      if (it->insert)
        DumpInsert(*it, oss);
      else
        DumpToken(*it, oss);
    }
  }
}

void DumpPropertyArray(const std::vector<std::string>& vos, std::ostream& oss) {
  for (auto& e : vos) {
    oss << e << ", ";
  }
  oss << std::endl;
}

void DumpKeyElements(const KeyElements& kel, std::ostream& oss) {
  if (kel.properties.size())
    oss << "properties: " << kel.properties.size() << std::endl;
  for (auto& e : kel.properties) {
    oss << "  +" << e.first << " = ";
    DumpPropertyArray(e.second, oss);
  }
}

void GenerateDump(File& file, CppTokenVector& src) {
  std::ostringstream oss;
  oss << "Plex Dump Version 001" << std::endl;
  oss << "token count: " << src.size() << std::endl;
  DumpTokens(src, oss);
  DumpKeyElements(*src[0].kelems, oss);
  file.Write(FromString(oss.str()));
}

#pragma endregion

enum OpMode {
  None       = 0,
  TreeDump   = 1,
  Generate   = 1 << 1,
};

File MakeOutputCodeFile(const FilePath& out_path, std::wstring name) {
  auto probe_path = out_path.Append(name);
  FilePath output_path = probe_path.Exists() ?
      out_path.Append(L"g_" + name) :
      probe_path;
  return File::Create(output_path,
                      FileParams::ReadWriteSharedRead(CREATE_ALWAYS),
                      FileSecurity());
}

File MakeTestDumpFile(const FilePath& out_path, std::wstring name) {
  auto output_path = out_path.Append(name + L".dmp");
  return File::Create(output_path,
                      FileParams::ReadWriteSharedRead(CREATE_ALWAYS),
                      FileSecurity());
}

// ################################################################################################
// #   main() entrypoint                                                                          #
// #   plex.exe [options] cpp_file                                                                #
// ################################################################################################

int wmain(int argc, wchar_t* argv[]) {
  CmdLine cmdline(argc, argv);

  int op_mode = None;
  if (cmdline.HasSwitch("dump-tree")) op_mode += TreeDump;
  if (cmdline.HasSwitch("generate")) op_mode += Generate;

  if (op_mode == None) {
    printf("plex by carlos.pizano@gmail.com. Version "__DATE__"\n");
    wprintf(L"usage: plex.exe options cc_file\n");
    wprintf(L"options:  --dump-tree and|or --generate\n");
    return 0;
  }

  try {
    // Input file, typically a c++ file. The catalog index is also an implicit input.
    FilePath path(cmdline.Extra(0));
    FilePath catalog = FilePath(path.Parent()).Append(L"catalog\\index.plex");

    auto out_path_str = AsciiToUTF16(cmdline.Value("out-dir"));
    if (!out_path_str.empty()) {
      if (!::CreateDirectoryW(out_path_str.c_str(), NULL)) {
        if (::GetLastError() != ERROR_ALREADY_EXISTS) {
          wprintf(L"unable to create output directory\n");
          return 1;
        }
      }
    } else {
      out_path_str = path.Parent().Raw();
    }

    FilePath out_path(out_path_str);
    Logger logger(path.Parent().Append(L"plex_log.txt"));

    // Phase 1 : process the input cc.
    CppTokenVector cc_tv = TokenizeCpp(path);
    LexCppTokens(LexMode::PlainCPP, cc_tv);

    // Phase 2 : process the catalog.
    CppTokenVector index_tv = TokenizeCpp(catalog);
    LexCppTokens(LexMode::PlexCPP, index_tv);
    XternDefs xdefs;
    ProcessCatalog(index_tv, xdefs);

    // Phase 3: find and resolve the needed catalog entities.
    GetExternalDefinitions(cc_tv, xdefs);
    XEntities entities = 
        LoadEntities(xdefs, catalog.Parent());
    // Phase 4: process each entity augmenting the source.
    ProcessEntities(cc_tv, entities);

    // Phase 5: Generate the outputs.
    if (op_mode & Generate) {
      File output_cc = MakeOutputCodeFile(out_path, path.Leaf());
      WriteOutputFile(output_cc, cc_tv);
    }

    if (op_mode & TreeDump) {  
      File test_dump = MakeTestDumpFile(out_path, path.Leaf());
      GenerateDump(test_dump, cc_tv);
    }

    return 0;

  } catch (TokenizerException& ex) {
    wprintf(L"\nerror: [%s] Tokenizer error\n"
            L"in source line %d file [%s]\n"
            L"in program line %d, version (%S)\n",
            cmdline.Extra(0).c_str(),
            ex.SourceLine(), ex.Path(),
            ex.Line(), __DATE__);

    if (Logger::HasLogger())
      Logger::Get().ReportException(ex);
  
  } catch (IOException& ex) {
    wprintf(L"error: can't open file [%s] (line %d)\n", ex.Name(), ex.Line());

    if (Logger::HasLogger())
      Logger::Get().ReportException(ex);

  } catch (PlexException& ex) {
    wprintf(L"\nerror: [%s] fatal exception [%S]\n"
            L"in program line %d, version (%S)\n",
            cmdline.Extra(0).c_str(), ex.Message(), ex.Line(), __DATE__);

    if (Logger::HasLogger())
      Logger::Get().ReportException(ex);
  }

  return 2;
}
