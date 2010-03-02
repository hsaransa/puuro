#include "parser.hpp"
#include "lexer.hpp"
#include "ast.hpp"
#include "type.hpp"

using namespace pr;

namespace pr
{
    extern Lexer* yy_lexer;
    extern AST* yy_ast;
}

extern int yyparse();

Type* Parser::type;

Parser::Parser(Lexer* l)
:   Object(get_type()),
    lexer(l),
    ast(0)
{
    assert(lexer);

    GC::block_gc();

    yy_lexer = lexer.get();
    yyparse();
    ast = yy_ast;

    GC::unblock_gc();
}

Parser::~Parser()
{
}

Type* Parser::get_type()
{
    if (!type)
    {
        type = new Type("parser");
    }

    return type;
}

void Parser::gc_mark()
{
    GC::mark(lexer);
    GC::mark(ast);
}

AST* Parser::get_ast()
{
    return ast.get();
}
