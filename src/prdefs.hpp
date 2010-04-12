#ifndef _pr_prdefs_hpp_
#define _pr_prdefs_hpp_

#include <cassert>

namespace pr
{
    class Object;
    class Type;
    class String;
    class List;
    class Name;
    class Lexer;
    class AST;
    class Code;
    class Frame;
    class Closure;
    class Integer;
    class MiniCode;
    class Exception;
    class Scope;

    struct FilePosition;

    typedef unsigned long ObjP;

    template<typename T> class Ref;

    typedef int int32;
    typedef long long int int64;

    static const int GC_BITS = 8;
}

#endif
