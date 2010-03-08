#include "parser.hpp"
#include "lexer.hpp"
#include "ast.hpp"
#include "type.hpp"

using namespace pr;

namespace pr
{
    extern Lexer* yy_lexer;
    extern int yy_file;
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

    yy_file = l->get_file().id();

    yy_lexer = lexer.get();

    //yy_ast = parse();
    yyparse();

    yy_lexer = 0;

    ast = yy_ast;
    dec_ref(yy_ast);

    yy_ast = 0;
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
    return_and_inc_ref(ast.get());
}
