#ifndef LIBCK2_PARSER_H
#define LIBCK2_PARSER_H

#include "common.h"
#include "FileLocation.h"
#include "cstr.h"
#include "date.h"
#include "fp_decimal.h"
#include "lexer.h"
#include "token.h"
#include "filesystem.h"
#include <ostream>
#include <memory>
#include <algorithm>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <iostream>

NAMESPACE_CK2;


typedef fp_decimal<3> fp3;
class block;
class list;
class parser;


enum class binary_op
{
  EQ,  // =
  LT,  // <
  GT,  // >
  LTE, // <=
  GTE, // >=
  EQ2, // ==
};


/* OBJECT -- generic "any"-type syntax tree node */

class object
{
public:
  enum {
    NIL,
    INTEGER,
    DATE,
    DECIMAL,
    BINARY_OP,
    STRING,
    BLOCK,
    LIST,
  } _type;

  using binop = binary_op;

private:
  struct cstr8
  {
    char* p;
    uint  n;

    cstr8() : p(nullptr), n(0) {}

    cstr8(cstr8 const& other)
    : p(nullptr)
    , n(other.n)
    {
      if (!n) return;
      p = new char[n];
      strcpy(p, other.p);
    }

    cstr8(cstr8 && other) noexcept : cstr8() { std::swap(p, other.p); std::swap(n, other.n); }

    cstr8(char const* src)
    : p(nullptr)
    , n(strlen(src) + 1)
    {
      p = new char[n];
      strcpy(p, src);
    }

    ~cstr8() { if (p) delete[] p; p = nullptr; }

    auto to_string_view() const noexcept { return (n > 0) ? std::string_view(p, n - 1) : std::string_view(); }
  };

  Loc _loc;

  union data_union
  {
    // trivial members:
    int    i;
    date   d;
    fp3    f;
    binop  o;
    // non-trivial members:
    cstr8  cs;
    std::shared_ptr<block> p_block;
    std::shared_ptr<list>  p_list;

    // tell compiler that we'll manage the nontrivial members of data_union outside of the union:
    data_union() {}
    ~data_union() {}
  } _data;

  void destroy() noexcept; // helper for dtor & copy/move-assignment (due to data_union)

public:
  const char* type_string() const noexcept;

  auto& loc() const noexcept { return _loc; }
  auto& loc()       noexcept { return _loc; }

  object(const Loc& l = Loc())          : _type(NIL),       _loc(l) { _data.i = 0; }
  object(int i,   const Loc& l = Loc()) : _type(INTEGER),   _loc(l) { _data.i = i; }
  object(date d,  const Loc& l = Loc()) : _type(DATE),      _loc(l) { _data.d = d; }
  object(fp3 f,   const Loc& l = Loc()) : _type(DECIMAL),   _loc(l) { _data.f = f; }
  object(binop o, const Loc& l = Loc()) : _type(BINARY_OP), _loc(l) { _data.o = o; }

  /* constructors for non-trivial types */

  object(const char* s, const Loc& l = Loc()) : _type(STRING), _loc(l) { new (&_data.cs) cstr8(s); }

  // potential optimization opportunity: pass shared_ptr by const ref (but, given that we are taking ownership,
  // we'd prefer to pass by value as a best practice, so see if all the extra refcount incr/decr makes any diff)
  object(std::shared_ptr<block> p, const Loc& l = Loc()) : _type(BLOCK), _loc(l)
  {
    new (&_data.p_block) std::shared_ptr<block>(p);
  }
  object(std::shared_ptr<list> p,  const Loc& l = Loc()) : _type(LIST),  _loc(l)
  {
    new (&_data.p_list) std::shared_ptr<list>(p);
  }

  /* copy-assignment operator */
  object& operator=(const object& other);

  /* copy constructor */
  object(const object& other);

  /* move-assignment operator */
  object& operator=(object&& other);

  /* move-constructor (implemented via move-assignment) */
  object(object&& other) : object() { *this = std::move(other); }

  /* destructor */
  ~object() { destroy(); }

  /* type accessors */
  bool is_null()      const noexcept { return _type == NIL; }
  bool is_integer()   const noexcept { return _type == INTEGER; }
  bool is_date()      const noexcept { return _type == DATE; }
  bool is_decimal()   const noexcept { return _type == DECIMAL; }
  bool is_binary_op() const noexcept { return _type == BINARY_OP; }
  bool is_string()    const noexcept { return _type == STRING; }
  bool is_block()     const noexcept { return _type == BLOCK; }
  bool is_list()      const noexcept { return _type == LIST; }
  bool is_number()    const noexcept { return is_integer() || is_decimal(); }

