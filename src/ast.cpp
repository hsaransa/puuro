#include "ast.hpp"
#include "primitives.hpp"
#include "type.hpp"

using namespace pr;

AST::AST(Name t, const FilePosition& p)
:   Object(get_type()),
    type(t),
    object(0),
    position(p)
{
}

AST::~AST()
{
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

#include <iostream>
void AST::add_child(AST* ast)
{
    children.push_back(ast);
    dec_ref(ast);
}

void AST::set_object(ObjP p)
{
    object = p;
}

#ifndef NDEBUG
#include <stdio.h>
void AST::debug_print(int in)
{
    printf("%3d %*.s", position.line, in*2, "");

    printf("%s", type.s());

    if (object)
    {
        if (is_symbol(object))
            printf(" :%s", symbol_to_name(object).s());
        else
            printf(" %lx", object.get());
    }

    printf("\n");

    for (int i = 0; i < (int)children.size(); i++)
        children[i]->debug_print(in+1);
}
#endif
