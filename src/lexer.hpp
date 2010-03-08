#ifndef _pr_lexer_hpp_
#define _pr_lexer_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "nametable.hpp"
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

        void set_allow_uminus(bool b) { allow_uminus = b; }

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

        bool allow_uminus;
    };

    String* read_file(const char* fn);
}

#endif
