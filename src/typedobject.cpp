#include "typedobject.hpp"
#include "gc.hpp"
#include "integer.hpp"
#include "code.hpp"
#include "list.hpp"
#include "string.hpp"

using namespace pr;

TypedObject::TypedObject(Type* t, ObjP o)
:   Object(t), type(t), object(o)
{
}

TypedObject::~TypedObject()
{
}

Type* TypedObject::get_type()
{
    return type;
}

void TypedObject::gc_mark()
{
    GC::mark(type);
    GC::mark(object);
}

#if 0
Integer* TypedObject::cast_integer()
{
    ObjP ret = method_call0(*this, "as_integer");
    // TODO: possible infinite loop, how to catch it?
    return to_integer(ret);
}

Code* TypedObject::cast_code()
{
    ObjP ret = method_call0(*this, "as_code");
    return to_code(ret);
}

List* TypedObject::cast_list()
{
    ObjP ret = method_call0(*this, "as_list");
    return to_list(ret);
}

String* TypedObject::cast_string()
{
    ObjP ret = method_call0(*this, "as_string");
    return to_string(ret);
}
#endif

ObjP TypedObject::get_boxed_object_()
{
    return object;
}

ObjP TypedObject::set_boxed_object_(ObjP o)
{
    object = o;
    return o;
}
