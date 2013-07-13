// plex.cpp : This is the plex lexer. It implements a funky c++ parser designed
// to be halfway between a c preprocessor and a real c++ lexer. Its output is a
// array of tokens of the form {type, line, range} where range is two pointers
// to the start and end of the token in the source file.
// Unlike a c preprocessor, nor macro or includes are processed, and tokens are
// tailored for source code transformation rather than code generation. For example
// a c++ comment '// blah blah' is represented as a single token and so are the
// following:   1) L"nikita is alive" 2) .999e+3 3) #if defined FOO
//
// The purpose behind it is to relieve the average programer from tedious tasks
// like #includes and to allow expressing code that today requires bespoke tools
// and auxiliary files in json or xml.
//
// BUGS to be fixed short term
// ---------------------------
// 001 coalese strings "aaa""bb""cc" even in different lines.
// 002 learn to ignore #if 0.
// 006 make 7 test files that run in the harness.
// 007 handle comments at the end of preprocessor lines.
// 015 recognize -> as a token.
// 016 have a test with printfs.
// 017 find definitions
// 018 remove definition names
// 019 coalease templated types in the name like Moo<int> moo;
// 020 handle namespace alias 'namespace foo = bar::doo'
//
// Longer term
// ---------------------------
// 101 define and load catalog information.
// 102 include referenced entities directly.
// 103 git or github integration .

#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>

#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#pragma region constants
const char* anonymous_namespace_mk = "<[anonymous]>";
const char* scope_namespace_mk = "<[scope]>";
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

public:
  IOException(int line)
      : PlexException(line, "IO problem"), error_code_(::GetLastError()) {
    PostCtor();
  }
  DWORD ErrorCode() const { return error_code_; }
};

class TokenizerException : public PlexException {
  int source_line_;

public:
  TokenizerException(int line, int source_line)
      : PlexException(line, "Tokenizer problem"), source_line_(source_line)  {
    PostCtor();
  }
  int SourceLine() const { return source_line_; }
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

template <typename T>
class MemRange {
private:
  T* start_;
  T* end_;

  bool plex_check_init() {
    return (start_ <= end_);
  }
  
  friend class LineIterator;

public:
  MemRange(T* start,
           T* end)
    : start_(start),
      end_(end) {
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

  T Next(T*& curr) const {
    if (curr > end_) {
      curr = nullptr;
      return T(0);
    }
    return *(++curr);
  }

  MemRange<T>& operator=(const MemRange& rhs) {
    start_ = rhs.start_;
    end_ = rhs.end_;
    return *this;
  }

  bool Equal(const MemRange<T>& rhs) {
    if (Size() != rhs.Size())
      return false;
    if (!Size())
      return true;
    return (memcmp(start_, rhs.start_, Size()) == 0);
  }
};

int DecodeUTF8Point(char*& start, const MemRange<char>& range) {
  return -1;
}

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

  const wchar_t* Raw() { return path_.c_str(); }

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
      attributes_(0),  
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
private:
  HANDLE handle_;
  unsigned int  status_;
  friend class FileView;

  File(HANDLE handle,
       unsigned int status)
    : handle_(handle),
      status_(status) {
  }

protected:
  File()
    : handle_(INVALID_HANDLE_VALUE),
      status_(0) {
  }

public:
  enum Status {
    existing        = 1,
    delete_on_close = 2,
    directory       = 4,   // $$$ just use backup semantics?
    exclusive       = 8,
    readonly        = 16,
    information     = 32,
  };

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
    unsigned int status = 0;

    if (gle == ERROR_FILE_EXISTS) status |= existing;
    if (params.flags_ == FILE_FLAG_BACKUP_SEMANTICS) status |= directory;
    if (params.flags_ & FILE_FLAG_DELETE_ON_CLOSE) status |= delete_on_close;
    if (params.sharing_ == 0) status |= exclusive;
    if (params.access_ == 0) status |= information;
    if (params.access_ == GENERIC_READ) status |= readonly;

