#ifndef _pr_lexer_hpp_
#define _pr_lexer_hpp_

#include "prdefs.hpp"
#include "gc.hpp"

namespace pr
{
    class Lexer : public Object
    {
    public:
        Lexer(Name f, String* s);
        virtual ~Lexer();

        virtual Type* get_type();
        virtual void gc_mark();

        int next();
        int get_line() { return line; }
        Name get_file() { return file; }

    private:
        Ref<String*> src;
        int pos;

        int current_token;
        Ref<ObjP> object;

        Name file;
        int line;
    };

    String* read_file(const char* fn);
}

#endif
