#include "type.hpp"

using namespace pr;

Exception::Exception(Name n, ObjP obj)
:   Object(get_type()),
    name(n),
    obj(obj)
{
    inc_ref(obj);
}

Exception::~Exception()
{
    dec_ref(obj);
}

Type* Exception::get_type()
{
    static Type* type;
    if (!type)
    {
        type = new Type(Name("exception"));
        type->add_method("to_string", (Callable::mptr0)&Exception::to_string);
        type->add_method("name", (Callable::mptr0)&Exception::name_);
        type->add_method("obj", (Callable::mptr0)&Exception::obj_);
        type->add_method("raise", (Callable::mptr0)&Exception::raise_);
    }
    return type;
}

void Exception::gc_mark()
{
    GC::mark(obj);
}

Exception* Exception::cast_exception()
{
    return this;
}

String* Exception::to_string()
{
    String* s = new String(name.s());
    s->append(" exception");
#if 0
    String* s2 = call_to_string(obj);
    s->append(": ");
    s->append(s2);
#endif
    return s;
}

ObjP Exception::name_()
{
    return name_to_symbol(name);
}

ObjP Exception::obj_()
{
    return obj;
}

ObjP Exception::raise_()
{
    throw this;
}
