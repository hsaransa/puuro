#include "scope.hpp"
#include "method.hpp"
#include "list.hpp"

using namespace pr;

Scope::Scope(Scope* parent)
:   Object(get_type()), parent(parent)
{
}

Scope::~Scope()
{
}

Type* Scope::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type("scope");
        type->add_method("set", (Callable::mptr2)&Scope::set_);
        type->add_method("set_local", (Callable::mptr2)&Scope::set_local_);
        type->add_method("get", (Callable::mptr1)&Scope::get_);
        type->add_method("get_local", (Callable::mptr1)&Scope::get_local_);
        type->add_method("pollute", (Callable::mptr1)&Scope::pollute_);
        type->add_method("parent", (Callable::mptr0)&Scope::parent_);
        type->add_method("set_parent", (Callable::mptr1)&Scope::set_parent_);
        type->add_method("locals", (Callable::mptr0)&Scope::locals_);
    }
    return type;
}

void Scope::gc_mark()
{
    GC::mark(parent);

    std::map<Name, Ref<ObjP> >::iterator iter;
    for (iter = locals.begin(); iter != locals.end(); iter++)
        GC::mark(iter->second);
}

void Scope::set_local(Name n, ObjP p)
{
    locals[n] = p;
}

void Scope::set(Name n, ObjP p)
{
    Scope* f = this;

    // Get the scope that has this variable assigned.

    while (f)
    {
        if (f->locals.count(n))
        {
            // Replace the old variable.
            f->locals[n] = p;
            return;
        }
        else
            f = f->parent.get();
    }

    // Not found, add variable to this scope.

    locals[n] = p;
}

ObjP Scope::lookup(Name n)
{
    if (locals.count(n) == 0)
    {
        if (parent)
            return parent->lookup(n);
        throw new Exception("not_defined", name_to_symbol(n));
    }
    else
        return inc_ref(locals[n]);
}

ObjP Scope::set_local_(ObjP n, ObjP v)
{
    if (!is_symbol(n))
        throw new Exception("bad_argument", n);
    set_local(symbol_to_name(n), v);
    return inc_ref(v);
}

ObjP Scope::set_(ObjP n, ObjP v)
{
    if (!is_symbol(n))
        throw new Exception("bad_argument", n);
    set(symbol_to_name(n), v);
    return inc_ref(v);
}

ObjP Scope::get_(ObjP n)
{
    if (!is_symbol(n))
        throw new Exception("bad_argument", n);
    Name nn = symbol_to_name(n);
    return lookup(nn);
}

ObjP Scope::get_local_(ObjP n)
{
    if (!is_symbol(n))
        throw new Exception("bad_argument", n);
    Name nn = symbol_to_name(n);
    if (locals.count(nn))
        return inc_ref(locals[nn]);
    else
        throw new Exception("not_defined", n);
}

ObjP Scope::pollute_(ObjP obj)
{
    Type* t = pr::get_type(obj);

    std::map<Name, Callable>::iterator iter;
    for (iter = t->methods.begin(); iter != t->methods.end(); iter++)
        set_local(iter->first, *new Method(obj, iter->second));

    return 0;
}

ObjP Scope::parent_()
{
    return inc_ref(*parent);
}

ObjP Scope::set_parent_(ObjP p)
{
    if (p)
    {
        Scope* s = cast_object<Scope*>(p);
        parent = s;
    }
    else
        parent = 0;
    return 0;
}

ObjP Scope::locals_()
{
    List* l = new List();
    std::map<Name, Ref<ObjP> >::iterator iter;
    for (iter = locals.begin(); iter != locals.end(); iter++)
        l->append(*new List(2, name_to_symbol(iter->first), iter->second.get()));
    return *l;
}
