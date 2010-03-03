#include "type.hpp"
#include "primitives.hpp"
#include "list.hpp"
#include "gc.hpp"
#include "method.hpp"
#include "string.hpp"
#include <stdio.h>

using namespace pr;

static ObjP get_object_type(ObjP p)
{
    return *get_type(p);
}

static ObjP obj_eq(ObjP p, ObjP a)
{
    return p == a ? true_object() : false_object();
}

static ObjP obj_ne(ObjP p, ObjP a)
{
    return p != a ? true_object() : false_object();
}

Type* Type::type;

Type::Type(Name n)
:   Object(n.id() == N_type ? this : get_type()), name(n)
{
    add_method(Name("type"), get_object_type);
    add_method(Name("eq"), obj_eq);
    add_method(Name("ne"), obj_ne);

    GC::add_root(this);
    GC::add_type(this);
}

Type::~Type()
{
}

Type* Type::get_type()
{
    if (!type)
    {
        type = new Type(Name("type"));
        type->add_method(Name("name"), (Callable::mptr0)&Type::name_);
        type->add_method(Name("to_string"), (Callable::mptr0)&Type::to_string_);
        type->add_method(Name("get_method"), (Callable::mptr1)&Type::get_method_);
        type->add_method(Name("set_method"), (Callable::mptr2)&Type::set_method_);
        type->add_method(Name("methods"), (Callable::mptr0)&Type::methods_);
    }

    return type;
}

void Type::gc_mark()
{
    std::map<Name, Callable>::iterator iter;
    for (iter = methods.begin(); iter != methods.end(); iter++)
        iter->second.gc_mark();
}

void Type::add_method(Name n, Callable c)
{
    std::map<Name, Callable>::iterator iter = methods.find(n);
    if (iter != methods.end())
        methods.erase(iter);
    methods.insert(std::make_pair(n, c));
}

Callable Type::get_method(Name n)
{
    //assert(methods.count(n));
    return methods[n];
}

ObjP Type::to_string_()
{
    String* s = new String("<type ");
    s->append(name.s());
    s->append(">");
    return *s;
}

ObjP Type::name_()
{
    return name_to_symbol(name);
}

ObjP Type::get_method_(ObjP n)
{
    if (!is_symbol(n))
        throw new Exception(Name("bad_argument"), n);

    Name name = symbol_to_name(n);
    Callable c = get_method(name);

    if (c.type == Callable::NONE)
        throw new Exception(Name("bad_method"), 0);

    return *new Method(this, c);
}

ObjP Type::set_method_(ObjP n, ObjP c)
{
    if (!is_symbol(n))
        throw new Exception(Name("bad_argument"), n);

    Name name = symbol_to_name(n);

    add_method(name, c);

    return 0;
}

ObjP Type::methods_()
{
    List* l = new List();

    std::map<Name, Callable>::iterator iter;
    for (iter = methods.begin(); iter != methods.end(); iter++)
        l->append(name_to_symbol(iter->first));

    return *l;
}