    return File(file, status);
  }

  ~File() {
    if (handle_ != INVALID_HANDLE_VALUE) {
      if (!::CloseHandle(handle_)) {
        throw IOException(__LINE__);
      }
    }
  }

  static long long GetUniqueId(const FilePath& path) {
    // $$$ tbd!!
    return 5;
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

#if 0
  bool Attributes() {
    BY_HANDLE_FILE_INFORMATION bhfi = {0};
    ::GetFileInformationByHandle(handle_, &bhfi);
    return true $$$??
  }
#endif
};

class FileView : public MemRange<char> {
private:
  HANDLE map_;

  FileView(HANDLE map,
           char* start,
           char* end)
    : MemRange(start, end),
      map_(map) {
  }

  bool plex_check_init() const {
    // $$$ map is valid.
  }

public:
  enum Mode {
    read_only,
    read_write,
  };

  static FileParams GetParams(FileView::Mode mode) {
    return (mode == read_only) ? 
      FileParams(GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, 0, 0, 0) : 
      FileParams(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS, 0, 0, 0);
  }

  static FileView Create(const File& file,
                         size_t offset,
                         size_t map_size,
                         const wchar_t* name) {
    DWORD protection = (file.status_ & File::readonly) ? PAGE_READONLY : PAGE_READWRITE;
    DWORD access = (protection == PAGE_READONLY) ? FILE_MAP_READ : FILE_MAP_WRITE | FILE_MAP_READ;
    HANDLE map = ::CreateFileMappingW(file.handle_, 0, protection, 0, 0, name);
    if (map == 0) {
      throw IOException(__LINE__);
    }

    ULARGE_INTEGER uli;
    uli.QuadPart = offset;
    char* start = reinterpret_cast<char*>(::MapViewOfFile(map, access, uli.HighPart, uli.LowPart, map_size));
    if (!start) {
      throw IOException(__LINE__);
    }

    return FileView(map, start, start + file.SizeInBytes());
  }

  size_t RegionSize() const {
    MEMORY_BASIC_INFORMATION mbi = {0};
    ::VirtualQuery(Start(), &mbi, sizeof(mbi));
    if (!mbi.RegionSize) {
      throw IOException(__LINE__);
    }
    return mbi.RegionSize;
  }

  ~FileView() {
    if (Start()) {
      if (!::UnmapViewOfFile(Start())) {
        throw IOException(__LINE__);
      }
    }
    if (map_) {
      if (!::CloseHandle(map_)) {
        throw IOException(__LINE__);
      }
    }
  }
};

#pragma endregion


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
    // Plex specific keywords in pragmas
    plex_test_pragma
  };

  MemRange<char> range;
  Type type;
  int line;

  CppToken(const MemRange<char>& range, CppToken::Type type, int line)
    : range(range), type(type), line(line) { }
};

std::string ToString(const CppToken& tok) {
  return std::string(tok.range.Start(), tok.range.Size());
}

bool EqualToStr(const CppToken& tok, const char* str) {
  return (::strncmp(str,  tok.range.Start(), tok.range.Size()) == 0);
}