  /* data accessors (unchecked type) */
  auto as_integer()     const noexcept { return _data.i; }
  auto as_date()        const noexcept { return _data.d; }
  auto as_decimal()     const noexcept { return (is_integer()) ? fp3(_data.i) : _data.f; }
  auto as_binary_op()   const noexcept { return _data.o; }
  auto as_string()      const noexcept { return _data.cs.p; }
  auto as_string_view() const noexcept { return _data.cs.to_string_view(); }
  auto as_block()       const noexcept { return _data.p_block.get(); }
  auto as_list()        const noexcept { return _data.p_list.get(); }

  struct TypeError : public Error
  {
    TypeError(const object* o, const char* requested_type)
    : Error("Bad access of {}-type object as a {}", o->type_string(), requested_type) {}
  };

  /* data accessors (checked type, throws object::TypeError if type requested doesn't match type of object) */
  auto get_integer()     const { if (!is_integer()) throw TypeError(this, "integer");    return as_integer(); }
  auto get_date()        const { if (!is_date()) throw TypeError(this, "date");          return as_date(); }
  auto get_decimal()     const { if (!is_number()) throw TypeError(this, "decimal");     return as_decimal(); }
  auto get_binary_op()   const { if (!is_binary_op()) throw TypeError(this, "operator"); return as_binary_op(); }
  auto get_string()      const { if (!is_string()) throw TypeError(this, "string");      return as_string(); }
  auto get_string_view() const { if (!is_string()) throw TypeError(this, "string_view"); return as_string_view(); }
  auto get_block()       const { if (!is_block()) throw TypeError(this, "block");        return as_block(); }
  auto get_list()        const { if (!is_list()) throw TypeError(this, "list");          return as_list(); }

  /* convenience equality operator overloads */
  bool operator==(int i)   const noexcept { return is_integer() && as_integer() == i; }
  bool operator==(date d)  const noexcept { return is_date() && as_date() == d; }
  bool operator==(fp3 f)   const noexcept { return is_number() && as_decimal() == f; }
  bool operator==(binop o) const noexcept { return is_binary_op() && as_binary_op() == o; }
  bool operator==(const char* s)        const noexcept { return is_string() && strcmp(as_string(), s) == 0; }
  bool operator==(std::string_view sv)  const noexcept { return is_string() && strcmp(as_string(), sv.data()) == 0; }
  bool operator==(const std::string& s) const noexcept { return is_string() && s == as_string(); }

  /* inequality operator overloads */
  template<typename T>
  bool operator!=(const T& other) const noexcept { return !(*this == other); }

  void print(std::ostream&, uint indent = 0) const;
};


/* LIST -- list of N objects */

class list {
  using ElemT = object;
  using VecT = std::vector<ElemT>;
  VecT _v;

public:
  list() {}
  list(parser&);

  auto size()   const noexcept { return _v.size(); }
  auto empty()  const noexcept { return _v.size() == 0; }
  auto begin()  const noexcept { return _v.cbegin(); }
  auto end()    const noexcept { return _v.cend(); }
  auto begin()        noexcept { return _v.begin(); }
  auto end()          noexcept { return _v.end(); }
  auto rbegin() const noexcept { return _v.crbegin(); }
  auto rend()   const noexcept { return _v.crend(); }
  auto rbegin()       noexcept { return _v.rbegin(); }
  auto rend()         noexcept { return _v.rend(); }

  /* do we even need to say auto& (by ref) here when that could be inferred directly from the return-expr? */
  auto& operator[](size_t i) const noexcept { return _v[i]; }
  auto& operator[](size_t i)       noexcept { return _v[i]; }

  void push_back(const ElemT& e) { _v.push_back(e); }
  void push_back(ElemT&& e) { _v.push_back(std::move(e)); }
  void pop_back() noexcept { _v.pop_back(); }
  void clear() noexcept { _v.clear(); }

  template<typename It> auto erase(It pos) { return _v.erase(pos); }
  template<typename It> auto erase(It first, It last) { return _v.erase(first, last); }

  void print(std::ostream&, uint indent = 0) const;
};


/* STATEMENT -- statements are pairs of objects and an operator/separator */

class statement {
public:
  statement() : _op(binary_op::EQ) {}
  statement(const object& k, const object& op, const object& v) : _k(k), _op(op), _v(v) {}
  statement(const object& k, const object& v) : statement(k, binary_op::EQ, v) {}

