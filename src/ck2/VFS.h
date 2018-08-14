#ifndef LIBCK2_VFS_H
#define LIBCK2_VFS_H


#include "common.h"
#include "filesystem.h"
#include <vector>
#include <string>


NAMESPACE_CK2;


struct VFS {
    VFS(const fs::path& base_path) : _root_paths({ base_path }) {}

    void push_mod_path(const fs::path& p) {
        if (!fs::exists(p)) throw PathNotFoundError(p);
        if (!fs::is_directory(p)) throw PathTypeError(p);
        _root_paths.push_back(p);
    }

    bool resolve_path(fs::path* p_real_path, const fs::path& virt_path) const {
        /* search path vector for a filesystem hit in reverse */
        for (auto i = _root_paths.crbegin(); i != _root_paths.crend(); ++i)
            if (fs::exists( *p_real_path = *i / virt_path ))
                return true;
        return false;
    }

    /* a more convenient accessor which auto-throws on a nonexistent path */
    fs::path operator[](const fs::path& virt_path) const {
        fs::path p;
        if (!resolve_path(&p, virt_path)) throw PathNotFoundError(p);
        return p;
    }

    auto to_string() {
        std::string s = "{";

        if (_root_paths.empty())
            return s += '}';

        // iterate from "bottom" (top of stack) to "top" (bottom of stack) of our vector
        for (auto it = _root_paths.crbegin(); it != _root_paths.crend(); ++it) {
            s.append("\n\t");
            s.append((*it).string());
        }

        s.append("\n}");
        return s;
    }

private:
    std::vector<fs::path> _root_paths;
};


NAMESPACE_CK2_END;
#endif