template<typename T, size_t count>
size_t FindToken(T (&kw)[count], const MemRange<char>& r) {
 auto f = std::lower_bound(&kw[0], &kw[count], r, 
    [] (const char* kw, const MemRange<char>& v) {
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

CppToken::Type GetCppKeywordType(const MemRange<char>& r) {
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

CppToken::Type GetCppPreprocessorKeyword(const MemRange<char>& r) {
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

bool IsPredefinedMacro(const MemRange<char>& r) {
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
    "-=", "//", "/=", "::", 
    "<<", "<=", "==", ">=",
    ">>", "^=", "||", "|=" 
  };
  if (first.range.End() != second.range.Start())
    return CppToken::unknown;

  size_t off = FindToken(kw, MemRange<char>(first.range.Start(), second.range.End()));
  return (off == -1) ?
      CppToken::unknown :
      static_cast<CppToken::Type>(CppToken::symbols_end + off + 1);
}

typedef std::vector<CppToken> CppTokenVector;

CppTokenVector TokenizeCpp(const MemRange<char>& range) {
  CppTokenVector tv;
  tv.reserve(range.Size() / 3);

  char* curr = range.Start();
  char* str = nullptr;
  char c = *curr;

  // The first token is always (s)tart-(o)f-(s)stream.
  tv.push_back(CppToken(MemRange<char>(curr, curr), CppToken::sos, 0));
    
  int line = 1;

  do {
    int point = (c < 0x80) ? c :  DecodeUTF8Point(curr, range);

    if (point < 0) {
      throw point;
    }

    if (point == 0) {
      if (curr != range.End()) {
        // End of the file, must only be at the end.
        throw TokenizerException(__LINE__, line);
      }
      if (str) {
        // Incomplete token, unexpected EoF.
        throw TokenizerException(__LINE__, line);
      }

      // Insert a final token to simplify further processing.
      tv.push_back(CppToken(MemRange<char>(curr, curr), CppToken::eos, line + 1));
      return tv;

    } else if ((point == 0x09) || (point == 0x0A) || (point == 0x20)) {
      // tab (0x09) linefeed (0x0A) and space (0x20).
      if (str) {
        tv.push_back(CppToken(MemRange<char>(str, curr), CppToken::string, line));
        str = nullptr;
      }

      if (point == 0x0A) {
        ++line;
      }

    } else if ((point <  0x20) || (point == 0x7F)) {
      // Nonprintables are unrecoberable erros.
      throw TokenizerException(__LINE__, line);
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
          tv.push_back(CppToken(MemRange<char>(str, curr), CppToken::string, line));
          str = nullptr;
        }
        tv.push_back(CppToken(MemRange<char>(curr, curr + 1), 
                              static_cast<CppToken::Type>(symbol_type),
                              line));
      }
    }

    c = range.Next(curr);
  } while (curr);

  // Range needs to be null terminated.
  throw TokenizerException(__LINE__, line);
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
  first->range = MemRange<char>(first->range.Start(),
                                last->range.End());
}

bool LexCppTokens(CppTokenVector& tokens) {
  auto it = tokens.begin();
  while (it != tokens.end()) {
    if ((it->type > CppToken::symbols_begin ) && (it->type < CppToken::symbols_end)) {
      switch (it->type) {
        case CppToken::double_quote : {
          // Handle coalesing all tokens inside a string.
          auto it2 = it + 1;
          while (*it2->range.Start() != '"') {
            if (it2->line != it->line)
              throw TokenizerException(__LINE__, it->line);   // $$ fix multiline.
            if (*it2->range.Start() == '\\') 
              ++it2;
            ++it2; 
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
              throw TokenizerException(__LINE__, it->line);
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
          if (it != tokens.begin()) {
            if ((it - 1)->line == it->line) {
              // can't have tokens before a # in the same line.
              throw TokenizerException(__LINE__, it->line);
            }
          }
          // Next token is the kind.
          auto it2 = it + 1;
          auto pp_type = GetCppPreprocessorKeyword(it2->range);
          if (pp_type == CppToken::unknown) {
            // unrecongized preprocessor directive;
            throw TokenizerException(__LINE__, it->line);
          }
          int count = 0;
          while (it2->line == it->line) { ++it2; ++count;}
          // $$$ need to handle the 'null directive' which is a # in a single line.         
          if (pp_type == CppToken::prep_pragma) {
            if (count > 2) {
              if (EqualToStr(*(it+2), "plex_test")) {                
                pp_type = CppToken::plex_test_pragma;
              }
            }
          }
          CoaleseToken(it, it2 - 1, pp_type);
          tokens.erase(it + 1, it2);
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
          std::string number(it->range.Start(), it->range.Size());
          std::istringstream ss(number);
          long long value;
          ss >> value;
          if (!ss)
            throw TokenizerException(__LINE__, it->line);
          size_t np = ss.tellg();
          if (np != -1) {
            // failed to fully consume the number. See if we have a
            // base or float or size specificator.
            switch (number[np]) {
              case 'e' :
              case 'E' : {
                // Mantissa + exponent form. 'e' must be the last char.
                if ((np + 1) != number.size())
                  throw TokenizerException(__LINE__, it->line);
              }
              break;
              case 'x' :
              case 'X' : {
                // Hex number. Verify that iostream can consume it fully.
                ss.seekg(0);
                ss >> std::hex >> value;
                if (!ss)
                  throw TokenizerException(__LINE__, it->line);
                np = ss.tellg();
                if (np != -1)
                  throw TokenizerException(__LINE__, it->line);
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
                throw TokenizerException(__LINE__, it->line);
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
          throw TokenizerException(__LINE__, it->line);
        }
      }
    } else if (it->type == CppToken::sos) {
      // first token.
    } else if (it->type == CppToken::eos) {
      return true;
    } else {
      throw TokenizerException(__LINE__, it->line);
    }
    // advance to next token.
    ++it;
  }

  throw TokenizerException(__LINE__, 0);
}

#pragma region catalog

struct XternDef {
  enum Type {
    kNone,
    kStruct,
    kClass,
    kUnion,
    kEnum,
    kFunction,
    kTypedef,
    kConstant,
  };

  Type type;
  MemRange<char> name;
  std::vector<size_t> src_pos;

  XternDef(const MemRange<char>& name) : type(kNone), name(name) {}
  XternDef() : type(kNone), name(nullptr, nullptr) {}
};

XternDef MakeXDef(const char* type, const MemRange<char>& name) {
  XternDef::Type xdt;
  if (type[0] == 'p' && type[1] == 'c')
    xdt = XternDef::kClass;
  else if (type[0] == 'p' && type[1] == 's')
    xdt = XternDef::kStruct;
  else if (type[0] == 'r' && type[1] == 'u')
    xdt = XternDef::kUnion;
  else if (type[0] == 'p' && type[1] == 'f')
    xdt = XternDef::kFunction;
  else if (type[0] == 'p' && type[1] == 'e')
    xdt = XternDef::kEnum;
  else if (type[0] == 'k' && type[1] == 'c')
    xdt = XternDef::kConstant;
  else
    throw CatalogException(0, __LINE__);

  XternDef xdef(name);
  xdef.type = xdt;
  return xdef;
}

typedef std::unordered_map<std::string, XternDef> XternDefs;

void ProcessCatalog(CppTokenVector& tv, XternDefs& defs) {
  for(auto it = begin(tv); it->type != CppToken::eos; ++it) {
    if (it->type == CppToken::identifier) {
      if (EqualToStr(*it, "end"))
        break;
      if (!EqualToStr(*it, "cen"))
        continue;
      ++it;
      if (it->type != CppToken::open_paren)
        throw CatalogException(it->line, __LINE__);
      ++it;
      if (it->type != CppToken::identifier)
        throw CatalogException(it->line, __LINE__);
      auto it2 = it + 1;
      if (it2->type != CppToken::comma)
        throw CatalogException(it->line, __LINE__);
      ++it2;

      defs[ToString(*it)] = MakeXDef(it2->range.Start(), it->range);
      it = ++it2;
    }
  }
}

#pragma endregion

// Here we prune the names that are part of a definition.
CppTokenVector GetExternalDefinitions(CppTokenVector& tv, XternDefs& xdefs) {

    auto IsBuiltIn = [](int t) -> bool {
      return (
        t == CppToken::kw_void ||
        t == CppToken::kw_int ||
        t == CppToken::kw_long ||
        t == CppToken::kw_char ||
        t == CppToken::kw_bool ||
        t == CppToken::kw_float ||
        t == CppToken::kw_const ||
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
        t == CppToken::kw_volatile);
    };

    auto IsInVector = [](const CppTokenVector& v,
                         const char* start) -> bool {
      for (auto it = begin(v); it != end(v); ++it) {
        if (EqualToStr(*it, start))
          return true;
      }
      return false;
    };

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

    std::vector<const char*> enclosing_namespace;
    std::vector<const char*> enclosing_definition;
    bool in_local_definition = false;

    auto last = begin(tv);

    for(auto it = ++last; it->type != CppToken::eos; ++it) {
      auto prev = *(it - 1);
      auto next = *(it + 1);

      if (it->type == CppToken::comment)
        continue;

      int ln = it->line;

      if (it->type == CppToken::open_cur_bracket) {
        lvars.push_back(CppTokenVector());

        if (prev.type == CppToken::kw_namespace) {
          enclosing_namespace.push_back(anonymous_namespace_mk);
        } else {
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
        } else if (!enclosing_namespace.empty()) {
          enclosing_namespace.pop_back();
        } else {
          __debugbreak();
        }
        continue;
      
      } else if (it->type == CppToken::identifier) {
        if (prev.type == CppToken::kw_namespace) {
          if (next.type != CppToken::open_cur_bracket)
            __debugbreak();
          enclosing_namespace.push_back(it->range.Start());
          lvars.push_back(CppTokenVector());
          ++it;
          continue;

        } else if (IsAgregateIntroducer(prev.type)) {
          if (next.type == CppToken::semicolon) {
            if (IsInVector(ldefs, it->range.Start()))
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

        if (!IsInVector(ldefs, it->range.Start())) {

          if(IsInVector(xrefs, it->range.Start())) {
            xrefs.push_back(*it);
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
            xdit->second.src_pos.push_back(it - tv.begin());
            xrefs.push_back(*it);
            continue;
          }

          // Many things can end up here for example method invocations
          // on the enclosing aggregate.

        }  // not seen before.

      }  // identifier.
      
    }

#if 0
    if (!enclosing_namespace.empty())
      __debugbreak();
    if (!enclosing_definition.empty())
      __debugbreak();
    if (lvars.size() != 1)
      __debugbreak();
#endif

    return xrefs;
}

#pragma region testing

struct TestResults {
  size_t tokens;
  int tests;
  int failures;
  const wchar_t* file;

  TestResults(const wchar_t* file)
     : tokens(0), tests(0), failures(0), file(file) {
  }
};

void TestErrorHeader(TestResults& results, int line) {
  ++results.failures;
  if (results.failures == 1) {
    wprintf(L"file [%s]\n",  results.file);
  }
  wprintf(L"[FAIL] test of line %d :", line); 
}

void TestErrorDump(TestResults& rs, 
                   int line, size_t actual, size_t expected, const CppTokenVector& tokens) {
  TestErrorHeader(rs, line);
  wprintf(L"token_count actual : %d expected: %d \n", actual, expected);
  for (auto& tok : tokens) {
    if (tok.line == line)
      wprintf(L"%d- %S\n", tok.type, ToString(tok).c_str());
  }
  wprintf(L"\n");
}

void TestErrorDump(TestResults& rs,
                   int line, const char* name) {
  TestErrorHeader(rs, line);
  wprintf(L"for external def %S\n\n", name);
}

bool ProcessTestPragmas(const CppTokenVector& tokens,
                        XternDefs& xdefs,
                        TestResults& results) {
  
  // It is very likely we are going to need this so we compute just once.
  CppTokenVector name_tokens;
  for (auto& tok : tokens) {
    if (tok.type == CppToken::identifier)
      name_tokens.push_back(tok);
  }

  std::unordered_map<int, long> line_counts;
  for (auto& tok : tokens) {
    ++line_counts[tok.line];
  }

  for(auto it = begin(tokens); it->type != CppToken::eos; ++it) {
    if (it->type != CppToken::plex_test_pragma)
      continue;

    ++results.tests;
    std::istringstream ss(it->range.Start());
    ss.ignore(17);   // size of '#pragma plex_test'  $$ make this better.
    std::string test_name;
    ss >> test_name;

    if (test_name == "token_count") {
      // token count format is: token_count line count
      long line, expected_count;
      ss >> line >> expected_count;
      if (!ss)
        throw TokenizerException(__LINE__, it->line);

      if (line_counts[line] == expected_count)
        continue;

      TestErrorDump(results, it->line, line_counts[line], expected_count, tokens);

    } else if (test_name == "name_count") {
      // name count format is: name_count count
      long expected_count;
      ss >> expected_count;
      if (!ss)
        throw TokenizerException(__LINE__, it->line);

      int count = 0;
      for (auto& tok : name_tokens) {
        if (tok.line < it->line)
          ++count;
      }

      if (count == expected_count)
        continue;

      TestErrorDump(results, it->line, name_tokens.size(), expected_count, name_tokens);

    } else if (test_name == "fixup") {
      long fix_line;
      std::string fix_type;
      std::string fix_name;
      ss >> fix_line >> fix_type >> fix_name;
      if (!ss)
        throw TokenizerException(__LINE__, it->line);

      bool found = false;
      auto xit = xdefs.find(fix_name);
      if (xit != xdefs.end()) {
        for (auto& fixup : xit->second.src_pos) {
          if (tokens[fixup].type != CppToken::identifier)
            continue;
          if (tokens[fixup].line == fix_line) {
            found = true;
            break;
          }
        }
      }

      if (found)
        continue;

      TestErrorDump(results, it->line, fix_name.c_str());

    } else {
      // Unknown test pragma.
      throw TokenizerException(__LINE__, it->line);
    }

  }  // for all tokens.

  return true;
}

#pragma endregion


int wmain(int argc, wchar_t* argv[]) {

  if (argc != 3) {
    wprintf(L"error: invalid # of options\n");
    return 1;
  }

  if (argv[1] != std::wstring(L"--tokens-test")) {
    wprintf(L"error: unrecognized option\n");
    return 1;
  }

  try {
    // Input file, typically a c++ file.
    FilePath path(argv[2]);
    FileParams fparams = FileView::GetParams(FileView::read_only);
    File cc_file = File::Create(path, fparams, FileSecurity());
    if (!cc_file.IsValid()) {
       wprintf(L"error: could not open input file\n");
       return 1;
    }
    // The catalog index is also an implicit input.
    FilePath catalog = FilePath(path.Parent()).Append(L"catalog\\index.plex");
    File index = File::Create(catalog, fparams, FileSecurity());
    if (!index.IsValid()) {
       wprintf(L"error: could not open catalog\n");
       return 1;
    }

    FileView index_view = FileView::Create(index, 0ULL, 0ULL, nullptr);
    CppTokenVector index_tv = TokenizeCpp(index_view);
    LexCppTokens(index_tv);
    XternDefs xdefs;
    ProcessCatalog(index_tv, xdefs);

    FileView cc_view = FileView::Create(cc_file, 0ULL, 0ULL, nullptr);
    CppTokenVector cc_tv = TokenizeCpp(cc_view);
    LexCppTokens(cc_tv);
    CppTokenVector xr = GetExternalDefinitions(cc_tv, xdefs);

    TestResults results(argv[2]);
    results.tokens = cc_tv.size();
    ProcessTestPragmas(cc_tv, xdefs, results);

    if (results.failures) {
      wprintf(L"done: [%s] got %d failures\n", argv[2], results.failures);
      return 1;
    } 
      
    wprintf(L"done: [%s] with %d OK tests\n", argv[2], results.tests);
    return 0;

  } catch (PlexException& ex) {
    wprintf(L"\nerror: [%s] fatal exception [%S]\n"
            L"in program line %d, version (%S)\n",
            argv[2], ex.Message(), ex.Line(), __DATE__);
  } catch (int line) {
    wprintf(L"\nerror: [%s] fatal exception [unknown]\n"
            L"in program line %d, version (%S)\n",
            argv[2], line, __DATE__);
  }

  return 2;
}
