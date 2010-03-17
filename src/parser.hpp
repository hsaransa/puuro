#ifndef _pr_parser_hpp_
#define _pr_parser_hpp_

#include "prdefs.hpp"
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

        void parse();
        AST* get_ast();

    private:
        Ref<Lexer*> lexer;
        Ref<AST*> ast;

        static Type* type;
    };
}

#endif
