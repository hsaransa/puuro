#include "assoc.hpp"
#include "type.hpp"
#include "gc.hpp"
#include "string.hpp"

using namespace pr;

Assoc::Assoc()
:   Object(get_type())
{
}

Assoc::~Assoc()
{
}

Type* Assoc::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type("assoc");
        type->add_method("to_string", (Callable::mptr0)&Assoc::to_string_);
        type->add_method("get", (Callable::mptr1)&Assoc::get_);
        type->add_method("set", (Callable::mptr2)&Assoc::set_);
        //type->add_method("copy", (Callable::mptr0)&Assoc::copy_);
    }
    return type;
}

void Assoc::gc_mark()
{
    std::map<Name, Ref<ObjP> >::iterator iter;
    for (iter = assoc.begin(); iter != assoc.end(); iter++)
        GC::mark(iter->second);
}

ObjP Assoc::to_string_()
{
    return *new String("assoc");
}

ObjP Assoc::get_(ObjP s)
{
    if (!is_symbol(s))
        throw new Exception("bad_type", s);
    Name n = symbol_to_name(s);
    if (assoc.find(n) == assoc.end())
        throw new Exception("key_not_found", s);
    return assoc[n];
}

ObjP Assoc::set_(ObjP s, ObjP v)
{
    if (!is_symbol(s))
        throw new Exception("bad_type", s);
    Name n = symbol_to_name(s);
    inc_ref(v);
    assoc[n] = v;
    return v;
}

ObjP Assoc::copy_()
{
    Assoc* a = new Assoc();

    //for (int i = 0; i < 

    return *a;
}
