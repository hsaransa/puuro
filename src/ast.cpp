#include "ast.hpp"
#include "primitives.hpp"

using namespace pr;

AST::AST(Name t)
:   Object(get_type()),
    type(t),
    object(0)
{
}

AST::~AST()
{
    for (int i = 0; i < (int)children.size(); i++)
        dec_ref(children[i]);
    dec_ref(object);
}

Type* AST::get_type()
{
    static Type* type;
    if (!type)
        type = new Type(Name("ast"));
    return type;
}

void AST::gc_mark()
{
    for (int i = 0; i < (int)children.size(); i++)
        GC::mark(children[i]);
    GC::mark(object);
}

void AST::add_child(AST* ast)
{
    inc_ref(ast);
    children.push_back(ast);
}

void AST::set_object(ObjP p)
{
    inc_ref(p);
    dec_ref(object);
    object = p;
}

#ifndef NDEBUG
#include <stdio.h>
void AST::debug_print(int in)
{
    printf("%*.s", in*2, "");

    printf("%s", type.s());

    if (object)
    {
        if (is_symbol(object))
            printf(" :%s", symbol_to_name(object).s());
        else
            printf(" %lx", object);
    }

    printf("\n");

    for (int i = 0; i < (int)children.size(); i++)
        children[i]->debug_print(in+1);
}
#endif