  // TODO: move-assign, move-ctor (with correct noexcept specifications so that STL will use them)

  // /* move-assignment operator */
  // statement& operator=(statement&& other);

  // /* move-constructor (implemented via move-assignment) */
  // statement(statement&& other) : statement() { *this = std::move(other); }

  auto& key()   const noexcept { return _k; }
  auto& op()    const noexcept { return _op; }
  auto& value() const noexcept { return _v; }

  void print(std::ostream&, uint indent = 0) const;

protected:
  object _k;
  object _op;
  object _v;

  void key(const object& o)       { _k = o; }
  void op(const object& o)        { _op = o; }
  void value(const object& o)     { _v = o; }
  void key(object&& o)   noexcept { _k = std::move(o); }
  void op(object&& o)    noexcept { _op = std::move(o); }
  void value(object&& o) noexcept { _v = std::move(o); }
};


/* BLOCK -- blocks contain N statements */

class block {
  /* we maintain two data structures for different ways of efficiently accessing
   * the statements in a block. a linear vector is the master data structure;
   * it actually owns the statement objects, which means that when it is
   * destroyed, so too are any memory resources associated with the objects
   * in its statements.
   *
   * the secondary data structure fulfills a more specialized use case: it maps
   * LHS string-type keys to an index into the statement vector to which the
   * key corresponds, if such a key occurs in this block. if it can occur
   * multiple times, then this block was not constructed via the RHS-merge
   * parsing method (i.e., folderization), and this access pattern may be a
   * lot less helpful. in such cases, the final occurrence of the key in
   * this block will be stored in the hash-map.
   *
   * string-type keys are the only type of keys for which I've encountered a
   * realistic use case for the hash-map access pattern, so rather than create
   * a generalized scalar any-type (like ck2::object but only for scalar data
   * types, or more generally, copy-constructible data types) and hash-map via
   * that any-type, I've simply chosen to stick to string keys for now.
   */

protected:
  // linear vector of statements
  using vec = std::vector<statement>;
  vec _v;

  // hash-map of LHS keys to their corresponding statement's index in _vec
  std::unordered_map<cstr, vec::difference_type> _map;

public:
  block() { }
  block(parser&, bool is_root = false, bool is_save = false);

  void print(std::ostream&, uint indent = 0) const;

  auto size()   const noexcept { return _v.size(); }
  auto empty()  const noexcept { return _v.size() == 0; }
  auto begin()  const noexcept { return _v.cbegin(); }
  auto end()    const noexcept { return _v.cend(); }
  auto begin()        noexcept { return _v.begin(); }
  auto end()          noexcept { return _v.end(); }
  auto rbegin() const noexcept { return _v.crbegin(); }
  auto rend()   const noexcept { return _v.crend(); }
  auto rbegin()       noexcept { return _v.rbegin(); }
  auto rend()         noexcept { return _v.rend(); }

  /* map accessor for statements by LHS statement key (if string-type)
   * if not found, returns this object's end iterator.
   * if found, returns a valid iterator which can be dereferenced. */
  vec::iterator find_key(const char* key) noexcept
  {
    auto i = _map.find(key);
    return (i != _map.end()) ? std::next(begin(), i->second) : end();
  }

  vec::const_iterator find_key(const char* key) const noexcept
  {
    auto i = _map.find(key);
    return (i != _map.end()) ? std::next(begin(), i->second) : end();
  }
};


/* PARSER -- construct a parse tree from a file whose resources are owned by the parser object */

class parser
{
public:
  parser(const std::string& path, bool is_save = false) : parser(path.c_str(), is_save) {}
  parser(const fs::path& path,    bool is_save = false) : parser(path.generic_string().c_str(), is_save) {}

  parser(const char* path, bool is_save = false)
  : _lex(path)
  , _tq_done(false)
  , _tq_head_idx(0)
  , _tq_n(0)
  {
    // hook our preallocated token text buffers into the lookahead queue
    for (uint i = 0; i < TQ_SZ; ++i)
    {
      _tq_text[i][0] = '\0';
      _tq[i].text(_tq_text[i], 0);
    }

    _p_root = std::make_shared<block>(*this, true, is_save);
  }

  auto& root_block()       noexcept { return _p_root; }
  auto& root_block() const noexcept { return _p_root; }

  const auto& path() const noexcept { return _lex.path(); }

