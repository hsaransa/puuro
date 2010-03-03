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

    struct FilePosition;

    typedef unsigned long ObjP;

    template<typename T> class Ref;
}

#endif
