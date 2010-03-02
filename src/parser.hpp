#ifndef _pr_parser_hpp_
#define _pr_parser_hpp_

#include "prdefs.hpp"
#include "object.hpp"
#include "gc.hpp"

namespace pr
{
    class Parser : public Object
    {
    public:
        Parser(Lexer*);
        virtual ~Parser();

        virtual Type* get_type();
        virtual void gc_mark();

        AST* get_ast();

    private:
        RefCount<Lexer> lexer;
        RefCount<AST> ast;

        static Type* type;
    };
}

#endif
