#include "method.hpp"
#include "list.hpp"

using namespace pr;

Type* Method::type;

Method::Method(ObjP obj, Callable c)
:   Object(get_type()),
    object(obj),
    object_type(0),
    callable(c)
{
    assert(callable.type);
    inc_ref(object);
}

Method::Method(Type* t, Callable c)
:   Object(get_type()),
    object(0),
    object_type(t),
    callable(c)
{
}

Method::~Method()
{
}

ObjP Method::to_string_()
{
    return *new String("<method>");
}

Type* Method::get_type()
{
    if (!type)
    {
        type = new Type("method");
        type->add_method("to_string", (Callable::mptr0)&Method::to_string_);
        type->add_method("call", (Callable::mptrx)&Method::call_);
        type->add_method("rcall", (Callable::mptrx)&Method::rcall_);
    }
    return type;
}

void Method::gc_mark()
{
    GC::mark(object);
    if (object_type)
        GC::mark(object_type);
    callable.gc_mark();
}

ObjP Method::call_(List* l)
{
    if (object_type)
        throw new Exception(Name("method_requires_object"), 0);
    return callable.callx(object, l);
}

ObjP Method::rcall_(List* l)
{
    if (l->get_size() < 1)
        throw new Exception(Name("bad_argument_count"), *l);

    ObjP p = l->get(0);

    if (object_type)
    {
        if (::get_type(p) != object_type)
            throw new Exception(Name("type_mismatch"), object);
    }
    else
    {
        if (::get_type(p) != ::get_type(object))
            throw new Exception(Name("type_mismatch"), object);
    }

    List* l2 = new List();
    for (int i = 1; i < l->get_size(); i++)
        l2->append(l->get(i));

    return callable.callx(p, l2);
}