  auto floc(const Location& loc) const noexcept { return FLoc(path(), loc); }
  auto floc(const object& obj)   const noexcept { return FLoc(path(), obj.loc()); }
  auto floc()                    const noexcept { return FLoc(path()); }

  template<typename... Args>
  auto err(const Location& loc, const char* format, Args&& ...args) const
  {
    return FLError(floc(loc), format, std::forward<Args>(args)...);
  }

  template<typename... Args>
  auto err(const object& obj, const char* format, Args&& ...args) const
  {
    return FLError(floc(obj), format, std::forward<Args>(args)...);
  }

  template<typename... Args>
  auto err(const char* format, Args&& ...args) const
  {
    return FLError(floc(), format, std::forward<Args>(args)...);
  }

protected:
  friend class block;
  friend class list;

  std::shared_ptr<block> _p_root;
  lexer _lex;

  static const uint NUM_LOOKAHEAD_TOKENS = 1;
  // actual token queue size is +1 for the "freebie" lookahead token (the next/current token) and +1 for a "spare"
  // token object which is necessary to guard against dequeuing a token & then peeking past the end of the filled
  // portion of the lookahead queue, which would then overwrite the token text buffer contents of the dequeued token
  // as the queue was refilled and would thus otherwise be an easy possible source of error and generally
  // inconvenient.
  static const uint TQ_SZ = NUM_LOOKAHEAD_TOKENS + 2;
  static const uint TEXT_MAX_SZ = 512; // size of preallocated token text buffers (511 max length)

  bool  _tq_done; // have we read everything from the input stream?
  uint  _tq_head_idx; // index of head of token queue (i.e., next to be handed to user)
  uint  _tq_n; // number of tokens actually in the queue (i.e., that have not been consumed and have been enqueued)
  token _tq[TQ_SZ];
  char  _tq_text[TQ_SZ][TEXT_MAX_SZ]; // text buffers for saving tokens from input scanner; parallel to _tq

  void enqueue_token()
  {
    uint slot = (_tq_head_idx + _tq_n++) % TQ_SZ;
    _tq_done = !( _lex.read_token_into(_tq[slot], TEXT_MAX_SZ) );
  }

  // fill the token queue such that its effective size is at least `sz`. returns false if that couldn't be satisfied.
  // preconditions: _tq_n <= sz < TQ_SZ
  bool fill_token_queue(uint sz)
  {
    for (uint needed = sz - _tq_n; needed > 0 && !_tq_done; --needed) enqueue_token();
    return _tq_n == sz;
  }

  bool next(token*, bool eof_ok = false);
  void next_expected(token*, uint type);
  void unexpected_token(const token&) const;

  // peek into the token lookahead queue by logical index, POS, of token. position 0 is simply the next token were we
  // to call next(...), and position 1 would be second to next in the input stream (the 1st lookahead token), and so
  // on. if the input stream has ended (EOF, unmatched text) sometime before the lookahead token at which we're
  // peeking, we'll return a nullptr.
  template<const uint POS>
  token* peek() noexcept
  {
    static_assert(POS <= NUM_LOOKAHEAD_TOKENS, "cannot peek at position greater than parser's number of lookahead tokens");
    return (POS >= _tq_n && !fill_token_queue(POS + 1)) ? nullptr : &_tq[ (_tq_head_idx + POS) % TQ_SZ ];
  }
};


/* MISC. UTILITY */

static const uint TIER_BARON   = 1;
static const uint TIER_COUNT   = 2;
static const uint TIER_DUKE    = 3;
static const uint TIER_KING    = 4;
static const uint TIER_EMPEROR = 5;

inline uint title_tier(const char* s) {
  switch (*s) {
  case 'b':
    return TIER_BARON;
  case 'c':
    return TIER_COUNT;
  case 'd':
    return TIER_DUKE;
  case 'k':
    return TIER_KING;
  case 'e':
    return TIER_EMPEROR;
  default:
    return 0;
  }
}

bool looks_like_title(const char*);


NAMESPACE_CK2_END;

inline std::ostream& operator<<(std::ostream& os, const ck2::block& a)     { a.print(os); return os; }
inline std::ostream& operator<<(std::ostream& os, const ck2::list& a)      { a.print(os); return os; }
inline std::ostream& operator<<(std::ostream& os, const ck2::statement& a) { a.print(os); return os; }
inline std::ostream& operator<<(std::ostream& os, const ck2::object& a)    { a.print(os); return os; }

#endif

