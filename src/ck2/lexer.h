#ifndef LIBCK2_LEXER_H
#define LIBCK2_LEXER_H

#include "common.h"
#include "scanner.h"
#include "filesystem.h"
#include <memory>
#include <cstdio>


NAMESPACE_CK2;


class token;

class lexer {
    unique_file_ptr _f;
    fs::path        _path;

    void reset_scanner() {
        yyin = nullptr;
        yylineno = 0;
        yyrestart(yyin);
    }

public:
    lexer() = delete;
    ~lexer() noexcept { reset_scanner(); }
    lexer(const fs::path& path);

    const auto& path() const noexcept { return _path; }

    // read a new token from the input into t. if max_copy_sz is nonzero, actually copy the token text buffer (capped by
    // this amount) into the token object's preexisting buffer. otherwise, when max_copy_sz == 0, simply swap the token
    // text buffers (zerocopy). returns false when the read token signals the end of input and true otherwise.
    bool read_token_into(token&, size_t max_copy_sz = 0);
};


END_NAMESPACE_CK2;
#endif
